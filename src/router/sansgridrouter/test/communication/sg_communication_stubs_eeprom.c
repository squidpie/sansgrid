/* read/write stubs for EEPROM
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
#ifdef SG_TEST_USE_EEPROM
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
#include <wiringPi.h>
#include <wiringPiSPI.h>

static pthread_mutex_t eeprom_lock;
static int eeprom_lock_initd = 0;

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

static int eepromSendData(uint8_t *buffer, uint16_t address, int size) { 
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



int eepromSend(SansgridSerial *sg_serial, uint32_t size, TalkStub *ts) {
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


int eepromReceive(SansgridSerial **sg_serial, uint32_t *size, TalkStub *ts) {
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


#endif

// vim: ft=c ts=4 noet sw=4:

