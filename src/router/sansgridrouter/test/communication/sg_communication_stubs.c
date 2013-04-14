/* read/write stubs
 *
 * Copyright (C) 2013 SansGrid
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#define _POSIX_C_SOURCE 200809L          // Required for nanosleep()

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include "../../../../sg_serial.h"
#include "../tests.h"
#include "sg_communication_stubs.h"
#ifdef SG_TEST_USE_EEPROM
#include <wiringPi.h>
#include <wiringPiSPI.h>

static pthread_mutex_t eeprom_lock;
static int eeprom_lock_initd = 0;
#endif


struct TalkStub {
#ifdef SG_TEST_USE_EEPROM
	// Only used when using EEPROM
	uint16_t eeprom_address;
#else
	// Used with named pipes
	FILE *FPTR_SPI_WRITE,		// Writing file descriptor
		 *FPTR_SPI_READ;		// Reading file descriptor
#endif
	sem_t *valid_read;			// Only return valid data if we're reading
	sem_t write_in_progress,	// A write is in progress
		  read_in_progress;		// A read is in progress
	int use_barrier;			// Whether or not to use write/read_in_progress
};

// Serial, TCP stub attributes
static TalkStub *ts_serial = NULL,
				*ts_tcp = NULL;


static TalkStub *talkStubUse(TalkStub *ts, int use_this) {

	if (ts)
		free(ts);
	if (!use_this) {
		ts = NULL;
	} else {
		ts = (TalkStub*)malloc(sizeof(TalkStub));
#ifdef SG_TEST_USE_EEPROM
		ts->eeprom_address = 0x0;
#else
		ts->FPTR_SPI_READ = NULL;
		ts->FPTR_SPI_WRITE = NULL;
#endif
		ts->valid_read = NULL;
		ts->use_barrier = 0;
	}
	return ts;
}

/*
 * Enable/Disable functions
 */
TalkStub *talkStubUseSerial(int use_serial) {
	return (ts_serial = talkStubUse(ts_serial, use_serial));
}

TalkStub *talkStubUseTCP(int use_tcp) {
	return (ts_tcp = talkStubUse(ts_tcp, use_tcp));
}


/*
 * File Descriptor setup
 */
#ifdef SG_TEST_USE_EEPROM
void talkStubSetEEPROMAddress(TalkStub *ts, uint32_t address) {
	ts->eeprom_address = address;
}
#else
void talkStubSetReader(TalkStub *ts, FILE *FPTR) {
	ts->FPTR_SPI_READ = FPTR;
}

void talkStubSetWriter(TalkStub *ts, FILE *FPTR) {
	ts->FPTR_SPI_WRITE = FPTR;
}
#endif

/*
 * Lock Setup
 */
void talkStubSetReadlock(TalkStub *ts, sem_t *readlock) {
	// Set up an optional lock that drops data if
	// the semaphore value == 0
	ts->valid_read = readlock;
}

void talkStubUseBarrier(TalkStub *ts, int value) {
	// Read/Write synchronization locks
	// This makes sure that one piece of data is read,
	// and one piece of data is written at a time
	if (value && !ts->use_barrier) {
		// initialize
		sem_init(&ts->write_in_progress, 0, 0);
		sem_init(&ts->read_in_progress, 0, 0);
	} else if (!value && ts->use_barrier) {
		// destroy
		sem_destroy(&ts->write_in_progress);
		sem_destroy(&ts->read_in_progress);
	}
	ts->use_barrier = value;
}


#ifdef SG_TEST_USE_EEPROM
/*
 * Use a 64K SPI EEPROM for read/write
 */



#define KHZ(freq) (1000*freq)
#define MHZ(freq) (1000*KHZ(freq))

// EEPROM-specific defines
#define SPI_SPEED_MHZ	2
#define WRITE 			0x02
#define READ			0x03
#define WRITE_ENABLE 	0x06
#define WRITE_CYCLE		5000

static int eepromSend(uint8_t *buffer, uint16_t address, int size) { 
	// Set up SPI
	int i;
	int fd;
	// only 32 bytes can be written at a time; see below
	int bounded_size = (size > 32 ? 32 : size);
	// prepend the command and address to the data
	uint8_t newbuffer[bounded_size+3];
	struct timespec required, remaining;

	if ((fd = wiringPiSPISetup (0, MHZ(SPI_SPEED_MHZ))) < 0)
		fprintf(stderr, "SPI Setup failed: %s\n", strerror (errno));

	// Have to make room for command and address
	for (i=3; i<bounded_size+3; i++)
		newbuffer[i] = buffer[i-3];

	// Allow writes to the EEPROM
	// Has to be done before every write cycle
	newbuffer[0] = WRITE_ENABLE;
	write(fd, newbuffer, 1);

	// Write to specified address
	newbuffer[0] = WRITE;
	newbuffer[1] = address >> 8;
	newbuffer[2] = address & 0xff;

	write(fd, newbuffer, bounded_size+3);
	// Wait for the write to cycle
	required.tv_sec = 0;
	required.tv_nsec = 1000L*WRITE_CYCLE;
	nanosleep(&required, &remaining);
	close(fd);
	if (size > 32) {
		// Only one page (of 32 bytes) can be written
		// at a time. If more than 32 bytes are being written,
		// break the line into multiple pages
		return eepromSend(&buffer[32], address+0x0020, size-32);
	}

	return 0;
}



static int talkStubSend(SansgridSerial *sg_serial, uint32_t size, TalkStub *ts) {
	SANSGRID_UNION(SansgridSerial, SGSU) sg_serial_union;
	uint8_t *buffer;

	if (!eeprom_lock_initd) {
		pthread_mutex_init(&eeprom_lock, NULL);
		eeprom_lock_initd = 1;
	}
	if (!ts->use_barrier) {
		talkStubUseBarrier(ts, 1);
	}


	sg_serial_union.formdata = sg_serial;
	buffer = sg_serial_union.serialdata;

	pthread_mutex_lock(&eeprom_lock);
	eepromSend(buffer, ts->eeprom_address, size);

	pthread_mutex_unlock(&eeprom_lock);

	sem_post(&ts->write_in_progress);
	sem_wait(&ts->read_in_progress);

	return 0;
}


static int talkStubReceive(SansgridSerial **sg_serial, uint32_t *size, TalkStub *ts) {
	// Read from an EEPROM chip over SPI
	int fd;
	uint8_t buffer[sizeof(SansgridSerial)];
	uint8_t newbuffer[sizeof(SansgridSerial)+3];
	int i;

	if (ts->valid_read) {
		if (sem_trywait(ts->valid_read) == -1) {
			return 1;
		}
	}
	if (!eeprom_lock_initd) {
		pthread_mutex_init(&eeprom_lock, NULL);
		eeprom_lock_initd = 1;
	}
	if (!ts->use_barrier) {
		talkStubUseBarrier(ts, 1);
	}


	*size = sizeof(SansgridSerial);

	sem_wait(&ts->write_in_progress);

	pthread_mutex_lock(&eeprom_lock);

	// Set up reading from EEPROM
	if ((fd = wiringPiSPISetup (0, MHZ(SPI_SPEED_MHZ))) < 0)
		fprintf(stderr, "SPI Setup failed: %s\n", strerror (errno));

	// Prepend command and address to buffer
	newbuffer[0] = READ;
	newbuffer[1] = ts->eeprom_address >> 8;
	newbuffer[2] = ts->eeprom_address & 0xff;

	// Send command/address, read data
	wiringPiSPIDataRW(0, newbuffer, *size+3);

	// shift the command and address out of buffer
	for (i=0; i<*size; i++)
		buffer[i] = newbuffer[i+3];
	*sg_serial = (SansgridSerial*)malloc(sizeof(SansgridSerial));
	memcpy(*sg_serial, buffer, sizeof(SansgridSerial));
	// finish up
	close(fd);

	pthread_mutex_unlock(&eeprom_lock);
	sem_post(&ts->read_in_progress);

	return 0;
}


#else
/*
 * Generic Send/Receive functions.
 * These hook into both serial and TCP stubs
 */
static int8_t talkStubSend(SansgridSerial *sg_serial, uint32_t size, TalkStub *ts) {
	// Send size bytes of serialdata
	int i;
	SANSGRID_UNION(SansgridSerial, SGSU) sg_serial_union;


	if (ts->FPTR_SPI_WRITE == NULL)
		return -1;

	sg_serial_union.formdata = sg_serial;
	
	if (ts->use_barrier) {
		sem_post(&ts->write_in_progress);
		sem_wait(&ts->read_in_progress);
	}
	for (i=0; i<sizeof(SansgridSerial) && ts->FPTR_SPI_WRITE; i++) {
		putc(sg_serial_union.serialdata[i], ts->FPTR_SPI_WRITE);
	}
	
	return 0;
}



static int8_t talkStubReceive(SansgridSerial **sg_serial, uint32_t *size, TalkStub *ts) {
	// Receive serialdata, size of packet stored in size
	int i;
	char lptr[sizeof(SansgridSerial)+1];

	if (ts->FPTR_SPI_READ == NULL)
		return -1;

	// Read from the pipe 
	if (ts->use_barrier) {
		sem_post(&ts->read_in_progress);
		sem_wait(&ts->write_in_progress);
	}
	for (i=0; i<(sizeof(SansgridSerial)) && ts->FPTR_SPI_READ; i++) {
		lptr[i] = fgetc(ts->FPTR_SPI_READ);
	}
	if (ts->valid_read) {
		if (sem_trywait(ts->valid_read) == -1) {
			return 1;
		}
	}
	if (i < sizeof(SansgridSerial)) {
#if TESTS_DEBUG_LEVEL > 0
		printf("(Serial) Dropping Packet at %i of %i\n", i, 
				sizeof(SansgridSerial));
#endif
		return 1;
	}
	*sg_serial = (SansgridSerial*)malloc(sizeof(SansgridSerial));
	memcpy(*sg_serial, lptr, sizeof(SansgridSerial));
	*size = sizeof(SansgridSerial);
	return 0;
}

#endif

/* Serial/TCP API Definitions
 * These hook directly into the send/receive stubs
 */
int8_t sgSerialSend(SansgridSerial *sg_serial, uint32_t size) {
	return talkStubSend(sg_serial, size, ts_serial);
}

int8_t sgSerialReceive(SansgridSerial **sg_serial, uint32_t *size) {
	return talkStubReceive(sg_serial, size, ts_serial);
}

int8_t sgTCPSend(SansgridSerial *sg_serial, uint32_t size) {
	return talkStubSend(sg_serial, size, ts_tcp);
}

int8_t sgTCPReceive(SansgridSerial **sg_serial, uint32_t *size) {
	return talkStubReceive(sg_serial, size, ts_tcp);
}


// vim: ft=c ts=4 noet sw=4:

