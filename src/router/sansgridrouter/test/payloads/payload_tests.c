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

#include "payload_tests.h"
pthread_t serial_reader_thr,
  		  tcp_reader_thr;
#ifndef SG_TEST_USE_EEPROM
static FILE *FPTR_SPI_WRITER,
	 		*FPTR_SPI_READER,
	 		*FPTR_TCP_WRITER,
	 		*FPTR_TCP_READER;
#endif
static int payload_ref_count = 0;


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
	TalkStub *ts_serial = talkStubGetSPI();
	mark_point();

#ifdef SG_TEST_USE_EEPROM
	talkStubSetEEPROMAddress(ts_serial, 0x0000);
#else
	mark_point();
	if (!(FPTR_SPI_READER = fopen("spi.fifo", "r"))) {
		fail("Can't open serial pipe for reading!");
	}
	mark_point();
	talkStubSetReader(ts_serial, FPTR_SPI_READER);
	mark_point();
#endif

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

#ifndef SG_TEST_USE_EEPROM
	talkStubCloseReader(ts_serial);
#endif
	mark_point();
	pthread_exit(arg);

}


void *tcpPayloadReader(void *arg) {
	// Reads from a TCP connection
	SansgridSerial *sg_serial = NULL;
	uint32_t packet_size;
	int excode;
	TalkStub *ts_tcp = talkStubGetTCP();

	mark_point();
#ifdef SG_TEST_USE_EEPROM
	talkStubSetEEPROMAddress(ts_tcp, 0x0080);
#else
	if (!(FPTR_TCP_READER = fopen("tcp.fifo", "r"))) {
		fail("Can't open TCP pipe for reading!");
	}
	talkStubSetReader(ts_tcp, FPTR_TCP_READER);
#endif

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

#ifndef SG_TEST_USE_EEPROM
	talkStubCloseReader(ts_tcp);
#endif
	mark_point();
	pthread_exit(arg);
}


int32_t payloadRoutingInit(void) {
	uint8_t base[IP_SIZE];

	mark_point();

	dispatch = queueInit(200);
	fail_if((dispatch == NULL), "Error: dispatch is not initialized!");
	for (int i=0; i<IP_SIZE; i++)
		base[i] = 0x0;
	mark_point();
	routing_table = routingTableInit(base);
	fail_if((routing_table == NULL), "Error: routing table is not initialized!");
	mark_point();

	return 0;
}


int32_t payloadRoutingDestroy(void) {
	mark_point();
	queueDestroy(dispatch);
	routingTableDestroy(routing_table);
	mark_point();
	return 0;
}


int32_t payloadRoutingAddReference(void) {
	if (++payload_ref_count > 1)
		return 0;
	return payloadRoutingInit();
}


int32_t payloadRoutingRemoveReference(void) {
	if (--payload_ref_count > 0)
		return 0;
	return payloadRoutingDestroy();
}

int32_t payloadStateInit(void) {
	// initialize routing table, dispatch,
	// and file descriptors, and threads for
	// tests

	TalkStub *ts_serial = talkStubGetSPI(),
			 *ts_tcp = talkStubGetTCP();

#ifdef SG_TEST_USE_EEPROM
#else
	struct stat buffer;

	if (stat("spi.fifo", &buffer) < 0)
		mkfifo("spi.fifo", 0644);
	if (stat("tcp.fifo", &buffer) < 0)
		mkfifo("tcp.fifo", 0644);
#endif



	mark_point();
	pthread_create(&serial_reader_thr, NULL, &spiPayloadReader, NULL);
	mark_point();
	pthread_create(&tcp_reader_thr, NULL, &tcpPayloadReader, NULL);
	mark_point();
	
#ifdef SG_TEST_USE_EEPROM
	talkStubSetEEPROMAddress(ts_serial, 0x0000);
	talkStubSetEEPROMAddress(ts_tcp, 0x0080);
#else
	// Initialize Pipes
	mark_point();
	if (!(FPTR_SPI_WRITER = fopen("spi.fifo", "w"))) {
		fail("Error: Can't open serial pipe for writing!");
	}

	mark_point();
	if (!(FPTR_TCP_WRITER = fopen("tcp.fifo", "w"))) {
		fail("Error: Can't open TCP pipe for writing!");
	}

	mark_point();
	talkStubSetWriter(ts_serial, FPTR_SPI_WRITER);
	mark_point();
	talkStubSetWriter(ts_tcp, FPTR_TCP_WRITER);
#endif

	mark_point();
	return 0;
}



int32_t payloadStateCommit(SansgridSerial **sg_serial_read, int packets) {
	// Close writing file descriptors, join threads, remove pipes
	void *arg;
	mark_point();
	TalkStub *ts_serial = talkStubGetSPI(),
			 *ts_tcp = talkStubGetTCP();
#ifdef SG_TEST_USE_EEPROM
#else
	talkStubCloseWriter(ts_serial);
	talkStubCloseWriter(ts_tcp);
#endif

	mark_point();

	// Finish reading
	pthread_join(serial_reader_thr, &arg);
	mark_point();
	pthread_join(tcp_reader_thr, &arg);

	mark_point();
	
#ifdef SG_TEST_USE_EEPROM
#else
	unlink("spi.fifo");
	unlink("tcp.fifo");
#endif

	mark_point();
	// Test current state
	fail_if((queueSize(dispatch) != packets), "Queue Size mismatch: Expected %i, Got %i", packets, queueSize(dispatch));
	if (packets > 0) {
		if (queueTryDequeue(dispatch, (void**)sg_serial_read) == -1)
			fail("Dispatch Failure");
		fail_if((queueSize(dispatch) > (packets-1)), "Too much data went on the dispatch: %i", queueSize(dispatch));

		fail_if((sg_serial_read == NULL), "payload lost");
	}

	mark_point();
	return 0;
}





// vim: ft=c ts=4 noet sw=4:

