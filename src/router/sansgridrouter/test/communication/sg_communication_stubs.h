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


/*
 * This header creates stubs for Serial communication
 * and for TCP communication from one place. It turns out that
 * these definitions, when stubbed out, are remarkably similar,
 * so serial and TCP are merged into one API.
 */

#ifndef __COMMUNICATION_STUBS_H__
#define __COMMUNICATION_STUBS_H__
// Use a 64K EEPROM Chip instead of named pipes
//#define SG_TEST_USE_EEPROM

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <sys/types.h>

#include <sgSerial.h>
#include "../tests.h"

enum CommTypeEnum {
	COMM_TYPE_NONE,
	COMM_TYPE_UNIX_PIPE,
	COMM_TYPE_EEPROM,
};

typedef struct TalkStub {
#ifdef SG_TEST_USE_EEPROM
	// Only used when using EEPROM
	uint16_t eeprom_address;
#endif
	// Used with named pipes
	FILE *FPTR_PIPE_WRITE,		// Writing file descriptor
		 *FPTR_PIPE_READ;		// Reading file descriptor
	int valid_read;				// Only return valid data if we're reading
	sem_t writelock,			// A write is in progress
		  readlock;				// A read is in progress
	int use_barrier;			// Whether or not to use write/read_in_progress
	// Callbacks to be registered
	int8_t (*receive)(SansgridSerial**, uint32_t*, struct TalkStub*);
	int8_t (*send)(SansgridSerial*, uint32_t, struct TalkStub*);
} TalkStub;


TalkStub *talkStubInit(void);
void talkStubUseAsSPI(TalkStub *ts);
void talkStubUseAsTCP(TalkStub *ts);
TalkStub *talkStubGetSPI(void);
TalkStub *talkStubGetTCP(void);
void talkStubAssertValid(TalkStub *ts);
void talkStubAssertInvalid(TalkStub *ts);
#ifdef SG_TEST_USE_EEPROM
void talkStubSetEEPROMAddress(TalkStub *ts, uint32_t address);
#endif
void talkStubSetReader(TalkStub *ts, FILE *FPTR);
void talkStubSetWriter(TalkStub *ts, FILE *FPTR);
void talkStubCloseReader(TalkStub *ts);
void talkStubRegisterReadWriteFuncs(TalkStub *ts, enum CommTypeEnum comm_type);
void talkStubCloseWriter(TalkStub *ts);
void talkStubDestroy(TalkStub *ts);



#endif
// vim: ft=c ts=4 noet sw=4:

