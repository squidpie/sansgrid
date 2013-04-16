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

#ifndef __TALK_STUBS_H__
#define __TALK_STUBS_H__
// Use a 64K EEPROM Chip instead of named pipes
//#define SG_TEST_USE_EEPROM

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <sys/types.h>
#include "../../../../lib/sgSerial.h"
#include "../tests.h"

typedef struct TalkStub TalkStub;

TalkStub *talkStubInit(void);
void talkStubUseAsSPI(TalkStub *ts);
void talkStubUseAsTCP(TalkStub *ts);
TalkStub *talkStubGetSPI(void);
TalkStub *talkStubGetTCP(void);
void talkStubAssertValid(TalkStub *ts);
void talkStubAssertInvalid(TalkStub *ts);
#ifdef SG_TEST_USE_EEPROM
void talkStubSetEEPROMAddress(TalkStub *ts, uint32_t address);
#else
void talkStubSetReader(TalkStub *ts, FILE *FPTR);
void talkStubSetWriter(TalkStub *ts, FILE *FPTR);
void talkStubCloseReader(TalkStub *ts);
void talkStubCloseWriter(TalkStub *ts);
#endif
void talkStubDestroy(TalkStub *ts);





#endif
// vim: ft=c ts=4 noet sw=4:

