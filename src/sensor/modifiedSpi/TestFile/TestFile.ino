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
//#define LED 13

SensorConfig sg_config;
SansgridSerial sg_serial;

void setup(){
    // This allows for debugging by displaying information 
    // in Arduino IDE terminal, just click the eyeglass top
    // right to see output after programming.
    Serial.begin(9600); 
    
    // Set Mate, true is automatic, false is push button based
    //sg_config.mate = true; 
    
    // Enable Slave Select
    pinMode(SLAVE_SELECT, OUTPUT);
    digitalWrite(SLAVE_SELECT, HIGH);
    // Set SansgridSerial data_out control byte
    sg_serial.control[0] = 0xAD;
    
    // Delay to setup Radio
    delay(5000);
    
    // Initialize interrupt for Slave Ready pin
    #ifdef DUE 
    attachInterrupt( SLAVE_READY , receive , FALLING );
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
    // and uncomment all sg_config.<fly,sing,squawk,Chirp,challenge...>
    // either false or true.
    //sg_config.mate = false;
    //sg_config.nest = false;
    sg_config.fly = true;
    //sg_config.sing = true;
    //sg_config.mock = true;
    //sg_config.squawk = true;
    //sg_config.chirp = true;
    //sg_config.nokey = true;
    //sg_config.challenge = true;
    //sg_config.received = true;
    //sg_serial.payload[0] = (uint8_t) 0xF0;
	sg_config.received = false;
	
}

void loop(){
	if ( sg_config.received == true ){
            // Received Packet
            sgSerialReceive( &sg_serial , 1 );
            // Process packet to verify Chirp received
            payloadHandler( &sg_config , &sg_serial);
            // Reset received to default value
            sg_config.received = false;
    }
	/*else {
	 // Set control byte to valid data
            sg_serial.control[0] = (uint8_t) 0xAD;
            // Set IP address to router ip
            memcpy( sg_serial.ip_addr , sg_config.router_ip , IP_ADDRESS );
            // Set Datatype to 0x21, Sensor to Server Chirp
			memset(sg_serial.payload, 0, 81);
            sg_serial.payload[0] = (uint8_t) 0x21;
            sgSerialSend( &sg_serial , 1 );
			delay(10000);
	}*/
}// End of Loop

void receive(){
    // Interrupt was initiated when SLAVE_READY was
    // asserted low, set received flag to initiate
    // processing SPI packet
    //Serial.println( "Interrupt Service Routine" );
    sg_config.received = true;
    // Display value of received
    //Serial.println( "Received flag set to true" );
}



    // If not connected to network, nest will be false.
    // Attempt to connect to network untill nested and
    // nest flag is true.
	
