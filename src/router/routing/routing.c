/* A quick implementation for a statically-defined table.
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

#define _POSIX_C_SOURCE 200809L		// Required for nanosleep()

#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>

#include "routing.h"
#include "../../payloads.h"





typedef struct RoutingNode {
	// A device that has an assigned IP address
	// For now, this is separate from properties.
	// Later, it may be combined with DeviceProperties
	//struct DeviceProperties properties;
	DeviceProperties *properties;
} RoutingNode;
	

struct RoutingTable {
	uint32_t tableptr;
	uint32_t table_alloc;
	uint8_t base[IP_SIZE];
	struct RoutingNode *routing_table[ROUTING_ARRAYSIZE];
};


/*
static void createBase(uint8_t base[IP_SIZE]) {
	// Return a base IP based on some attributes
	
	int i;
	uint32_t arrsize = ROUTING_ARRAYSIZE;

	for (i=0; i<IP_SIZE; i++)
		base[i] = 0;
	memcpy(base, &arrsize, sizeof(uint32_t));

	return;
}
*/



static int maskip(uint8_t ip_addr[IP_SIZE], uint8_t base[IP_SIZE], uint32_t tableptr) {
	// Mask the tableptr with the base to make an ip address
	
	int i, ioffset;
	uint8_t tablebyteptr[4];

	wordToByte(tablebyteptr, &tableptr, 1*sizeof(uint32_t));

	memcpy(ip_addr, base, IP_SIZE*sizeof(uint8_t));
	for (i=0; i<4; i++) {
		ioffset = IP_SIZE-(4-i);
		ip_addr[ioffset] = base[ioffset] | tablebyteptr[i];
	}

	return 0;
}



static uint32_t locationToTablePtr(uint8_t ip_addr[IP_SIZE], uint8_t base[IP_SIZE]) {
	// Take a location and show where it should be in the routing table

	int32_t i;
	uint8_t offset[IP_SIZE];
	uint32_t location[IP_SIZE/4];

	for (i=0; i<IP_SIZE; i++)
		offset[i] = ip_addr[i] - base[i];
	byteToWord(location, offset, IP_SIZE*sizeof(uint8_t));

	return location[IP_SIZE/4-1];
}



void wordToByte(uint8_t *bytes, uint32_t *words, size_t bytesize) {
	// converts an array of words to an array of bytes
	// This is an unsafe function. It does no bounds checking.
	// bytes[] isn't size-checked. it must be 4x the size of wordsize.

	uint32_t i;
	uint32_t endianconv;
	size_t wordsize = (bytesize * sizeof(uint8_t)) / sizeof(uint32_t);

	for (i=0; i<wordsize; i++) {
		endianconv = htonl(words[i]);
		memcpy(&bytes[4*i], &endianconv, sizeof(uint32_t));
	}

	return;
}



int byteToWord(uint32_t *words, uint8_t *bytes, size_t bytesize) {
	// converts an array of bytes into an array of words
	// This is an unsafe function. It does no bounds checking.
	// bytesize must be a multiple of 4.
	// words[] isn't size-checked. it must be >= (bytesize/4)+1.
	
	uint32_t i;
	uint32_t endianconv;

	if (bytesize % 4)
		return -1;

	for (i=0; i<(bytesize/4); i++) {
		memcpy(&endianconv, &bytes[i*4], sizeof(uint32_t));
		words[i] = ntohl(endianconv);
	}

	return 0;
}




RoutingTable *routingTableInit(uint8_t base[IP_SIZE]) {
	// Initialize the routing table
	
	int i;
	RoutingTable *table;

	table = (RoutingTable*)malloc(sizeof(RoutingTable));
	table->tableptr = 0;
	table->table_alloc = 0;
	memcpy(table->base, base, IP_SIZE*sizeof(uint8_t));
	// TODO: Check to make sure lowest ROUTING_UNIQUE_BITS are 0
	

	for (i=0; i<ROUTING_ARRAYSIZE; i++)
		table->routing_table[i] = NULL;

	return table;
}


RoutingTable *routingTableDestroy(RoutingTable *table) {
	// Free any memory associated with the routing table
	
	int i;

	for (i=0; i<ROUTING_ARRAYSIZE; i++) {
		if (table->routing_table[i])
			free(table->routing_table[i]);
	}

	free(table);
	table = NULL;
	return table;
}


int32_t routingTableAssignIPStatic(RoutingTable *table, uint8_t ip_addr[IP_SIZE],
	   DeviceProperties *properties) {
	// Statically assign IP Address if possible
	// return 0 if success, -1 if failure
	
	DeviceProperties *dev_prop;
	int32_t index;

	if (table == NULL)
		return -1;
	if (table->table_alloc >= ROUTING_ARRAYSIZE)
		return -1;

	index = locationToTablePtr(ip_addr, table->base);

	if (routingTableLookup(table, ip_addr) == 0) {
		// TODO: statically assign IP
		// Allocate space for the device
		table->routing_table[index] = (RoutingNode*)malloc(sizeof(RoutingNode));
		dev_prop = (DeviceProperties*)malloc(sizeof(DeviceProperties));
		memcpy(dev_prop, properties, sizeof(DeviceProperties));
		table->routing_table[index]->properties = dev_prop;
	}

	return 0;
}



int32_t routingTableAssignIP(RoutingTable *table, uint8_t ip_addr[IP_SIZE], 
		DeviceProperties *properties) {
	// Allocate the next available block and give it an IP address

	uint32_t tableptr;

	if (table == NULL)
		return -1;
	if (table->table_alloc >= ROUTING_ARRAYSIZE)
		return -1;

	tableptr = table->tableptr;

	// Find next available slot
	while (table->routing_table[tableptr]) {
		tableptr = (tableptr + 1) % ROUTING_ARRAYSIZE;
	}
	maskip(ip_addr, table->base, tableptr);	// create IP address
	table->tableptr = tableptr;

	// Allocate space for the device
	if (!routingTableAssignIPStatic(table, ip_addr, properties)) {
		table->table_alloc++;
		return 0;
	}
	else
		return -1;
}



int32_t routingTableFreeIP(RoutingTable *table, uint8_t ip_addr[IP_SIZE]) {
	// Release IP Address
	
	uint32_t index;


	if (table == NULL)
		return -1;
	if (!table->table_alloc)
		return -1;

	// table lookup
	index = locationToTablePtr(ip_addr, table->base);

	if (index >= ROUTING_ARRAYSIZE)
		return -1;
	if (table->routing_table[index] == NULL)
		return -1;

	// Release slot
	free(table->routing_table[index]->properties);
	free(table->routing_table[index]);
	table->routing_table[index] = NULL;
	table->table_alloc--;

	return 0;
}



int32_t routingTableLookup(RoutingTable *table, uint8_t ip_addr[IP_SIZE]) {
	// Lookup an IP Address in the table
	// return true if found, false if not found
	
	uint32_t index;


	if (table == NULL)
		return 0;
	if (!table->table_alloc)
		return 0;

	// table lookup
	index = locationToTablePtr(ip_addr, table->base);

	if (index >= ROUTING_ARRAYSIZE)
		return 0;

	if (table->routing_table[index] == NULL)
		return 0;

	return 1;
}



// vim: ft=c ts=4 noet sw=4:
