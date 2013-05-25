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
/// \file

#include "payload_tests.h"
/// Thread for reading from SPI
pthread_t serial_reader_thr;
/// Thread for reading from TCP
pthread_t tcp_reader_thr;

// Number of times allocated
static int payload_ref_count = 0;


/// Make sure size is sane
void checkSize(const char *pkname, size_t pksize) {
	fail_unless((pksize == PAYLOAD_SIZE), 
			"\n%s is wrong size: \
			\n\tExpected: %i\
			\n\tGot: %i", pkname, PAYLOAD_SIZE, pksize);
}



/// SPI reader runtime
void *spiPayloadReader(void *arg) {
	// Reads from a serial connection
	SansgridSerial *sg_serial = NULL;
	uint32_t packet_size;
	int excode;
	mark_point();

	mark_point();
	if ((excode = sgSerialReceive(&sg_serial, &packet_size)) == -1)
		fail("Failed to read packet");
	else if (excode == 0) {
		mark_point();
		if (queueEnqueue(dispatch, sg_serial) == -1)
			fail("Dispatch Enqueue Failure (Serial)");
#if TESTS_DEBUG_LEVEL > 0
		printf("(Serial) Queued\n");
#endif
	} else {
#if TESTS_DEBUG_LEVEL > 1
		printf("(Serial) Dropped\n");
#endif
	}
	mark_point();

	pthread_exit(arg);

}


/// TCP reader runtime
void *tcpPayloadReader(void *arg) {
	// Reads from a TCP connection
	SansgridSerial *sg_serial = NULL;
	uint32_t packet_size;
	int excode;

	mark_point();
	if ((excode = sgTCPReceive(&sg_serial, &packet_size)) == -1)
		fail("Failed to read TCP packet");
	else if (excode == 0) {
		if (queueEnqueue(dispatch, sg_serial) == -1)
			fail("Dispatch Enqueue Failure (TCP)");
#if TESTS_DEBUG_LEVEL > 0
		printf("(TCP) Queued\n");
#endif
	} else {
#if TESTS_DEBUG_LEVEL > 1
		printf("(TCP) Dropped\n");
#endif
	}
	mark_point();

	pthread_exit(arg);
}



/// Initialize dispatch, routing, etc
int32_t payloadRoutingInit(void) {
	uint8_t base[IP_SIZE];

	mark_point();

	dispatch = queueInit(200);
	fail_if((dispatch == NULL), "Error: dispatch is not initialized!");
	for (int i=0; i<IP_SIZE; i++)
		base[i] = 0x0;
	mark_point();
	routing_table = routingTableInit(base, "Test ESSID");
	fail_if((routing_table == NULL), "Error: routing table is not initialized!");
	mark_point();

	return 0;
}


/// Free resources like dispatch, routing, etc
int32_t payloadRoutingDestroy(void) {
	mark_point();
	queueDestroy(dispatch);
	routingTableDestroy(routing_table);
	mark_point();
	return 0;
}


/// Initialize if system hasn't been yet. Add a reference
int32_t payloadRoutingAddReference(void) {
	if (++payload_ref_count > 1)
		return 0;
	return payloadRoutingInit();
}


/// Free resources if no more references exist
int32_t payloadRoutingRemoveReference(void) {
	if (--payload_ref_count > 0)
		return 0;
	return payloadRoutingDestroy();
}


/// Spin off threads
int32_t payloadStateInit(void) {
	// initialize routing table, dispatch,
	// and file descriptors, and threads for
	// tests

	mark_point();
	pthread_create(&serial_reader_thr, NULL, &spiPayloadReader, NULL);
	mark_point();
	pthread_create(&tcp_reader_thr, NULL, &tcpPayloadReader, NULL);
	mark_point();
	
	return 0;
}


/// Join threads back in, do some tests 
int32_t payloadStateCommit(SansgridSerial **sg_serial_read, int packets) {
	// Close writing file descriptors, join threads, remove pipes
	void *arg;
	mark_point();

	// Finish reading
	pthread_join(serial_reader_thr, &arg);
	mark_point();
	pthread_join(tcp_reader_thr, &arg);

	mark_point();
	
	// Test current state
	fail_if((queueSize(dispatch) != packets), "Queue Size mismatch: Expected %i, Got %i", packets, queueSize(dispatch));
	if (packets > 0) {
		if (queueTryDequeue(dispatch, (void**)sg_serial_read) == -1)
			fail("Dispatch Failure");
		fail_if((queueSize(dispatch) > (packets-1)), "Too much data went on the dispatch: %i", queueSize(dispatch));

		fail_if((sg_serial_read == NULL), "payload lost");
	} else {
		*sg_serial_read = NULL;
	}

	mark_point();
	return 0;
}



// vim: ft=c ts=4 noet sw=4:

