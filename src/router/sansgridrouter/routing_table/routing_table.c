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
#include <arpa/inet.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>

#include "routing_table.h"




typedef struct RoutingNode {
	// A device that has an assigned IP address
	// For now, this is separate from properties.
	// Later, it may be combined with DeviceProperties
	//struct DeviceProperties properties;
	int32_t lost_pings;
	DeviceProperties *properties;
} RoutingNode;
	

struct RoutingTable {
	uint32_t tableptr;			// index, used for alloc
	uint32_t table_alloc;
	uint8_t base[IP_SIZE];
	uint8_t broadcast[IP_SIZE];
	uint32_t hbptr;				// index, used for heartbeat
	struct RoutingNode *routing_table[ROUTING_ARRAYSIZE];
};



static int tableAssertValid(RoutingTable *table) {
	// Exit if the table isn't allocated
	if (table == NULL) {
		syslog(LOG_ERR, "routing table not initialized; quitting");
		exit(EXIT_FAILURE);
	}
	return 0;
}


static int tableChkFull(RoutingTable *table) {
	if (table->table_alloc >= ROUTING_ARRAYSIZE) {
		syslog(LOG_DEBUG, "routing table full");
		return 1;
	}
	return 0;
}


static int tableChkEmpty(RoutingTable *table) {
	if (table->table_alloc == 0) {
		syslog(LOG_DEBUG, "routing table empty");
		return 1;
	}
	return 0;
}



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
	table->hbptr = 0;
	memcpy(table->base, base, IP_SIZE*sizeof(uint8_t));
	memset(table->broadcast, 0xff, IP_SIZE*sizeof(uint8_t));
	// TODO: Check to make sure lowest ROUTING_UNIQUE_BITS are 0
	

	for (i=0; i<ROUTING_ARRAYSIZE; i++)
		table->routing_table[i] = NULL;

	return table;
}


