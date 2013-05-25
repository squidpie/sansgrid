/* TalkStub defs for stubbing out server and sensor/radio
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
/// \file
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

int8_t eepromSend(SansgridSerial *sg_serial, uint32_t size, TalkStub *ts);
int8_t eepromReceive(SansgridSerial **sg_serial, uint32_t *size, TalkStub *ts);
#endif

int8_t unixpipeSend(SansgridSerial *sg_serial, uint32_t size, TalkStub *ts);
int8_t unixpipeReceive(SansgridSerial **sg_serial, uint32_t *size, TalkStub *ts);



// Serial, TCP stub attributes
static TalkStub *ts_serial = NULL,
				*ts_tcp = NULL;


/**
 * Initialize a stub configuration structure
 */
TalkStub *talkStubInit(char *name) {
	mark_point();
	TalkStub *ts = (TalkStub*)malloc(sizeof(TalkStub));
	memcpy(ts->name, name, 50*sizeof(char));
#ifdef SG_TEST_USE_EEPROM
	ts->eeprom_address = 0x0;
#endif
	ts->FPTR_PIPE_READ = NULL;
	ts->FPTR_PIPE_WRITE = NULL;
	ts->valid_read = 0;

	mark_point();
	sem_init(&ts->writelock, 0, 0);
	sem_init(&ts->readlock, 0, 0);

	ts->receive = NULL;
	ts->send = NULL;

	return ts;
}

/*
 * Enable/Disable functions
 */

/**
 * Set this Stub Config Structure to be SPI
 */
void talkStubUseAsSPI(TalkStub *ts) {
	mark_point();
	ts_serial = ts;
	return;
}

/**
 * Set this Stub Config Structure to be TCP
 */
void talkStubUseAsTCP(TalkStub *ts) {
	mark_point();
	ts_tcp = ts;
	return;
}


/**
 * Return the SPI Stub configuration structure
 */
TalkStub *talkStubGetSPI(void) {
	return ts_serial;
}

/**
 * Return the TCP Stub configuration structure
 */
TalkStub *talkStubGetTCP(void) {
	return ts_tcp;
}


/**
 * \brief Allow reads/writes using the Stub Config
 *
 * Use this if you are expecting data to pass through
 * this stub. 
 */
void talkStubAssertValid(TalkStub *ts) {
	ts->valid_read = 1;
}


/**
 * Don't allow reads/writes using the Stub Config
 */
void talkStubAssertInvalid(TalkStub *ts) {
	ts->valid_read = 0;
}


/*
 * File Descriptor setup
 */
#ifdef SG_TEST_USE_EEPROM
/**
 * Set what address will be written to / read from on the EEPROM
 */
void talkStubSetEEPROMAddress(TalkStub *ts, uint32_t address) {
	mark_point();
	ts->eeprom_address = address;
}
#endif


/**
 * Set the read file descriptor
 */
void talkStubSetReader(TalkStub *ts, FILE *FPTR) {
	mark_point();
	ts->FPTR_PIPE_READ = FPTR;
}

/**
 * Set the write file descriptor
 */
void talkStubSetWriter(TalkStub *ts, FILE *FPTR) {
	mark_point();
	ts->FPTR_PIPE_WRITE = FPTR;
}


/**
 * Close the reading file descriptor
 */
void talkStubCloseReader(TalkStub *ts) {
	fclose(ts->FPTR_PIPE_READ);
	ts->FPTR_PIPE_READ = NULL;
}


/**
 * Close the writing file descriptor
 */
void talkStubCloseWriter(TalkStub *ts) {
	fclose(ts->FPTR_PIPE_WRITE);
	ts->FPTR_PIPE_WRITE = NULL;
}


/**
 * Set up what stub will be used for reading and writing
 */
void talkStubRegisterReadWriteFuncs(TalkStub *ts, enum CommTypeEnum comm_type) {
	switch(comm_type) {
		case COMM_TYPE_NONE: 
			ts->send = NULL;
			ts->receive = NULL;
			break;
		case COMM_TYPE_EEPROM:
#ifdef SG_TEST_USE_EEPROM
			ts->send = &eepromSend;
			ts->receive = &eepromReceive;
#else
			ts->send = NULL;
			ts->receive = NULL;
#endif
			break;
		case COMM_TYPE_UNIX_PIPE:
			ts->send = &unixpipeSend;
			ts->receive = &unixpipeReceive;
			break;
		default:
			ts->send = NULL;
			ts->receive = NULL;
			break;
	}
	return;
}



/**
 * Free all references to the Stub Configuration Structure
 */
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

/**
 * Send data using the specified Stub Configuration Structure
 */
int8_t sgSerialSend(SansgridSerial *sg_serial, uint32_t size) {
	mark_point();
	if (!ts_serial->send) {
		printf("Serial: No Sending Function Registered!\n");
		return -1;
	}
	return ts_serial->send(sg_serial, size, ts_serial);
}

/**
 * Receive data using the specified Stub Configuration Structure
 */
int8_t sgSerialReceive(SansgridSerial **sg_serial, uint32_t *size) {
	int8_t exit_code;
	mark_point();
	if (!ts_serial->receive) {
		printf("Serial: No Receiving Function Registered!\n");
		return -1;
	}
	exit_code = ts_serial->receive(sg_serial, size, ts_serial);
	mark_point();
	return exit_code;
}

/**
 * Send data using the specified Stub Configuration Structure
 */
int8_t sgTCPSend(SansgridSerial *sg_serial, uint32_t size) {
	mark_point();
	if (!ts_tcp->send) {
		printf("TCP: No Sending Function Registered!\n");
		return -1;
	}
	return ts_tcp->send(sg_serial, size, ts_tcp);
}

/**
 * Receive data using the specified Stub Configuration Structure
 */
int8_t sgTCPReceive(SansgridSerial **sg_serial, uint32_t *size) {
	int8_t exit_code;
	mark_point();
	if (!ts_tcp->receive) {
		printf("TCP: No Receiving Function Registered!\n");
		return -1;
	}
	exit_code = ts_tcp->receive(sg_serial, size, ts_tcp);
	mark_point();
	return exit_code;
}


// vim: ft=c ts=4 noet sw=4:

