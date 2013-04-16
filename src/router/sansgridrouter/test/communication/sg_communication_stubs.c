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
#include "../tests.h"
#include <sgSerial.h>
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
	FILE *FPTR_PIPE_WRITE,		// Writing file descriptor
		 *FPTR_PIPE_READ;		// Reading file descriptor
#endif
	int valid_read;				// Only return valid data if we're reading
	sem_t writelock,			// A write is in progress
		  readlock;				// A read is in progress
	int use_barrier;			// Whether or not to use write/read_in_progress
};

// Serial, TCP stub attributes
static TalkStub *ts_serial = NULL,
				*ts_tcp = NULL;


TalkStub *talkStubInit(void) {
	mark_point();
	TalkStub *ts = (TalkStub*)malloc(sizeof(TalkStub));
#ifdef SG_TEST_USE_EEPROM
	ts->eeprom_address = 0x0;
#else
	ts->FPTR_PIPE_READ = NULL;
	ts->FPTR_PIPE_WRITE = NULL;
#endif
	ts->valid_read = 0;

	mark_point();
	sem_init(&ts->writelock, 0, 0);
	sem_init(&ts->readlock, 0, 0);

	return ts;
}

/*
 * Enable/Disable functions
 */
void talkStubUseAsSPI(TalkStub *ts) {
	mark_point();
	ts_serial = ts;
	return;
}

void talkStubUseAsTCP(TalkStub *ts) {
	mark_point();
	ts_tcp = ts;
	return;
}

TalkStub *talkStubGetSPI(void) {
	return ts_serial;
}

TalkStub *talkStubGetTCP(void) {
	return ts_tcp;
}


void talkStubAssertValid(TalkStub *ts) {
	ts->valid_read = 1;
}

void talkStubAssertInvalid(TalkStub *ts) {
	ts->valid_read = 0;
}


/*
 * File Descriptor setup
 */
#ifdef SG_TEST_USE_EEPROM
void talkStubSetEEPROMAddress(TalkStub *ts, uint32_t address) {
	mark_point();
	ts->eeprom_address = address;
}
#else
void talkStubSetReader(TalkStub *ts, FILE *FPTR) {
	mark_point();
	ts->FPTR_PIPE_READ = FPTR;
}

void talkStubSetWriter(TalkStub *ts, FILE *FPTR) {
	mark_point();
	ts->FPTR_PIPE_WRITE = FPTR;
}


void talkStubCloseReader(TalkStub *ts) {
	fclose(ts->FPTR_PIPE_READ);
	ts->FPTR_PIPE_READ = NULL;
}


void talkStubCloseWriter(TalkStub *ts) {
	fclose(ts->FPTR_PIPE_WRITE);
	ts->FPTR_PIPE_WRITE = NULL;
}


#endif


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
	int excode;

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

	//printf("Writing to %x\n", address);
	write(fd, newbuffer, bounded_size+3);
	close(fd);
	// Wait for the write to cycle
	required.tv_sec = 0;
	required.tv_nsec = 1000L*WRITE_CYCLE;
	do {
		if ((excode = nanosleep(&required, &remaining)) == -1) {
			if (errno == EINTR)
				required.tv_nsec = remaining.tv_nsec;
			else
				return -1;
		}
	} while (excode);

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


	sg_serial_union.formdata = sg_serial;
	buffer = sg_serial_union.serialdata;

	pthread_mutex_lock(&eeprom_lock);
	eepromSend(buffer, ts->eeprom_address, size);

	pthread_mutex_unlock(&eeprom_lock);

	sem_post(&ts->writelock);
	sem_wait(&ts->readlock);

	return 0;
}


static int talkStubReceive(SansgridSerial **sg_serial, uint32_t *size, TalkStub *ts) {
	// Read from an EEPROM chip over SPI
	int fd;
	uint8_t buffer[sizeof(SansgridSerial)];
	uint8_t newbuffer[sizeof(SansgridSerial)+3];
	int i;

	mark_point();
	if (!ts->valid_read)
		return 1;

	mark_point();
	if (!eeprom_lock_initd) {
		pthread_mutex_init(&eeprom_lock, NULL);
		eeprom_lock_initd = 1;
	}

	mark_point();


	mark_point();
	*size = sizeof(SansgridSerial);
	mark_point();

	sem_wait(&ts->writelock);

	mark_point();
	pthread_mutex_lock(&eeprom_lock);

	// Set up reading from EEPROM
	mark_point();
	if ((fd = wiringPiSPISetup (0, MHZ(SPI_SPEED_MHZ))) < 0)
		fprintf(stderr, "SPI Setup failed: %s\n", strerror (errno));

	// Prepend command and address to buffer
	newbuffer[0] = READ;
	newbuffer[1] = ts->eeprom_address >> 8;
	newbuffer[2] = ts->eeprom_address & 0xff;

	// Send command/address, read data
	wiringPiSPIDataRW(0, newbuffer, (*size)+3);

	// shift the command and address out of buffer
	for (i=0; i<*size; i++)
		buffer[i] = newbuffer[i+3];
	*sg_serial = (SansgridSerial*)malloc(sizeof(SansgridSerial));
	memcpy(*sg_serial, buffer, sizeof(SansgridSerial));
	// finish up
	close(fd);

	pthread_mutex_unlock(&eeprom_lock);
	sem_post(&ts->readlock);

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

	fail_if((ts == NULL), "TalkStub is NULL!");

	fail_if((ts->FPTR_PIPE_WRITE == NULL), "Write descriptor is NULL!");
	fail_if((sg_serial == NULL), "sg_serial is NULL!");

	sg_serial_union.formdata = sg_serial;
	
	sem_post(&ts->writelock);
	sem_wait(&ts->readlock);
	for (i=0; i<sizeof(SansgridSerial) && ts->FPTR_PIPE_WRITE; i++) {
		putc(sg_serial_union.serialdata[i], ts->FPTR_PIPE_WRITE);
	}
	
	return 0;
}



static int8_t talkStubReceive(SansgridSerial **sg_serial, uint32_t *size, TalkStub *ts) {
	// Receive serialdata, size of packet stored in size
	int i;
	char lptr[sizeof(SansgridSerial)+1];

	if (!ts->valid_read) {
		return 1;
	}
	fail_if((ts == NULL), "TalkStub is NULL!");
	mark_point();
	if (ts->FPTR_PIPE_READ == NULL) {
#if TESTS_DEBUG_LEVEL > 0
		printf("Read file descriptor is null!\n");
#endif
		return -1;
	}
	mark_point();

	// Read from the pipe 
	sem_post(&ts->readlock);
	sem_wait(&ts->writelock);
	
	mark_point();
	for (i=0; i<(sizeof(SansgridSerial)) && ts->FPTR_PIPE_READ; i++) {
		lptr[i] = fgetc(ts->FPTR_PIPE_READ);
	}

	mark_point();

	if (i < sizeof(SansgridSerial)) {
#if TESTS_DEBUG_LEVEL > 0
		printf("Dropping Packet at %i of %i\n", i, 
				sizeof(SansgridSerial));
#endif
		return 1;
	}

	mark_point();
	*sg_serial = (SansgridSerial*)malloc(sizeof(SansgridSerial));
	memcpy(*sg_serial, lptr, sizeof(SansgridSerial));
	*size = sizeof(SansgridSerial);

	mark_point();
	return 0;
}

#endif


void talkStubDestroy(TalkStub *ts) {
	if (ts_serial == ts)
		ts_serial = NULL;
	if (ts_tcp == ts)
		ts_tcp = NULL;
	free(ts);
}


/* Serial/TCP API Definitions
 * These hook directly into the send/receive stubs
 */
int8_t sgSerialSend(SansgridSerial *sg_serial, uint32_t size) {
	mark_point();
	return talkStubSend(sg_serial, size, ts_serial);
}

int8_t sgSerialReceive(SansgridSerial **sg_serial, uint32_t *size) {
	int8_t exit_code;
	mark_point();
	exit_code = talkStubReceive(sg_serial, size, ts_serial);
	mark_point();
	return exit_code;
}

int8_t sgTCPSend(SansgridSerial *sg_serial, uint32_t size) {
	mark_point();
	return talkStubSend(sg_serial, size, ts_tcp);
}

int8_t sgTCPReceive(SansgridSerial **sg_serial, uint32_t *size) {
	int8_t exit_code;
	mark_point();
	exit_code = talkStubReceive(sg_serial, size, ts_tcp);
	mark_point();
	return exit_code;
}


// vim: ft=c ts=4 noet sw=4:

