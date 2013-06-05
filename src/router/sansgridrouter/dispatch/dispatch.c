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
 * Link with -lpthread
 */
/// \file

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <errno.h>

#include "dispatch.h"



/**
 * \brief An atomic ring buffer implementation
 *
 * The Queue is a ring buffer whose entries
 * are pointers to blocks of memory. \n
 * Note that queue and dequeue operations are atomic,
 * but the data stored on the queue is not protected.
 * Therefore data should be dynamically allocated before
 * being enqueued.
 */
struct Queue {
	/// Storage
	void **list;
	/// Number of indeces in the list
	uint32_t size;
	/// index that data is added at
	uint32_t queue_index_end;
	/// index that data is taken from
	uint32_t queue_index_start;
	// Synchronization Primitives
	/// Queue entry lock
	pthread_mutex_t queue_lock;		// make all queue access atomic
	/// Lock that blocks when the queue is empty
	sem_t queue_empty_lock;
	/// Lock that blocks when the queue is full
	sem_t queue_full_lock;
};


/// Increment a, set to 0 if a == m
static void modInc(uint32_t *a, uint32_t m) {
	// increment and take the modulo.
	*a = (*a+1) % m;

	return;
}


/**
 * \brief Atomically Enqueue data
 *
 * This is meant to be used by other functions to enqueue data.
 * The function pointed to by sem_fn is used for synchronization.
 * \param[in]	queue			The dispatch queue
 * \param[in]	serial_data		The data to enqueue
 * \param[in]	sem_fn			The synchronization function to use
 *
 * Note that data on the queue is not protected at all. 
 * serial_data should be data that has been allocated on the heap 
 * to prevent inadvertant modification of the data contained.
 *
 * \returns
 * On enqueue success, returns 0. \n
 * On failure, returns -1
 */
static int enqueue(Queue *queue, void *serial_data, int (*sem_fn)(sem_t*)) {
	// put a piece of data onto the queue
	// Use the semaphore function supplied to determine the action
	// 		if the queue is full

	if (sem_fn(&queue->queue_full_lock) == -1)
		return -1;

	pthread_mutex_lock(&queue->queue_lock);		// get atomic access to queue

	queue->list[queue->queue_index_end] = serial_data;
	//printf("Enqueue: %i: %p\n", queue->queue_index_end, serial_data);
	modInc(&queue->queue_index_end, queue->size);

	sem_post(&queue->queue_empty_lock);			// signal more data is available
	pthread_mutex_unlock(&queue->queue_lock);

	return 0;
}



/**
 * \brief Atomically Dequeue data
 *
 * This is meant to be used by other functions to dequeue data.
 * The function pointed to by sem_fn is used for synchronization.
 * \param[in]	queue		The dispatch queue
 * \param[out]	serial_data	The data to dequeue
 * \param[in]	sem_fn		The synchronization function to use
 * \returns
 * On dequeue failure, -1 is returned. \n
 * Otherwise 0 is returned
 */
static int dequeue(Queue *queue, void **serial_data, int (*sem_fn)(sem_t*)) {
	// take a piece of data off the queue

	if (sem_fn(&queue->queue_empty_lock) == -1) 
		return -1;

	pthread_mutex_lock(&queue->queue_lock);		// get atomic access to queue

	*serial_data = queue->list[queue->queue_index_start];
	//printf("Dequeue: %i: %p\n", queue->queue_index_start, *serial_data);
	modInc(&queue->queue_index_start, queue->size);

	sem_post(&queue->queue_full_lock);			// signal more space is available
	pthread_mutex_unlock(&queue->queue_lock);

	return 0;
}


/* Visible Functions */

/**
 * \brief Initialize the queue
 * \param	size	Max number of entries that can be stored. \n
 * Note that the size must be greater than 1.
 * \returns
 * On Success, returns a pointer to the queue
 * On Failure, returns NULL
 */
Queue *queueInit(uint32_t size) {
	// Create a queue with size elements

	Queue *queue;

	if (size < 2)
		return NULL;	// not enough size will be allocated for the queue to work

	queue = (Queue*)malloc(sizeof(Queue));
	if (!queue) {
		// TODO: Log an error here
		return NULL;		// allocation error
	}

	queue->size = size;

	pthread_mutex_init(&queue->queue_lock, NULL);
	sem_init(&queue->queue_empty_lock, 0, 0);
	sem_init(&queue->queue_full_lock, 0, queue->size-1);

	queue->list = (void**)malloc(queue->size*sizeof(void*));
	if (!queue->list) {
		// TODO: Log an error here
		return NULL;
	}

	queue->queue_index_end = 0;
	queue->queue_index_start = 0;

	return queue;
}



/**
 * \brief Free queue resources 
 *
 * The entries on the queue are not freed
 */
Queue *queueDestroy(Queue *queue) {
	// Free up a queue

	free(queue->list);
	pthread_mutex_destroy(&queue->queue_lock);
	sem_destroy(&queue->queue_empty_lock);
	sem_destroy(&queue->queue_full_lock);
	free(queue);

	return NULL;
}



/**
 * \brief Return the number of elements in the queue
 */
int queueSize(Queue *queue) {
	// Get the number of elements in the queue

	int lower_bound = queue->queue_index_start, 
		upper_bound = queue->queue_index_end;

	if (upper_bound < lower_bound)
		upper_bound += queue->size;

	return (upper_bound - lower_bound);
}



/**
 * \brief Return the max number of elements in the queue
 */
int queueMaxSize(Queue *queue) {
	// Get the max number of elements we can enqueue
	
	return queue->size;
}



/**
 * \brief Try to enqueue data, fail if data can't be enqueued
 *
 * If data can't be enqueued, return right away with a failure. \n
 * Note that data on the queue is not protected at all. 
 * serial_data should be data that has been allocated on the heap 
 * to prevent inadvertant modification of the data contained.
 * \param[in]	queue		The dispatch queue
 * \param[in]	serial_data	The data to enqueue
 * \returns
 * If data couldn't immediately be enqueued, the function immediately returns -1. \n
 * On success, return 0
 */
int queueTryEnqueue(Queue *queue, void *serial_data) {
	// try to put data onto the queue
	// If the queue is full, return an error

	return enqueue(queue, serial_data, &sem_trywait);
}



/**
 * \brief Enqueue data, block if data can't be enqueued
 *
 * If data can't be enqueued, block until there is a slot
 * available. \n
 * Note that data on the queue is not protected at all. 
 * serial_data should be data that has been allocated on the heap 
 * to prevent inadvertant modification of the data contained.
 * \param[in]	queue		The dispatch queue
 * \param[in]	serial_data	The data to enqueue
 * \returns
 * If data couldn't immediately be enqueued, the function blocks until there is
 * space to enqueue. Nevertheless, if there is some synchronization failure,
 * -1 will be returned. \n
 * On success, return 0
 */
int queueEnqueue(Queue *queue, void *serial_data) {
	// Put data onto the queue
	// If the queue is full, block until space is available

	return enqueue(queue, serial_data, &sem_wait);
}



/**
 * \brief Try to dequeue data, fail if data can't be dequeued
 *
 * If data can't be dequeued, return right away with a failure. \n
 * Note that data on the queue is not protected at all. 
 * \param[in]	queue		The dispatch queue
 * \param[out]	serial_data	The data to dequeue
 * \returns
 * If data couldn't immediately be dequeued, the function immediately returns -1. \n
 * On success, return 0
 */
int queueTryDequeue(Queue *queue, void **serial_data) {
	// Take data off the queue.
	// If the queue is empty, return an error

	return dequeue(queue, serial_data, &sem_trywait);
}



/**
 * \brief Dequeue data, block if data can't be dequeued
 *
 * If data can't be dequeued, block until there is 
 * data in the queue
 * Note that data on the queue is not protected at all. 
 * \param[in]	queue		The dispatch queue
 * \param[out]	serial_data	The data to dequeue
 * \returns
 * If data couldn't immediately be dequeued, the function blocks until there is
 * something on the queue to dequeue. Nevertheless, if there is 
 * some synchronization failure, -1 will be returned. \n
 * On success, return 0
 */
int queueDequeue(Queue *queue, void **serial_data) {
	// Take data off the queue.
	// If the queue is empty, block until data is available

	return dequeue(queue, serial_data, &sem_wait);
}



// vim: ft=c ts=4 noet sw=4:
