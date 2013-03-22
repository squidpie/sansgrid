/* A thread-safe queue implementation 
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
 * The dynamic allocation that this uses allows:
 * 		- Dynamic resizing at runtime
 * 		- Variable-length entries in the queue
 *
 * TODO: change all normal locks to timed locks
 *
 * Link with -lpthread
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <errno.h>

#include "dispatch.h"



struct Queue {
	uint8_t **list;					// actual data storage
	uint32_t size;					// number of indeces in the list
	// Head/Tail points
	uint32_t queue_index_end;		// where data is added
	uint32_t queue_index_start;		// where data is taken
	// Synchronization Primitives
	pthread_mutex_t queue_lock;		// make all queue access atomic
	sem_t queue_empty_lock;			// lock that triggers on empty queue
	sem_t queue_full_lock;			// lock that triggers on full queue
};

static void modInc(uint32_t *a, uint32_t m) {
	// increment and take the modulo.
	*a = (*a+1) % m;

	return;
}


static int enqueue(Queue *queue, uint8_t *serial_data, int (*sem_fn)(sem_t*)) {
	// put a piece of data onto the queue
	// Use the semaphore function supplied to determine the action
	// 		if the queue is full

	if (sem_fn(&queue->queue_full_lock) == -1)
		return -1;

	pthread_mutex_lock(&queue->queue_lock);		// get atomic access to queue

	queue->list[queue->queue_index_end] = serial_data;
	modInc(&queue->queue_index_end, queue->size);

	sem_post(&queue->queue_empty_lock);			// signal more data is available
	pthread_mutex_unlock(&queue->queue_lock);

	return 0;
}



static int dequeue(Queue *queue, uint8_t **serial_data, int (*sem_fn)(sem_t*)) {
	// take a piece of data off the queue

	if (sem_fn(&queue->queue_empty_lock) == -1) 
		return -1;

	pthread_mutex_lock(&queue->queue_lock);		// get atomic access to queue

	*serial_data = queue->list[queue->queue_index_start];
	modInc(&queue->queue_index_start, queue->size);

	sem_post(&queue->queue_full_lock);			// signal more space is available
	pthread_mutex_unlock(&queue->queue_lock);

	return 0;
}


/* Visible Functions */

Queue *queueInit(uint32_t size) {
	// Create a queue with size elements

	Queue *queue;

	if (size < 2)
		return NULL;		// not enough size will be allocated for the queue to work

	queue = (Queue*)malloc(sizeof(Queue));
	if (!queue) {
		// TODO: Log an error here
		return NULL;		// allocation error
	}

	queue->size = size;

	pthread_mutex_init(&queue->queue_lock, NULL);
	sem_init(&queue->queue_empty_lock, 0, 0);
	sem_init(&queue->queue_full_lock, 0, queue->size-1);

	queue->list = (uint8_t**)malloc(queue->size*sizeof(uint8_t*));
	if (!queue->list) {
		// TODO: Log an error here
		return NULL;
	}

	queue->queue_index_end = 0;
	queue->queue_index_start = 0;

	return queue;
}



Queue *queueDestroy(Queue *queue) {
	// Free up a queue

	free(queue->list);
	pthread_mutex_destroy(&queue->queue_lock);
	sem_destroy(&queue->queue_empty_lock);
	sem_destroy(&queue->queue_full_lock);
	free(queue);

	return NULL;
}



int queueSize(Queue *queue) {
	// Get the number of elements in the queue

	int lower_bound = queue->queue_index_start, 
		upper_bound = queue->queue_index_end;

	if (upper_bound < lower_bound)
		upper_bound += queue->size;

	return (upper_bound - lower_bound);
}



int queueTryEnqueue(Queue *queue, uint8_t *serial_data) {
	// try to put data onto the queue
	// If the queue is full, return an error

	return enqueue(queue, serial_data, &sem_trywait);
}



int queueEnqueue(Queue *queue, uint8_t *serial_data) {
	// Put data onto the queue
	// If the queue is full, block until space is available

	return enqueue(queue, serial_data, &sem_wait);
}



int queueTryDequeue(Queue *queue, uint8_t **serial_data) {
	// Take data off the queue.
	// If the queue is empty, return an error

	return dequeue(queue, serial_data, &sem_trywait);
}



int queueDequeue(Queue *queue, uint8_t **serial_data) {
	// Take data off the queue.
	// If the queue is empty, block until data is available

	return dequeue(queue, serial_data, &sem_wait);
}



// vim: ft=c ts=4 noet sw=4:
