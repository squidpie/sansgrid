/* Mock Tests
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


static int testMocking(enum SansgridDataTypeEnum mock_type, const char *message,
		enum SansgridDeviceStatusEnum next_packet) {
	// unit test code to test the Eyeball data type
	// (The next packet would be a Squawk)
	SansgridEyeball sg_eyeball;
	SansgridPeck sg_peck;
	SansgridSing sg_sing;
	SansgridMock sg_mock;
	SansgridSerial sg_serial;
	SansgridSerial *sg_serial_read;
	sem_t spi_readlock,
		  tcp_readlock;

#if TESTS_DEBUG_LEVEL > 0
	printf("\n");
	printf("%s\n", message);
#endif
	// initialize dispatch/routing, set up fifos/threads
	payloadRoutingInit();
	sem_init(&spi_readlock, 0, 0);
	sem_init(&tcp_readlock, 0, 0);

	sgSerialTestSetReadlock(&spi_readlock);
	sgTCPTestSetReadlock(&tcp_readlock);


	// Make packet
	payloadMkSerial(&sg_serial);
	payloadMkEyeball(&sg_eyeball, SG_EYEBALL_MATE);
	payloadMkPeck(&sg_peck, SG_PECK_MATE);
	payloadMkSing(&sg_sing, SG_SING_WITH_KEY);
	payloadMkMock(&sg_mock, mock_type);

	// Call Eyeball handler
	sem_post(&tcp_readlock);	// Data flows from sensor to server
	payloadStateInit();
	memcpy(&sg_serial.payload, &sg_eyeball, sizeof(SansgridEyeball));
	routerHandleEyeball(routing_table, &sg_serial);
	// Commit Eyeball handler
	payloadStateCommit(&sg_serial_read);

#if TESTS_DEBUG_LEVEL > 0
	printf("Successfully Eyeballed\n");
#endif

	// Call Peck handler
	sem_post(&spi_readlock);	// Data flows from server to sensor
	payloadStateInit();
	memcpy(&sg_serial.payload, &sg_peck, sizeof(SansgridPeck));
	routerHandlePeck(routing_table, &sg_serial);
	// Commit Peck handler
	payloadStateCommit(&sg_serial_read);

#if TESTS_DEBUG_LEVEL > 0
	printf("Successfully Pecked\n");
#endif

	// Call Sing handler
	sem_post(&spi_readlock);	// Data flows from server to sensor
	payloadStateInit();
	memcpy(&sg_serial.payload, &sg_sing, sizeof(SansgridSing));
	routerHandleSing(routing_table, &sg_serial);
	// Commit Sing handler
	payloadStateCommit(&sg_serial_read);

#if TESTS_DEBUG_LEVEL > 0
	printf("Successfully Sung\n");
#endif

	// Call Mock handler
	sem_post(&tcp_readlock);	// Data flows from sensor to server
	payloadStateInit();
	memcpy(&sg_serial.payload, &sg_mock, sizeof(SansgridMock));
	routerHandleMock(routing_table, &sg_serial);
	// Commit Sing handler
	payloadStateCommit(&sg_serial_read);


#if TESTS_DEBUG_LEVEL > 0
	printf("Origin IP: ");
	routingTablePrint(sg_serial_read->origin_ip);
	printf("Dest   IP: ");
	routingTablePrint(sg_serial_read->dest_ip);
	printf("Sent: ");
	for (int i=0; i<sizeof(SansgridMock); i++)
		printf("%.2x", sg_serial.payload[i]);
	printf("\n");
	printf("Read: ");
	for (int i=0; i<sizeof(SansgridMock); i++)
		printf("%.2x", sg_serial_read->payload[i]);
	printf("\n");
#endif
	if (memcmp(sg_serial_read->payload, &sg_serial.payload, sizeof(SansgridMock)))
		fail("Packet Mismatch");
	if (!routingTableLookup(routing_table, sg_serial_read->origin_ip))
		fail("No IP assigned");
	if (next_packet != routingTableLookupNextExpectedPacket(routing_table, sg_serial_read->origin_ip))
		fail("Control Flow Mismatch \
				\n\tExpected: %i \
				\n\tGot: %i", next_packet, 
				routingTableLookupNextExpectedPacket(routing_table, 
					sg_serial_read->origin_ip));

	// Final Cleanup
	queueDestroy(dispatch);
	routingTableDestroy(routing_table);
	sem_destroy(&spi_readlock);
	sem_destroy(&tcp_readlock);
#if TESTS_DEBUG_LEVEL > 0
	printf("Successfully Mocked\n");
#endif
	return 0;
}


START_TEST (testMockWithKey) {
	testMocking(SG_MOCK_WITH_KEY, "Test Mock (with key)", SG_DEVSTATUS_PEACOCKING);
}
END_TEST


START_TEST (testMockWithoutKey) {
	testMocking(SG_MOCK_WITHOUT_KEY, "Test Mock (without key)", SG_DEVSTATUS_PEACOCKING);
}
END_TEST


Suite *payloadMockTesting (void) {
	Suite *s = suite_create("Mock Payload Tests");
	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, testMockWithKey);
	tcase_add_test(tc_core, testMockWithoutKey);

	suite_add_tcase(s, tc_core);

	return s;
}


// vim: ft=c ts=4 noet sw=4:
