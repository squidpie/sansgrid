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
#include "payload-stub-handlers.h"

// Setup fifo reading/writing
void sgSerialTestSetReader(FILE *FPTR);
void sgSerialTestSetWriter(FILE *FPTR);
void sgTCPTestSetReader(FILE *FPTR);
void sgTCPTestSetWriter(FILE *FPTR);

static Queue *dispatch;
static RoutingTable *routing_table;
static FILE *FPTR_SPI_WRITER,
			*FPTR_SPI_READER,
			*FPTR_TCP_WRITER,
			*FPTR_TCP_READER;
static pthread_t serial_reader_thr,
				 tcp_reader_thr;

void checkSize(const char *pkname, size_t pksize) {
	fail_unless((pksize == PAYLOAD_SIZE), 
			"\n%s is wrong size: \
			\n\tExpected: %i\
			\n\tGot: %i", pkname, PAYLOAD_SIZE, pksize);
}



void *spiPayloadReader(void *arg) {
	// Reads from a serial connection
	SansgridSerial *sg_serial = NULL;
	uint32_t packet_size;
	int excode;

	if (!(FPTR_SPI_READER = fopen("spi.fifo", "r"))) {
		fail("Can't open serial pipe for reading!");
	}
	sgSerialTestSetReader(FPTR_SPI_READER);

	if ((excode = sgSerialReceive(&sg_serial, &packet_size)) == -1)
		fail("Failed to read packet");
	else if (excode == 0) {
#if TESTS_DEBUG_LEVEL > 0
		printf("Origin IP: ");
		routingTablePrint(sg_serial->origin_ip);
		if (queueEnqueue(dispatch, sg_serial) == -1)
			fail("Dispatch Enqueue Failure (Serial)");
#endif
	}

	fclose(FPTR_SPI_READER);
	pthread_exit(arg);

}


void *tcpPayloadReader(void *arg) {
	// Reads from a TCP connection
	SansgridSerial *sg_serial = NULL;
	uint32_t packet_size;
	int excode;

	if (!(FPTR_TCP_READER = fopen("tcp.fifo", "r"))) {
		fail("Can't open TCP pipe for reading!");
	}
	sgTCPTestSetReader(FPTR_TCP_READER);

	if ((excode = sgTCPReceive(&sg_serial, &packet_size)) == -1)
		fail("Failed to read TCP packet");
	else if (excode == 0) {
#if TESTS_DEBUG_LEVEL > 0
		//printf("Address: %p\n", sg_serial);
		printf("Origin IP: ");
		routingTablePrint(sg_serial->origin_ip);
#endif
		if (queueEnqueue(dispatch, sg_serial) == -1)
			fail("Dispatch Enqueue Failure (TCP)");
	}

	fclose(FPTR_TCP_READER);
	pthread_exit(arg);
}



static int32_t payloadStateInit(void) {
	// initialize routing table, dispatch,
	// and file descriptors, and threads for
	// tests
	uint8_t base[IP_SIZE];

	struct stat buffer;

	if (stat("spi.fifo", &buffer) < 0)
		mkfifo("spi.fifo", 0644);
	if (stat("tcp.fifo", &buffer) < 0)
		mkfifo("tcp.fifo", 0644);

	for (int i=0; i<IP_SIZE; i++)
		base[i] = 0x0;

	dispatch = queueInit(200);
	fail_if((dispatch == NULL), "Error: dispatch is not initialized!");
	routing_table = routingTableInit(base);
	fail_if((routing_table == NULL), "Error: routing table is not initialized!");

	pthread_create(&serial_reader_thr, NULL, &spiPayloadReader, NULL);
	pthread_create(&tcp_reader_thr, NULL, &tcpPayloadReader, NULL);
	
	// Initialize Pipes
	if (!(FPTR_SPI_WRITER = fopen("spi.fifo", "w"))) {
		fail("Error: Can't open serial pipe for writing!");
	}
	if (!(FPTR_TCP_WRITER = fopen("tcp.fifo", "w"))) {
		fail("Error: Can't open TCP pipe for writing!");
	}
	sgSerialTestSetWriter(FPTR_SPI_WRITER);
	sgTCPTestSetWriter(FPTR_TCP_WRITER);

	return 0;
}

static int32_t payloadStateCommit(void) {
	// Close writing file descriptors, join threads, remove pipes
	void *arg;
	fclose(FPTR_SPI_WRITER);
	fclose(FPTR_TCP_WRITER);

	// Finish reading
	pthread_join(serial_reader_thr, &arg);
	pthread_join(tcp_reader_thr, &arg);
	
	unlink("spi.fifo");
	unlink("tcp.fifo");
	return 0;
}


START_TEST (testPayloadSize) {
	// unit test code for making sure all payloads are the same size
	checkSize("SansgridHatching", sizeof(SansgridHatching));
	checkSize("SansgridFly", sizeof(SansgridFly));
	checkSize("SansgridEyeball", sizeof(SansgridEyeball));
	checkSize("SansgridPeck", sizeof(SansgridPeck));
	checkSize("SansgridSing", sizeof(SansgridSing));
	checkSize("SansgridMock", sizeof(SansgridMock));
	checkSize("SansgridPeacock", sizeof(SansgridPeacock));
	checkSize("SansgridNest", sizeof(SansgridNest));
	checkSize("SansgridSquawk", sizeof(SansgridSquawk));
	checkSize("SansgridHeartbeat", sizeof(SansgridHeartbeat));
	checkSize("SansgridChirp", sizeof(SansgridChirp));
}
END_TEST


START_TEST (testEyeball) {
	// unit test code to test the Eyeball data type
	SansgridEyeball sg_eyeball;
	SansgridSerial sg_serial;
	SansgridSerial *sg_serial_read;


	// initialize
	payloadStateInit();

	// Make packet
	payloadMkSerial(&sg_serial);
	payloadMkEyeball(&sg_eyeball, SG_EYEBALL_MATE);
	memcpy(&sg_serial.payload, &sg_eyeball, sizeof(SansgridEyeball));

	// Call handler
	routerHandleEyeball(routing_table, &sg_serial);

	// Finish up with pipes
	payloadStateCommit();

	// Run test on current state
	if (queueDequeue(dispatch, (void**)&sg_serial_read) == -1)
		fail("Dispatch Failure");

	fail_if((sg_serial_read == NULL), "payload lost");
#if TESTS_DEBUG_LEVEL > 0
	printf("Address: %p\n", sg_serial_read);
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
}
END_TEST



Suite *payloadTesting (void) {
	Suite *s = suite_create("Payload Tests");
	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, testPayloadSize);
	tcase_add_test(tc_core, testEyeball);

	suite_add_tcase(s, tc_core);

	return s;
}


// vim: ft=c ts=4 noet sw=4:
