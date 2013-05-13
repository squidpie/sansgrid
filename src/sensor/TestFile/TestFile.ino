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
SansgridSerial sg_data_in;
SansgridSerial sg_data_out;

void setup(){
    Serial.begin(9600);
    // Initialize Slave ready pin as input
    //pinMode( SLAVE_READY, INPUT );
    // Initialize interrupt for Slave Ready pin
    //attachInterrupt( SLAVE_READY , receive , LOW );
    // Set Mate, true is automatic, false is push button based
    sg_config.mate = true; 
    // Set SansgridSerial data_out and data_in control byte
    sg_data_in.control[0] = 0xFD;
    sg_data_out.control[0] = 0xAD;
    // Setup Sensor Module Configurations
    // Sensor A
    sg_config.a.id = { 0x01 };
    // Classification : 0x00 = digital, 0x01 = analog
    sg_config.a.classification = { 0x01 };
    // Direction: 0x00 from Sensor to Server (example: Temperature reading)
    // 0x01 from Server to Sensor (example: Turn light on and off)
    sg_config.a.direction = { 0x00 };
    // Sensor label 30 characters or less
    // Change only value in the array char label[] = "<value>"
    char label[] = "Temperature";
    for( int i = 0 ; i < LABEL - sizeof(label) + 1 ; i++ )
        sg_config.a.label[i] = (int8_t) 0x00;
    memcpy( sg_config.a.label + sizeof(label) - 1 , label , sizeof(label));
    // Sensor units 6 characters or less
    // Change only value in the array char units[] = "<value>"
    char units[] = "deg F";
    for( int i = 0 ; i < LABEL - sizeof(units) + 1 ; i++ )
        sg_config.a.units[i] = (uint8_t) 0x00;
    memcpy( sg_config.a.units + sizeof(units) - 1 , units , sizeof(units));
    // Sensor B
    sg_config.b.id = { 0x00 };
    sg_config.b.classification = { 0x00 };
    sg_config.b.direction = { 0x00 };
    sg_config.b.label = { 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    sg_config.b.units = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    
    // Test Packets
    sg_config.nest = false;
    //sg_config.fly = true;
    //sg_config.sing = true;
    sg_config.mock = true;
    //sg_config.squawk = true;
    //sg_config.chirp = true;
    //sg_config.challenge = true;
    
}

void loop(){
    while( sg_config.nest == false ){
        // Received a Squawk packet, now send a Squawk back
        Serial.println( "connect");
        sensorConnect( &sg_config , &sg_data_out );
    }
    // Sensor Module Code goes here
    
    if( sg_config.nest ){
        // Send Chirp with data to sensor
        SansgridChirp sg_chirp;
        transmitChirp( &sg_data_out , &sg_chirp );
    } 
}

void receive(){
    // Interrupt was initiated when SLAVE_READY was
    // asserted low, call Payload Handler to receive
    // SPI packet from radio MCU, and process packet
    //sgSerialReceive( &sg_data_in, 1 );
    //payloadHandler( &sg_config , &sg_data_in );
}
