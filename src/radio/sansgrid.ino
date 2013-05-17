#include <stdio.h>
#include <assert.h>
//#include <SerialDebug.h>
#include <SPI.h>
#include <sgSerial.h>
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
volatile byte pos;
volatile boolean process_flag;
volatile boolean spi_active;

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
	SPCR |= _BV(SPIE);
	
	//SerialDebugger.debug(NOTIFICATION,__FUNC__,"Setup Complete\n");
	//sgRadio = new SansgridRadio;
	
	//SPI.attachInterrupt();
	Serial.println("Setup Complete");
}

ISR(SPI_STC_vect) {
	byte c;
	Serial.println("SPI IRQ");
	switch(spi_rw) 
	{
		case 0:
			if (pos < NUM_BYTES) {
				SPDR = rx[pos++];
				if (pos == NUM_BYTES - 1) {
					spi_active = false;
					spi_rw = 1;
					digitalWrite(SLAVE_READY, HIGH);
					pos = 0;
				}
			}
			break;
		case 1:
			c = SPDR;
			if (pos < NUM_BYTES) {
				rx[pos++] = c;
				if (pos == NUM_BYTES - 1) {
					process_flag = true;
				}
			}
			break;
		default:
			Serial.println("SPI Default Error");
			break;
	}

}

void loop() {
	if (process_flag) {
		memcpy(&SpiData, rx, sizeof(SpiData));
		memset(rx,0,sizeof(SpiData));
		pos = 0;
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
			//Serial.println("Reading form XBee");
			//SerialDebugger.debug(NOTIFICATION,__FUNC__,"Reading\n");
		 // Serial.flush();
			//readPacket();
	//readPacket();
	
			sgRadio.read();
			if(sgRadio.defrag()) {
				sgRadio.processPacket();
				memcpy(rx,&SpiData,sizeof(SpiData)); 
				spi_active = true;
				Serial.write("Sending SPI");
				delay(50);
				//Serial.write((const uint8_t *)rx,sizeof(SpiData));
				//delay(5000);
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
