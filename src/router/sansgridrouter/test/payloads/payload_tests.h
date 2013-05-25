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
/// \file

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

#include <sgSerial.h>
#include <payloads.h>
#include "../../communication/sg_tcp.h"
#include "../../routing_table/routing_table.h"
#include "../../payload_handlers/payload_handlers.h"
#include "../communication/sg_communication_stubs.h"
#include "../../dispatch/dispatch.h"
#include "../tests.h"

int8_t sgTCPReceive(SansgridSerial **sg_serial, uint32_t *size);

/**
 * \brief Communication Write Direction
 *
 * Choose whether to write to SPI, TCP, neither or both
 */
enum CommReadDirEnum {
	SG_TEST_COMM_WRITE_NONE,
	SG_TEST_COMM_WRITE_SPI,
	SG_TEST_COMM_WRITE_TCP,
	SG_TEST_COMM_WRITE_BOTH
};

/**
 * \brief Test state
 */
typedef struct PayloadTestNode {
	/// Read direction
	enum CommReadDirEnum read_dir;
	/// Next expected packet
	enum SansgridDeviceStatusEnum next_packet;
	/// Expected exit code
	int expected_exit_code;
} PayloadTestNode;


/// Test structure
typedef struct PayloadTestStruct {
	// eyeball
	/// Eyeball mode
	enum SansgridEyeballModeEnum eyeball_mode;
	/// Eyeball test state
	PayloadTestNode *eyeball;
	// peck
	/// Peck Mode
	enum SansgridPeckRecognitionEnum peck_mode;
	/// Peck test state
	PayloadTestNode *peck;
	// sing
	/// Sing Mode
	enum SansgridDataTypeEnum sing_mode;
	/// Sing test state
	PayloadTestNode *sing;
	// mock
	/// Mock Mode
	enum SansgridDataTypeEnum mock_mode;
	/// Mock test state
	PayloadTestNode *mock;
	// peacock
	/// Peacock Mode
	enum SansgridDataTypeEnum peacock_mode;
	/// Peacock test state
	PayloadTestNode *peacock;
	// nest
	/// Nest Mode
	enum SansgridDataTypeEnum nest_mode;
	/// Nest test state
	PayloadTestNode *nest;
	// squawk
	/// Squawk Server Mode
	enum SansgridDataTypeEnum squawk_server_mode;
	/// Squawk Sensor Mode
	enum SansgridDataTypeEnum squawk_sensor_mode;
	/// Squawk server test state
	PayloadTestNode *squawk_server;
	/// Squawk sensor test state
	PayloadTestNode *squawk_sensor;
	// heartbeat
	/// Heartbeat Mode
	enum SansgridDataTypeEnum heartbeat_mode;
	/// Heartbeat test state
	PayloadTestNode *heartbeat;
	// chirp
	/// Chirp Mode
	enum SansgridDataTypeEnum chirp_mode;
	/// Chirp test state
	PayloadTestNode *chirp;
} PayloadTestStruct;


/// Global dispatch
Queue *dispatch;
/// Global routing table
RoutingTable *routing_table;

// Payload Test Structure functions
void testStructInit(PayloadTestStruct *test_struct);
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
void payloadMkChirp(SansgridChirp *sg_chirp, PayloadTestStruct *test_specs);

// Size checking handlers
void checkSize(const char *pkname, size_t pksize);

// Payload Readers
void *spiPayloadReader(void *arg);
void *tcpPayloadReader(void *arg);

// Payload State
int32_t payloadRoutingInit(void);
int32_t payloadRoutingDestroy(void);
int32_t payloadRoutingAddReference(void);
int32_t payloadRoutingRemoveReference(void);
int32_t payloadStateInit(void);
int32_t payloadStateCommit(SansgridSerial **, int packets);

// TestSuites
Suite *payloadSizeTesting (void);
Suite *payloadTestEyeball(void);
Suite *payloadTestPeck(void);
Suite *payloadTestSing(void);
Suite *payloadTestMock(void);
Suite *payloadTestPeacock(void);
Suite *payloadTestSquawk(void);
Suite *payloadTestNest(void);
Suite *payloadTestChirp(void);

// Payload Tests
int testPayload(PayloadTestStruct *test_struct);
int testEyeballPayload(PayloadTestStruct *test_struct);
int testPeckPayload(PayloadTestStruct *test_struct);
int testSingPayload(PayloadTestStruct *test_struct);
int testMockPayload(PayloadTestStruct *test_struct);
int testPeacockPayload(PayloadTestStruct *test_struct);
int testSquawkPayloadAuthBoth(PayloadTestStruct *test_struct);
int testSquawkPayloadAuthSensor(PayloadTestStruct *test_struct);
int testSquawkPayloadAuthServer(PayloadTestStruct *test_struct);
int testSquawkPayloadNoAuth(PayloadTestStruct *test_struct);
int testNestPayload(PayloadTestStruct *test_struct);
int testChirpPayloadSensorToServer(PayloadTestStruct *test_struct);
int testChirpPayloadServerToSensor(PayloadTestStruct *test_struct);



#endif
// vim: ft=c ts=4 noet sw=4:


