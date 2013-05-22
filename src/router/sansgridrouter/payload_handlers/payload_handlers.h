/* Payload Handlers
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
 */

/** \file */
#ifndef __SG_ROUTER_HANDLERS_H__
#define __SG_ROUTER_HANDLERS_H__

#include <stdint.h>
#include "../routing_table/routing_table.h"
#include <sgSerial.h>
#include <payloads.h>




int32_t routerFreeDevice(RoutingTable *routing_table, uint8_t ip_addr[IP_SIZE]);
int32_t routerFreeAllDevices(RoutingTable *routing_table);
enum SansgridDeviceStatusEnum sgPayloadGetType(enum SansgridDataTypeEnum dt);
int32_t sgPayloadGetPayloadName(enum SansgridDeviceStatusEnum devstatus, char *name);
int routerHandleHatching(RoutingTable *routing_table, SansgridSerial *sg_serial);
int routerHandleFly(RoutingTable *routing_table, SansgridSerial *sg_serial);
int routerHandleEyeball(RoutingTable *routing_table, SansgridSerial *sg_serial);
int routerHandlePeck(RoutingTable *routing_table, SansgridSerial *sg_serial);
int routerHandleSing(RoutingTable *routing_table, SansgridSerial *sg_serial);
int routerHandleMock(RoutingTable *routing_table, SansgridSerial *sg_serial);
int routerHandlePeacock(RoutingTable *routing_table, SansgridSerial *sg_serial);
int routerHandleNest(RoutingTable *routing_table, SansgridSerial *sg_serial);
int routerHandleSquawk(RoutingTable *routing_table, SansgridSerial *sg_serial);
int routerHandleHeartbeat(RoutingTable *routing_table, SansgridSerial *sg_serial);
int routerHandleChirp(RoutingTable *routing_table, SansgridSerial *sg_serial);
int routerHandleServerStatus(RoutingTable *routing_table, SansgridSerial *sg_serial);


#endif
