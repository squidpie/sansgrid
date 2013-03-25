/* Eyeball Tests
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



START_TEST (testEyeball) {
	// unit test code to test the Eyeball data type
	sem_t spi_readlock,
		  tcp_readlock;
	SansgridEyeball sg_eyeball;
	SansgridSerial sg_serial;
	SansgridSerial *sg_serial_read;

	// initialize dispatch/routing, set up fifos/threads
	payloadRoutingInit();
	sem_init(&spi_readlock, 0, 0);
	sem_init(&tcp_readlock, 0, 0);

	sgSerialTestSetReadlock(&spi_readlock);
	sgTCPTestSetReadlock(&tcp_readlock);


	// Make packet
	payloadMkSerial(&sg_serial);
	payloadMkEyeball(&sg_eyeball, SG_EYEBALL_MATE);

	// Call handler
	sem_post(&tcp_readlock);	// Data flows from sensor to server
	payloadStateInit();
	memcpy(&sg_serial.payload, &sg_eyeball, sizeof(SansgridEyeball));
	routerHandleEyeball(routing_table, &sg_serial);
	// Finish up with pipes/threads
	payloadStateCommit();

	// Test current state
	fail_if((queueSize(dispatch) == 0), "No data on the dispatch!");
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
	for (int i=0; i<sizeof(SansgridEyeball); i++)
		printf("%.2x", sg_serial.payload[i]);
	printf("\n");
	printf("Read: ");
	for (int i=0; i<sizeof(SansgridEyeball); i++)
		printf("%.2x", sg_serial_read->payload[i]);
	printf("\n");
#endif
	if (memcmp(sg_serial_read->payload, &sg_serial.payload, sizeof(SansgridEyeball)))
		fail("Packet Mismatch");
	if (!routingTableLookup(routing_table, sg_serial_read->origin_ip))
		fail("No IP assigned");

	// Final Cleanup
	queueDestroy(dispatch);
	routingTableDestroy(routing_table);
	sem_destroy(&spi_readlock);
	sem_destroy(&tcp_readlock);
#if TESTS_DEBUG_LEVEL > 0
	printf("Successfully Eyeballed\n");
#endif
}
END_TEST



Suite *payloadEyeballTesting (void) {
	Suite *s = suite_create("Eyeball Payload Tests");
	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, testEyeball);

	suite_add_tcase(s, tc_core);

	return s;
}


// vim: ft=c ts=4 noet sw=4:
