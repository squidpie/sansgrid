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
 * This implementation defines the table size at compile-time instead of at run-time,
 * making the system less prone to bugs. The table is an array of pointers, which
 * facilitates creating/moving/deleting IP addresses.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <check.h>

#ifndef __H_RADIO_STUB__
#define __H_RADIO_STUB__
#include "stubs/radio-stub.h"
#endif

#include "../routing/routing.h"
#include "../synchronous_queue/sync_queue.h"
#include "tests.h"


void *routingTableRuntime(void *arg) {

	Queue *queue = (Queue*)arg;
	uint8_t *serial_data;
	int oldstate;

	while (1) {
		queueDequeue(queue, &serial_data);
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
		free(serial_data);
		//printf("Data\n");
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
	}

	pthread_exit(arg);
}


void *spiReader(void *arg) {
	// reads from a named pipe and sends info to the dispatch queue
	int timeout = 0;
	Queue *queue = (Queue*)arg;
	FILE *FPTR;
	uint8_t *serial_data;
	char filedata[101];
	int oldstate;

	if (!(FPTR = fopen("test/rstubin.fifo", "r"))) {
		fail("Error: Cannot open rstubin.fifo!");
		//exit(EXIT_FAILURE);
	}

	while (1) {
		// Read from the pipe forever
		while (fgets(filedata, 100*sizeof(char), FPTR) == NULL) {
			timeout++;
			if (timeout > 10000)
				pthread_exit(arg);
			sched_yield();
		}
		timeout = 0;
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
		serial_data = (uint8_t*)malloc(101*sizeof(uint8_t));
		memcpy(serial_data, filedata, 100*sizeof(char));
		//printf("Queuing\n");
		queueEnqueue(queue, serial_data);
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
	}

	pthread_exit(arg);
}
	

START_TEST (testAdvancedDispatch) {
	// unit test code for more dispatch testing
	pid_t chpid;
	Queue *queue;
	pthread_t rtable_thread,
			  spi_thread;
	int fildes[2];
	char lptr[5];
	void *arg;

	pipe(fildes);

	chpid = fork();

	fail_unless((chpid >= 0), "Error: Fork Failed!");
	if (chpid == 0) {
		// child
		radioStubRuntime(fildes);
		exit(EXIT_SUCCESS);
	}

	// parent

	queue = queueInit(200);
	fail_unless((queue != NULL), "Error: Queue not allocated!");

	pthread_create(&rtable_thread, NULL, &routingTableRuntime, queue);
	pthread_create(&spi_thread, NULL, &spiReader, queue);

	
	// wait until radio stub is done writing
	close(fildes[1]);
	read(fildes[0], lptr, 5);
	close(fildes[0]);
	while (queueSize(queue) > 0)
		sched_yield();


	pthread_join(spi_thread, &arg);
	pthread_cancel(rtable_thread);
	pthread_join(rtable_thread, &arg);

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
