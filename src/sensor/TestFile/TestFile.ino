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
#define LED 4

SensorConfig sg_config;
SansgridSerial sg_serial;
boolean Send = false;

void setup(){
    // This allows for debugging by displaying information 
    // in Arduino IDE terminal, just click the eyeglass top
    // right to see output after programming.
    Serial.begin(9600); 
    
    // Set Mate, true is automatic, false is push button based
    //sg_config.mate = true; 
    
    // Enable Slave Select
    pinMode( SLAVE_SELECT , OUTPUT );
    digitalWrite( SLAVE_SELECT , HIGH );
    
    // LED Signal
    pinMode( LED , OUTPUT );
    digitalWrite( LED , HIGH );
    
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
    //sg_config.nest = true;
    //sg_config.fly = true;
    //sg_config.sing = true;
    //sg_config.mock = true; 
    //sg_config.squawk = true;
    //sg_config.chirp = true;
    //sg_config.nokey = true;
    //sg_config.challenge = true;
    //sg_config.received = true;
    //sg_serial.payload[0] = (uint8_t) 0xF0;
    //sg_config.forget = false;
}

void loop(){
    // If not connected to network, nest will be false.
    // Attempt to connect to network untill nested and
    // nest flag is true.
    while( sg_config.nest == false ){
        //Serial.println(freeRam());
        sensorConnect( &sg_config , &sg_serial );  
    }
    // DEBUG message
    Serial.println( "Connected to Network" );
    // Signal Input Code goes here in this loop
    while(sg_config.nest == true ){  
        // Delay between sending Packets atleast 1 second
        delay(1000);
        // Received packet over SPI
        if ( sg_config.received == true ){
            // Received Packet
            //Serial.println( "RECEIVING SPI PACKET" );
            sgSerialReceive( &sg_serial , 1 );
            // Test code
            Serial.println( sg_serial.payload[0] , HEX );
            // Process packet to verify Chirp received
            payloadHandlerB( &sg_config , &sg_serial);
            //Serial.println( "setting received to false");
            // Reset receive d to default value
            sg_config.received = false;
            // Process received Chirp packet
            if( sg_config.chirp == true ){
                Serial.println( "Chirp Received" );
                // Received Chirp from Sensor
                // Need to process payload to perform action
                // on Signal. Put code in here.
                if( sg_serial.payload[0] == 0x20 ){
                    if( sg_serial.payload[1] == 0x01 ){
                        if( sg_serial.payload[2] == 0x31 )
                            digitalWrite( LED , HIGH );
                        else
                            digitalWrite( LED , LOW );
                  }
                }      
                // Reset Chirp to false
                sg_config.chirp = false;
                Send = true; 
            }// End of received Chirp
        }
        // Code to Send Chirp
        if (  Send == true ){
            // Testing Update
            Serial.println( "Updated Light Status" );
            // Set control byte to valid data
            sg_serial.control[0] = (uint8_t) 0xAD;
            // Set IP address to router ip
            memcpy( sg_serial.ip_addr , sg_config.router_ip , IP_ADDRESS );
            // Set Datatype to 0x21, Sensor to Server Chirp
            sg_serial.payload[0] = (uint8_t) 0x21;
            // Copy data into Payload
            // Which Signal Id are you using?
            sg_serial.payload[1] = (uint8_t) 0x02;
            // What are you transmitting???
            if(digitalRead( LED ) == LOW ){
                sg_serial.payload[2] = (uint8_t) 0x4F;
                sg_serial.payload[3] = (uint8_t) 0x46;
                sg_serial.payload[4] = (uint8_t) 0x46;
            }
            else if (digitalRead( LED ) == HIGH ){
                sg_serial.payload[2] = (uint8_t) 0x4F;
                sg_serial.payload[3] = (uint8_t) 0x4E;
                sg_serial.payload[4] = (uint8_t) 0x20;
            }
            // Make sure to pad the unused with 0x00
            for( int i = 5 ; i < PAYLOAD ; i++ )
                sg_serial.payload[i] = (uint8_t) 0x00;
            // Transmit Payload over SPI  
            if( Send == true ){
                sgSerialSend( &sg_serial , 1 );
                Send = false;
            }
        }// End of Send Chirp
    }// End of Nested
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
