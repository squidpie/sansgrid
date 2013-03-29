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
#define _POSIX_C_SOURCE 200809L		// Required for nanosleep()

#include "../../payloads.h"

enum SansgridHeartbeatStatusEnum {
	SG_DEVICE_NOT_PRESENT,
	SG_DEVICE_PRESENT,
	SG_DEVICE_STALE,
	SG_DEVICE_LOST
};

int sleepMicro(uint32_t usecs);



#endif

// vim: ft=c ts=4 noet sw=4:
