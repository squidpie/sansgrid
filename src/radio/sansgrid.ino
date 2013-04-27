#include <stdio.h>
#include <assert.h>
#include <SerialDebug.h>

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
//SansgridRadio Radio = SansgridRadio(&Serial,&SpiData, &RouteTable);



uint8_t packet_buffer[PACKET_SZ];

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
  if(digitalRead(ROUTER_MODE_PIN) == HIGH) {
    //SerialDebugger.debug(NOTIFICATION,__FUNC__,"ROUTER MODE\n");
  	//Radio.set_mode(ROUTER);
	}
	//SerialDebugger.debug(NOTIFICATION,__FUNC__,"Setup Complete\n");
	Serial.println("this is a test of the emergency broadcasting system");
	//Radio.test();
}


void loop() {
  // put your main code here, to run repeatedly: 
  if (Serial.peek() >= 0) {
		//Serial.println("Reading form XBee");
    //SerialDebugger.debug(NOTIFICATION,__FUNC__,"Reading\n");
   // Serial.flush();
    readPacket();
	//Radio.read();
		write_spi();
  }
  if (digitalRead(SPI_IRQ_PIN) == LOW) {
    //SerialDebugger.debug(NOTIFICATION,__FUNC__,"Writing\n");
    read_spi();
    //Radio.write();
  }
    
}


void readPacket() {
	int i = 0;
	while(Serial.available() > 0 && i < PACKET_SZ) {
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
