/* Definitions for heartbeats
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


#ifndef __SG_ROUTING_HEARTBEAT_H__
#define __SG_ROUTING_HEARTBEAT_H__

#define HEARTBEAT_INTERVAL 240		// interval, in seconds, between device heartbeats
#define PING_THRESHOLD 8			// number of lost pings before device is considered lost
#define STALE_THRESHOLD 4			// number of lost pings before device is considered stale

#include <payloads.h>

enum __attribute__((deprecated))SansgridHeartbeatStatusEnum {
	SG_DEVICE_NOT_PRESENT,
	SG_DEVICE_PRESENT,
	SG_DEVICE_PINGING,
	SG_DEVICE_STALE,
	SG_DEVICE_LOST
};

typedef struct HeartbeatStatus HeartbeatStatus;

int32_t hbIsDeviceStale(HeartbeatStatus *hb);
int32_t hbIsDeviceLost(HeartbeatStatus *hb);
HeartbeatStatus *hbInit(int32_t ping_thres, int32_t stale_thres);
HeartbeatStatus *hbInitDefault(void);
void hbDestroy(HeartbeatStatus *hb);
int32_t hbDecrement(HeartbeatStatus *hb);
int32_t hbRefresh(HeartbeatStatus *hb);


#endif

// vim: ft=c ts=4 noet sw=4:
