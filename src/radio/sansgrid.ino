#include <stdio.h>
#include <assert.h>
#include <SerialDebug.h>
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


SansgridSerial SpiData;
SnIpTable RouteTable;
HardwareSerial * test;

SansgridRadio * Radio;

uint8_t packet_buffer[SG_PACKET_SZ];

// SPI Setup 
#define SLAVE_READY 7
#define NUM_BYTES 98

char rx[sizeof(SpiData)]; //NUM_BYTES + 1];
volatile byte pos;
volatile boolean process_flag;
volatile boolean spi_active;

void setup() {
	#if DEBUG 
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
	#else
	Serial.begin(115200);
	#endif
  pinMode(ledPin, OUTPUT);
  pinMode(ROUTER_MODE_PIN, INPUT);
  digitalWrite(ROUTER_MODE_PIN, HIGH);
  pinMode(SPI_IRQ_PIN, INPUT);
  digitalWrite(SPI_IRQ_PIN, HIGH);
	pinMode(MISO, OUTPUT);
	SPCR |= _BV(SPE);
	pos = 0;
	process_flag = false;
	spi_active = false;
	pinMode(SLAVE_READY, OUTPUT);
	digitalWrite(SLAVE_READY, HIGH);
	SPI.attachInterrupt();

	//SerialDebugger.debug(NOTIFICATION,__FUNC__,"Setup Complete\n");
	Radio = new SansgridRadio;
	Radio->init(&Serial, &SpiData, &RouteTable);
  if(digitalRead(ROUTER_MODE_PIN) == HIGH) {
    //SerialDebugger.debug(NOTIFICATION,__FUNC__,"ROUTER MODE\n");
  	Radio->set_mode(ROUTER);
	}
	Serial.println("Setup Complete");
}

ISR(SPI_STC_vect) {
	byte c = SPDR;
	if (pos < NUM_BYTES) {
		rx[pos++] = c;
		if (pos == NUM_BYTES - 1) process_flag = true;
	}
}

void loop() {
	if (process_flag) {
		memcpy(&SpiData, rx, pos);
		rx[pos] = 0;
		pos = 0;
		process_flag = false;
		spi_active = false;
		Radio->processSpi();
		digitalWrite(SLAVE_READY, HIGH);
		Radio->loadFrame(0);
		Radio->write();
		Radio->loadFrame(1);
		Radio->write();
		Serial.println("Packet written");
	}
	if (!spi_active) {
// put your main code here, to run repeatedly: 
		if (Serial.peek() >= 0) {
			//Serial.println("Reading form XBee");
			//SerialDebugger.debug(NOTIFICATION,__FUNC__,"Reading\n");
		 // Serial.flush();
			//readPacket();
	readPacket();
	/*
			Radio.read();
			Radio.processPacket();
			if(Radio.rxComplete()) {
				memcpy(rx,&SpiData,sizeof(SpiData)); 
				spi_active = true;
				digitalWrite(SLAVE_READY, LOW);
			}*/
		}
	}
    
}
// assert slave interrupt pin 7 to initate SPI tansfer

void readPacket() {
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
