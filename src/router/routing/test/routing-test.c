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

#include "../routing.h"

void routingTablePrint(uint32_t ip_addr[IP_SIZE]) {
	int i;
	const uint32_t masklength = 4*sizeof(uint32_t);
	for (i=0; i<IP_SIZE; i++) {
		printf("%.2X:", ip_addr[i] >> masklength);
		printf("%.2X", ip_addr[i] & (~0 >> masklength));
		if (i < (IP_SIZE-1))
			printf(":");
	}	
	printf("\n");
	return;
}


int main(void) {
	int i;
	uint32_t ip_addr[IP_SIZE];

	routingTableInit();

	for (i=0; i<32; i++) {
		routingTableAssignIP(ip_addr);
		routingTablePrint(ip_addr);
	}

	if (routingTableFreeIP(ip_addr))
		printf("Oops!\n");


	routingTableDestroy();
	return 0;
}


// vim: ft=c ts=4 noet sw=4:
