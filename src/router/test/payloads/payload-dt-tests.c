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
#include "payload-tests.h"


sem_t spi_readlock,
  	  tcp_readlock;


static int testPayloadSpecific(SansgridSerial *sg_serial, PayloadTestNode *test_node,
		int(*fn)(RoutingTable*, SansgridSerial*)) {

	SansgridSerial *sg_serial_read;
	sgSerialTestSetReadlock(&spi_readlock);
	sgTCPTestSetReadlock(&tcp_readlock);


	// Call Eyeball handler
	switch (test_node->read_dir) {
		case SG_TEST_COMM_READ_SPI:
			sem_post(&spi_readlock);
			break;
		case SG_TEST_COMM_READ_TCP:
			sem_post(&tcp_readlock);
			break;
		case SG_TEST_COMM_READ_BOTH:
			sem_post(&spi_readlock);
			sem_post(&tcp_readlock);
			break;
		default:
			break;
	}

	payloadStateInit();
	fn(routing_table, sg_serial);
	// Commit handler
	payloadStateCommit(&sg_serial_read);

	if (memcmp(sg_serial_read->payload, &sg_serial->payload, sizeof(SansgridMock)))
		fail("Packet Mismatch");
	if (test_node->next_packet != routingTableLookupNextExpectedPacket(routing_table, sg_serial_read->origin_ip))
		fail("Control Flow Mismatch \
				\n\tExpected: %i \
				\n\tGot: %i", test_node->next_packet, 
				routingTableLookupNextExpectedPacket(routing_table, 
					sg_serial_read->origin_ip));
	if (!routingTableLookup(routing_table, sg_serial_read->origin_ip)
			&& !routingTableLookup(routing_table, sg_serial_read->dest_ip))
		fail("No IP assigned");
	memcpy(sg_serial, sg_serial_read, sizeof(SansgridSerial));
	return 0;
}



static int testPayload(PayloadTestStruct *test_struct) {
	// unit test code to test the Eyeball data type
	// (The next packet would be a Squawk)
	SansgridEyeball sg_eyeball;
	SansgridPeck sg_peck;
	SansgridSing sg_sing;
	SansgridMock sg_mock;
	SansgridPeacock sg_peacock;
	SansgridSerial sg_serial;

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

	payloadMkSerial(&sg_serial);
	if (test_struct->eyeball) {
		payloadMkEyeball(&sg_eyeball, test_struct);
		memcpy(&sg_serial.payload, &sg_eyeball, sizeof(SansgridEyeball));
		testPayloadSpecific(&sg_serial, test_struct->eyeball, routerHandleEyeball);
	}
	if (test_struct->peck) {
		payloadMkPeck(&sg_peck, test_struct);
		memcpy(&sg_serial.payload, &sg_peck, sizeof(SansgridPeck));
		testPayloadSpecific(&sg_serial, test_struct->peck, routerHandlePeck);
	}
	if (test_struct->sing) {
		payloadMkSing(&sg_sing, test_struct);
		memcpy(&sg_serial.payload, &sg_sing, sizeof(SansgridMock));
		testPayloadSpecific(&sg_serial, test_struct->sing, routerHandleSing);
	}
	if (test_struct->mock) {
		payloadMkMock(&sg_mock, test_struct);
		memcpy(&sg_serial.payload, &sg_mock, sizeof(SansgridMock));
		testPayloadSpecific(&sg_serial, test_struct->mock, routerHandleMock);
	}


#if TESTS_DEBUG_LEVEL > 0
	printf("Origin IP: ");
	routingTablePrint(sg_serial.origin_ip);
	printf("Dest   IP: ");
	routingTablePrint(sg_serial.dest_ip);
	printf("Sent: ");
	for (int i=0; i<sizeof(SansgridMock); i++)
		printf("%.2x", sg_serial.payload[i]);
	printf("\n");
	printf("Read: ");
	for (int i=0; i<sizeof(SansgridMock); i++)
		printf("%.2x", sg_serial.payload[i]);
	printf("\n");
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
	test_struct->squawk = NULL;
	test_struct->nest = NULL;
	test_struct->heartbeat = NULL;
	test_struct->chirp = NULL;
}

void testEyeballPayload(PayloadTestStruct *test_struct) {
	// Call Eyeball tests with all options
	PayloadTestNode eyeball = { SG_TEST_COMM_READ_TCP, SG_DEVSTATUS_PECKING };
	test_struct->eyeball = &eyeball;
	test_struct->eyeball_mode = SG_EYEBALL_MATE;
	testPayload(test_struct);
	test_struct->eyeball_mode = SG_EYEBALL_NOMATE;
	testPayload(test_struct);
	return;
}

void testPeckPayload(PayloadTestStruct *test_struct) {
	// Call Peck tests with all options
	PayloadTestNode eyeball = { SG_TEST_COMM_READ_TCP, SG_DEVSTATUS_PECKING };
	PayloadTestNode peck;

	// Set defaults
	peck.read_dir = SG_TEST_COMM_READ_SPI;
	test_struct->eyeball_mode = SG_EYEBALL_MATE;
	// Assign nodes
	test_struct->peck = &peck;
	test_struct->eyeball = &eyeball;

	// Test device unrecognized
	peck.next_packet = SG_DEVSTATUS_SINGING;
	test_struct->peck_mode = SG_PECK_MATE;
	testPayload(test_struct);

	// Test device recognized
	peck.next_packet = SG_DEVSTATUS_SQUAWKING;
	test_struct->peck_mode = SG_PECK_RECOGNIZED;
	testPayload(test_struct);

	return;
}


void testSingPayload(PayloadTestStruct *test_struct) {
	// Call Sing tests with all options
	PayloadTestNode eyeball = { SG_TEST_COMM_READ_TCP, SG_DEVSTATUS_PECKING };
	PayloadTestNode peck = { SG_TEST_COMM_READ_SPI, SG_DEVSTATUS_SINGING };
	PayloadTestNode sing = { SG_TEST_COMM_READ_SPI, SG_DEVSTATUS_MOCKING };

	// Set defaults
	sing.read_dir = SG_TEST_COMM_READ_SPI;
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
	PayloadTestNode mock = { SG_TEST_COMM_READ_TCP, SG_DEVSTATUS_PEACOCKING };
	test_struct->mock = &mock;
	test_struct->mock_mode = SG_MOCK_WITH_KEY;
	testSingPayload(test_struct);
	test_struct->mock_mode = SG_MOCK_WITHOUT_KEY;
	testSingPayload(test_struct);
	return;
}

START_TEST (testEyeball) {
#if TESTS_DEBUG_LEVEL > 0
	printf("\nTesting Eyeball\n");
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
	printf("\nTesting Peck\n");
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
	printf("\nTesting Singing\n");
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
	printf("\nTesting Mocking\n");
#endif
	PayloadTestStruct test_struct;
	testStructInit(&test_struct);
	testMockPayload(&test_struct);
#if TESTS_DEBUG_LEVEL > 0
	printf("Successfully Mocked\n");
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


	suite_add_tcase(s, tc_core);

	return s;
}


// vim: ft=c ts=4 noet sw=4:
