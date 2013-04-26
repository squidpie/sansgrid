/* Definitions for communication functions
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
#ifndef __SPIMASTER_H__
#define __SPIMASTER_H__

#include <Arduino.h>

#define SLAVE_SELECT  10           // SS pin
#define SLAVE_READY  8             // Hand shake pin identifying Slave has 
                                   // data to send
#define NUM_BYTES 98			       // Number of bytes sent or received over 
                                   // SPI per packet
#define CONTROL 1				   // Number of bytes in control 
#define IP_ADDRESS 16			   // Number of bytes in source/destination 
                                   // IP address
#define PAYLOAD 81				   // Number of bytes in payload 
#define RECEIVE 0x00			   // Control byte for receiving data
#define TRANSMIT 0x01			   // Control byte for transmit data
#define DELAY 6				   	   // Delay

// Initialize SPI Master
void spiMasterInit( int8_t ss , int8_t sr );

// Receive data to Master from Slave
int8_t spiMasterReceive( int8_t data_out , int8_t * data_in , int8_t size , int8_t ss );

// Transmit data to Slave from Master
int8_t spiMasterTransmit( int8_t * data_out , int8_t size , int8_t ss );

// Open SPI bus for Slave Select pin
void spiMasterOpen( int8_t ss );

// Close SPI bus for Slave Select pin
void spiMasterClose( int8_t ss );

#endif // __SPIMASTER_H__