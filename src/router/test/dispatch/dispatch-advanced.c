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
#include <check.h>

#ifndef __H_RADIO_STUB__
#define __H_RADIO_STUB__
#include "../stubs/radio-stub.h"
#include "../communication/sg-serial-test.c"
#include "../../../payloads.h"
#endif

#include "../../dispatch/dispatch.h"
#include "../tests.h"


void *routingTableRuntime(void *arg) {
	// Dispatch read/execute
	Queue *queue = (Queue*)arg;
	uint8_t *serial_data;
	SansgridGeneric sg_gen;
	SansgridFly sg_fly;
	int oldstate;

	while (1) {
		queueDequeue(queue, &serial_data);
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
		fail_if((serial_data == NULL), "serial_data not initialized!");
		memcpy(&sg_gen, serial_data, sizeof(SansgridGeneric));
		memcpy(&sg_fly, &sg_gen.serial_data, sizeof(SansgridFly));
		printf("%s\n", sg_fly.network_name);
		//free(serial_data);
		//printf("Data\n");
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
	}

	pthread_exit(arg);
}


void *spiReader(void *arg) {
	// reads from a serial connection and enqueues data
	SansgridGeneric *sg_gen = NULL;
	uint32_t packet_size;
	Queue *queue = (Queue*)arg;
	int oldstate;
	SANSGRID_UNION(SansgridGeneric, SGUn) sg_gen_union;


	while (1) {
		// Read from the pipe forever
		
		// Read from serial
		if (sgSerialReceive(&sg_gen, &packet_size) == -1)
			pthread_exit(arg);
		// Convert (for now) back to serial
		sg_gen_union.formdata = sg_gen;

		// Enqueue
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
		queueEnqueue(queue, sg_gen_union.serialdata);
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
	}

	pthread_exit(arg);
}

void *spiWriter(void *arg) {
	// Writes to a serial connection

	int i;
	SansgridGeneric *sg_gen;
	SansgridFly sg_fly;

	sg_fly.datatype = SG_FLY;
	snprintf(sg_fly.network_name, 78, "Ping");


	for (i=0; i<10; i++) {
		// write ping 10 times, then signal exiting using 
		// the unnamed pipe
		sg_gen = (SansgridGeneric*)malloc(sizeof(SansgridGeneric));
		memcpy(&sg_gen->serial_data, &sg_fly, sizeof(SansgridFly));
		
		if (sgSerialSend(sg_gen, sizeof(SansgridGeneric)) == -1)
			pthread_exit(arg);
	}

	pthread_exit(arg);
}
	


START_TEST (testAdvancedDispatch) {
	// unit test code for more dispatch testing
	Queue *queue;
	pthread_t spi_read_thread,
			  spi_write_thread,
			  routing_table_thread;
	void *arg;


	// parent

	queue = queueInit(200);
	fail_unless((queue != NULL), "Error: Queue not allocated!");

	pthread_create(&spi_read_thread, NULL, &spiReader, queue);
	pthread_create(&spi_write_thread, NULL, &spiWriter, queue);
	pthread_create(&routing_table_thread, NULL, &routingTableRuntime, queue);

	
	// wait until radio stub is done writing
	while (queueSize(queue) > 0)
		sched_yield();


	pthread_join(spi_write_thread, &arg);
	pthread_cancel(spi_read_thread);
	pthread_join(spi_read_thread, &arg);
	pthread_cancel(routing_table_thread);
	pthread_join(routing_table_thread, &arg);
	queueDestroy(queue);
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
