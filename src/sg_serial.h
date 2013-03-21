/* Serial Communication API
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

#ifndef __SG_SERIAL_H__
#define __SG_SERIAL_H__

#include <stdint.h>
#include <arpa/inet.h>
#include "payloads.h"

typedef struct SansgridGeneric {
	// Generic Packet
	// Tagged with IP Address and packet's origin,
	// along with the data
	uint8_t datatype;
	uint8_t ip[IP_SIZE];		// Origin's IP address
	uint8_t packet_origin;		// Communication Source
	uint8_t serial_data[80];
	// Because of all the conversions and copying that is going on,
	// for now I'm adding a bounds check to make sure no data is
	// being cropped
	uint8_t bounds_check;
} SansgridGeneric;


// Send size bytes of serialdata serially
int8_t sgSerialSend(SansgridGeneric *serial_packet, uint32_t size);
// Get data from serial in. Data size will be in size.
int8_t sgSerialReceive(SansgridGeneric *serial_packet, uint32_t *size);



#endif

// vim: ft=c ts=4 noet sw=4:
