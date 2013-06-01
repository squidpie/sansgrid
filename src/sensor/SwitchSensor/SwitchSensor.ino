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

//Code for switch digital I/O

//sets led to pin 4 and switch to pin 5
#define SLAVE_READY 2
#define switch_led 4

//initialization
int switch_state = 0; //state of dip switch
int led_state = 0;
int sid = 0;
int led_value = 0;

SensorConfig sg_config;
SansgridSerial sg_serial;

//setting up the pins to proper i/o
void setup() {
  Serial.begin(9600);
  //Initialize interrupt for Slave Ready pin
  //#ifdef DUE 
  //attachInterrupt( SLAVE_READY , receive , RISING );
  //#else
  //attachInterrupt( 0 , receive , FALLING );
  //#endif // end of DUE
  attachInterrupt( 1 , dipswitch , CHANGE );
  
  // Set Mate, true is automatic, false is push button based
  sg_config.mate = true; 
  
  // Set SansgridSerial data_out control byte
  sg_serial.control[0] = 0xAD;
    
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
  sg_config.nest = true;
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
            Serial.println( "RECEIVING SPI PACKET" );
            sgSerialReceive( &sg_serial , 1 );
            // Process packet to verify Chirp received
            payloadHandler( &sg_config , &sg_serial);
            Serial.println( "setting received to false");
            // Reset received to default value
            sg_config.received = false;
            // Process received Chirp packet
            if( sg_config.chirp == true ){
              
              if (sg_serial.payload[0] == 0x20){
                sid = sg_serial.payload[1];
                if (sid == 0x31){
                  led_value = sg_serial.payload[2];
                  if (led_value == 0x31) //comparing value to ascii 1
                    digitalWrite(switch_led, HIGH); // turn LED on
                  else
                    digitalWrite(switch_led, LOW); // turn LED off
                }
              }
                // Reset Chirp to false
                sg_config.chirp = false; 
            }// End of received Chirp
        }
        // Code to Send Chirp
        else{
            // Set control byte to valid data
            sg_serial.control[0] = (uint8_t) 0xAD;
            // Set IP address to router ip
            memcpy( sg_serial.ip_addr , sg_config.router_ip , IP_ADDRESS );
            // Set Datatype to 0x21, Sensor to Server Chirp
            sg_serial.payload[0] = (uint8_t) 0x21;
            // Copy data into Payload
            // Which Signal Id are you using?
            sg_serial.payload[1] = (uint8_t) 0x02; //switch's sid, led's sid will be 0x01
            //if switch is on, send code to server to tell it to turn led on on other arduino
            if (switch_state == 1) {
      
              //zero out the payload to ensure no unwanted data
              for(int x = 0; x < DATA_SIZE; x++)
                sg_serial.payload[x+2] = (uint8_t) 0x0;
              char ch_array[1];
              int n = sprintf(ch_array, "%d", led_state);
              for(int i = 0; i < n; i++)
                sg_serial.payload[i+2] = ch_array[i];
              
              //led_state = 1;
              //code for TESTING
              Serial.write("led: ");
              Serial.println(led_state);
              digitalWrite(switch_led, HIGH); // turn LED on
              delay(50);
            }
          //if switch is off, send code to server to tell it to turn led on on other arduino 
            else {
      
              //zero out the payload to ensure no unwanted data
              for(int x = 0; x < DATA_SIZE; x++)
                sg_serial.payload[x+2] = (uint8_t) 0x0;
              char ch_array[1];
              int n = sprintf(ch_array, "%d", led_state);
              for(int i = 0; i < n; i++)
                sg_serial.payload[i+2] = ch_array[i];
              
              //led_state = 0;
              //code for TESTING
              Serial.write("led: ");
              Serial.println(led_state);
              digitalWrite(switch_led, LOW); // turn LED off
              delay(50); 
            }
            // Transmit Payload over SPI  
            sgSerialSend( &sg_serial , 1 );
        }// End of Send Chirp
    }// End of Nested
}

/*void receive(){
    // Interrupt was initiated when SLAVE_READY was
    // asserted low, set received flag to initiate
    // processing SPI packet
    Serial.println( "Interrupt Service Routine" );
    sg_config.received = true;
    // Display value of received
    Serial.println( "Received flag set to true" );
}*/

void dipswitch(){
  
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200) 
  {
    //led_state = !digitalRead(3);
    //digitalWrite(4, !digitalRead(4));
    led_state = !led_state;
    //Serial.println("led_state: ");
    //Serial.println(led_state);
    switch_state = led_state;
    }
 
  last_interrupt_time = interrupt_time;  

}
