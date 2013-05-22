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

/** \file */


#ifndef __SG_ROUTING_H__
#define __SG_ROUTING_H__

/// Number of unique bits for the IP address
#define ROUTING_UNIQUE_BITS	8			

#if ROUTING_UNIQUE_BITS > 20			// Don't let the routing table be too big
#error "Routing Table too Large!"
#endif

/// Number of entries in the routing table
#define ROUTING_ARRAYSIZE		(1 << ROUTING_UNIQUE_BITS)

#include <stdint.h>
#include <stdlib.h>

#include <payloads.h>
#include "heartbeat.h"


typedef struct RoutingTable RoutingTable;


void wordToByte(uint8_t *bytes, uint32_t *words, size_t bytesize);
int byteToWord(uint32_t *words, uint8_t *bytes, size_t bytesize);
RoutingTable *routingTableInit(uint8_t base[IP_SIZE], char essid[80]);
RoutingTable *routingTableDestroy(RoutingTable *table);
void routingTableGetEssid(RoutingTable *table, char essid[80]);
int32_t routingTableAssignIPStatic(RoutingTable *table, uint8_t ip_addr[IP_SIZE]);
int32_t routingTableAssignIP(RoutingTable *table, uint8_t ip_addr[IP_SIZE]);
int32_t routingTableFreeIP(RoutingTable *table, uint8_t ip_addr[IP_SIZE]);
int32_t routingTableFreeAllIPs(RoutingTable *table);
int32_t routingTableLookupRDID(RoutingTable *table, uint32_t rdid);
int32_t routingTableLookup(RoutingTable *table, uint8_t ip_addr[IP_SIZE]);
uint32_t routingTableIPToRDID(RoutingTable *table, uint8_t ip_addr[IP_SIZE]);
int32_t routingTableRDIDToIP(RoutingTable *table, uint32_t rdid, uint8_t ip_addr[IP_SIZE]);
void routingTableGetBroadcast(RoutingTable *table, uint8_t broadcast[IP_SIZE]);
void routingTableGetRouterIP(RoutingTable *table, uint8_t ip_addr[IP_SIZE]);
int32_t routingTableCheckValidPacket(RoutingTable *table, uint8_t ip_addr[IP_SIZE], enum SansgridDataTypeEnum dt);
int32_t routingTableRequireStrictAuth(RoutingTable *table);
int32_t routingTableAllowLooseAuth(RoutingTable *table);
int32_t routingTableSetAuthFiltered(RoutingTable *table);
int32_t routingTableIsAuthStrict(RoutingTable *table);
enum SansgridDeviceStatusEnum routingTableLookupNextExpectedPacket(RoutingTable *table, uint8_t ip_addr[IP_SIZE]);
int32_t routingTableSetNextExpectedPacket(RoutingTable *table, uint8_t ip_addr[IP_SIZE], enum SansgridDeviceStatusEnum nextstatus);
int32_t routingTableSetCurrentPacket(RoutingTable *table, uint8_t ip_addr[IP_SIZE], enum SansgridDeviceStatusEnum thisstatus);
uint32_t routingTableGetCurrentPacket(RoutingTable *table, uint8_t ip_addr[IP_SIZE]);
int32_t routingTableHeartbeatDevice(RoutingTable *table, uint8_t ip_addr[IP_SIZE]);
int32_t routingTableHeardDevice(RoutingTable *table, uint8_t ip_addr[IP_SIZE]);
int32_t routingTableIsDeviceLost(RoutingTable *table, uint8_t ip_addr[IP_SIZE]);
int32_t routingTableIsDeviceStale(RoutingTable *table, uint8_t ip_addr[IP_SIZE]);
int32_t routingTableGetDeviceCount(RoutingTable *table);
int32_t routingTableGetStatus(RoutingTable *table, int devnum, char *str);
int routingTableStepNextDevice(RoutingTable *table, uint8_t ip_addr[IP_SIZE]);
int routingTableForEachDevice(RoutingTable *table, uint8_t ip_addr[IP_SIZE]);


#endif

// vim: ft=c ts=4 noet sw=4:
