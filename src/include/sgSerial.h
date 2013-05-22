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
/** \file */

#include <stdint.h>
//#include <arpa/inet.h>
#include "payloads.h"

typedef struct SansgridSerial {
	uint8_t control;				// control byte
	uint8_t ip_addr[IP_SIZE];		// Overloaded IP Field
									// contains origin or destination IP address
	uint8_t payload[81];			// payload
} SansgridSerial;


enum SGSerialCtrlByte {
	SG_SERIAL_CTRL_NO_DATA 			= 0xFD,
	SG_SERIAL_CTRL_VALID_DATA 		= 0xAD,
	SG_SERIAL_CTRL_SETAS_SENSOR 	= 0x10,
	SG_SERIAL_CTRL_SETAS_ROUTER 	= 0x11,
	SG_SERIAL_CTRL_DEV_NOT_READY	= 0xDD,
};

// initialize serial connection
int8_t sgSerialOpen(void);
// Configure radio as a router radio
// Radio will be configured as a sensor radio by default.
int8_t sgSerialSetAsRouter(void);
int8_t sgSerialSetAsSensor(void);
// Send size bytes of serialdata serially
int8_t sgSerialSend(SansgridSerial *sg_serial, uint32_t size);
// Get data from serial in. Data size will be in size.
int8_t sgSerialReceive(SansgridSerial **sg_serial, uint32_t *size);
// Finish serial connection
// WARNING: This function will change
int8_t sgSerialClose(void);



#endif

// vim: ft=c ts=4 noet sw=4:
