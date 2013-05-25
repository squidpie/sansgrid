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
/** \file */
// Allow a 64K EEPROM Chip in addition to named pipes
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

/**
 * \brief Communication Type Enumerated Type
 *
 * Used to choose which stub type to use.
 * This allows a choice between what stub to use 
 */
enum CommTypeEnum {
	COMM_TYPE_NONE,
	COMM_TYPE_UNIX_PIPE,
	COMM_TYPE_EEPROM,
};

/**
 * \brief Configuration for stubs
 *
 * This allows fine control over how communication stubs are used.
 */
typedef struct TalkStub {
	/// Name of the stub
	char name[50];
#ifdef SG_TEST_USE_EEPROM
	/// Address to write to, Only used when using EEPROM
	uint16_t eeprom_address;
#endif
	// Used with named pipes
	FILE *FPTR_PIPE_WRITE;		///< Writing file descriptor
	FILE *FPTR_PIPE_READ;		///< Reading file descriptor
	int valid_read;				///< Only return valid data if we're reading
	sem_t writelock,			///< A write is in progress
		  readlock;				///< A read is in progress
	int use_barrier;			///< Whether or not to use write/read_in_progress
	// Callbacks to be registered
	/// Receive callback
	int8_t (*receive)(SansgridSerial**, uint32_t*, struct TalkStub*);
	/// Sending callback
	int8_t (*send)(SansgridSerial*, uint32_t, struct TalkStub*);
} TalkStub;


TalkStub *talkStubInit(char *name);
void talkStubUseAsSPI(TalkStub *ts);
void talkStubUseAsTCP(TalkStub *ts);
TalkStub *talkStubGetSPI(void);
TalkStub *talkStubGetTCP(void);
void talkStubAssertValid(TalkStub *ts);
void talkStubAssertInvalid(TalkStub *ts);
#ifdef SG_TEST_USE_EEPROM
//void talkStubSetEEPROMAddress(TalkStub *ts, uint32_t address);
#endif
void talkStubRegisterReadWriteFuncs(TalkStub *ts, enum CommTypeEnum comm_type);
void talkStubDestroy(TalkStub *ts);



#endif
// vim: ft=c ts=4 noet sw=4:

