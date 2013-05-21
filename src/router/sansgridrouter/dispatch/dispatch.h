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
 */

#ifndef __SG_DISPATCH_H__
#define __SG_DISPATCH_H__
/** \file */

#include <stdint.h>

typedef struct Queue Queue;



// Construct/Destruct
Queue *queueInit(uint32_t size);
Queue *queueDestroy(Queue*);
// Queue State
int queueSize(Queue*);
int queueMaxSize(Queue*);
// Queuing Operations
int queueTryEnqueue(Queue *queue, void *serial_data);
int queueEnqueue(Queue *queue, void *serial_data);
int queueTryDequeue(Queue *queue, void **serial_data);
int queueDequeue(Queue *queue, void **serial_data);

#endif

// vim: ft=c ts=4 noet sw=4:

