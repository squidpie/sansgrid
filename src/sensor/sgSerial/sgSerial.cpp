/* Serial Communication Implementation
 * Specific to the Arduino Platform
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 */

#include <Arduino.h>
#include "sgSerial.h"

//#define DUE 1
 
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
    #ifdef DUE
    SPI.setClockDivider( 252 );
    #else
    SPI.setClockDivider( SPI_CLOCK_DIV32 );
    #endif // DUE
    // Set SPI Mode 0-3
    SPI.setDataMode( SPI_MODE0 );
    return 0;
}

// Configure radio as a router or sensor radio
// Radio will be configured as a sensor radio by default.
uint8_t sgSerialSetAsRouter(void){
    // TBD
    return -1;
}

uint8_t sgSerialSetAsSensor(void){
    //TBD
    return -1;
}

// Send size bytes of serial data over SPI.
uint8_t sgSerialSend(SansgridSerial *sg_serial, int size ){
    // Buffer to store data array to send to Slave over SPI
    uint8_t data_out[ NUM_BYTES ];
    // Copy SansgridSerial data to buffer
    memcpy( data_out , sg_serial->control, CONTROL );
    memcpy( data_out + CONTROL , sg_serial->ip_addr, IP_ADDRESS  );
    memcpy( data_out + CONTROL + IP_ADDRESS , sg_serial->payload, PAYLOAD );
    // Open SPI bus
    sgSerialOpen();
	// Delay to allow SPI bus to Initiate
    delayMicroseconds(DELAY);
	// Assert Slave Select (Chip Enable, Chip Select)
	digitalWrite( SLAVE_SELECT , LOW );
    // Send dummy byte to Set command on Slave
    SPI.transfer( data_out[0] );
	// Delay to allow Slave to process byte
    delayMicroseconds(DELAY);
    // Loop through buffer sending one byte at a time over SPI
    for( int i = 0 ; i < NUM_BYTES ; i++){
        // Send a byte over SPI
        SPI.transfer( data_out[i] );
		// Delay to allow Slave to process byte
        delayMicroseconds(DELAY);
    }
	// Place Slave Select High again (Chip Enable, Chip Select)
	digitalWrite( SLAVE_SELECT , HIGH );
	// Close SPI bus - NOT USED
    //sgSerialClose();
    return 0;
}

// Receive size bytes of serial data over SPI.
uint8_t sgSerialReceive(SansgridSerial *sg_serial, int size){
    // Array of size NUM_BYTES to store SPI packet
    uint8_t data_in[NUM_BYTES];
    // Dummy byte sent to slave 
    uint8_t rec = 0xFD;
    // Open SPI bus
    sgSerialOpen();
	// Delay to allow SPI bus to Initiate
    delayMicroseconds(DELAY);
    // Assert Slave Select (Chip Enable, Chip Select)
	digitalWrite( SLAVE_SELECT , LOW );
	// First dummy transfer defines the command 
    // for valid or not valid data
    SPI.transfer( rec );
	// Delay to allow Slave to process byte
    delayMicroseconds(DELAY);
    // Second dummy transfer allows the first 
    // byte transferred from Slave to be placed
    // in SPDR register and will be stored on the
    // next SPI.transfer() in the for Loop.
    SPI.transfer( rec );
    // Delay to allow Slave to process byte
	delayMicroseconds(DELAY);
    // Loop through receiving bytes the length 
    // of packet defined as NUM_BYTES
    for( int i = 0 ; i < NUM_BYTES ; i++){
        // Send a byte over SPI and store
        // byte received in data_in buffer
        data_in[i] = SPI.transfer( rec );
		// Delay to allow Slave to process byte
        delayMicroseconds(DELAY);
    }
	// Place Slave Select High again (Chip Enable, Chip Select)
	digitalWrite( SLAVE_SELECT , HIGH );
    // Close SPI bus - NOT USED
    //sgSerialClose();
	// Copy data from array into SansgridSerial structure
    // containing Control byte, IP address, and Payload
    // Copy SansgridSerial data to buffer
    memcpy( sg_serial->control , data_in , CONTROL );
    memcpy( sg_serial->ip_addr , data_in + CONTROL , IP_ADDRESS  );
    memcpy( sg_serial->payload , data_in + CONTROL + IP_ADDRESS , PAYLOAD );
    return 0;
}

// Function is called when all SPI input and output is completed. Stops SPI 
// from being transmitted and received. 
uint8_t sgSerialClose(void){
    SPI.end();
    return 0;
}