RoutingTable *routingTableDestroy(RoutingTable *table) {
	// Free any memory associated with the routing table
	
	int i;
	uint8_t ip_addr[IP_SIZE];

	for (i=0; i<ROUTING_ARRAYSIZE; i++) {
		if (table->routing_table[i]) {
			maskip(ip_addr, table->base, i);
			routingTableFreeIP(table, ip_addr);
		}
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

	tableAssertValid(table);
	if (tableChkFull(table))
		return 1;

	index = locationToTablePtr(ip_addr, table->base);

	if (routingTableLookup(table, ip_addr) == 0) {
		// Allocate space for the device
		syslog(LOG_INFO, "Allocating a device at %i", index);
		table->routing_table[index] = (RoutingNode*)malloc(sizeof(RoutingNode));
		dev_prop = (DeviceProperties*)malloc(sizeof(DeviceProperties));
		memcpy(dev_prop, properties, sizeof(DeviceProperties));
		table->routing_table[index]->properties = dev_prop;
		table->routing_table[index]->lost_pings = 0;
		table->table_alloc++;
		return 0;
	} else
		return 1;
}



int32_t routingTableAssignIP(RoutingTable *table, uint8_t ip_addr[IP_SIZE], 
		DeviceProperties *properties) {
	// Allocate the next available block and give it an IP address

	uint32_t tableptr;


	tableAssertValid(table);
	if (tableChkFull(table))
		return 1;

	tableptr = table->tableptr;

	// Find next available slot
	while (table->routing_table[tableptr] || tableptr < 2) {
		tableptr = (tableptr + 1) % ROUTING_ARRAYSIZE;
	}
	maskip(ip_addr, table->base, tableptr);	// create IP address
	table->tableptr = tableptr;

	// Allocate space for the device
	return routingTableAssignIPStatic(table, ip_addr, properties);
}



int32_t routingTableFreeIP(RoutingTable *table, uint8_t ip_addr[IP_SIZE]) {
	// Release IP Address
	
	uint32_t index;


	tableAssertValid(table);
	if (tableChkEmpty(table))
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


	tableAssertValid(table);
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


uint32_t routingTableIPToRDID(RoutingTable *table, uint8_t ip_addr[IP_SIZE]) {
	// Lookup an IP address in the table
	// return a unique identifier if found,
	// return 0 if false
	
	if (routingTableLookup(table, ip_addr) == 0) {
		// device not found
		return 0;
	}
	// For now, just use the index as a unique identifier
	return locationToTablePtr(ip_addr, table->base);
}

int32_t routingTableRDIDToIP(RoutingTable *table, uint32_t rdid, uint8_t ip_addr[IP_SIZE]) {
	// convert an rdid to an IP address
	maskip(ip_addr, table->base, rdid);
	return 0;
}

void routingTableGetBroadcast(RoutingTable *table, uint8_t broadcast[IP_SIZE]) {
	// get the broadcast address
	memcpy(broadcast, table->broadcast, IP_SIZE);
	return;
}

void routingTableGetRouterIP(RoutingTable *table, uint8_t ip_addr[IP_SIZE]) {
	// Get the router's IP address
	maskip(ip_addr, table->base, 1);
	return;
}


int32_t routingTableFindByAttr(RoutingTable *table, DeviceProperties *dev_prop, uint8_t ip_addr[IP_SIZE]) {
	// Find a device from the table based on device's properties
	// return 1 if device found with device's IP address stored in ip_addr
	// return 0 if device is not found

	DeviceProperties *table_dprop;

	tableAssertValid(table);
	if (!table->table_alloc)
		return 0;

	for (int i=0; i<ROUTING_ARRAYSIZE; i++) {
		if (!table->routing_table[i])
			continue;
		table_dprop = table->routing_table[i]->properties;
		if (memcmp(dev_prop->dev_attr.manid, table_dprop->dev_attr.manid, 4))
			continue;
		else if (memcmp(dev_prop->dev_attr.modnum, table_dprop->dev_attr.modnum, 4))
			continue;
		else if (memcmp(dev_prop->dev_attr.serial_number, table_dprop->dev_attr.serial_number, 8))
			continue;
		else {
			// found the device
			maskip(ip_addr, table->base, i);
			return 1;
		}
	}
	return 0;
}

enum SansgridDeviceStatusEnum routingTableLookupNextExpectedPacket(
		RoutingTable *table,
		uint8_t ip_addr[IP_SIZE]) {
	// Check to see what packet we're expecting next from this IP address

	tableAssertValid(table);

	if (!table->table_alloc)
		return SG_DEVSTATUS_NULL;

	uint32_t index = locationToTablePtr(ip_addr, table->base);
	if (index >= ROUTING_ARRAYSIZE || table->routing_table[index] == NULL)
		return SG_DEVSTATUS_NULL;
	return table->routing_table[index]->properties->next_expected_packet;
}


int32_t routingTableSetNextExpectedPacket(
		RoutingTable *table,
		uint8_t ip_addr[IP_SIZE],
		enum SansgridDeviceStatusEnum nextstatus) {
	// Set what packet we're expecting next from this IP address

	tableAssertValid(table);

	if (!table->table_alloc)
		return -1;

	uint32_t index = locationToTablePtr(ip_addr, table->base);
	if (index >= ROUTING_ARRAYSIZE || table->routing_table[index] == NULL)
		return -1;
	table->routing_table[index]->properties->next_expected_packet = nextstatus;
	return 0;
}


int32_t routingTableFindNextDevice(RoutingTable *table, uint8_t ip_addr[IP_SIZE]) {
	// Find the next device to send a heartbeat to, set ip_addr to the ip address of that device
	int i;
	tableAssertValid(table);

	if (!table->table_alloc)
		return 0;
	for (i=(table->hbptr+1)%ROUTING_ARRAYSIZE; 
			!table->routing_table[i] || (i == 0) || (i == 1);
			i = (i+1)%ROUTING_ARRAYSIZE) {
	}
	table->hbptr = i;
	maskip(ip_addr, table->base, i);

	return 1;
}

int32_t routingTableSetHeartbeatStatus(RoutingTable *table, uint8_t ip_addr[IP_SIZE], enum SansgridHeartbeatStatusEnum hb_status) {
	// Set the heartbeat status of a device
	// A ping is a special case where it's handled based on the device's attributes
	tableAssertValid(table);

	if (!table->table_alloc)
		return -1;

	uint32_t index = locationToTablePtr(ip_addr, table->base);
	if (index >= ROUTING_ARRAYSIZE || table->routing_table[index] == NULL)
		return -1;
	enum SansgridHeartbeatStatusEnum device_status = table->routing_table[index]->properties->heartbeat_status;
	if (hb_status != SG_DEVICE_PINGING) {
		table->routing_table[index]->properties->heartbeat_status = hb_status;
		// FIXME: set lost_pings based on hb_status?
		table->routing_table[index]->lost_pings = 0;
		return 0;
	}
	switch (device_status) {
		case SG_DEVICE_NOT_PRESENT:
			return -1;
			break;
		case SG_DEVICE_PINGING:
		case SG_DEVICE_STALE:
			// Haven't received the last ping
			table->routing_table[index]->lost_pings++;
			// TODO: Add "device lost" code here
			break;
		case SG_DEVICE_PRESENT:
			table->routing_table[index]->properties->heartbeat_status = SG_DEVICE_PINGING;
			break;
		default:
			return -1;
	}
	return 0;
}

enum SansgridHeartbeatStatusEnum routingTableGetHeartbeatStatus(RoutingTable *table,
		uint8_t ip_addr[IP_SIZE]) {
	// Get the current state (active, stale, lost) of this IP address

	tableAssertValid(table);

	if (!table->table_alloc)
		return SG_DEVICE_NOT_PRESENT;

	uint32_t index = locationToTablePtr(ip_addr, table->base);
	if (index >= ROUTING_ARRAYSIZE || table->routing_table[index] == NULL)
		return SG_DEVICE_NOT_PRESENT;
	return table->routing_table[index]->properties->heartbeat_status;
}
	
	

int32_t routingTableGetDeviceCount(RoutingTable *table) {
	// Return the number of devices that are authenticated
	tableAssertValid(table);
	return table->table_alloc;
}


int32_t routingTableGetStatus(RoutingTable *table, char *str) {
	// Print table status
	uint32_t i, j;
	uint8_t ip_addr[IP_SIZE];
	str[0] = '\0';
	syslog(LOG_DEBUG, "table alloc = %i", table->table_alloc);
	for (i=0; i<ROUTING_ARRAYSIZE; i++) {
		if (table->routing_table[i]) {
			syslog(LOG_DEBUG, "found one!");
			maskip(ip_addr, table->base, i);
			for (j=0; j<IP_SIZE; j++)
				sprintf(str, "%s%x:", str, ip_addr[j]);
			sprintf(str, "%s\n", str);
		}
	}
	return 0;
}

// vim: ft=c ts=4 noet sw=4:
