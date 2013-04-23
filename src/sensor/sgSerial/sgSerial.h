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

#define CONTROL 1				   // Number of bytes in control 
#define IP_ADDRESS 16			   // Number of bytes in source/destination 
                                   // IP address
#define PAYLOAD 81				   // Number of bytes in payload 
#define RECEIVE 0x00			   // Control byte for receiving data
#define TRANSMIT 0x01			   // Control byte for transmit data

// Set up a macro depending on architecture
#ifndef __ARCH_DUE__
#define ARCH_DUE

#include <Arduino.h>
#include <spiMaster.h>

#define SLAVE_SELECT  10           // SS pin 10
#define SLAVE_READY  8             // Hand shake pin identifying Slave has 
                                   // data to send
#define DELAY 6				   	   // Delay in microseconds between bytes sent

typedef struct SansgridSerial{
	char control[ CONTROL + 1 ];	   	 // Control
	char ip_addr[ IP_ADDRESS + 1 ];      // Overloaded IP Field
                                         // contains origin or destination IP address
	char payload[ PAYLOAD + 1 ];	     // Payload
} SansgridSerial;
#else
#define ARCH_PI

#include <stdint.h>
//#include <arpa/inet.h>
#include "payloads.h"

typedef struct SansgridSerial {
	uint8_t control[ CONTROL + 1 ];			// Control byte
	uint8_t ip_addr[ IP_ADDRESS + 1 ];		// Overloaded IP Field
											// contains origin or destination IP address
	uint8_t payload[ PAYLOAD + 1 ];			// Payload
} SansgridSerial;

#endif // __ARCH_DUE__

// Opens serial device for reading/writing, configures ports, sets order data 
// bits  are shifted in as MSB or LSB, and sets the clock frequency. Function 
// is called prior to sending or receiving any data over the serial line. 
int8_t sgSerialOpen(void);
// Configure radio as a router radio
// Radio will be configured as a sensor radio by default.
// Sets radio as a "router radio" 
int8_t sgSerialSetAsRouter(void);
// Sets radio as a "sensor radio".
int8_t sgSerialSetAsSensor(void);
// Transmit number of bytes "size" from SansgridSerial structure to Slave over 
// SPI. For full duplex a second SansgridSerial is passed in with a second size
// integer to receive data from the slave in return. The most significant bit 
// first in Big Endian byte order. Number of bytes is not to exceed control 
// byte plus IP address destination/source bytes plus payload bytes. 
int8_t sgSerialSend(SansgridSerial *sg_serial, uint32_t size);
// Receive number of bytes "size" and places in SansgridSerial structure. The 
// most significant bit first in Big Endian byte order. Number of bytes is not 
// to exceed control byte plus IP address destination/source bytes plus payload
// bytes. 
int8_t sgSerialReceive(SansgridSerial *sg_serial, uint32_t size);
// Function is called when all SPI input and output is completed. Stops SPI 
// from being transmitted and received. 
int8_t sgSerialClose(void);

#endif // __SG_SERIAL_H

// vim: ft=c ts=4 noet sw=4:
