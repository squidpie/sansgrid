/* System Test Sensor 2 
 * Ashley Murray
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

// libraries made to allow sensor code to work with Sangride protocol
#include <sgSerial.h>
#include <sensorPayloads.h>
#include <sensorPayloadHandler.h>
#include <sensorParse.h>
#include <SPI.h>

// This is the code for a pair of sensors that each have an LED and a switch. The switch 
// is interrupt driven and sets the state of a variable accordingly. The state 
// determines if a command is sent out to turn the LED on the other sensor on or off. A
// trigger system would be implemented in order to make it work. This code was not used
// during the demonstration of the capstone project due to hardware issues.

//sets the switch led to pin 9
#define SWITCH_LED 9

// state of dip switch
int switch_state = 0;
// state of led
int led_state = 0;      

// local instances of functions
SensorConfig sg_config;
SansgridSerial sg_serial;
boolean Send = false;

// variable for how often to send a chirp 
int32_t time = 0;

// setting up the pins to proper i/o
void setup() {
  
  // initializes led to output and turns it off
  pinMode(SWITCH_LED, OUTPUT); 
  digitalWrite(SWITCH_LED, LOW);
  
  // initializes slave select as output
  pinMode(SLAVE_SELECT, OUTPUT);
  digitalWrite(SLAVE_SELECT, LOW);  
  
  // This allows for debugging by displaying information 
  // in Arduino IDE terminal, just click the eyeglass top
  // right to see output after programming.
  Serial.begin(9600);
  
  // initialize interrupt for Slave Ready pin
  #ifdef DUE 
  attachInterrupt( SLAVE_READY , receive , FALLING );
  #else
  attachInterrupt( 0 , receive , FALLING );
  #endif // end of DUE
  
  // switch interrupt
  attachInterrupt( 1 , dipSwitch , RISING );
  
  // Set Mate, true is automatic, false is push button based
  //sg_config.mate = true; 
   
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
  sg_config.nokey = true;
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

        // Received packet over SPI
        if ( sg_config.received == true ){
            // received packet
            sgSerialReceive( &sg_serial , 1 );
            // process packet to verify Chirp received
            payloadHandlerB( &sg_config , &sg_serial);
            // reset received to default value
            sg_config.received = false;
            
            // process received Chirp packet
            if( sg_config.chirp == true ){
              Serial.println("Chirp Received");
              
              // Received Chirp from Sensor
              // Need to process payload to perform action
              // on Signal. Put code in here.
              // data coming from server
              if (sg_serial.payload[0] == 0x20){
                //sensor id = b  
                  if(sg_serial.payload[1] == 0x02){     
                    //if data = 1, turn on led    
                    if(sg_serial.payload[2] == 0x31)   
                            digitalWrite(SWITCH_LED, HIGH);      
                    else
                            digitalWrite(SWITCH_LED, LOW); //turn led off
                }
              }
                // reset Chirp to false
                sg_config.chirp = false; 
                Send = true;
            }// end of received Chirp
        }
        // code to send Chirp
        if (time == 0){
            // set control byte to valid data
            sg_serial.control[0] = (uint8_t) 0xAD;
            // set ip address to router ip
            memcpy( sg_serial.ip_addr , sg_config.router_ip , IP_ADDRESS );
            // data from sensor to server
            sg_serial.payload[0] = (uint8_t) 0x21;
            // signal id = a
            sg_serial.payload[1] = (uint8_t) 0x01; 
            
            // if switch is on, send code to server to tell it to 
            // turn led on on other sensor
            if (switch_state == 1) {
      
              // zero out the payload to ensure no unwanted data
              for(int x = 0; x < DATA_SIZE; x++)
                sg_serial.payload[x+2] = (uint8_t) 0x0;
              // put a 1 in payload data to send to server  
              char ch_array[1];
              int n = sprintf(ch_array, "%d", led_state);
              for(int i = 0; i < n; i++)
                sg_serial.payload[i+2] = ch_array[i];
              
              // DEBUG code
              //led_state = 1;
              //Serial.write("led: ");
              //Serial.println(led_state);
              //digitalWrite(SWITCH_LED, HIGH); // turn LED on
            }
          // if switch is off, send code to server to tell it to turn 
          // led on on other sensor 
            else {
      
              // zero out the payload to ensure no unwanted data
              for(int x = 0; x < DATA_SIZE; x++)
                sg_serial.payload[x+2] = (uint8_t) 0x0;
              char ch_array[1];
              // put a 0 in payload data to send to server
              int n = sprintf(ch_array, "%d", led_state);
              for(int j = 0; j < n; j++)
                sg_serial.payload[j+2] = ch_array[j];
              
              // DEBUG code
              //led_state = 0;
              //Serial.write("led: ");
              //Serial.println(led_state);
              //digitalWrite(SWITCH_LED, LOW); // turn LED off
            }
            // transmit payload over SPI  
            sgSerialSend( &sg_serial , 1 );
        }// end of send Chirp
        
        // increment time so chirp is only sent every so often
        time++;
        // chirp sent approx. every 10s
        if (time >= 3000000)
          time = 0;
          
    }// end of Nested
}

void receive(){
    // Interrupt was initiated when SLAVE_READY was
    // asserted low, set received flag to initiate
    // processing SPI packet
    //Serial.println( "Interrupt Service Routine" );
    sg_config.received = true;
}

void dipSwitch(){
  
  // debouncing code
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a 
  // bounce and ignore
  if (interrupt_time - last_interrupt_time > 200) 
  {
    // sets switch state to opposite of whatever it just was
    led_state = !led_state;
    
    //DEBUG code
    //Serial.println("led_state: ");
    //Serial.println(led_state);
    
    // determines state of switch so it is known if a
    // 1 or a 0 should be sent to turn the led on the
    // other sensor on/off
    switch_state = led_state;
    }
 
  last_interrupt_time = interrupt_time;  

}
