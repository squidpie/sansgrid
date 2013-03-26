/* Payload Tests
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
 *
 *
 * The payload tests are designed to closely simulate a serial pipe.
 * Semaphores are important when using the functions defined in payload-stub-handlers.c
 * You should do a sem_post():
 * 		Receiving from server: post the TCP semaphore
 * 		Receiving from sensor: post the SPI semaphore 
 * 	If the semaphores are uninitialized the pipe will be read
 * 			(regardless of whether valid data exists in the pipe).
 */
#ifndef __SG_PAYLOAD_STUBS_H__
#define __SG_PAYLOAD_STUBS_H__

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <check.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../../../sg_serial.h"
#include "../../communication/sg_tcp.h"
#include "../../../payloads.h"
#include "../../routing/routing.h"
#include "../../dt_handlers/handlers.h"

#include "../../dispatch/dispatch.h"
#include "../tests.h"

// Setup fifo reading/writing
void sgSerialTestSetReader(FILE *FPTR);
void sgSerialTestSetReadlock(sem_t *readlock);
void sgSerialTestSetWriter(FILE *FPTR);
void sgTCPTestSetReader(FILE *FPTR);
void sgTCPTestSetReadlock(sem_t *readlock);
void sgTCPTestSetWriter(FILE *FPTR);

Queue *dispatch;
RoutingTable *routing_table;
sem_t tcp_readlock,
	  spi_readlock;
// Payload Stub Handlers
void payloadMkSerial(SansgridSerial *sg_serial);
void payloadMkEyeball(SansgridEyeball *sg_eyeball, enum SansgridEyeballModeEnum ebmate_type);
void payloadMkPeck(SansgridPeck *sg_peck, enum SansgridPeckRecognitionEnum pkrec_type);
void payloadMkSing(SansgridSing *sg_sing, enum SansgridDataTypeEnum sing_type);

// Size checking handlers
void checkSize(const char *pkname, size_t pksize);

// Payload Readers
void *spiPayloadReader(void *arg);
void *tcpPayloadReader(void *arg);

// Payload State
int32_t payloadRoutingInit(void);
int32_t payloadStateInit(void);
int32_t payloadStateCommit(SansgridSerial **);

// Tests
Suite *payloadSizeTesting (void);
Suite *payloadEyeballTesting (void);
Suite *payloadPeckTesting (void);
Suite *payloadSingTesting (void);

#endif
// vim: ft=c ts=4 noet sw=4:


