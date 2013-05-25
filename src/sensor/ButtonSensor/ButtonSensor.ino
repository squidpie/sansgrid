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
int btn_val = 0; //state of input pin
int old_value = 0; //previous state
boolean button_state = false; //1 = led off, 0 = led on
boolean led_state = true;

const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to
const int analogOutPin = 9; // Analog output pin that the LED is attached to

int sensorValue = 0;        // value read from the pot
int outputValue = 0;
int percentValue = 0;

int intPin = 0;

unsigned long time = 0; //when did we begin pressing the button

SensorConfig sg_config;
SansgridSerial sg_data_in;
SansgridSerial sg_data_out;
SansgridChirp sg_chirp;

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
  sg_config.nest = true;
  //sg_config.fly = true;
  //sg_config.sing = true;
  //sg_config.mock = true;
  //sg_config.squawk = true;
  //sg_config.chirp = true;
  //sg_config.nokey = true;
  //sg_config.challenge = true;
  
  sgSerialOpen();
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
    
//need to add code to send pot value in ascii to server and then this section will be done    
//pot input area
  // read the input on analog pin 0:
  
  // read the analog in value:
  sensorValue = analogRead(analogInPin);            
  // map it to the range of the analog out:
  outputValue = map(sensorValue, 0, 681, 0, 255); 
  percentValue = map(sensorValue, 0, 681, 0, 100);
  // change the analog out value:
  analogWrite(analogOutPin, outputValue);
 
  sg_chirp.dt [0] = (uint8_t) 0x21;
  sg_chirp.id [0] = (uint8_t) 0x01;
  
  memset(sg_chirp.data, '\0', DATA_SIZE);
  int analog_size = sizeof(percentValue);
  char ch_array[analog_size];
  int n;
  n = sprintf(ch_array, "%d", percentValue);
  for(int i = 0; i < n; i++)
    sg_chirp.data[i] = ch_array[i];
  Serial.println(ch_array);
  //transmitChirp( &sg_data_out , &sg_chirp);
  

  // print the results to the serial monitor:
  Serial.print("sensor = " );                       
  Serial.println(sensorValue);      
  Serial.print("percent lit up = ");
  Serial.println(percentValue);

  // wait 2 milliseconds before the next loop
  // for the analog-to-digital converter to settle
  // after the last reading:
  delay(100);
  
}

/*void receive(){
    // Interrupt was initiated when SLAVE_READY was
    // asserted low, set received flag to initiate
    // processing SPI packet
    Serial.println( "Interrupt Service Routine" );
    sg_config.received = true;
    // Display value of received
    Serial.println( sg_config.received );
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
      Serial.println("led is on");
      sg_config.mate = true;
      Serial.println("mating state: ");
      Serial.println(sg_config.mate);
    } else{
      digitalWrite( mate_led, HIGH );
      led_state = true;
      Serial.println("led is off");
      sg_config.mate = false;
      Serial.println("mating state: ");
      Serial.println(sg_config.mate);
    }
  }
  last_interrupt_time = interrupt_time;
  
}

