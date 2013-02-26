/* A quick implementation for a statically-defined
 * dynamic routing table.
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


#define ROUTING_UNIQUE_BITS	8			// Number of unique bits for the IP address

#if ROUTING_UNIQUE_BITS > 20			// Don't let the routing table be too big
#error "Routing Table too Large!"
#endif

#define ROUTING_ARRAYSIZE		(1 << ROUTING_UNIQUE_BITS)
#define IP_SIZE 4

#include <stdint.h>


typedef struct DeviceIP DeviceIP;


int littleEndian(void);
void routingTableInit(void);
void routingTableDestroy(void);
int routingTableAssignIP(uint32_t ip_addr[IP_SIZE]);
int routingTableFreeIP(uint32_t ip_addr[IP_SIZE]);
int routingTableLookup(uint32_t ip_addr[IP_SIZE]);




// vim: ft=c ts=4 noet sw=4:
