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

#include <stdint.h>

typedef struct Queue Queue;


typedef struct DispatchNode {
	// Generic Packet
	// Tagged with IP Address and packet's origin,
	// along with the data
	uint8_t packet_origin;		// Communication Source
	uint32_t timestamp;
	uint8_t datatype;
	uint8_t serial_data[80];
	// Because of all the conversions and copying that is going on,
	// for now I'm adding a bounds check to make sure no data is
	// being cropped
	uint8_t bounds_check;
} DispatchNode;

// Construct/Destruct
Queue *queueInit(uint32_t size);
Queue *queueDestroy(Queue*);
// Queue State
int queueSize(Queue*);
// Queuing Operations
int queueTryEnqueue(Queue *queue, uint8_t *serial_data);
int queueEnqueue(Queue *queue, uint8_t *serial_data);
int queueTryDequeue(Queue *queue, uint8_t **serial_data);
int queueDequeue(Queue *queue, uint8_t **serial_data);

#endif

// vim: ft=c ts=4 noet sw=4:

