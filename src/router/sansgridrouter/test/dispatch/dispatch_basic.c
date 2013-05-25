/* Test of the atomic queue.
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
 *
 * A test to make sure the queue really is atomic.
 *
 * Has multiple writers, one heartbeat, and one reader.
 * All of these atomically access the queue.
 *
 * This is ultimately a test of a solution to having
 * 2 inputs (one from apache, one from SPI)
 * 2 outputs (one to apache, one to SPI)
 * with a heartbeat going out over SPI periodically.
 *
 * We have two "listening" threads (the writers),
 * A heartbeat thread (heartbeat),
 * and a dispatch thread (reader).
 *
 * Link with -lpthread
 */

#define _POSIX_C_SOURCE 200809L
/// \file


#include <stdio.h>					// I/O operations
#include <stdlib.h>					// malloc(), free()
#include <stdint.h>					// Exact-Width Integers
#include <pthread.h>				// POSIX threads
#include <check.h>

#include "../../dispatch/dispatch.h"
#include "../../routing_table/heartbeat.h"
#include "../tests.h"


#define NUM_DATA 		100			///< Amount of data to send through
#define QUEUE_SIZE 		10			///< number of entries in the queue
#define DATA_LENGTH 	32 			///< Size of the raw data used (in bytes)




/// Thread that writes to the queue
static void *writerFunc(void *arg) {
	// thread that writes to the queue
	int i;
	uint8_t *data;
	char *datastring;
	int excode;
	Queue *queue;

	queue = (Queue*)arg;

	for (i=0; i<NUM_DATA; i++) {
		datastring = (char*)malloc(DATA_LENGTH*sizeof(char));
		sprintf(datastring, "%i", i);
		data = (uint8_t*)datastring;
#if TESTS_DEBUG_LEVEL > 0
		printf("W");
#endif

		// queue data onto the queue
		if ((excode = queueTryEnqueue(queue, data)) == -1) {
#if TESTS_DEBUG_LEVEL > 0
			printf("F\n");
#endif
			excode = queueEnqueue(queue, data);
		}

		// try to yield if more than 3/4 of the queue is full
		if (queueSize(queue) > ((QUEUE_SIZE * 3) / 4))
			sched_yield();
	}

	pthread_exit(arg);
}



/**
 * Thread that does a heartbeat every 1 msec
 */
static void *heartbeatFunc(void *arg) {
	// thread that does a heartbeat every 1 msec
	// This uses timediffs to calculate when to send
	// a heartbeat to a device.

	int i;
	uint8_t *data;
	char *datastring;
	int excode;			// queue status
	const uint32_t USLEEP_TIME = 1000;
	int oldtype;		// old thread cancellation state
	Queue *queue;
	struct timespec req, rem;

	queue = (Queue*)arg;

	for (i=0;; i++) {
		req.tv_sec = 0;
		req.tv_nsec = 1000L*USLEEP_TIME;
		nanosleep(&req, &rem);

		// Critical Action: Don't cancel the thread here
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldtype);
		datastring = (char*)malloc(DATA_LENGTH*sizeof(char));
		sprintf(datastring, "H %i", i);
		data = (uint8_t*)datastring;
		if ((excode = queueEnqueue(queue, data)) == -1)
			break;
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldtype);

	}

	pthread_exit(arg);
}



/**
 * Thread that reads the queue. Blocks until data is ready.
 * Doesn't exit on its own.
 */
static void *readerFunc(void *arg) {
	// Thread that reads the queue.
	// Blocks until data is ready
	// This does not exit on its own;
	// pthread_cancel() must be called for now
	
	int i;
	uint8_t *data;
	Queue *queue = (Queue*)arg;
	int oldtype;

	for (i=0;; i++) {
		// Read indefinitely
		// When we're using non-blocking functions, make sure we aren't
		// 	cancelled
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldtype);
		if (queueTryDequeue(queue, (void**)&data) == -1) {
			// queue is empty. Block until data is available
#if TESTS_DEBUG_LEVEL > 0
			printf("E\n");
#endif
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldtype);
			queueDequeue(queue, (void**)&data);
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldtype);
		}

#if TESTS_DEBUG_LEVEL > 0
		if (data[0] == 'H') {
			printf(" %s ", data);
		} else {
			printf("R");
		}
#endif

		free(data);
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldtype);
	}

	pthread_exit(arg);
}



/**
 * Basic Dispatch testing with few threads
 */
START_TEST (testDispatchBasic) {
	// Simple tests for the queue
	int i;
	Queue *queue;
	uint8_t *in_data, *out_data;
	int excode;

	queue = queueInit(QUEUE_SIZE);
	fail_unless((queue != NULL), "Error: Queue not initialized!");

	for (i=0; i<QUEUE_SIZE-1; i++) {
		// Write data to the queue
		out_data = (uint8_t*)malloc(1*sizeof(uint8_t));
		*out_data = (i & 0xFF);
		excode = queueTryEnqueue(queue, out_data);
		fail_unless((excode == 0), "Error: Queue Full!");
	}

	for (i=0; i<QUEUE_SIZE-1; i++) {
		// Read data from the queue
		excode = queueTryDequeue(queue, (void**)&in_data);
		fail_unless((excode == 0), "Error: No data to read!");
		fail_unless((in_data != NULL), "Error: invalid pointer!");
		fail_unless((*in_data == (i & 0xFF)), 
				"Error: Expected %i, got %i", (i & 0xFF), *in_data);
		free(in_data);
	}
	queueDestroy(queue);

}
END_TEST;


/**
 * Basic Dispatch testing with lots of threads
 */
START_TEST (testDispatchWithLotsOfThreads) {
	// Throw a lot of threads at the queue
	
	// Thread Variables
	pthread_t writer_thread_1,
			  writer_thread_2,
			  heartbeat_thread,
			  reader_thread;
	// Thread exit variables
	int arg = 0;
	void *arg_r = &arg;
	// Shared queue
	Queue *queue;


	// We do direct conversions from uint8_t to char.
	// If these aren't the same size, just stop right here.
	fail_unless((sizeof(char) == sizeof(uint8_t)),
			"ERROR: sizeof(char) != sizeof(uint8_t)");

	// Create the queue
	queue = queueInit(QUEUE_SIZE);
	fail_unless((queue != NULL), "ERROR: Invalid queue size!");

	// Create all threads
	pthread_create(&reader_thread, NULL, readerFunc, (void*)queue);
	pthread_create(&heartbeat_thread, NULL, heartbeatFunc, (void*)queue);
	pthread_create(&writer_thread_1, NULL, writerFunc, (void*)queue);
	pthread_create(&writer_thread_2, NULL, writerFunc, (void*)queue);

	// Wait for Writers to finish
	pthread_join(writer_thread_1, &arg_r);
	pthread_join(writer_thread_2, &arg_r);

	// Writers are done. Start killing other threads
	
	// kill the heartbeat
	pthread_cancel(heartbeat_thread);
	pthread_join(heartbeat_thread, &arg_r);
	// Wait for all data to be processed
	while (queueSize(queue) > 0);
	// All data processed. Kill the reader thread
	pthread_cancel(reader_thread);
	pthread_join(reader_thread, &arg_r);


	// Clean up
	queueDestroy(queue);
	
}
END_TEST;



/**
 * \brief Basic Dispatch Testing
 *
 * This does some basic dispatch testing, doing reading/writing
 * to a dispatch to test.
 */
Suite *dispatchBasicTesting (void) {
	Suite *s = suite_create("Basic Dispatch testing");

	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, testDispatchBasic);
	tcase_add_test(tc_core, testDispatchWithLotsOfThreads);

	suite_add_tcase(s, tc_core);

	return s;
}



// vim: ft=c ts=4 noet sw=4:
