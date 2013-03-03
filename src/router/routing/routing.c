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


struct DeviceIP {
	uint8_t ip[IP_SIZE];		// IP address
	uint32_t clkdiff;			// ping offset
};

struct DeviceIP *routing_table[ROUTING_ARRAYSIZE];

static uint32_t tableptr = 0;
static uint32_t table_alloc = 0;

static void createBase(uint8_t base[IP_SIZE]) {
	int i;
	uint32_t arrsize = ROUTING_ARRAYSIZE;

	for (i=0; i<IP_SIZE; i++)
		base[i] = 0;
	memcpy(base, &arrsize, sizeof(uint32_t));

	return;
}


void wordToByte(uint8_t *bytes, uint32_t *words, size_t bytesize) {
	// converts an array of words to an array of bytes
	// Note: bytes[] isn't size-checked! it must be 4x the size of wordsize!

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
	// Note: words[] isn't size-checked! it must be >= (bytesize/4)+1
	
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

int32_t littleEndian(void) {
	// Tests endianness
	// Returns 1 if little endian
	// Returns 0 if big endian
	int i = 1;
	char *p = (char*)&i;
	return (p[0] == 1);
}


void routingTableInit(void) {
	// Initialize the routing table
	
	int i;

	for (i=0; i<ROUTING_ARRAYSIZE; i++)
		routing_table[i] = NULL;

	return;
}


void routingTableDestroy(void) {
	// Free any memory associated with the routing table
	
	int i;

	for (i=0; i<ROUTING_ARRAYSIZE; i++) {
		if (routing_table[i])
			free(routing_table[i]);
	}

	return;
}

int maskip(uint8_t ip_addr[IP_SIZE], uint8_t base[IP_SIZE], uint32_t tableptr) {
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


uint32_t locationToTablePtr(uint8_t ip_addr[IP_SIZE], uint8_t base[IP_SIZE]) {
	// Take a location and show where it should be in the routing table

	int32_t i;
	uint8_t offset[IP_SIZE];
	uint32_t location[IP_SIZE/4];

	for (i=0; i<IP_SIZE; i++)
		offset[i] = ip_addr[i] - base[i];
	byteToWord(location, offset, IP_SIZE*sizeof(uint8_t));

	return location[IP_SIZE/4-1];
}

int32_t routingTableAssignIP(uint8_t ip_addr[IP_SIZE]) {
	// Allocate the next available block and give it an IP address

	uint8_t base[IP_SIZE];

	createBase(base);

	if (table_alloc >= ROUTING_ARRAYSIZE)
		return -1;

	// Find next available slot
	while (routing_table[tableptr]) {
		tableptr = (tableptr + 1) % ROUTING_ARRAYSIZE;
	}
	maskip(ip_addr, base, tableptr);	// create IP address

	// Allocate space for the device
	routing_table[tableptr] = (DeviceIP*)malloc(sizeof(DeviceIP));
	table_alloc++;

	// copy the IP address to the new slot
	memcpy(routing_table[tableptr]->ip, ip_addr, IP_SIZE*sizeof(uint8_t));

	return 0;
}


int32_t routingTableFreeIP(uint8_t ip_addr[IP_SIZE]) {
	// Release IP Address
	
	uint8_t base[IP_SIZE];
	uint32_t index;

	createBase(base);

	if (!table_alloc)
		return -1;

	// table lookup
	index = locationToTablePtr(ip_addr, base);

	if (index >= ROUTING_ARRAYSIZE)
		return -1;
	if (memcmp(ip_addr, routing_table[index]->ip, IP_SIZE*sizeof(uint8_t))) {
		return -1;
	}

	// Release slot
	free(routing_table[index]);
	routing_table[index] = NULL;
	table_alloc--;

	return 0;
}


int32_t routingTableLookup(uint8_t ip_addr[IP_SIZE]) {
	// Lookup an IP Address in the table
	// return true if found, false if not found
	
	uint8_t base[IP_SIZE];
	uint32_t index;

	createBase(base);

	if (!table_alloc)
		return 0;

	// table lookup
	index = locationToTablePtr(ip_addr, base);

	if (index >= ROUTING_ARRAYSIZE)
		return 0;

	if (memcmp(ip_addr, routing_table[index]->ip, IP_SIZE*sizeof(uint8_t))) {
		return 0;
	}

	return 1;
}



// vim: ft=c ts=4 noet sw=4:
