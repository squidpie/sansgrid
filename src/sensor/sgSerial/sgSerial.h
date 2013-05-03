/* Serial Communication interface
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
#include <spiMaster.h>
#include <SPI.h>

#define CONTROL 1				   // Number of bytes in control 
#define IP_ADDRESS 16			   // Number of bytes in source/destination 
                                   // IP address
#define PAYLOAD 81				   // Number of bytes in payload 
#define RECEIVE 0x00			   // Control byte for receiving data
#define TRANSMIT 0x01			   // Control byte for transmit data
#define SLAVE_SELECT  10           // SS pin 10
#define SLAVE_READY  8             // Hand shake pin identifying Slave has 
                                   // data to send
#define DELAY 6				   	   // Delay in microseconds between bytes sent

typedef struct SansgridSerial{
	byte control[ CONTROL ];	   	 	// control
	byte ip_addr[ IP_ADDRESS ];      	// Overloaded IP Field
                                        // contains origin or destination IP address
	char payload[ PAYLOAD ];	     	// payload
} SansgridSerial;

// Opens serial device for reading/writing, configures ports, sets order data 
// bits  are shifted in as MSB or LSB, and sets the clock frequency. Function 
// is called prior to sending or receiving any data over the serial line. 
int sgSerialOpen(void);
// Configure radio as a router radio
// Radio will be configured as a sensor radio by default.
// Sets radio as a "router radio" 
int sgSerialSetAsRouter(void);
// Sets radio as a "sensor radio".
int sgSerialSetAsSensor(void);
// Transmit number of bytes "size" from SansgridSerial structure to Slave over 
// SPI. For full duplex a second SansgridSerial is passed in with a second size
// integer to receive data from the slave in return. The most significant bit 
// first in Big Endian byte order. Number of bytes is not to exceed control 
// byte plus IP address destination/source bytes plus payload bytes. 
int sgSerialSend(SansgridSerial *sg_serial, int size);
// Receive number of bytes "size" and places in SansgridSerial structure. The 
// most significant bit first in Big Endian byte order. Number of bytes is not 
// to exceed control byte plus IP address destination/source bytes plus payload
// bytes. 
int sgSerialReceive(SansgridSerial *sg_serial, int size);
// Function is called when all SPI input and output is completed. Stops SPI 
// from being transmitted and received. 
int sgSerialClose(void);

#endif // __SG_SERIAL_H

