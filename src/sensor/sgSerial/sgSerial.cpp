/* Serial Communication implementation
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

#include <Arduino.h>
#include "sgSerial.h"
 
// Opens serial device for reading/writing, configures ports, sets order data 
// bits  are shifted in as MSB or LSB, and sets the clock frequency. Function 
// is called prior to sending or receiving any data over the serial line. 
uint8_t sgSerialOpen(void){
    // Initialize Sensor SPI communication
    SPI.begin();
    // Set order bits are shifted onto the SPI bus
    SPI.setBitOrder( MSBFIRST ); 
    // Set SPI Baud Rate to 500 KHz
    // 84 MHz / 252 = 500 KHz
    SPI.setClockDivider( 168 );
    // Set SPI Mode 0-3
    SPI.setDataMode( SPI_MODE0 );
	return 0;
}

// Configure radio as a router radio
// Radio will be configured as a sensor radio by default.
uint8_t sgSerialSetAsRouter(void){
    // TBD
	return -1;
}

uint8_t sgSerialSetAsSensor(void){
	//TBD
	return -1;
}

// Send size uint8_ts of serialdata serially
uint8_t sgSerialSend(SansgridSerial *sg_serial, int size ){
	uint8_t data_out[ NUM_BYTES ];
	memcpy( data_out , sg_serial->control , CONTROL );
	memcpy( data_out + 1 , sg_serial->ip_addr , IP_ADDRESS );
	memcpy( data_out + 17 , sg_serial->payload , PAYLOAD );
    for( int i = 0 ; i < NUM_BYTES ; i++){
        SPI.transfer( data_out[i] );
        delayMicroseconds( DELAY );
    }
	return 0;
}

// Get data from serial in. Data size will be in size.
uint8_t sgSerialReceive(SansgridSerial *sg_serial, int size){
	uint8_t data_in[NUM_BYTES];
	uint8_t rec = 0xFD;
	SPI.transfer( rec );
    delayMicroseconds( DELAY );
	for( int i = 0 ; i < NUM_BYTES ; i++){
        data_in[i] = SPI.transfer( rec );
        delayMicroseconds( DELAY );
    }
	memcpy( sg_serial->control , data_in , CONTROL );
	memcpy( sg_serial->ip_addr , data_in + 1 , IP_ADDRESS );
	memcpy( sg_serial->payload , data_in + 17 , PAYLOAD );
	return 0;
}

// Function is called when all SPI input and output is completed. Stops SPI 
// from being transmitted and received. 
uint8_t sgSerialClose(void){
	SPI.end();
	return 0;
}
