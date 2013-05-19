/* Sensor Payload Handler Implementation
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


#include <sgSerial.h>
#include <sensorPayloads.h>
#include <sensorPayloadHandler.h>
#include <sensorParse.h>
#include <SPI.h>

//This is the code for the sensor that takes a poteniometers value in and displays it and 
//shows the capabilities of the mating sansgrid data type in the eyeball phase. In order to
//the button must be pressed. Button not pressed = not ready to mate.

//sets led to pin 6 and switch to pin 4
#define SLAVE_READY 2
#define dip 5 //active low, needs to switch to pin 3 for interrupt

// state of the button initialized
int switch_state = 0; //state of dip switch
int led_state = 0;

int intPin = 0;

SensorConfig sg_config;
SansgridSerial sg_data_in;
SansgridSerial sg_data_out;
SansgridChirp sg_chirp;

//setting up the pins to proper i/o
void setup() {
  Serial.begin(9600);
  //Initialize interrupt for Slave Ready pin
  //#ifdef DUE 
  //attachInterrupt( SLAVE_READY , receive , RISING );
  //#else
  attachInterrupt( 0 , receive , FALLING );
  //#endif // end of DUE
  attachInterrupt( 1 , dipswitch , CHANGE );
  
  // Set Mate, true is automatic, false is push button based
  sg_config.mate = true; 
  
  // Set SansgridSerial data_out control byte
  sg_data_out.control[0] = 0xAD;
    
  // Call Sensor Configuration which sets the:
  // Serial Number, Model Number, Manufacture ID, Sensor Public Key
  // Sensor signal A and B
  //configureSensor( &sg_config );     **Said it was not declared in this scope so commented out for right now**
   
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
    }
  
    delay(1000);
    Serial.println( "Connected to Network" );
    // Signal Output Code goes here
    //delay(1000);
    //Serial.println( "Connected to Network" );
    
    //Serial.println( "Interrupt Service Routine" );
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
    // asserted low, set received flag to initiate
    // processing SPI packet
    Serial.println( "Interrupt Service Routine" );
    sg_config.received = true;
    // Display value of received
    Serial.println( sg_config.received );
}

void dipswitch(){
  
    //dip switch to led code
    switch_state = digitalRead(dip); //reads current state of dip switch    
    Serial.println(switch_state);
    delay(100);
    
    //if switch is on, send code to server to tell it to turn led on on other arduino
    if (switch_state == 1) {
      
      sg_chirp.dt [0] = (uint8_t) 0x21;
      sg_chirp.id [0] = (uint8_t) 0x01;
      memset(sg_chirp.data, '\0', DATA_SIZE);
      char ch_array[1];
      int n;
      n = sprintf(ch_array, "%d", 1);
      for(int i = 0; i < n; i++)
      sg_chirp.data[i] = ch_array[i];
  
      transmitChirp( &sg_data_out , &sg_chirp);

      /*led_state = 1;
      Serial.write("led: ");
      Serial.println(led_state);
      digitalWrite(switch_led, HIGH); // turn LED on
      delay(50);*/
        
    //if switch is off, send code to server to tell it to turn led on on other arduino 
    } 
    else {
      
      sg_chirp.dt [0] = (uint8_t) 0x21;
      sg_chirp.id [0] = (uint8_t) 0x01;
      memset(sg_chirp.data, '\0', DATA_SIZE);
      char ch_array[1];
      int n;
      n = sprintf(ch_array, "%d", 0);
      for(int i = 0; i < n; i++)
      sg_chirp.data[i] = ch_array[i];
      
      transmitChirp( &sg_data_out , &sg_chirp);
      
      /*led_state = 0;
      Serial.write("led: ");
      Serial.println(led_state);
      digitalWrite(switch_led, LOW); // turn LED off
      delay(50); */
    }
}
