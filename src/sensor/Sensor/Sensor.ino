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
#define button 7  
#define mate_led 6 //active low
#define SLAVE_READY 8
#define switch_led 4
#define dip 5 //active low

// state of the button initialized
int btn_val = 0; //state of input pin
int old_value = 0; //previous state
int button_state = 1; //1 = led off, 0 = led on
int switch_state = 0; //state of dip switch
int led_state = 0;
//int mode = 00;

unsigned long time = 0; //when did we begin pressing the button

SensorConfig sg_config;
//SansgridSerial sg_data_in;
//SansgridSerial sg_data_out;

//setting up the pins to proper i/o
void setup() {
  Serial.begin(9600);
  // initialize the button led as output:
  pinMode(mate_led, OUTPUT);      
  // initialize the button as input:
  pinMode(button, INPUT);     
  //initialize dip as input
  pinMode(dip, INPUT);
  //initialize switch led as output
  pinMode(switch_state, OUTPUT);
  // Set SansgridSerial data_out and data_in control byte
  /*sg_data_in.control[0] = 0xFD;
  sg_data_out.control[0] = 0xAD;
  // Setup Sensor Module Configurations
  // Sensor A
  sg_config.a.id = { 0x02 };
  // Classification : 0x00 = digital, 0x01 = analog
  sg_config.a.classification = { 0x01 };
  // Direction: 0x00 from Sensor to Server (example: Temperature reading)
  // 0x01 from Server to Sensor (example: Turn light on and off)
  sg_config.a.direction = { 0x00 };
  // Sensor label 30 characters or less
  // Change only value in the array char label[] = "<value>"
  char label[] = "Potentiometer";
  for( int i = 0 ; i < LABEL - sizeof(label) + 1 ; i++ )
      sg_config.a.label[i] = (int8_t) 0x00;
  memcpy( sg_config.a.label + sizeof(label) - 1 , label , sizeof(label));
  // Sensor units 6 characters or less
  // Change only value in the array char units[] = "<value>"
  char units[] = "ohms";
  for( int i = 0 ; i < LABEL - sizeof(units) + 1 ; i++ )
      sg_config.a.units[i] = (uint8_t) 0x00;
  memcpy( sg_config.a.units + sizeof(units) - 1 , units , sizeof(units));
  // Sensor B
  sg_config.b.id = { 0x03 };
  sg_config.b.classification = { 0x00 };
  sg_config.b.direction = { 0x01 };
  char label[] = "Light Switch";
  for( int i = 0 ; i < LABEL - sizeof(label) + 1 ; i++ )
      sg_config.b.label[i] = (int8_t) 0x00;
  memcpy( sg_config.b.label + sizeof(label) - 1 , label , sizeof(label));
  sg_config.b.units = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  */
  // Test Packets
//  sg_config.nest = false;
  //sg_config.fly = true;
  //sg_config.sing = true;
  //sg_config.mock = true;
  //sg_config.squawk = true;
  //sg_config.chirp = true;
  //sg_config.challenge = true;  
}

void loop(){
  
  //while( sg_config.nest == false ){
        // Received a Squawk packet, now send a Squawk back
        //Serial.println( "connect");
        //sensorConnect( &sg_config , &sg_data_out );
   // }
  
  //reads state of button and turns on/off led 
    btn_val = digitalRead(button); // reads current state of button
    
    // check if there was a transition 
    if ((btn_val == LOW) && (old_value == HIGH)) {
        button_state = 1 - button_state; // change the state from off to on or on to off
        time = millis(); //returns how many milliseconds have passed 
        delay(10);
    }

    old_value = btn_val; // value is now old, let’s store it 

//button mating area
    if (button_state == 1) {
        digitalWrite(mate_led, HIGH); // turn LED OFF
        sg_config.mate = false;
        //code for testing:
        //Serial.write("Button state: ");
        //Serial.println(button_state);
        //delay(50);
        Serial.write("Mating state: ");
        Serial.println(sg_config.mate);
        delay(50);
        
               
    } else {
        digitalWrite(mate_led, LOW); // turn LED ON
        sg_config.mate = true;
        //code for testing:
        //Serial.write("Button state: "); 
        //Serial.println(button_state);
        //delay(50);
        Serial.write("Mating state: ");
        Serial.println(sg_config.mate);
        delay(50); 
    }
    
    //dip switch to led code
    switch_state = digitalRead(dip); //reads current state of dip switch    
    Serial.println(switch_state);
    delay(100);
    
    //if switch is on, send code to server to tell it to turn led on on other arduino
    if (switch_state == 1) {
      int somedatamember;
      
      led_state = 1;
      somedatamember = led_state;
      Serial.write("led: ");
      Serial.println(somedatamember);
      digitalWrite(switch_led, HIGH); // turn LED on
      delay(50);
        
    //if switch is off, send code to server to tell it to turn led on on other arduino 
    } 
    else {
      int somedatamember;
      
      led_state = 0;
      somedatamember = led_state;
      Serial.write("led: ");
      Serial.println(somedatamember);
      digitalWrite(switch_led, LOW); // turn LED off
      delay(50); 
    }
    
//pot input area
  // read the input on analog pin 0:
  int potvalue = analogRead(A0);
  // print out the value of the pot:
  Serial.println(potvalue);
  //need to add code to send pot value to server
  delay(50);        // delay for better readability
}


