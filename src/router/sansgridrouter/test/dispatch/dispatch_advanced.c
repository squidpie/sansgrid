/* Advanced Dispatch Tests
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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <check.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <payloads.h>
#include <sgSerial.h>

#include "../communication/sg_communication_stubs.h"
#include "../../dispatch/dispatch.h"
#include "../tests.h"

/**
 * Arguments to be passed to a thread
 */
struct ThreadArgs {
	/// Stub Configuration structure
	TalkStub *ts_serial;
	/// ring buffer dispatch system
	Queue *queue;
};

void payloadMkSerial(SansgridSerial *sg_serial);


/**
 * Runtime for reading to / writing from a routing table
 */
static void *routingTableRuntime(void *arg) {
	// Dispatch read/execute
	int i;
	struct ThreadArgs *thr_args = (struct ThreadArgs*)arg;
	//int oldstate = 0;
	SansgridSerial *sg_serial = NULL;
#if TESTS_DEBUG_LEVEL > 0
	SansgridFly *sg_fly;
	SANSGRID_UNION(SansgridFly, SGFU) sg_fly_union;
	int numpackets = 0;
#endif

	for (i=0; i<10; i++) {
		if (queueDequeue(thr_args->queue, (void**)&sg_serial) == -1)
			fail("Dispatch Dequeue Failure");
		fail_if((sg_serial == NULL), "serial_data not initialized!");
		//pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
#if TESTS_DEBUG_LEVEL > 0
		sg_fly_union.serialdata = sg_serial->payload;
		sg_fly = sg_fly_union.formdata;
		printf("Packet %i:\t0x%x\t%s\n", numpackets++, sg_fly->datatype, sg_fly->network_name);
#endif
		free(sg_serial);
		//pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
	}

	pthread_exit(arg);
}


/**
 * SPI reading thread
 */
static void *spiReader(void *arg) {
	// reads from a serial connection and enqueues data
	int i;
	SansgridSerial *sg_serial = NULL;
	uint32_t packet_size = 0;
	struct ThreadArgs *thr_args = (struct ThreadArgs*)arg;
	//int oldstate;
	int8_t excode;


	for (i=0; i<10; i++) {
		
#if TESTS_DEBUG_LEVEL > 1
		printf("Reading %i\n", i);
#endif

		// Read from serial
		if ((excode = sgSerialReceive(&sg_serial, &packet_size)) == -1)
			fail("Failed to read packet");
		else if (sg_serial == NULL)
			fail("No data received!");
		else if (excode == 0) {
			// Enqueue
			if (queueEnqueue(thr_args->queue, sg_serial) == -1)
				fail("Queue Enqueue Failure");
		}
	}

#if TESTS_DEBUG_LEVEL > 1
	printf("Reader exiting\n");
#endif
	pthread_exit(arg);
}




/**
 * SPI writing thread
 */
static void *spiWriter(void *arg) {
	// Writes to a serial connection

	int i;
	SansgridFly sg_fly;
	SansgridSerial sg_serial;

	payloadMkSerial(&sg_serial);
	sg_fly.datatype = SG_FLY;



	for (i=0; i<10; i++) {
		// write ping 10 times, then signal exiting using 
		// the pipe
#if TESTS_DEBUG_LEVEL > 1
		printf("Writing %i\n", i);
#endif

		snprintf(sg_fly.network_name, 78, "Ping %i", i);
		memcpy(&sg_serial.payload, &sg_fly, sizeof(SansgridFly));
		if (sgSerialSend(&sg_serial, sizeof(SansgridSerial)) == -1)
			fail("Failed to send packet");
	}


#if TESTS_DEBUG_LEVEL > 1
	printf("Writer Exiting\n");
#endif
	pthread_exit(arg);
}
	


/**
 * Unit test for advanced dispatch testing
 */
START_TEST (testAdvancedDispatch) {
	// unit test code for more dispatch testing
	pthread_t spi_read_thread,
			  spi_write_thread,
			  routing_table_thread;
	void *arg;
	struct ThreadArgs thr_args  = {
		.ts_serial = talkStubInit("dispatch"),
		.queue = queueInit(200)
	};
	fail_unless((thr_args.queue != NULL), "Error: Queue not allocated!");


#ifndef SG_TEST_USE_EEPROM
	talkStubRegisterReadWriteFuncs(thr_args.ts_serial, COMM_TYPE_UNIX_PIPE);
#else
	talkStubRegisterReadWriteFuncs(thr_args.ts_serial, COMM_TYPE_EEPROM);
#endif
	talkStubUseAsSPI(thr_args.ts_serial);
	talkStubAssertValid(thr_args.ts_serial);

	pthread_create(&spi_read_thread, NULL, &spiReader, &thr_args);
	pthread_create(&spi_write_thread, NULL, &spiWriter, &thr_args);
	pthread_create(&routing_table_thread, NULL, &routingTableRuntime, &thr_args);

	


	pthread_join(spi_read_thread, &arg);
	pthread_join(spi_write_thread, &arg);
	while (queueSize(thr_args.queue) > 0)
		sched_yield();
	pthread_cancel(routing_table_thread);
	pthread_join(routing_table_thread, &arg);
	queueDestroy(thr_args.queue);

	talkStubDestroy(thr_args.ts_serial);
}
END_TEST;



/**
 * \brief Advanced Dispatch Testing
 *
 * This does more advanced dispatch testing, instantiating more of the system
 * than the basic dispatch testing.
 */
Suite *dispatchAdvancedTesting (void) {
	Suite *s = suite_create("Advanced Dispatch testing");

	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, testAdvancedDispatch);

	suite_add_tcase(s, tc_core);

	return s;
}


// vim: ft=c ts=4 noet sw=4:
