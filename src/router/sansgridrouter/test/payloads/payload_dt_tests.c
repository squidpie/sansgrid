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
 */
/// \file
#include "payload_tests.h"



/**
 * \brief Test a single payload
 *
 * This is the toplevel for payload testing.
 * \param	sg_serial	payload to test with
 * \param	test_node	specific test node to use for testing
 * \param	fn			callback to use to test
 * \param	message		message to use for logs
 */
static int testPayloadSpecific(SansgridSerial *sg_serial, PayloadTestNode *test_node,
		int(*fn)(RoutingTable*, SansgridSerial*), const char *message) {

	mark_point();
	int packets = 0;
	uint8_t ip_addr[IP_SIZE];

	TalkStub *ts_serial = talkStubInit("serial"),
			 *ts_tcp = talkStubInit("tcp");
	talkStubUseAsSPI(ts_serial);
	talkStubUseAsTCP(ts_tcp);
	int exit_code;
	SansgridSerial *sg_serial_read;
	SansgridSerial sg_serial_orig;

#ifdef SG_TEST_USE_EEPROM
	talkStubRegisterReadWriteFuncs(ts_serial, COMM_TYPE_EEPROM);
	talkStubRegisterReadWriteFuncs(ts_tcp, COMM_TYPE_EEPROM);
#else
	talkStubRegisterReadWriteFuncs(ts_serial, COMM_TYPE_UNIX_PIPE);
	talkStubRegisterReadWriteFuncs(ts_tcp, COMM_TYPE_UNIX_PIPE);
#endif

	mark_point();

	// Set which fifos we're writing to
	// (This is so we don't read back bad data)
	switch (test_node->read_dir) {
		case SG_TEST_COMM_WRITE_NONE:
			talkStubAssertInvalid(ts_serial);
			talkStubAssertInvalid(ts_tcp);
			packets = 0;
			break;
		case SG_TEST_COMM_WRITE_SPI:
			talkStubAssertValid(ts_serial);
			talkStubAssertInvalid(ts_tcp);
			packets = 1;
			break;
		case SG_TEST_COMM_WRITE_TCP:
			talkStubAssertInvalid(ts_serial);
			talkStubAssertValid(ts_tcp);
			packets = 1;
			break;
		case SG_TEST_COMM_WRITE_BOTH:
			talkStubAssertValid(ts_serial);
			talkStubAssertValid(ts_tcp);
			packets = 2;
			break;
		default:
			break;
	}

	mark_point();

#if TESTS_DEBUG_LEVEL > 0
	printf("%s\n", message);
#endif
	payloadStateInit();
	mark_point();
	memcpy(&sg_serial_orig, sg_serial, sizeof(SansgridSerial));
	exit_code = fn(routing_table, sg_serial);
	mark_point();
	// Commit handler
	payloadStateCommit(&sg_serial_read, packets);
	mark_point();
#if TESTS_DEBUG_LEVEL > 0
	printf("Handled with status: %i\n", exit_code);
#endif

#if TESTS_DEBUG_LEVEL > 1
	printf("Sent: ");
	for (int i=0; i<PAYLOAD_SIZE; i++)
		printf("%.2x", sg_serial->payload[i]);
	printf("\n");
	printf("Read: ");
	if (sg_serial_read) {
		for (int i=0; i<PAYLOAD_SIZE; i++)
			printf("%.2x", sg_serial_read->payload[i]);
		printf("\n");
	} else {
		printf("Nothing\n");
	}
#endif
	mark_point();
	if (sg_serial_orig.payload[0] == SG_PECK) {
		// can't do broadcast, have to get actual IP
		SansgridPeck sg_peck;
		memcpy(&sg_peck, sg_serial_orig.payload, sizeof(SansgridPeck));
		memcpy(ip_addr, sg_peck.assigned_ip, IP_SIZE);
	} else {
		memcpy(ip_addr, sg_serial_orig.ip_addr, IP_SIZE);
	}

	if (test_node->expected_exit_code != exit_code) {
		fail("Exit code Mismatch with %s\
				\n\tExpected: %i \
				\n\tGot: 	  %i", message, test_node->expected_exit_code, exit_code);
	}
	if (!exit_code && sg_serial_read) {
		if (memcmp(sg_serial_read->payload, &sg_serial_orig.payload, sizeof(SansgridMock)))
			fail("Packet Mismatch with %s", message);
		int orig = routingTableLookupNextExpectedPacket(routing_table, ip_addr);
		if (test_node->next_packet != orig)
			fail("Control Flow Mismatch with %s\
					\n\tExpected: %i \
					\n\tGot:  	  %i", message, test_node->next_packet, 
					orig);
		if (!routingTableLookup(routing_table, ip_addr))
			fail("No IP assigned with %s", message);
		memcpy(sg_serial, sg_serial_read, sizeof(SansgridSerial));
	}

	mark_point();
	if (packets > 1)
		free(sg_serial_read);
	mark_point();
	talkStubDestroy(ts_serial);
	talkStubDestroy(ts_tcp);
	return exit_code;
}



/// Test a payload path
int testPayload(PayloadTestStruct *test_struct) {
	// unit test code to test data types
	int32_t exit_code = 0;
	SansgridEyeball sg_eyeball;
	SansgridPeck sg_peck;
	SansgridSing sg_sing;
	SansgridMock sg_mock;
	SansgridPeacock sg_peacock;
	SansgridSquawk sg_squawk;
	SansgridNest sg_nest;
	SansgridChirp sg_chirp;
	SansgridSerial sg_serial;
	uint8_t ip_addr[IP_SIZE];

	// initialize dispatch/routing, set up fifos/threads
	payloadRoutingAddReference();
	mark_point();

	// Payloads
	if (!exit_code && test_struct->eyeball) {
		payloadMkSerial(&sg_serial);
		payloadMkEyeball(&sg_eyeball, test_struct);
		memcpy(sg_serial.payload, &sg_eyeball, sizeof(SansgridEyeball));
		mark_point();
		exit_code = testPayloadSpecific(&sg_serial, test_struct->eyeball, routerHandleEyeball, "Eyeballing");
		mark_point();
		memcpy(ip_addr, sg_serial.ip_addr, IP_SIZE);
	}
	mark_point();
	if (!exit_code && test_struct->peck) {
		payloadMkSerial(&sg_serial);
		//memcpy(sg_serial.ip_addr, ip_addr, IP_SIZE);
		memset(sg_serial.ip_addr, 0xff, IP_SIZE);
		payloadMkPeck(&sg_peck, test_struct);
		memcpy(sg_peck.assigned_ip, ip_addr, IP_SIZE);
		memcpy(sg_serial.payload, &sg_peck, sizeof(SansgridPeck));
		exit_code = testPayloadSpecific(&sg_serial, test_struct->peck, routerHandlePeck, "Pecking");
	}
	mark_point();
	if (!exit_code && test_struct->sing) {
		payloadMkSerial(&sg_serial);
		memcpy(&sg_serial.ip_addr, ip_addr, IP_SIZE);
		payloadMkSing(&sg_sing, test_struct);
		memcpy(&sg_serial.payload, &sg_sing, sizeof(SansgridSing));
		exit_code = testPayloadSpecific(&sg_serial, test_struct->sing, routerHandleSing, "Singing");
	}
	mark_point();
	if (!exit_code && test_struct->mock) {
		payloadMkSerial(&sg_serial);
		memcpy(&sg_serial.ip_addr, ip_addr, IP_SIZE);
		payloadMkMock(&sg_mock, test_struct);
		memcpy(&sg_serial.payload, &sg_mock, sizeof(SansgridMock));
		exit_code = testPayloadSpecific(&sg_serial, test_struct->mock, routerHandleMock, "Mocking");
	}
	mark_point();
	if (!exit_code && test_struct->peacock) {
		payloadMkSerial(&sg_serial);
		memcpy(&sg_serial.ip_addr, ip_addr, IP_SIZE);
		payloadMkPeacock(&sg_peacock, test_struct);
		memcpy(&sg_serial.payload, &sg_peacock, sizeof(SansgridPeacock));
		exit_code = testPayloadSpecific(&sg_serial, test_struct->peacock, routerHandlePeacock, "Peacocking");
	}
	mark_point();
	if (!exit_code && test_struct->squawk_server) {
		int sensor_req_auth = 0;
		payloadMkSerial(&sg_serial);
		switch (test_struct->squawk_server_mode) {
			case SG_SQUAWK_SERVER_CHALLENGE_SENSOR:
			case SG_SQUAWK_SERVER_NOCHALLENGE_SENSOR:
			case SG_SQUAWK_SERVER_DENY_SENSOR:
			case SG_SQUAWK_SERVER_RESPOND:
				memcpy(&sg_serial.ip_addr, ip_addr, IP_SIZE);
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
				memcpy(&sg_serial.ip_addr, ip_addr, IP_SIZE);
				break;
			default:
				// error
				fail("Squawk: bad mode");
				break;
		}
		// Server: Challenge/Nochallenge
		payloadMkSerial(&sg_serial);
		memcpy(&sg_serial.ip_addr, ip_addr, IP_SIZE);
		payloadMkSquawkServer(&sg_squawk, test_struct);
		memcpy(&sg_serial.payload, &sg_squawk, sizeof(SansgridSquawk));
		exit_code = testPayloadSpecific(&sg_serial, test_struct->squawk_server,
				routerHandleSquawk, "Squawking (Server challenge/nochallenge)");

		if (!exit_code) {
			// Sensor: Respond
			payloadMkSerial(&sg_serial);
			memcpy(&sg_serial.ip_addr, ip_addr, IP_SIZE);
			payloadMkSquawkSensor(&sg_squawk, test_struct);
			sg_squawk.datatype = sensor_req_auth ?
					SG_SQUAWK_SENSOR_RESPOND_REQUIRE_CHALLENGE
					: SG_SQUAWK_SENSOR_RESPOND_NO_REQUIRE_CHALLENGE;
			memcpy(&sg_serial.payload, &sg_squawk, sizeof(SansgridSquawk));
			test_struct->squawk_sensor->next_packet = SG_DEVSTATUS_SQUAWKING;
			exit_code = testPayloadSpecific(&sg_serial, test_struct->squawk_sensor,
					routerHandleSquawk, "Squawking (Sensor response)");
		}

		// If sensor requires challenge, send it
		if (!exit_code && sensor_req_auth) {
			payloadMkSerial(&sg_serial);
			memcpy(&sg_serial.ip_addr, ip_addr, IP_SIZE);
			payloadMkSquawkSensor(&sg_squawk, test_struct);
			sg_squawk.datatype = SG_SQUAWK_SENSOR_CHALLENGE_SERVER;
			memcpy(&sg_serial.payload, &sg_squawk, sizeof(SansgridSquawk));
			exit_code = testPayloadSpecific(&sg_serial, test_struct->squawk_sensor,
					routerHandleSquawk, "Squawking (Sensor challenge)");
		}
		if (!exit_code) {
			// Server response
			payloadMkSerial(&sg_serial);
			memcpy(&sg_serial.ip_addr, ip_addr, IP_SIZE);
			payloadMkSquawkServer(&sg_squawk, test_struct);
			// TODO: Allow respond/deny here
			sg_squawk.datatype = SG_SQUAWK_SERVER_RESPOND;
			memcpy(&sg_serial.payload, &sg_squawk, sizeof(SansgridSquawk));
			exit_code = testPayloadSpecific(&sg_serial, test_struct->squawk_server,
					routerHandleSquawk, "Squawking (Server response)");
		}

		if (!exit_code) {
			// Sensor accepts/rejects
			// TODO: Allow respond/deny here
			payloadMkSerial(&sg_serial);
			memcpy(&sg_serial.ip_addr, ip_addr, IP_SIZE);
			payloadMkSquawkSensor(&sg_squawk, test_struct);
			sg_squawk.datatype = SG_SQUAWK_SENSOR_ACCEPT_RESPONSE;
			memcpy(&sg_serial.payload, &sg_squawk, sizeof(SansgridSquawk));
			test_struct->squawk_sensor->next_packet = SG_DEVSTATUS_NESTING;
			exit_code = testPayloadSpecific(&sg_serial, test_struct->squawk_sensor,
					routerHandleSquawk, "Squawking (Sensor Accept/Reject)");
		}
	}
	mark_point();
	if (!exit_code && test_struct->nest) {
		payloadMkSerial(&sg_serial);
		memcpy(&sg_serial.ip_addr, ip_addr, IP_SIZE);
		payloadMkNest(&sg_nest, test_struct);
		memcpy(&sg_serial.payload, &sg_nest, sizeof(SansgridNest));
		exit_code = testPayloadSpecific(&sg_serial, test_struct->nest,
				routerHandleNest, "Nesting");
	}
	mark_point();
	if (!exit_code && test_struct->chirp) {
		payloadMkSerial(&sg_serial);
		payloadMkChirp(&sg_chirp, test_struct);
		memcpy(&sg_serial.payload, &sg_chirp, sizeof(SansgridChirp));
		if (test_struct->chirp_mode == SG_CHIRP_DATA_SENSOR_TO_SERVER)
			memcpy(&sg_serial.ip_addr, ip_addr, IP_SIZE);
		else if (test_struct->chirp_mode == SG_CHIRP_COMMAND_SERVER_TO_SENSOR)
			memcpy(&sg_serial.ip_addr, ip_addr, IP_SIZE);
		else
			fail("Chirp: Bad datatype");
		exit_code = testPayloadSpecific(&sg_serial, test_struct->chirp,
				routerHandleChirp, "Chirping");
	}
	mark_point();


#if TESTS_DEBUG_LEVEL > 0
	printf("IP: ");
	routingTablePrint(sg_serial.ip_addr);
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
	payloadRoutingRemoveReference();
	return 0;
}

/// Initialize a test structure
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



// vim: ft=c ts=4 noet sw=4:
