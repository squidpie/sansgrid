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

// Set up a macro depending on architecture
#ifndef __ARCH_DUE__
#define ARCH_DUE

#include <Arduino.h>
#include "sgSerial_due.h"

#define SLAVE_SELECT  10           // SS pin 10
#define SLAVE_READY  9             // Hand shake pin identifying Slave has 
                                   // data to send
#define NUM_BYTES 98			   // Number of bytes sent or received over 
                                   // SPI per packet
#define CONTROL 1				   // Number of bytes in control 
#define IP_ADDRESS 16			   // Number of bytes in source/destination 
                                   // IP address
#define PAYLOAD 81				   // Number of bytes in payload 
#define RECEIVE 0x00			   // Control byte for receiving data
#define TRANSMIT 0x01			   // Control byte for transmit data

typedef struct SansgridSerial{
	char control[CONTROL];	   	   // control
	char ip_addr[IP_ADDRESS];      // Overloaded IP Field
                                   // contains origin or destination IP address
	char payload[PAYLOAD];	       // payload
} SansgridSerial;

// Opens serial device for reading/writing, configures ports, sets order data 
// bits  are shifted in as MSB or LSB, and sets the clock frequency. Function 
// is called prior to sending or receiving any data over the serial line. 
int8_t sgSerialOpen(void){
    spiMasterInit();
	return 0;
}

// Configure radio as a router radio
// Radio will be configured as a sensor radio by default.
int8_t sgSerialSetAsRouter(void){
    // TBD
	return -1;
}

int8_t sgSerialSetAsSensor(void){
	//TBD
	return -1;
}

// Send size bytes of serialdata serially
int8_t sgSerialSend(SansgridSerial *tx, SansgridSerial *rx, int size){
	spiMasterOpen( SLAVE_SELECT );
	spiMasterTransmit( tx->control , rx->control , SLAVE_SELECT );
	spiMasterTransmit( tx->ip_addr , rx->ip_addr , SLAVE_SELECT );
	spiMasterTransmit( tx->payload , rx->payload , SLAVE_SELECT );
	spiMasterClose( SLAVE_SELECT );
	return 0;
}

// Get data from serial in. Data size will be in size.
int8_t sgSerialReceive(SansgridSerial *rx, int size){
	spiMasterOpen( SLAVE_SELECT );
	spiMasterReceive( RECEIVE , rx->control , CONTROL );
	spiMasterReceive( RECEIVE , rx->ip_addr , IP_ADDRESS );
	spiMasterReceive( RECEIVE , rx->payload , PAYLOAD );
	spiMasterClose( SLAVE_SELECT );
	return 0;
}

// Function is called when all SPI input and output is completed. Stops SPI 
// from being transmitted and received. 
int8_t sgSerialClose(void){
	SPI.end();
	return 0;
}

#else
#define ARCH_PI

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
int8_t sgSerialSend(SansgridSerial *tx, SansgridSerial *rx, uint32_t size);
// Receive number of bytes "size" and places in SansgridSerial structure. The 
// most significant bit first in Big Endian byte order. Number of bytes is not 
// to exceed control byte plus IP address destination/source bytes plus payload
// bytes. 
int8_t sgSerialReceive(SansgridSerial *rx, uint32_t size);
// Function is called when all SPI input and output is completed. Stops SPI 
// from being transmitted and received. 
int8_t sgSerialClose(void);

#endif // __SAM3X8E__
#endif // __SG_SERIAL_H

// vim: ft=c ts=4 noet sw=4:
