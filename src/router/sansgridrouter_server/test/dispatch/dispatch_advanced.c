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

#include <stdio.h>
//#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <check.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

#include "../../../../sg_serial.h"
#include "../../../../payloads.h"
#include "../communication/sg_communication_stubs.h"
#include "../../dispatch/dispatch.h"
#include "../tests.h"

TalkStub *ts_serial = NULL;

void payloadMkSerial(SansgridSerial *sg_serial);


static void *routingTableRuntime(void *arg) {
	// Dispatch read/execute
	int i;
	Queue *queue = (Queue*)arg;
	//int oldstate = 0;
	SansgridSerial *sg_serial = NULL;
#if TESTS_DEBUG_LEVEL > 0
	SansgridFly *sg_fly;
	SANSGRID_UNION(SansgridFly, SGFU) sg_fly_union;
	int numpackets = 0;
#endif

	for (i=0; i<10; i++) {
		if (queueDequeue(queue, (void**)&sg_serial) == -1)
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


static void *spiReader(void *arg) {
	// reads from a serial connection and enqueues data
	int i;
	SansgridSerial *sg_serial = NULL;
	uint32_t packet_size = 0;
	Queue *queue = (Queue*)arg;
	//int oldstate;
	FILE *FPTR;
	int8_t excode;


	for (i=0; i<10; i++) {
		
#if TESTS_DEBUG_LEVEL > 1
		printf("Reading %i\n", i);
#endif
		if (!(FPTR = fopen("rstubin.fifo", "r"))) {
			fail("Can't open fifo for reading");
		}
		talkStubSetReader(ts_serial, FPTR);
		// Read from serial
		if ((excode = sgSerialReceive(&sg_serial, &packet_size)) == -1)
			fail("Failed to read packet");
		else if (sg_serial == NULL)
			fail("No data received!");
		else if (excode == 0) {
			// Enqueue
			//pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
			if (queueEnqueue(queue, sg_serial) == -1)
				fail("Queue Enqueue Failure");
			//pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
		}
		if (fclose(FPTR) == EOF)
			fail("File Descriptor failed to close");
		talkStubSetReader(ts_serial, NULL);
	}

#if TESTS_DEBUG_LEVEL > 1
	printf("Reader exiting\n");
#endif
	pthread_exit(arg);
}

static void *spiWriter(void *arg) {
	// Writes to a serial connection

	int i;
	SansgridFly sg_fly;
	SansgridSerial sg_serial;
	FILE *FPTR = NULL;

	payloadMkSerial(&sg_serial);
	sg_fly.datatype = SG_FLY;



	for (i=0; i<10; i++) {
		// write ping 10 times, then signal exiting using 
		// the pipe
#if TESTS_DEBUG_LEVEL > 1
		printf("Writing %i\n", i);
#endif
		if (!(FPTR = fopen("rstubin.fifo", "w"))) {
			fail("Can't open fifo for writing");
		}
		talkStubSetWriter(ts_serial, FPTR);
		snprintf(sg_fly.network_name, 78, "Ping %i", i);
		memcpy(&sg_serial.payload, &sg_fly, sizeof(SansgridFly));
		if (sgSerialSend(&sg_serial, sizeof(SansgridSerial)) == -1)
			fail("Failed to send packet");
		if (fclose(FPTR) == EOF)
			fail("File Descriptor Failed to close");
		talkStubSetWriter(ts_serial, NULL);
	}


#if TESTS_DEBUG_LEVEL > 1
	printf("Writer Exiting\n");
#endif
	pthread_exit(arg);
}
	


START_TEST (testAdvancedDispatch) {
	// unit test code for more dispatch testing
	Queue *queue;
	pthread_t spi_read_thread,
			  spi_write_thread,
			  routing_table_thread;
	sem_t readlock, writelock;
	void *arg;
	ts_serial = talkStubUseSerial(1);


	struct stat buffer;
	// parent

	queue = queueInit(200);
	fail_unless((queue != NULL), "Error: Queue not allocated!");

	if (stat("rstubin.fifo", &buffer) < 0)
		mkfifo("rstubin.fifo", 0644);
	sem_init(&readlock, 0, 0);
	sem_init(&writelock, 0, 0);
	talkStubUseBarrier(ts_serial, 1);

	pthread_create(&spi_read_thread, NULL, &spiReader, queue);
	pthread_create(&spi_write_thread, NULL, &spiWriter, queue);
	pthread_create(&routing_table_thread, NULL, &routingTableRuntime, queue);

	


	pthread_join(spi_read_thread, &arg);
	pthread_join(spi_write_thread, &arg);
	while (queueSize(queue) > 0)
		sched_yield();
	pthread_cancel(routing_table_thread);
	pthread_join(routing_table_thread, &arg);
	queueDestroy(queue);
	sem_destroy(&readlock);
	sem_destroy(&writelock);

	talkStubUseBarrier(ts_serial, 0);


	unlink("rstubin.fifo");
}
END_TEST



Suite *dispatchAdvancedTesting (void) {
	Suite *s = suite_create("Advanced Dispatch testing");

	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, testAdvancedDispatch);

	suite_add_tcase(s, tc_core);

	return s;
}


// vim: ft=c ts=4 noet sw=4:
