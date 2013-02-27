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
 * Has multiple writers, one heartbeat, and one reader.
 * All of these atomically access the queue.
 *
 * This is ultimately a test of a possible solution to having
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


#define _POSIX_C_SOURCE 200809L		// Required for nanosleep()

#include <stdio.h>					// I/O operations
#include <stdlib.h>					// malloc(), free()
#include <stdint.h>					// Exact-Width Integers
#include <pthread.h>				// POSIX threads
#include <time.h>					// nanosleep()

#include "../synchronous_queue/sync_queue.h"


#define NUM_DATA 		100			// Amount of data to send through
#define QUEUE_SIZE 		10			// number of entries in the queue
#define DATA_LENGTH 	32 			// Size of the raw data used (in bytes)



int sleepMicro(uint32_t usecs) {
	// Sleep a specified number of microseconds

	int excode;
	struct timespec required, remaining;

	// Set sleep time in microseconds
	required.tv_sec = 0;
	required.tv_nsec = 1000L*usecs;

	do {
		// sleep the full amount of time
		excode = nanosleep(&required, &remaining);
		required.tv_nsec = remaining.tv_nsec;
	} while (excode);

	return 0;
}



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
		printf("W");

		// queue data onto the queue
		if ((excode = queueTryEnqueue(queue, data)) == -1) {
			printf("F\n");
			excode = queueEnqueue(queue, data);
		}

		// try to yield if more than 3/4 of the queue is full
		if (queueSize(queue) > ((QUEUE_SIZE * 3) / 4))
			sched_yield();
	}

	pthread_exit(arg);
}



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

	queue = (Queue*)arg;

	for (i=0;; i++) {
		sleepMicro(USLEEP_TIME);

		// Critical Action: Don't cancel the thread here
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldtype);
		datastring = (char*)malloc(DATA_LENGTH*sizeof(char));
		sprintf(datastring, "H %i", i);
		data = (uint8_t*)datastring;
		excode = queueEnqueue(queue, data);
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldtype);

	}

	pthread_exit(arg);
}



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
		if (queueTryDequeue(queue, &data) == -1) {
			// queue is empty. Block until data is available
			printf("E\n");
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldtype);
			queueDequeue(queue, &data);
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldtype);
		}

		if (data[0] == 'H') {
			printf(" %s ", data);
		} else {
			printf("R");
		}

		free(data);
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldtype);
	}

	pthread_exit(arg);
}



int main(void) {
	// Main function
	
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


	if (sizeof(char) != sizeof(uint8_t)) {
		// We do direct conversions from uint8_t to char.
		// If these aren't the same size, just stop right here.
		printf("ERROR: sizeof(char) != sizeof(uint8_t)\n");
		exit(EXIT_FAILURE);
	}

	// Create the queue
	queue = queueInit(QUEUE_SIZE);
	if (!queue) {
		printf("ERROR: Invalid queue size!\n");
		exit(EXIT_FAILURE);
	}

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
	
	return EXIT_SUCCESS;
}



// vim: ft=c ts=4 noet sw=4:
