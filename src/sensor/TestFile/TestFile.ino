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
#define DUE 1

SensorConfig sg_config;
SansgridSerial sg_data_in;
SansgridSerial sg_data_out;

int intPin = 0;

void setup(){
    // This allows for debugging by displaying information 
    // in Arduino IDE terminal, just click the eyeglass top
    // right to see output after programming.
    Serial.begin(9600); 
    
    // Set Mate, true is automatic, false is push button based
    //sg_config.mate = true; 
    
    // Set SansgridSerial data_out control byte
    sg_data_out.control[0] = 0xAD;
    
    // Initialize interrupt for Slave Ready pin
    #ifdef DUE 
    attachInterrupt( SLAVE_READY , receive , RISING );
    #else
    attachInterrupt( 0 , receive , FALLING );
    #endif // end of DUE
    
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
    //sg_config.mock = true;
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
        sensorConnect( &sg_config , &sg_data_out );  
        delayMicroseconds(DELAY);
    }
  
    delay(1000);
    Serial.println( "Connected to Network" );
    
    // Signal Input Code goes here in this loop
    while(sg_config.nest == true ){
        delay(1000);
    }
    
}

void receive(){
    // Interrupt was initiated when SLAVE_READY was
    // asserted low, set received flag to initiate
    // processing SPI packet
    Serial.println( "Interrupt Service Routine" );
    sg_config.received = true;
    // Display value of received
    Serial.println( "Received flag set to true" );
}
