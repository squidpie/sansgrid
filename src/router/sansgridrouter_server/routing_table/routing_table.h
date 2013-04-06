/* A statically-defined routing table.
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


#ifndef __SG_ROUTING_H__
#define __SG_ROUTING_H__

#define ROUTING_UNIQUE_BITS	8			// Number of unique bits for the IP address

#if ROUTING_UNIQUE_BITS > 20			// Don't let the routing table be too big
#error "Routing Table too Large!"
#endif

#define ROUTING_ARRAYSIZE		(1 << ROUTING_UNIQUE_BITS)

#include <stdint.h>
#include <stdlib.h>

#include "../../payloads.h"
#include "heartbeat.h"


//typedef struct DeviceIP DeviceIP;
typedef struct RoutingTable RoutingTable;
typedef struct DeviceProperties DeviceProperties;


struct DeviceProperties {
	// A device's general properties
	SansgridEyeball dev_attr;
	enum SansgridHeartbeatStatusEnum heartbeat_status;
	enum SansgridDeviceStatusEnum dev_status;
	enum SansgridDeviceStatusEnum next_expected_packet;
};



//int32_t littleEndian(void);
void wordToByte(uint8_t *bytes, uint32_t *words, size_t bytesize);
int byteToWord(uint32_t *words, uint8_t *bytes, size_t bytesize);
RoutingTable *routingTableInit(uint8_t base[IP_SIZE]);
RoutingTable *routingTableDestroy(RoutingTable *table);
int32_t routingTableAssignIPStatic(RoutingTable *table, uint8_t ip_addr[IP_SIZE], DeviceProperties *properties);
int32_t routingTableAssignIP(RoutingTable *table, uint8_t ip_addr[IP_SIZE], DeviceProperties *properties);
int32_t routingTableFreeIP(RoutingTable *table, uint8_t ip_addr[IP_SIZE]);
int32_t routingTableLookup(RoutingTable *table, uint8_t ip_addr[IP_SIZE]);
enum SansgridDeviceStatusEnum routingTableLookupNextExpectedPacket(RoutingTable *table, uint8_t ip_addr[IP_SIZE]);
int32_t routingTableFindByAttr(RoutingTable *table, DeviceProperties *dev_prop, uint8_t ip_addr[IP_SIZE]);
int32_t routingTableSetNextExpectedPacket(RoutingTable *table, uint8_t ip_addr[IP_SIZE], enum SansgridDeviceStatusEnum nextstatus);
int32_t routingTableFindNextDevice(RoutingTable *table, uint8_t ip_addr[IP_SIZE]);
int32_t routingTableSetHeartbeatStatus(RoutingTable *table, uint8_t ip_addr[IP_SIZE], enum SansgridHeartbeatStatusEnum hb_status);
enum SansgridHeartbeatStatusEnum routingTableGetHeartbeatStatus(RoutingTable *table, uint8_t ip_addr[IP_SIZE]);
int32_t routingTableGetDeviceCount(RoutingTable *table);


#endif

// vim: ft=c ts=4 noet sw=4: