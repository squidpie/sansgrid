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

#include "routing.h"


struct DeviceIP {
	uint32_t ip[IP_SIZE];		// IP address
	uint32_t clkdiff;			// ping offset
};

struct DeviceIP *routing_table[ROUTING_ARRAYSIZE];

static uint32_t tableptr = 0;
static uint32_t table_alloc = 0;

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

int maskip(uint32_t ip_addr[IP_SIZE], uint32_t base[IP_SIZE], uint32_t tableptr) {
	// Mask the tableptr with the base to make an ip address
	
	memcpy(ip_addr, base, IP_SIZE*sizeof(uint32_t));
	ip_addr[IP_SIZE-1] = base[IP_SIZE-1] | tableptr;

	return 0;
}


uint32_t locationToTablePtr(uint32_t ip_addr[IP_SIZE], uint32_t base[IP_SIZE]) {
	// Take a location and show where it should be in the routing table

	int i;
	uint32_t offset[IP_SIZE];

	for (i=0; i<IP_SIZE; i++)
		offset[i] = ip_addr[i] - base[i];

	return offset[IP_SIZE-1];
}

int routingTableAssignIP(uint32_t ip_addr[IP_SIZE]) {
	// Allocate the next available block and give it an IP address

	uint32_t base[4] = {0, 0, 0, 0};		// dummy base

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
	memcpy(routing_table[tableptr]->ip, ip_addr, IP_SIZE*sizeof(uint32_t));

	return 0;
}


int routingTableFreeIP(uint32_t ip_addr[IP_SIZE]) {
	// Release IP Address
	
	uint32_t base[4] = {0, 0, 0, 0};	// dummy base
	uint32_t index;

	if (!table_alloc)
		return -1;

	// table lookup
	index = locationToTablePtr(ip_addr, base);

	if (index >= ROUTING_ARRAYSIZE)
		return -1;
	if (memcmp(ip_addr, routing_table[index]->ip, IP_SIZE*sizeof(uint32_t))) {
		return -1;
	}

	// Release slot
	free(routing_table[index]);
	routing_table[index] = NULL;
	table_alloc--;

	return 0;
}


int routingTableLookup(uint32_t ip_addr[IP_SIZE]) {
	// Lookup an IP Address in the table
	// return true if found, false if not found
	
	uint32_t base[4] = {0, 0, 0, 0};	// dummy base
	uint32_t index;

	if (!table_alloc)
		return 0;

	// table lookup
	index = locationToTablePtr(ip_addr, base);

	if (index >= ROUTING_ARRAYSIZE)
		return 0;

	if (memcmp(ip_addr, routing_table[index]->ip, IP_SIZE*sizeof(uint32_t))) {
		return 0;
	}

	return 1;
}



// vim: ft=c ts=4 noet sw=4:
