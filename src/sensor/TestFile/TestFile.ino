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

//#define PUSH_BUTTON 1
#define DUE 1

SensorConfig sg_config;
SansgridSerial sg_serial;

void setup(){
    Serial.begin(9600);
    // Initialize Slave ready pin as input
    pinMode( SLAVE_READY, INPUT );
    // Initialize interrupt for Slave Ready pin
    attachInterrupt( SLAVE_READY , receive , LOW );
    // Set Mate, true is automatic, false is push button based
    sg_config.mate = true; 
}

void loop(){
    while( !sg_config.nest ){
        // Received a Squawk packet, now send a Squawk back
        sensorConnect( &sg_config , &sg_serial );
    }
    // Sensor Module Code goes here
    
    if( sg_config.nest ){
        // Send Chirp with data to sensor
        SansgridChirp sg_chirp;
        transmitChirp( &sg_serial , &sg_chirp );
    } 
}

void receive(){
    // Interrupt was initiated when SLAVE_READY was
    // asserted low, call Payload Handler to receive
    // SPI packet from radio MCU, and process packet
    sgSerialReceive( &sg_serial, 1 );
    payloadHandler( &sg_config , &sg_serial );
}
