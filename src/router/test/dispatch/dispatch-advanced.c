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
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <check.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../../../sg_serial.h"
#include "../../../payloads.h"

#include "../../dispatch/dispatch.h"
#include "../tests.h"

void payloadMkSerial(SansgridSerial *sg_serial);

// Setup fifo reading/writing
void sgSerialTestSetReader(FILE *FPTR);
void sgSerialTestSetWriter(FILE *FPTR);

void *routingTableRuntime(void *arg) {
	// Dispatch read/execute
	Queue *queue = (Queue*)arg;
	int oldstate;
	SansgridSerial *sg_serial;
#if TESTS_DEBUG_LEVEL > 0
	SansgridFly *sg_fly;
	SANSGRID_UNION(SansgridFly, SGFU) sg_fly_union;
	int numpackets = 0;
#endif

	while (1) {
		queueDequeue(queue, (void**)&sg_serial);
		fail_if((sg_serial == NULL), "serial_data not initialized!");
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
#if TESTS_DEBUG_LEVEL > 0
		sg_fly_union.serialdata = sg_serial->payload;
		sg_fly = sg_fly_union.formdata;
		printf("Packet %i:\t0x%x\t%s\n", numpackets++, sg_fly->datatype, sg_fly->network_name);
#endif
		free(sg_serial);
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
	}

	pthread_exit(arg);
}


void *spiReader(void *arg) {
	// reads from a serial connection and enqueues data
	int i;
	SansgridSerial *sg_serial;
	uint32_t packet_size;
	Queue *queue = (Queue*)arg;
	int oldstate;
	FILE *FPTR;
	uint32_t excode;

	if (!(FPTR = fopen("rstubin.fifo", "r"))) {
		fail("Can't open fifo for reading");
	}
	sgSerialTestSetReader(FPTR);

	for (i=0; i<10; i++) {
		
		// Read from serial
		if ((excode = sgSerialReceive(&sg_serial, &packet_size)) == -1)
			fail("Failed to read packet");
		else if (excode == 0) {
			// Enqueue
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
			queueEnqueue(queue, sg_serial);
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
		}
	}
	fclose(FPTR);

	pthread_exit(arg);
}

void *spiWriter(void *arg) {
	// Writes to a serial connection

	int i;
	SansgridFly sg_fly;
	SansgridSerial sg_serial;
	FILE *FPTR = NULL;

	payloadMkSerial(&sg_serial);
	sg_fly.datatype = SG_FLY;
	snprintf(sg_fly.network_name, 78, "Ping");
	memcpy(&sg_serial.payload, &sg_fly, sizeof(SansgridFly));


	if (!(FPTR = fopen("rstubin.fifo", "w"))) {
		fail("Can't open fifo for writing");
	}
	sgSerialTestSetWriter(FPTR);

	for (i=0; i<10; i++) {
		// write ping 10 times, then signal exiting using 
		// the pipe
		
		if (sgSerialSend(&sg_serial, sizeof(SansgridSerial)) == -1)
			fail("Failed to send packet");
	}

	//printf("Ping Start\n");
	fclose(FPTR);
	//printf("Next ping\n");

	pthread_exit(arg);
}
	


START_TEST (testAdvancedDispatch) {
	// unit test code for more dispatch testing
	Queue *queue;
	pthread_t spi_read_thread,
			  spi_write_thread,
			  routing_table_thread;
	void *arg;


	struct stat buffer;
	// parent

	queue = queueInit(200);
	fail_unless((queue != NULL), "Error: Queue not allocated!");

	if (stat("rstubin.fifo", &buffer) < 0)
		mkfifo("rstubin.fifo", 0644);

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
