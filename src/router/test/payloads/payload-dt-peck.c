/* Peck Tests
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


#include "payload-tests.h"


static int testPecking(enum SansgridPeckRecognitionEnum sg_peck_rec, const char *message) {
	// unit test code to test the Eyeball data type
	// (The next packet would be a Squawk)
	SansgridEyeball sg_eyeball;
	SansgridPeck sg_peck;
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
	payloadMkPeck(&sg_peck, sg_peck_rec);

	// Call Eyeball handler
	sem_post(&tcp_readlock);	// Data flows from sensor to server
	payloadStateInit();
	memcpy(&sg_serial.payload, &sg_eyeball, sizeof(SansgridEyeball));
	routerHandleEyeball(routing_table, &sg_serial);
	// Commit Eyeball handler
	payloadStateCommit();

	fail_if((queueSize(dispatch) == 0), "(Peck: Eyeball): No data on the dispatch!");
	if (queueDequeue(dispatch, (void**)&sg_serial_read) == -1)
		fail("Dispatch Failure");
	fail_if((queueSize(dispatch) > 0), "Too much data went on the dispatch");

	fail_if((sg_serial_read == NULL), "payload lost");
#if TESTS_DEBUG_LEVEL > 0
	printf("Successfully Eyeballed\n");
#endif

	// Call Peck handler
	sem_post(&spi_readlock);	// Data flows from server to sensor
	payloadStateInit();
	memcpy(&sg_serial.payload, &sg_peck, sizeof(SansgridPeck));
	routerHandlePeck(routing_table, &sg_serial);
	// Commit Peck handler
	payloadStateCommit();

	// Test current state
	fail_if((queueSize(dispatch) == 0), "(Peck: Peck): No data on the dispatch!");
	if (queueDequeue(dispatch, (void**)&sg_serial_read) == -1)
		fail("Dispatch Failure");
	fail_if((queueSize(dispatch) > 0), "Too much data went on the dispatch");

	fail_if((sg_serial_read == NULL), "payload lost");
#if TESTS_DEBUG_LEVEL > 0
	printf("Origin IP: ");
	routingTablePrint(sg_serial_read->origin_ip);
	printf("Dest   IP: ");
	routingTablePrint(sg_serial_read->dest_ip);
	printf("Sent: ");
	for (int i=0; i<sizeof(SansgridPeck); i++)
		printf("%.2x", sg_serial.payload[i]);
	printf("\n");
	printf("Read: ");
	for (int i=0; i<sizeof(SansgridPeck); i++)
		printf("%.2x", sg_serial_read->payload[i]);
	printf("\n");
#endif
	if (memcmp(sg_serial_read->payload, &sg_serial.payload, sizeof(SansgridPeck)))
		fail("Packet Mismatch");
	if (!routingTableLookup(routing_table, sg_serial_read->dest_ip))
		fail("No IP assigned");

	// Final Cleanup
	queueDestroy(dispatch);
	routingTableDestroy(routing_table);
	sem_destroy(&spi_readlock);
	sem_destroy(&tcp_readlock);
#if TESTS_DEBUG_LEVEL > 0
	printf("Successfully Pecked\n");
#endif
	return 0;
}


START_TEST (testPeckMating) {
	testPecking(SG_PECK_MATE, "Test Peck (Mate)");
}
END_TEST


START_TEST (testPeckRecognized) {
	testPecking(SG_PECK_RECOGNIZED, "Test Peck (Recognized)");
}
END_TEST


Suite *payloadPeckTesting (void) {
	Suite *s = suite_create("Peck Payload Tests");
	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, testPeckMating);
	tcase_add_test(tc_core, testPeckRecognized);

	suite_add_tcase(s, tc_core);

	return s;
}


// vim: ft=c ts=4 noet sw=4:
