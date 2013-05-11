/* Serial Communication uint8_terface
 * Specific to the Arduino DUE Platform
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

#include <Arduino.h>
#include <SPI.h>

#define NUM_BYTES 98		// Number of uuint8_t8_ts tranferred over SPI
#define CONTROL 1			// Number of uuint8_t8_ts in control 
#define IP_ADDRESS 16		// Number of uuint8_t8_ts in source/destination IP address
#define PAYLOAD 81			// Number of uuint8_t8_ts in payload 
#define RECEIVE 0x00		// Control uint8_tfor receiving data
#define TRANSMIT 0x01		// Control uint8_tfor transmit data
#define SLAVE_SELECT  10	// SS pin 10
#define SLAVE_READY  8		// Hand shake pin identifying Slave has data to send
#define DELAY 12			// Delay in microseconds between uuint8_t8_ts sent

typedef struct SansgridSerial{
	uint8_t control[ CONTROL ];		// Control uuint8_t8_t
	uint8_t ip_addr[ IP_ADDRESS ];		// Overloaded IP Field contains origin or 
									// destination IP address
	uint8_t payload[ PAYLOAD ];		// payload
} SansgridSerial;

// Opens serial device for reading/writing, configures ports, sets order data 
// bits  are shifted in as MSB or LSB, and sets the clock frequency. Function 
// is called prior to sending or receiving any data over the serial line. 
uint8_t sgSerialOpen(void);
// Configure radio as a router radio
// Radio will be configured as a sensor radio by default.
// Sets radio as a "router radio" 
uint8_t sgSerialSetAsRouter(void);
// Sets radio as a "sensor radio".
uint8_t sgSerialSetAsSensor(void);
// Transmit number of uuint8_t8_ts "size" from SansgridSerial structure to Slave over 
// SPI. For full duplex a second SansgridSerial is passed in with a second size
// uint8_teger to receive data from the slave in return. The most significant bit 
// first in Big Endian uint8_torder. Number of uuint8_t8_ts is not to exceed control 
// uint8_tplus IP address destination/source uuint8_t8_ts plus payload uuint8_t8_ts. 
uint8_t sgSerialSend(SansgridSerial *sg_serial, int size);
// Receive number of uuint8_t8_ts "size" and places in SansgridSerial structure. The 
// most significant bit first in Big Endian uint8_torder. Number of uuint8_t8_ts is not 
// to exceed control uint8_tplus IP address destination/source uuint8_t8_ts plus payload
// uuint8_t8_ts. 
uint8_t sgSerialReceive(SansgridSerial *sg_serial, int size);
// Function is called when all SPI input and output is completed. Stops SPI 
// from being transmitted and received. 
uint8_t sgSerialClose(void);

#endif // __SG_SERIAL_H

