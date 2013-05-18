#include <stdio.h>
#include <assert.h>
//#include <SerialDebug.h>
#include <SPI.h>
#include "sgSerial.h"
#include "sgRadio.h"

#define __ASSERT_USE_STDERR
#define DEBUG false
#define DEBUG_LEVEL 1

void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp);
void readPacket();
const int ledPin = 13;
int length;
byte spi_rw = 1;

SansgridSerial SpiData;
SnIpTable RouteTable;
HardwareSerial * test;

SansgridRadio sgRadio;

// SPI Setup 
#define SLAVE_READY 8
#define NUM_BYTES 98

char rx[sizeof(SpiData)]; //NUM_BYTES + 1];
volatile uint8_t pos;
volatile uint8_t pos_b;

// SPI command byte
volatile uint8_t command;
volatile boolean process_flag;
volatile boolean spi_active;
volatile boolean spi_err;

void setup() {
	/*#if DEBUG 
		//SerialDebugger.begin(115200);
		switch(DEBUG_LEVEL) {
			case 1:
				//SerialDebugger.enable(ERROR);
			case 2:
				//SerialDebugger.enable(WARNING);
			case 3:
				//SerialDebugger.enable(NOTIFICATION);
		}
		//SerialDebugger.debug(ERROR,__FUNC__,"Error test\n");
		//SerialDebugger.debug(WARNING,__FUNC__,"Warn test\n");
		//SerialDebugger.debug(NOTIFICATION,__FUNC__,"Notify test\n");
		delay(50);
		sgDebugInit(&SerialDebugger);
		//SerialDebugger.debug(NOTIFICATION,__FUNC__, "Entering Setup\n");
	#else*/
	Serial.begin(115200);
  //Serial.println("Setup starts here");
	//pinMode(ledPin, OUTPUT);
  pinMode(ROUTER_MODE_PIN, INPUT);
  digitalWrite(ROUTER_MODE_PIN, LOW);
  
	sgRadio.init(&Serial, &SpiData, &RouteTable);
  if(digitalRead(ROUTER_MODE_PIN) == HIGH) {
    //SerialDebugger.debug(NOTIFICATION,__FUNC__,"ROUTER MODE\n");
  	sgRadio.set_mode(ROUTER);
  }

  //pinMode(SPI_IRQ_PIN, INPUT);
  //digitalWrite(SPI_IRQ_PIN, HIGH);
      pinMode(MISO, OUTPUT);
	pinMode(SLAVE_READY, OUTPUT);
	digitalWrite(SLAVE_READY, HIGH);
	SPCR |= _BV(SPE);
	pos = 0;
	process_flag = false;
	spi_active = false;
        command = 0;
  	
    //SPCR |= _BV(SPIE);
	
	//SerialDebugger.debug(NOTIFICATION,__FUNC__,"Setup Complete\n");
	//sgRadio = new SansgridRadio;
	
  //Serial.println("Setup Complete");
  //delay(500);
  SPI.attachInterrupt();
}
// SPI Interrupt Service Routine
ISR (SPI_STC_vect)
{
  // Read byte from SPI SPDR register
  uint8_t c = SPDR;
  // Call Command Byte
  switch(command){
  case 0x00:
      // Set Command byte from intial dummy byte
      command = c;
      break;
  case 0xAD:
      // Receive SPI Packet from Master
      if ( pos < NUM_BYTES ){
          rx[pos++] = c;
          // If buffer is full set process_it flag
          if ( pos == NUM_BYTES - 1 )    
              process_flag = true; 
        }
      break;
  case 0xFD:
      // Transmit SPI Packet to Master
      SPDR = rx[ pos_b++ ];
      // If buffer is full set process_it flag
      if ( pos_b == NUM_BYTES ){
              spi_active = false;
              pos = 0;
              command = 0;
              digitalWrite(SLAVE_READY, HIGH);
      }
      break;
  default:
      if ( pos < NUM_BYTES ){
          pos++;
          // If buffer is full set process_it flag
          if ( pos == NUM_BYTES - 1 ) {
            pos = 0;
            command = 0;    
          }
      }
      break;
    }  // End of Command Byte
}  // End of Interupt Service Routine


void loop() {
  bad_data: 
	if (spi_err) {
		if (!spi_rw) digitalWrite(SLAVE_READY, HIGH);
		else Serial.println("How do we recover?");
		spi_err = false;
	}
	
	if (process_flag) {
         // Serial.println("Process It");
	   memcpy(&SpiData, rx, sizeof(SpiData));
		memset(rx,0,sizeof(SpiData));
		pos = 0;
                command = 0;
		process_flag = false;
		spi_active = false;
		sgRadio.processSpi();
		digitalWrite(SLAVE_READY, HIGH);
		sgRadio.loadFrame(0);
		sgRadio.write();
		sgRadio.loadFrame(1);
		sgRadio.write();
		Serial.println("Packet written");
             
	}
	if (!spi_active) {
// put your main code here, to run repeatedly: 
		if (Serial.peek() >= 0) {
                Serial.println("peek true");
                delay(50);
  			//Serial.println("Reading form XBee");
			//SerialDebugger.debug(NOTIFICATION,__FUNC__,"Reading\n");
		 // Serial.flush();
			//readPacket();
	//readPacket();

			byte head = Serial.peek();
			if (head != 0x00 && head != 0x01) {
                                Serial.println("throwing out the bath water");
                                delay(50);
				while (Serial.available() > 0) { Serial.read(); }
				goto bad_data;
			}
			
			sgRadio.read();
                        
			if(sgRadio.defrag()) {
				sgRadio.processPacket();
				memcpy(rx,&SpiData,sizeof(SpiData)); 
				spi_active = true;
				Serial.write("Sending SPI");
				delay(50);
				Serial.write((const uint8_t *)rx,sizeof(SpiData));
				delay(1000);
				spi_rw = 0;
				digitalWrite(SLAVE_READY, LOW);
			}
		}
	}
}
// assert slave interrupt pin 7 to initate SPI tansfer

void readPacket() {
uint8_t packet_buffer[SG_PACKET_SZ];
	int i = 0;
	while(Serial.available() > 0 && i < SG_PACKET_SZ) {
		delay(2);
		packet_buffer[i++] = Serial.read();
	}
	Serial.write(packet_buffer, i);
	//processPacket();
}

// handle diagnostic informations given by assertion and abort program execution:
void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp) {
    // transmit diagnostic informations through serial link. 
    Serial.println(__func);
    Serial.println(__file);
    Serial.println(__lineno, DEC);
    Serial.println(__sexp);
    Serial.flush();
    // abort program execution.
    abort();
}

int getMY() {
  int rv, timeout;
  timeout = 50;
  Serial.write("+++");
  Serial.write("ATMY");
  while(timeout){
    delay(50);
    if (Serial.available() > 0) {
      rv = Serial.read();
      timeout = 0;
    }
    else timeout--;
  }
  Serial.write("ATCN");
  return rv;
}

