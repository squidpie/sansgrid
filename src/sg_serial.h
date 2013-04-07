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

typedef struct SansgridSerial {
	uint8_t origin_ip[IP_SIZE];		// ip of the payload sender
	uint8_t dest_ip[IP_SIZE];		// ip of the destination
	uint8_t payload[81];			// payload
} SansgridSerial;



// initialize serial connection
// WARNING: This function will change
int8_t sgSerialOpen(void);
// Send size bytes of serialdata serially
int8_t sgSerialSend(SansgridSerial *sg_serial, uint32_t size);
// Get data from serial in. Data size will be in size.
int8_t sgSerialReceive(SansgridSerial **sg_serial, uint32_t *size);
// Finish serial connection
// WARNING: This function will change
int8_t sgSerialClose(void);



#endif

// vim: ft=c ts=4 noet sw=4:
