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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#ifndef __H_RADIO_STUB__
#define __H_RADIO_STUB__
#include "stubs/radio-stub.h"
#endif

#include "../routing/routing.h"
#include "../synchronous_queue/sync_queue.h"

void routingTablePrint(uint32_t ip_addr[IP_SIZE]) {
	// Print the IP address like an IPv6 address
	int i;
	union WordToByte {
		uint32_t word[IP_SIZE];
		uint8_t byte[IP_SIZE*4];
	} wtb;

	for (i=0; i<IP_SIZE; i++) {
		if (littleEndian())
			wtb.word[i] = htonl(ip_addr[i]);
		else
			wtb.word[i] = ip_addr[i];
	}

	for (i=0; i<4*IP_SIZE; i++) {
		printf("%.2X", wtb.byte[i]);
		if (i < 4*IP_SIZE-1)
			printf(":");
	}	
	printf("\n");
	return;
}


int main(void) {
	int i;
	uint32_t ip_addr[IP_SIZE];
	pid_t chpid;
	Queue *queue;

	chpid = fork();

	if (chpid < 0)
		exit(EXIT_FAILURE);
	else if (chpid == 0) {
		// child
		radioStubRuntime();
		exit(EXIT_SUCCESS);
	}
	// parent

	queue = queueInit(200);
	routingTableInit();

	if (littleEndian()) {
		printf("Machine is Little Endian\n");
		printf("Conversion is required.\n");
	} else {
		printf("Machine is Big Endian\n");
		printf("No Conversion is required.\n");
	}

	for (i=0; i<32; i++) {
		routingTableAssignIP(ip_addr);
		routingTablePrint(ip_addr);
	}

	if (routingTableLookup(ip_addr)) {
		printf("IP Address is resident: ");
		routingTablePrint(ip_addr);
	}

	if (routingTableFreeIP(ip_addr))
		printf("Oops!\n");




	routingTableDestroy();
	return 0;
}


// vim: ft=c ts=4 noet sw=4:
