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

#include "payload-tests.h"
TalkStub *ts_serial,
		 *ts_tcp;
pthread_t serial_reader_thr,
  		  tcp_reader_thr;
static FILE *FPTR_SPI_WRITER,
	 		*FPTR_SPI_READER,
	 		*FPTR_TCP_WRITER,
	 		*FPTR_TCP_READER;


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
	talkStubSetReader(ts_serial, FPTR_SPI_READER);

	if ((excode = sgSerialReceive(&sg_serial, &packet_size)) == -1)
		fail("Failed to read packet");
	else if (excode == 0) {
		if (queueEnqueue(dispatch, sg_serial) == -1)
			fail("Dispatch Enqueue Failure (Serial)");
#if TESTS_DEBUG_LEVEL > 0
		printf("(Serial) Queued\n");
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
	talkStubSetReader(ts_tcp, FPTR_TCP_READER);

	if ((excode = sgTCPReceive(&sg_serial, &packet_size)) == -1)
		fail("Failed to read TCP packet");
	else if (excode == 0) {
		if (queueEnqueue(dispatch, sg_serial) == -1)
			fail("Dispatch Enqueue Failure (TCP)");
#if TESTS_DEBUG_LEVEL > 0
		printf("(TCP) Queued\n");
#endif
	}

	fclose(FPTR_TCP_READER);
	pthread_exit(arg);
}


int32_t payloadRoutingInit(void) {
	uint8_t base[IP_SIZE];

	ts_serial = talkStubUseSerial(1);
	ts_tcp = talkStubUseTCP(1);
	dispatch = queueInit(200);
	fail_if((dispatch == NULL), "Error: dispatch is not initialized!");
	for (int i=0; i<IP_SIZE; i++)
		base[i] = 0x0;
	routing_table = routingTableInit(base);
	fail_if((routing_table == NULL), "Error: routing table is not initialized!");

	return 0;
}


int32_t payloadStateInit(void) {
	// initialize routing table, dispatch,
	// and file descriptors, and threads for
	// tests

	struct stat buffer;

	if (stat("spi.fifo", &buffer) < 0)
		mkfifo("spi.fifo", 0644);
	if (stat("tcp.fifo", &buffer) < 0)
		mkfifo("tcp.fifo", 0644);



	pthread_create(&serial_reader_thr, NULL, &spiPayloadReader, NULL);
	pthread_create(&tcp_reader_thr, NULL, &tcpPayloadReader, NULL);
	
	// Initialize Pipes
	if (!(FPTR_SPI_WRITER = fopen("spi.fifo", "w"))) {
		fail("Error: Can't open serial pipe for writing!");
	}
	if (!(FPTR_TCP_WRITER = fopen("tcp.fifo", "w"))) {
		fail("Error: Can't open TCP pipe for writing!");
	}
	talkStubSetWriter(ts_serial, FPTR_SPI_WRITER);
	talkStubSetWriter(ts_tcp, FPTR_TCP_WRITER);

	return 0;
}



int32_t payloadStateCommit(SansgridSerial **sg_serial_read) {
	// Close writing file descriptors, join threads, remove pipes
	void *arg;
	fclose(FPTR_SPI_WRITER);
	fclose(FPTR_TCP_WRITER);


	// Finish reading
	pthread_join(serial_reader_thr, &arg);
	pthread_join(tcp_reader_thr, &arg);

	
	unlink("spi.fifo");
	unlink("tcp.fifo");


	// Test current state
	fail_if((queueSize(dispatch) == 0), "No data on the dispatch!");
	if (queueDequeue(dispatch, (void**)sg_serial_read) == -1)
		fail("Dispatch Failure");
	fail_if((queueSize(dispatch) > 0), "Too much data went on the dispatch");

	fail_if((sg_serial_read == NULL), "payload lost");

	return 0;
}





// vim: ft=c ts=4 noet sw=4:

