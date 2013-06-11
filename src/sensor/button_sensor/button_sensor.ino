/* System Test Sensor 1
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

// This is the code for a sensor that has a potentiometer, 3 LEDs, and a push button. 
// The poteniometer's value is taken in, and mapped from 0-100. The mapped value is sent 
// to the server to be displayed. The potentiometer's value is also mapped from 0-255 to 
// set the brightness of an LED accordingly. The push button is used to demonstrate the 
// mating capabilities of the SansGrid protocol. An LED is turned on when mate is ready.
// Off when mate is not ready. The yellow LED is used to for showcasing the trigger 
// system. Turns on when it gets a 1, off when it gets a 0. 

// sets led to pin 6, active low
#define MATE_LED 6
//yellow led set to pin 5, used to demonstrate trigger system
#define LED 5 

// state of the mating led initialized
boolean led_state = true;

// analog input pin that the potentiometer is attached to
const int analog_in_pin = A0; 
// analog output pin that the LED is attached to
const int analog_out_pin = 9; 

// value read from the pot
int sensor_value = 0; 
// value read from led
int output_value = 0; 
// value sent to server
int percent_value = 0;       

// local instances of functions
SensorConfig sg_config;
SansgridSerial sg_serial;
boolean Send = false;

// variable for how often to send a chirp 
int32_t time = 0;

// setting up the pins to proper i/o
void setup() {
  // This allows for debugging by displaying information 
  // in Arduino IDE terminal, just click the eyeglass top
  // right to see output after programming.
  Serial.begin(9600);
  
  // initialize the button led as output and turned off
  pinMode(MATE_LED, OUTPUT); 
  digitalWrite(MATE_LED, LOW); 
  
  // initialize slave select as output
  pinMode(SLAVE_SELECT, OUTPUT);
  digitalWrite(SLAVE_SELECT, HIGH);  
  
  // yellow led signal, initialized as output and turned off
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
 
  // initialize interrupt for Slave Ready pin
  #ifdef DUE 
  attachInterrupt(SLAVE_READY, receive, FALLING);
  #else
  attachInterrupt(0, receive, FALLING);
  #endif // end of DUE
  
  // button interrupt
  attachInterrupt(1, buttonPress, RISING);
   
  // set mate, true is automatic, false is push button based
  sg_config.mate = false; 
   
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
    while(sg_config.nest == false){
        sensorConnect(&sg_config, &sg_serial);  
    }
  
    // DEBUG message
    Serial.println("Connected to Network");
    
    // Signal Input Code goes here in this loop
    while(sg_config.nest == true){  
        
        // received packet over SPI
        if (sg_config.received == true){
            // received packet
            sgSerialReceive(&sg_serial, 1);
            // process packet to verify Chirp received
            payloadHandlerB(&sg_config, &sg_serial);
            // reset received to default value
            sg_config.received = false;
            
            // process received Chirp packet
            if(sg_config.chirp == true){
                Serial.println("Chirp Received");      
                
                // Received Chirp from Sensor
                // Need to process payload to perform action
                // on Signal. Put code in here.
                // data coming from server
                if(sg_serial.payload[0] == 0x20){       
                  //sensor id = b  
                  if(sg_serial.payload[1] == 0x02){     
                    //if data = 1, turn on led    
                    if(sg_serial.payload[2] == 0x31)   
                            digitalWrite(LED, HIGH);      
                    else
                            digitalWrite(LED, LOW); //turn led off
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
            memcpy( sg_serial.ip_addr, sg_config.router_ip, IP_ADDRESS);
            // data from sensor to server
            sg_serial.payload[0] = (uint8_t) 0x21;
            // signal id = a
            sg_serial.payload[1] = (uint8_t) 0x01;
            
            // read the analog in value
            sensor_value = analogRead(analog_in_pin);            
            // map it to the range of the analog out
            output_value = map(sensor_value, 0, 1023, 0, 255);
            // map it to a percantage range 
            percent_value = map(sensor_value, 0, 1023, 0, 100);
            // change brightness of led
            analogWrite(analog_out_pin, output_value);
            // zero out the payload to ensure no unwanted data
            for(int j = 0; j < DATA_SIZE; j++)
              sg_serial.payload[j+2] = 0;
            // code to setup percantage value to send to server  
            int analog_size = sizeof(percent_value);
            char ch_array[analog_size];
            int n;
            n = sprintf(ch_array, "%d", percent_value);
            for(int i = 0; i < n; i++)
              sg_serial.payload[i+2] = ch_array[i];

            // print the results to the serial monitor for TESTING:
            //Serial.println(ch_array);     
            //Serial.print("percent lit up = ");
            //Serial.println(percent_value);
            
            // transmit payload over SPI
            sgSerialSend(&sg_serial, 1);
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

void buttonPress(){
  
  // debouncing code
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's
  // a bounce and ignore
  
  if (interrupt_time - last_interrupt_time > 200) 
  {
    // set mate flag according to button press, mate flag 
    // is initialized to false earlier
    // led_state true means led is off and will be turned on with
    // this button press
    if(led_state){
      digitalWrite(MATE_LED, HIGH);
      led_state = false;
      // sets mate flag to true which in turn sets mode in
      // eyeball to 0x01
      sg_config.mate = true;
      // DEBUG statements
      //Serial.println("mating state: ");
      //Serial.println(sg_config.mate);
      
      // led is on and will now be turned off
    } 
    else {
      digitalWrite(MATE_LED, LOW);
      led_state = true;
      // sets mate flag to true which in turn sets mode in
      // eyeball to 0x01
      sg_config.mate = false;
      // DEBUG statements
      //Serial.println("mating state: ");
      //Serial.println(sg_config.mate);
    }
  }
  last_interrupt_time = interrupt_time;
  
  // if router is looping through flys every so often, flags
  // need to be reset so process does not get stuck in middle
  // of protocol if button mate is pressed any time after the
  // first fly is sent
  sg_config.fly = false;
  sg_config.sing = false;
  sg_config.mock = false;
  sg_config.connecting = false;
  sg_config.squawk = false;
}

