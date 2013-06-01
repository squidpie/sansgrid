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

//sets led to pin 6 and button to pin 7
#define mate_led 6 //active low
#define SLAVE_READY 2

// state of the button initialized
boolean led_state = true;

const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to
const int analogOutPin = 9; // Analog output pin that the LED is attached to

int sensorValue = 0;        // value read from the pot
int outputValue = 0;        // value read from led
int percentValue = 0;       // value sent to server

SensorConfig sg_config;
SansgridSerial sg_serial;

//setting up the pins to proper i/o
void setup() {
  // initialize the button led as output:
  pinMode(mate_led, OUTPUT); 
  pinMode(SLAVE_SELECT, OUTPUT);
  digitalWrite(SLAVE_SELECT, LOW);  
  // initialize led to off
  digitalWrite(mate_led, HIGH); 
  Serial.begin(9600);
  //Initialize interrupt for Slave Ready pin
  //#ifdef DUE 
  //attachInterrupt( SLAVE_READY , receive , RISING );
  //#else
  //attachInterrupt( 0 , receive , FALLING );
  //#endif // end of DUE
  attachInterrupt( 1 , buttonpress , RISING );
  
  // Set Mate, true is automatic, false is push button based
  sg_config.mate = false; 
  
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
  //sg_config.nest = true;
  sg_config.fly = true;
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
                // if (sg_serial.payload[0] == 0x20){
                //blah blah blah}
                //check for sid at [1]
                //last position is at [2]
                //Received Chirp from Sensor
                // Need to process payload to perform action
                // on Signal. Put code in here.
              
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
            sg_serial.payload[1] = (uint8_t) 0x01;
            // read the analog in value:
            sensorValue = analogRead(analogInPin);            
            // map it to the range of the analog out:
            outputValue = map(sensorValue, 0, 681, 0, 255); 
            percentValue = map(sensorValue, 0, 681, 0, 100);
            // change the analog out value:
            analogWrite(analogOutPin, outputValue);
            //zero out the payload to ensure no unwanted data
            for(int x = 0; x < DATA_SIZE; x++)
              sg_serial.payload[x+2] = 0;
            //code to setup value to send to server  
            int analog_size = sizeof(percentValue);
            char ch_array[analog_size];
            int n;
            n = sprintf(ch_array, "%d", percentValue);
            for(int i = 0; i < n; i++)
              sg_serial.payload[i+2] = ch_array[i];
            
            
            
            
            //print the results to the serial monitor for TESTING:
            Serial.println(ch_array);     
            Serial.print("percent lit up = ");
            Serial.println(percentValue);
            // Transmit Payload over SPI
            delay (10000);  
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

void buttonpress(){
  
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200) 
  {
    if(led_state){
      digitalWrite( mate_led, LOW );
      led_state = false;
      //Serial.println("led is on");
      sg_config.mate = true;
      //Serial.println("mating state: ");
      //Serial.println(sg_config.mate);
    } else{
      digitalWrite( mate_led, HIGH );
      led_state = true;
      //Serial.println("led is off");
      sg_config.mate = false;
      //Serial.println("mating state: ");
      //Serial.println(sg_config.mate);
    }
  }
  last_interrupt_time = interrupt_time;
  
}

