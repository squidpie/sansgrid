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
 * This dispatch test uses a named pipe as a stub to read from.
 * The data from the stub is enqueued, and the dispatch thread dequeues the data.
 */
#ifndef __SG_PAYLOAD_STUBS_H__
#define __SG_PAYLOAD_STUBS_H__

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
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
void sgSerialTestSetWriter(FILE *FPTR);
void sgTCPTestSetReader(FILE *FPTR);
void sgTCPTestSetWriter(FILE *FPTR);

Queue *dispatch;
RoutingTable *routing_table;
// Payload Stub Handlers
void payloadMkSerial(SansgridSerial *sg_serial);
void payloadMkEyeball(SansgridEyeball *sg_eyeball, enum SansgridEyeballModeEnum ebmate_type);
void payloadMkPeck(SansgridPeck *sg_peck, enum SansgridPeckRecognitionEnum pkrec_type);

// Size checking handlers
void checkSize(const char *pkname, size_t pksize);

// Payload Readers
void *spiPayloadReader(void *arg);
void *tcpPayloadReader(void *arg);

// Payload State
int32_t payloadRoutingInit(void);
int32_t payloadStateInit(void);
int32_t payloadStateCommit(void);

// Tests
Suite *payloadSizeTesting (void);
Suite *payloadEyeballTesting (void);
Suite *payloadPeckTesting (void);

#endif
// vim: ft=c ts=4 noet sw=4:


