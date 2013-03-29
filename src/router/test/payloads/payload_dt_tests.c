/* Peacock Tests
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
 */
#include "payload_tests.h"


sem_t spi_readlock,
  	  tcp_readlock;


static int testPayloadSpecific(SansgridSerial *sg_serial, PayloadTestNode *test_node,
		int(*fn)(RoutingTable*, SansgridSerial*), const char *message) {

	TalkStub *ts_serial = talkStubUseSerial(1),
			 *ts_tcp = talkStubUseTCP(1);
	int exit_code;
	SansgridSerial *sg_serial_read;
	talkStubSetReadlock(ts_serial, &spi_readlock);
	talkStubSetReadlock(ts_tcp, &tcp_readlock);


	// Set which fifos we're writing to
	// (This is so we don't read back bad data)
	switch (test_node->read_dir) {
		case SG_TEST_COMM_WRITE_SPI:
			sem_post(&spi_readlock);
			break;
		case SG_TEST_COMM_WRITE_TCP:
			sem_post(&tcp_readlock);
			break;
		case SG_TEST_COMM_WRITE_BOTH:
			sem_post(&spi_readlock);
			sem_post(&tcp_readlock);
			break;
		default:
			break;
	}

#if TESTS_DEBUG_LEVEL > 0
	printf("%s\n", message);
#endif
	payloadStateInit();
	exit_code = fn(routing_table, sg_serial);
	// Commit handler
	payloadStateCommit(&sg_serial_read);
#if TESTS_DEBUG_LEVEL > 0
	printf("Handled with status: %i\n", exit_code);
#endif

#if TESTS_DEBUG_LEVEL > 1
	printf("Sent: ");
	for (int i=0; i<PAYLOAD_SIZE; i++)
		printf("%.2x", sg_serial->payload[i]);
	printf("\n");
	printf("Read: ");
	for (int i=0; i<PAYLOAD_SIZE; i++)
		printf("%.2x", sg_serial_read->payload[i]);
	printf("\n");
#endif
	if (memcmp(sg_serial_read->payload, &sg_serial->payload, sizeof(SansgridMock)))
		fail("Packet Mismatch");
	int orig = routingTableLookupNextExpectedPacket(routing_table, sg_serial_read->origin_ip);
	int dest = routingTableLookupNextExpectedPacket(routing_table, sg_serial_read->dest_ip);
	if (test_node->next_packet != (orig | dest))
		fail("Control Flow Mismatch \
				\n\tExpected: %i \
				\n\tGot: %i", test_node->next_packet, 
				(orig | dest));
	if (!routingTableLookup(routing_table, sg_serial_read->origin_ip)
			&& !routingTableLookup(routing_table, sg_serial_read->dest_ip))
		fail("No IP assigned");
	memcpy(sg_serial, sg_serial_read, sizeof(SansgridSerial));
	free(sg_serial_read);
	return exit_code;
}



static int testPayload(PayloadTestStruct *test_struct) {
	// unit test code to test the Eyeball data type
	// (The next packet would be a Squawk)
	int32_t exit_code;
	SansgridEyeball sg_eyeball;
	SansgridPeck sg_peck;
	SansgridSing sg_sing;
	SansgridMock sg_mock;
	SansgridPeacock sg_peacock;
	SansgridSquawk sg_squawk;
	SansgridNest sg_nest;
	SansgridSerial sg_serial;
	uint8_t ip_addr[IP_SIZE];

	/*
#if TESTS_DEBUG_LEVEL > 0
	printf("\n");
	printf("%s\n", message);
#endif
*/
	// initialize dispatch/routing, set up fifos/threads
	payloadRoutingInit();
	sem_init(&spi_readlock, 0, 0);
	sem_init(&tcp_readlock, 0, 0);

	if (test_struct->eyeball) {
		payloadMkSerial(&sg_serial);
		payloadMkEyeball(&sg_eyeball, test_struct);
		memcpy(&sg_serial.payload, &sg_eyeball, sizeof(SansgridEyeball));
		exit_code = testPayloadSpecific(&sg_serial, test_struct->eyeball, routerHandleEyeball, "Eyeballing");
		if (exit_code)
			return exit_code;
		memcpy(&ip_addr, &sg_serial.origin_ip, IP_SIZE);
	}
	if (test_struct->peck) {
		payloadMkSerial(&sg_serial);
		memcpy(&sg_serial.dest_ip, ip_addr, IP_SIZE);
		payloadMkPeck(&sg_peck, test_struct);
		memcpy(&sg_serial.payload, &sg_peck, sizeof(SansgridPeck));
		exit_code = testPayloadSpecific(&sg_serial, test_struct->peck, routerHandlePeck, "Pecking");
		if (exit_code)
			return exit_code;
	}
	if (test_struct->sing) {
		payloadMkSerial(&sg_serial);
		memcpy(&sg_serial.dest_ip, ip_addr, IP_SIZE);
		payloadMkSing(&sg_sing, test_struct);
		memcpy(&sg_serial.payload, &sg_sing, sizeof(SansgridSing));
		exit_code = testPayloadSpecific(&sg_serial, test_struct->sing, routerHandleSing, "Singing");
		if (exit_code)
			return exit_code;
	}
	if (test_struct->mock) {
		payloadMkSerial(&sg_serial);
		memcpy(&sg_serial.origin_ip, ip_addr, IP_SIZE);
		payloadMkMock(&sg_mock, test_struct);
		memcpy(&sg_serial.payload, &sg_mock, sizeof(SansgridMock));
		exit_code = testPayloadSpecific(&sg_serial, test_struct->mock, routerHandleMock, "Mocking");
		if (exit_code)
			return exit_code;
	}
	if (test_struct->peacock) {
		payloadMkSerial(&sg_serial);
		memcpy(&sg_serial.origin_ip, ip_addr, IP_SIZE);
		payloadMkPeacock(&sg_peacock, test_struct);
		memcpy(&sg_serial.payload, &sg_peacock, sizeof(SansgridPeacock));
		exit_code = testPayloadSpecific(&sg_serial, test_struct->peacock, routerHandlePeacock, "Peacocking");
		if (exit_code)
			return exit_code;
	}
	if (test_struct->squawk_server) {
		int sensor_req_auth = 0;
		payloadMkSerial(&sg_serial);
		switch (test_struct->squawk_server_mode) {
			case SG_SQUAWK_SERVER_CHALLENGE_SENSOR:
			case SG_SQUAWK_SERVER_NOCHALLENGE_SENSOR:
			case SG_SQUAWK_SERVER_DENY_SENSOR:
			case SG_SQUAWK_SERVER_RESPOND:
				memcpy(&sg_serial.dest_ip, ip_addr, IP_SIZE);
				break;
			default:
				// error
				fail("Squawk: bad mode");
		}
		switch (test_struct->squawk_sensor_mode) {
			case SG_SQUAWK_SENSOR_RESPOND_REQUIRE_CHALLENGE:
			case SG_SQUAWK_SENSOR_CHALLENGE_SERVER:
				sensor_req_auth = 1;
			case SG_SQUAWK_SENSOR_RESPOND_NO_REQUIRE_CHALLENGE:
			case SG_SQUAWK_SENSOR_ACCEPT_RESPONSE:
				memcpy(&sg_serial.origin_ip, ip_addr, IP_SIZE);
				break;
			default:
				// error
				fail("Squawk: bad mode");
				break;
		}
		// Server: Challenge/Nochallenge
		payloadMkSquawkServer(&sg_squawk, test_struct);
		memcpy(&sg_serial.payload, &sg_squawk, sizeof(SansgridSquawk));
		exit_code = testPayloadSpecific(&sg_serial, test_struct->squawk_server,
				routerHandleSquawk, "Squawking (Server challenge/nochallenge)");
		if (exit_code)
			return exit_code;

		// Sensor: Respond
		payloadMkSquawkSensor(&sg_squawk, test_struct);
		sg_squawk.datatype = sensor_req_auth ?
				SG_SQUAWK_SENSOR_RESPOND_REQUIRE_CHALLENGE
				: SG_SQUAWK_SENSOR_RESPOND_NO_REQUIRE_CHALLENGE;
		memcpy(&sg_serial.payload, &sg_squawk, sizeof(SansgridSquawk));
		exit_code = testPayloadSpecific(&sg_serial, test_struct->squawk_sensor,
				routerHandleSquawk, "Squawking (Sensor response)");
		if (exit_code)
			return exit_code;

		// If sensor requires challenge, send it
		if (sensor_req_auth) {
			payloadMkSquawkSensor(&sg_squawk, test_struct);
			sg_squawk.datatype = SG_SQUAWK_SENSOR_CHALLENGE_SERVER;
			memcpy(&sg_serial.payload, &sg_squawk, sizeof(SansgridSquawk));
			exit_code = testPayloadSpecific(&sg_serial, test_struct->squawk_sensor,
					routerHandleSquawk, "Squawking (Sensor challenge)");
			if (exit_code)
				return exit_code;
		}
		// Server response
		payloadMkSquawkServer(&sg_squawk, test_struct);
		// TODO: Allow respond/deny here
		sg_squawk.datatype = SG_SQUAWK_SERVER_RESPOND;
		memcpy(&sg_serial.payload, &sg_squawk, sizeof(SansgridSquawk));
		exit_code = testPayloadSpecific(&sg_serial, test_struct->squawk_server,
				routerHandleSquawk, "Squawking (Server response)");
		if (exit_code)
			return exit_code;

		// Sensor accepts/rejects
		// TODO: Allow respond/deny here
		payloadMkSquawkSensor(&sg_squawk, test_struct);
		sg_squawk.datatype = SG_SQUAWK_SENSOR_ACCEPT_RESPONSE;
		memcpy(&sg_serial.payload, &sg_squawk, sizeof(SansgridSquawk));
		test_struct->squawk_sensor->next_packet = SG_DEVSTATUS_NESTING;;
		exit_code = testPayloadSpecific(&sg_serial, test_struct->squawk_sensor,
				routerHandleSquawk, "Squawking (Sensor Accept/Reject)");
		if (exit_code)
			return exit_code;
		
	}
	if (test_struct->nest) {
		payloadMkSerial(&sg_serial);
		memcpy(&sg_serial.dest_ip, ip_addr, IP_SIZE);
		payloadMkNest(&sg_nest, test_struct);
		memcpy(&sg_serial.payload, &sg_nest, sizeof(SansgridNest));
		exit_code = testPayloadSpecific(&sg_serial, test_struct->nest, routerHandleNest, "Nesting");
		if (exit_code)
			return exit_code;
	}


#if TESTS_DEBUG_LEVEL > 0
	printf("Origin IP: ");
	routingTablePrint(sg_serial.origin_ip);
	printf("Dest   IP: ");
	routingTablePrint(sg_serial.dest_ip);
	printf("Sent: ");
	for (int i=0; i<PAYLOAD_SIZE; i++)
		printf("%.2x", sg_serial.payload[i]);
	printf("\n");
	printf("Read: ");
	for (int i=0; i<PAYLOAD_SIZE; i++)
		printf("%.2x", sg_serial.payload[i]);
	printf("\n\n");
#endif

	// Final Cleanup
	queueDestroy(dispatch);
	routingTableDestroy(routing_table);
	sem_destroy(&spi_readlock);
	sem_destroy(&tcp_readlock);
	return 0;
}

void testStructInit(PayloadTestStruct *test_struct) {
	test_struct->eyeball = NULL;
	test_struct->peck = NULL;
	test_struct->sing = NULL;
	test_struct->mock = NULL;
	test_struct->peacock = NULL;
	test_struct->squawk_server = NULL;
	test_struct->squawk_sensor = NULL;
	test_struct->nest = NULL;
	test_struct->heartbeat = NULL;
	test_struct->chirp = NULL;
}

void testEyeballPayload(PayloadTestStruct *test_struct) {
	// Call Eyeball tests with all options
	// Mate, NoMate
	// Next payload is always pecking
	PayloadTestNode eyeball = { SG_TEST_COMM_WRITE_TCP, SG_DEVSTATUS_PECKING };
	test_struct->eyeball = &eyeball;
	test_struct->eyeball_mode = SG_EYEBALL_MATE;
	testPayload(test_struct);
	// FIXME: This path is not implemented yet
	//test_struct->eyeball_mode = SG_EYEBALL_NOMATE;
	//testPayload(test_struct);
	return;
}

void testPeckPayload(PayloadTestStruct *test_struct) {
	// Call Peck tests with all options
	PayloadTestNode peck;

	// Set defaults
	peck.read_dir = SG_TEST_COMM_WRITE_SPI;
	// Assign nodes
	test_struct->peck = &peck;

	// Test device unrecognized
	peck.next_packet = SG_DEVSTATUS_SINGING;
	test_struct->peck_mode = SG_PECK_MATE;
	testEyeballPayload(test_struct);

	// Test device recognized
	peck.next_packet = SG_DEVSTATUS_SQUAWKING;
	test_struct->peck_mode = SG_PECK_RECOGNIZED;
	testEyeballPayload(test_struct);

	return;
}


void testSingPayload(PayloadTestStruct *test_struct) {
	// Call Sing tests with all options
	PayloadTestNode eyeball = { SG_TEST_COMM_WRITE_TCP, SG_DEVSTATUS_PECKING };
	PayloadTestNode peck = { SG_TEST_COMM_WRITE_SPI, SG_DEVSTATUS_SINGING };
	PayloadTestNode sing = { SG_TEST_COMM_WRITE_SPI, SG_DEVSTATUS_MOCKING };

	// Set defaults
	sing.read_dir = SG_TEST_COMM_WRITE_SPI;
	sing.next_packet = SG_DEVSTATUS_MOCKING;
	test_struct->eyeball_mode = SG_EYEBALL_MATE;
	test_struct->peck_mode = SG_PECK_MATE;
	// Assign nodes
	test_struct->eyeball = &eyeball;
	test_struct->peck = &peck;
	test_struct->sing = &sing;

	// Test server authenticating with key
	test_struct->sing_mode = SG_SING_WITH_KEY;
	testPayload(test_struct);

	// Test server authenticating without key
	test_struct->sing_mode = SG_SING_WITHOUT_KEY;
	testPayload(test_struct);

	return;
}


void testMockPayload(PayloadTestStruct *test_struct) {
	// Call mock tests with all options
	PayloadTestNode mock = { SG_TEST_COMM_WRITE_TCP, SG_DEVSTATUS_PEACOCKING };
	test_struct->mock = &mock;
	test_struct->mock_mode = SG_MOCK_WITH_KEY;
	testSingPayload(test_struct);
	test_struct->mock_mode = SG_MOCK_WITHOUT_KEY;
	testSingPayload(test_struct);
	return;
}


void testPeacockPayload(PayloadTestStruct *test_struct) {
	// Call peacock tests with all valid options
	PayloadTestNode peacock = { SG_TEST_COMM_WRITE_TCP, SG_DEVSTATUS_NESTING };
	test_struct->peacock = &peacock;
	test_struct->peacock_mode = SG_PEACOCK;
	testMockPayload(test_struct);
	return;
}


void testNestPayload(PayloadTestStruct *test_struct) {
	// Call Nest tests with all valid options
	PayloadTestNode nest = { SG_TEST_COMM_WRITE_SPI, SG_DEVSTATUS_LEASED };
	test_struct->nest = &nest;
	test_struct->nest_mode = SG_NEST;
	testPeacockPayload(test_struct);
	return;
}


void testSquawkPayloadAuthBoth(PayloadTestStruct *test_struct) {
	// Call squawk tests with all valid options
	PayloadTestNode squawk_server,
					squawk_sensor;
	PayloadTestNode peck = { SG_TEST_COMM_WRITE_SPI, SG_DEVSTATUS_SQUAWKING };
	test_struct->squawk_server = &squawk_server;
	test_struct->squawk_sensor = &squawk_sensor;
	test_struct->peck = &peck;
	test_struct->peck_mode = SG_PECK_RECOGNIZED;

	// Server Challenges sensor
	squawk_server.read_dir = SG_TEST_COMM_WRITE_SPI;
	squawk_server.next_packet = SG_DEVSTATUS_SQUAWKING;
	test_struct->squawk_server_mode = SG_SQUAWK_SERVER_CHALLENGE_SENSOR;

	// Sensor Challenges sever
	squawk_sensor.read_dir = SG_TEST_COMM_WRITE_TCP;
	squawk_sensor.next_packet = SG_DEVSTATUS_SQUAWKING;
	test_struct->squawk_sensor_mode = SG_SQUAWK_SENSOR_CHALLENGE_SERVER;

	testEyeballPayload(test_struct);
}


void testSquawkPayloadAuthSensor(PayloadTestStruct *test_struct) {
	// Call squawk: sensor requires authentication
	// with all valid options
	PayloadTestNode squawk_server,
					squawk_sensor;
	PayloadTestNode peck = { SG_TEST_COMM_WRITE_SPI, SG_DEVSTATUS_SQUAWKING };
	test_struct->squawk_server = &squawk_server;
	test_struct->squawk_sensor = &squawk_sensor;
	test_struct->peck = &peck;
	test_struct->peck_mode = SG_PECK_RECOGNIZED;

	// Server Doesn't challenge sensor
	squawk_server.read_dir = SG_TEST_COMM_WRITE_SPI;
	squawk_server.next_packet = SG_DEVSTATUS_SQUAWKING;
	test_struct->squawk_server_mode = SG_SQUAWK_SERVER_NOCHALLENGE_SENSOR;

	// Sensor Challenges server
	squawk_sensor.read_dir = SG_TEST_COMM_WRITE_TCP;
	squawk_sensor.next_packet = SG_DEVSTATUS_SQUAWKING;
	test_struct->squawk_sensor_mode = SG_SQUAWK_SENSOR_CHALLENGE_SERVER;

	testEyeballPayload(test_struct);
}
	

void testSquawkPayloadAuthServer(PayloadTestStruct *test_struct) {
	// Call squawk: server requires authentication
	// with all valid options
	PayloadTestNode squawk_server,
					squawk_sensor;
	PayloadTestNode peck = { SG_TEST_COMM_WRITE_SPI, SG_DEVSTATUS_SQUAWKING };
	test_struct->squawk_server = &squawk_server;
	test_struct->squawk_sensor = &squawk_sensor;
	test_struct->peck = &peck;
	test_struct->peck_mode = SG_PECK_RECOGNIZED;

	// Server Doesn't challenge sensor
	squawk_server.read_dir = SG_TEST_COMM_WRITE_SPI;
	squawk_server.next_packet = SG_DEVSTATUS_SQUAWKING;
	test_struct->squawk_server_mode = SG_SQUAWK_SERVER_CHALLENGE_SENSOR;

	// Sensor Challenges server
	squawk_sensor.read_dir = SG_TEST_COMM_WRITE_TCP;
	squawk_sensor.next_packet = SG_DEVSTATUS_SQUAWKING;
	test_struct->squawk_sensor_mode = SG_SQUAWK_SENSOR_RESPOND_NO_REQUIRE_CHALLENGE;

	testEyeballPayload(test_struct);
}

void testSquawkPayloadNoAuth(PayloadTestStruct *test_struct) {
	// Call squawk: No authentication
	// with all valid options
	PayloadTestNode squawk_server,
					squawk_sensor;
	PayloadTestNode peck = { SG_TEST_COMM_WRITE_SPI, SG_DEVSTATUS_SQUAWKING };
	test_struct->squawk_server = &squawk_server;
	test_struct->squawk_sensor = &squawk_sensor;
	test_struct->peck = &peck;
	test_struct->peck_mode = SG_PECK_RECOGNIZED;

	// Server Doesn't challenge sensor
	squawk_server.read_dir = SG_TEST_COMM_WRITE_SPI;
	squawk_server.next_packet = SG_DEVSTATUS_SQUAWKING;
	test_struct->squawk_server_mode = SG_SQUAWK_SERVER_NOCHALLENGE_SENSOR;

	// Sensor Challenges server
	squawk_sensor.read_dir = SG_TEST_COMM_WRITE_TCP;
	squawk_sensor.next_packet = SG_DEVSTATUS_SQUAWKING;
	test_struct->squawk_sensor_mode = SG_SQUAWK_SENSOR_RESPOND_NO_REQUIRE_CHALLENGE;

	testEyeballPayload(test_struct);
}


// Unit test definitions

START_TEST (testEyeball) {
#if TESTS_DEBUG_LEVEL > 0
	printf("\n\nTesting Eyeball\n");
#endif
	PayloadTestStruct test_struct;
	testStructInit(&test_struct);
	testEyeballPayload(&test_struct);
#if TESTS_DEBUG_LEVEL > 0
	printf("Successfully Eyeballed\n");
#endif
}
END_TEST


START_TEST (testPeck) {
#if TESTS_DEBUG_LEVEL > 0
	printf("\n\nTesting Peck\n");
#endif
	PayloadTestStruct test_struct;
	testStructInit(&test_struct);
	testPeckPayload(&test_struct);
#if TESTS_DEBUG_LEVEL > 0
	printf("Successfully Pecked\n");
#endif
}
END_TEST


START_TEST (testSing) {
#if TESTS_DEBUG_LEVEL > 0
	printf("\n\nTesting Singing\n");
#endif
	PayloadTestStruct test_struct;
	testStructInit(&test_struct);
	testSingPayload(&test_struct);
#if TESTS_DEBUG_LEVEL > 0
	printf("Successfully Sung\n");
#endif
}
END_TEST


START_TEST (testMock) {
#if TESTS_DEBUG_LEVEL > 0
	printf("\n\nTesting Mocking\n");
#endif
	PayloadTestStruct test_struct;
	testStructInit(&test_struct);
	testMockPayload(&test_struct);
#if TESTS_DEBUG_LEVEL > 0
	printf("Successfully Mocked\n");
#endif
}
END_TEST


START_TEST (testPeacock) {
#if TESTS_DEBUG_LEVEL > 0
	printf("\n\nTesting Peacocking\n");
#endif
	PayloadTestStruct test_struct;
	testStructInit(&test_struct);
	testPeacockPayload(&test_struct);
#if TESTS_DEBUG_LEVEL > 0
	printf("Successfully Peacocked\n");
#endif
}
END_TEST


START_TEST (testNest) {
#if TESTS_DEBUG_LEVEL > 0
	printf("\n\nTesting Nesting\n");
#endif
	PayloadTestStruct test_struct;
	testStructInit(&test_struct);
	testNestPayload(&test_struct);
#if TESTS_DEBUG_LEVEL > 0
	printf("Successfully Nested\n");
#endif
}
END_TEST


START_TEST (testSquawk) {
#if TESTS_DEBUG_LEVEL > 0
	printf("\n\nTesting Squawking\n");
#endif
	PayloadTestStruct test_struct;
	// Test both authenticate
	testStructInit(&test_struct);
	testSquawkPayloadAuthBoth(&test_struct);
	// Test sensor authenticates
	testStructInit(&test_struct);
	testSquawkPayloadAuthSensor(&test_struct);
	// Test server authenticates
	testStructInit(&test_struct);
	testSquawkPayloadAuthServer(&test_struct);
	// Test no authentication
	testStructInit(&test_struct);
	testSquawkPayloadNoAuth(&test_struct);
#if TESTS_DEBUG_LEVEL > 0
	printf("Successfully Squawked\n");
#endif
}
END_TEST

Suite *payloadTesting (void) {
	Suite *s = suite_create("Payload Handling Tests");
	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, testEyeball);
	tcase_add_test(tc_core, testPeck);
	tcase_add_test(tc_core, testSing);
	tcase_add_test(tc_core, testMock);
	tcase_add_test(tc_core, testPeacock);
	tcase_add_test(tc_core, testNest);
	tcase_add_test(tc_core, testSquawk);


	suite_add_tcase(s, tc_core);

	return s;
}


// vim: ft=c ts=4 noet sw=4:
