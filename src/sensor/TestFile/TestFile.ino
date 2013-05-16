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
 *    SLAVE_SELECT  10 - SS pin
 *    SLAVE_READY  8 - Hand shake pin identifying Slave is ready to transmit on DUE
 *    SLAVE_READY  2 - Hand shake pin identifying Slave is ready to transmit on UNO
 *     
 */
 
#include <sensorPayloadHandler.h>
#include <sgSerial.h>
#include <sensorPayloads.h>
#include <sensorParse.h>
#include <SPI.h>

//#define PUSH_BUTTON 1
//#define DUE 1

SensorConfig sg_config;
SansgridSerial sg_data_in;
SansgridSerial sg_data_out;

void setup(){
    // This allows for debugging by displaying information 
    // at Arduino IDE terminal, just click the eyeglass top
    // right to see output after programming.
    Serial.begin(9600); 
    // Initialize Slave ready pin as input
    pinMode( SLAVE_READY, INPUT_PULLUP );
    // Initialize interrupt for Slave Ready pin
    #ifdef DUE 
    //attachInterrupt( SLAVE_READY , receive , LOW );
    #else
    //attachInterrupt( int.0 , receive , LOW );
    #endif // end of DUE
    // Set Mate, true is automatic, false is push button based
    //sg_config.mate = true; 
    // Set SansgridSerial data_out control byte
    sg_data_out.control[0] = 0xAD;
    // Call Sensor Configuration which sets the:
    // Serial Number, Model Number, Manufacture ID, Sensor Public Key
    // Sensor signal A and B
    //configureSensor( &sg_config );
    
    // Commenting and Uncommenting these will intiate 
    // sending Test Packets, these will not be needed in final
    // code it is strictly for test purposes. Will only send a single
    // packet. To run through entire process, go to payloadHandler
    // and uncomment all sg_config.<fly,sing,squawk,chirp,challenge...>
    // either false or true.
    //sg_config.mate = false;
    //sg_config.nest = true;
    //sg_config.fly = true;
    //sg_config.sing = true;
    sg_config.mock = true;
    //sg_config.squawk = true;
    //sg_config.chirp = true;
    //sg_config.nokey = true;
    //sg_config.challenge = true;
    
}

void loop(){
    // If not connected to network, nest will be false.
    // Attempt to connect to network untill nested and
    // nest flag is true.
    while( sg_config.nest == false ){
        delay(1000);
        Serial.println( "Conecting to Network");
        sensorConnect( &sg_config , &sg_data_out );
    }
    // Signal Output Code goes here
    //delay(1000);
    //Serial.println( "Connected to Network" );
    
    Serial.println( "Interrupt Service Routine" );
    //sgSerialReceive( &sg_data_in, 1 );
    
    // Testing Squawk - remove for final source code
    /*sg_data_in.control[0] = (uint8_t) 0xAD;
    // Set IP address to router ip
    memcpy( &sg_data_in.ip_addr , &sg_config.router_ip , IP_ADDRESS );
    sg_data_in.payload[0] = 0x12;
    payloadHandler( &sg_config , &sg_data_in );
    delay(1000);
    // Received Squawk payload, respond to 
    // squawk before leaving interrupt.
    while( sg_config.squawk == true ){
        // Received a Squawk packet, now send a Squawk back
        Serial.println( "RETURN SQUAWKING" );
        // Set control byte to valid data
	sg_data_in.control[0] = (uint8_t) 0xAD;
        // Set IP address to router ip
	memcpy( &sg_data_in.ip_addr , &sg_config.router_ip , IP_ADDRESS );
	// Call Payload Handler to send return squawk
        payloadHandler( &sg_config , &sg_data_in);
    }  
    // Signal Input Code goes here
    while(1){
        delay(1000);
    }*/
    // Remove to Here
}

void receive(){
    // Interrupt was initiated when SLAVE_READY was
    // asserted low, call sgSerialReceive to receive
    // SPI packet from radio MCU, and process packet
    // with payloadHandler().
    Serial.println( "Interrupt Service Routine" );
    sgSerialReceive( &sg_data_in, 1 );
    payloadHandler( &sg_config , &sg_data_in );
    // Received Squawk payload, respond to 
    // squawk before leaving interrupt.
    while( sg_config.squawk == true ){
        // Received a Squawk packet, now send a Squawk back
        Serial.println( "RETURN SQUAWKING" );
        // Set control byte to valid data
	sg_data_in.control[0] = (uint8_t) 0xAD;
        // Set IP address to router ip
	memcpy( &sg_data_in.ip_addr , &sg_config.router_ip , IP_ADDRESS );
	// Call Payload Handler to send return squawk
        payloadHandler( &sg_config , &sg_data_in);
    }  
    // Signal Input Code goes here
}
