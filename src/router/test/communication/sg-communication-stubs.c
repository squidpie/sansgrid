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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <sys/types.h>
#include "../../../sg_serial.h"
#include "../tests.h"
#include "sg-communication-stubs.h"

struct TalkStub {
	FILE *FPTR_SPI_WRITE,		// Writing file descriptor
		 *FPTR_SPI_READ;		// Reading file descriptor
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
	}
	else {
		ts = (TalkStub*)malloc(sizeof(TalkStub));
		ts->FPTR_SPI_READ = NULL;
		ts->FPTR_SPI_WRITE = NULL;
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
void talkStubSetReader(TalkStub *ts, FILE *FPTR) {
	ts->FPTR_SPI_READ = FPTR;
}

void talkStubSetWriter(TalkStub *ts, FILE *FPTR) {
	ts->FPTR_SPI_WRITE = FPTR;
}

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
	}
	else if (!value && ts->use_barrier) {
		// destroy
		sem_destroy(&ts->write_in_progress);
		sem_destroy(&ts->read_in_progress);
	}
	ts->use_barrier = value;
}



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

