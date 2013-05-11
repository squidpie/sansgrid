/* SPI Master Test Code
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
 *    SLAVE_SELECT  10	  // SS pin
 *    SLAVE_READY  8      // Hand shake pin identifying Slave has
 *     
 */
 
#include <sensorPayloadHandler.h>
#include <sgSerial.h>
#include <sensorPayloads.h>
#include <sensorParse.h>
#include <SPI.h>

#define LED 13
#define NUM_BYTES 98
#define SLAVE_READY 8

SensorConfig sg_config;
SansgridSerial sg_serial;

byte data_in[NUM_BYTES];
byte rec = 0xFD;

void setup(){
    Serial.begin(9600);
    // Initialize Sensor SPI communication
    SPI.begin();
    // Set order bits are shifted onto the SPI bus
    SPI.setBitOrder( MSBFIRST ); 
    // Set SPI Baud Rate to 500 KHz
    // 84 MHz / 252 = 500 KHz
    SPI.setClockDivider( 168 );
    // Set SPI Mode 0-3
    SPI.setDataMode( SPI_MODE0 );
    // Initialize Slave ready pin as input
    pinMode( SLAVE_READY, INPUT );
    // Initialize interrupt for Slave Ready pin
    attachInterrupt( SLAVE_READY , receive , LOW );
}

void loop(){    
}

void receive(){
    // Interrupt was initiated when SLAVE_READY was
    // asserted low, call Payload Handler to receive
    // SPI packet from radio MCU, and process packet
    /*SPI.transfer( rec );
    delayMicroseconds(60);
    for( int i = 0 ; i < NUM_BYTES ; i++){
        data_in[i] = SPI.transfer( rec );
        delayMicroseconds(60);
    }
    for( int i = 0 ; i < NUM_BYTES ; i++)
        Serial.println( data_in[i] );*/
    payloadHandler( &sg_config );
}
