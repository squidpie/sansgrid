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
#include "../../routing_table/routing_table.h"
#include "../../payload_handlers/payload_handlers.h"
#include "../communication/sg_communication_stubs.h"
#include "../../dispatch/dispatch.h"
#include "../tests.h"

enum CommReadDirEnum {
	SG_TEST_COMM_WRITE_NONE,
	SG_TEST_COMM_WRITE_SPI,
	SG_TEST_COMM_WRITE_TCP,
	SG_TEST_COMM_WRITE_BOTH
};

typedef struct PayloadTestNode {
	enum CommReadDirEnum read_dir;
	enum SansgridDeviceStatusEnum next_packet;
} PayloadTestNode;

typedef struct PayloadTestStruct {
	// eyeball
	enum SansgridEyeballModeEnum eyeball_mode;
	PayloadTestNode *eyeball;
	// peck
	enum SansgridPeckRecognitionEnum peck_mode;
	PayloadTestNode *peck;
	// sing
	enum SansgridDataTypeEnum sing_mode;
	PayloadTestNode *sing;
	// mock
	enum SansgridDataTypeEnum mock_mode;
	PayloadTestNode *mock;
	// peacock
	enum SansgridDataTypeEnum peacock_mode;
	PayloadTestNode *peacock;
	// nest
	enum SansgridDataTypeEnum nest_mode;
	PayloadTestNode *nest;
	// squawk
	enum SansgridDataTypeEnum squawk_server_mode;
	enum SansgridDataTypeEnum squawk_sensor_mode;
	PayloadTestNode *squawk_server;
	PayloadTestNode *squawk_sensor;
	// heartbeat
	enum SansgridDataTypeEnum heartbeat_mode;
	PayloadTestNode *heartbeat;
	// chirp
	enum SansgridDataTypeEnum chirp_mode;
	PayloadTestNode *chirp;
} PayloadTestStruct;


Queue *dispatch;
RoutingTable *routing_table;
sem_t tcp_readlock,
	  spi_readlock;
// Payload Stub Handlers
void payloadMkSerial(SansgridSerial *sg_serial);
void payloadMkEyeball(SansgridEyeball *sg_eyeball, PayloadTestStruct *test_specs);
void payloadMkPeck(SansgridPeck *sg_peck, PayloadTestStruct *test_specs);
void payloadMkSing(SansgridSing *sg_sing, PayloadTestStruct *test_specs);
void payloadMkMock(SansgridMock *sg_mock, PayloadTestStruct *test_specs);
void payloadMkPeacock(SansgridPeacock *sg_peacock, PayloadTestStruct *test_specs);
void payloadMkNest(SansgridNest *sg_nest, PayloadTestStruct *test_specs);
void payloadMkSquawkServer(SansgridSquawk *sg_squawk, PayloadTestStruct *test_specs);
void payloadMkSquawkSensor(SansgridSquawk *sg_squawk, PayloadTestStruct *test_specs);

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
Suite *payloadMockTesting (void);

#endif
// vim: ft=c ts=4 noet sw=4:


