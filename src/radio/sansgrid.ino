#include <stdio.h>
#include <assert.h>
#include <SerialDebug.h>

#include <sgSerial.h>
#include "sgRadio.h"

#define __ASSERT_USE_STDERR
#define DEBUG true
#define DEBUG_LEVEL 1

void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp);

const int ledPin = 13;
int length;

SansgridSerial SpiData;
SnIpTable RouteTable;
SansgridRadio Radio = SansgridRadio(&SpiData, &RouteTable);


void setup() {
	#if DEBUG 
		SerialDebugger.begin(9600);
		switch(DEBUG_LEVEL) {
			case 1:
				SerialDebugger.enable(ERROR);
			case 2:
				SerialDebugger.enable(WARNING);
			case 3:
				SerialDebugger.enable(NOTIFICATION);
		}
		SerialDebugger.debug(ERROR,__FUNC__,"Error test\n");
		SerialDebugger.debug(WARNING,__FUNC__,"Warn test\n");
		SerialDebugger.debug(NOTIFICATION,__FUNC__,"Notify test\n");
		delay(50);
	#endif
	sgDebugInit(&SerialDebugger);
	SerialDebugger.debug(NOTIFICATION,__FUNC__, "Entering Setup\n");
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(ROUTER_MODE_PIN, INPUT);
  digitalWrite(ROUTER_MODE_PIN, HIGH);
  pinMode(SPI_IRQ_PIN, INPUT);
  digitalWrite(SPI_IRQ_PIN, HIGH);
  if(digitalRead(ROUTER_MODE_PIN) == HIGH) {
    SerialDebugger.debug(NOTIFICATION,__FUNC__,"ROUTER MODE\n");
  	Radio.set_mode(ROUTER);
	}
	SerialDebugger.debug(NOTIFICATION,__FUNC__,"Setup Complete\n");
}


void loop() {
  // put your main code here, to run repeatedly: 
  if (Serial.peek() >= 0) {
    SerialDebugger.debug(NOTIFICATION,__FUNC__,"Reading\n");
    Serial.flush();
    Radio.read();
		write_spi();
  }
  if (digitalRead(SPI_IRQ_PIN) == LOW) {
    SerialDebugger.debug(NOTIFICATION,__FUNC__,"Writing\n");
    read_spi();
    Radio.write();
  }
    
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
