#define DUE

#include <Arduino.h>
#include <stdio.h>
#include <assert.h>

#ifndef DUE
	#include <SPI.h>
#endif

#include "sgSerial.h"
#include "sgRadio.h"

#define __ASSERT_USE_STDERR
#define DEBUG false
#define DEBUG_LEVEL 1

#define SPI_RD_CNTRL 0xFD
#define SPI_WR_CNTRL 0xAD
#define SPI_ERR_CNTRL 0xFE

#define XB_BAUD 115200

void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp);
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


#ifdef DUE
	#define CE 		10
	#define MISO 	11
	#define MOSI 	12
	#define SCK 	13
	#define SPI_MAX_BUFFER 100

	uint8_t spi_rx[SPI_MAX_BUFFER];
	uint8_t spi_tx[SPI_MAX_BUFFER];
	volatile uint8_t bitptr;
	volatile uint8_t spi_inbyte;
	volatile uint8_t spi_outbyte;
	volatile unsigned int byteptr;
#endif

char rx[NUM_BYTES]; //NUM_BYTES + 1];
char tx[NUM_BYTES];
volatile uint8_t rx_pos;
volatile uint8_t tx_pos;

volatile boolean process_flag;
volatile boolean spi_active;
volatile boolean spi_err;
volatile bool spi_setup;

void setup() {

	// Open serial com with XBee
	Serial.begin(XB_BAUD);
  
	// Set up ROUTER_PIN
	pinMode(ROUTER_MODE_PIN, INPUT);
  digitalWrite(ROUTER_MODE_PIN, LOW);
  
	// Initialize sansgrid Radio Class
	sgRadio.init(&Serial, &SpiData, &RouteTable);
  if(digitalRead(ROUTER_MODE_PIN) == HIGH) {
  	sgRadio.set_mode(ROUTER);
  }

	// Initialize SPI Pins for Uno and Due
  pinMode(MISO, OUTPUT);
	digitalWrite(MISO, LOW);
	pinMode(SLAVE_READY, OUTPUT);
	digitalWrite(SLAVE_READY, HIGH);
	
	// Init Due Spi Pins
	#ifdef DUE
		pinMode(MOSI, INPUT);
		pinMode(SCK, INPUT);
		pinMode(CE, INPUT);
	
	// else Enable Uno as SPI slave 
	#else
		SPCR |=	_BV(SPE);
	#endif	
	
	// Prepare spi buffers and flags
	rx_pos = 0;
	tx_pos = 0;
	process_flag = false;
	spi_active = false;
  	
  //SPCR |= _BV(SPIE);
	
	// Flag to dump false spi interrupts
	spi_setup = true;
	
	#ifndef DUE
		SPI.attachInterrupt();
	#else
		attachInterrupt(CE, spi_enable, CHANGE);
	#endif
	// Allow spi interrupts to operate as normal
	spi_setup = false;
	
	#if DEBUG
		Serial.println("Setup Complete");
  	delay(50);
	#endif
}

void loop() {
  loop_head:

	// Check for spi errors and resolve
	if (spi_err) {
		if (!spi_rw) {
			digitalWrite(SLAVE_READY, HIGH);
			spi_rw = 1;
		}
		else { 
			Serial.println("How do we recover?");
			spi_rw = 1;
		}
		spi_err = false;
	}
	
	// Check for completion of Spi transfer, process packet and send out over
	// radio
	if (process_flag) {
		memcpy(&SpiData.payload, spi_rx, sizeof(SpiData));
		memset(spi_rx,0, SPI_MAX_BUFFER);
		rx_pos = 0;
		process_flag = false;
		spi_active = false;
		sgRadio.processSpi();
		digitalWrite(SLAVE_READY, HIGH);
		sgRadio.loadFrame(0);
		sgRadio.write();
		sgRadio.loadFrame(1);
		sgRadio.write();
		
		#if DEBUG
			Serial.println("Packet written");
			delay(50);
		#endif
	}
	
	// Scan for incoming serial data if not serving spi
	if (!spi_active) {
		if (Serial.peek() >= 0) {
			// Dump invalid packets, such as debug messages
			byte head = Serial.peek();
			if (head != 0x00 && head != 0x01) {
        
				#if DEBUG
					Serial.println("throwing out the bath water");
        	delay(50);
				#endif
				// flush serial input and return to loop
				while (Serial.available() > 0) { Serial.read(); }
				return;
			}

			else if (!sgRadio.read()) {
				// This packet is not addressed to this XBee, diregard
				return;
			}
      
			// process packet as fragment, if fragment 2, send over spi to host
			// device
			if(sgRadio.defrag()) {
				sgRadio.processPacket();
				memcpy(spi_tx,&SpiData,sizeof(SpiData)); 
				spi_active = true;
				spi_rw = 0;
				
				#if DEBUG
					Serial.println("Sending SPI");
					delay(50);
					Serial.write((const uint8_t *)rx,sizeof(SpiData));
					delay(1000);
				#endif
				// Initiate Spi Slave write
				digitalWrite(SLAVE_READY, LOW);
			}
		}
	}
}

#ifndef DUE
	// SPI Interrupt Service Routine
	ISR (SPI_STC_vect) {
		if (spi_setup) {
			return;
		}
		uint8_t cntrl = spi_rw ? SPI_RD_CNTRL : SPI_WR_CNTRL;
		// Read byte from SPI SPDR register
		uint8_t c = SPDR;

		// Call Command Byte
		switch(spi_rw){
			case 1:
					// Receive SPI Packet from Master
					if ( rx_pos == 0 && c != SPI_RD_CNTRL ) {
						SPDR = SPI_ERR_CNTRL;
						rx_pos = 0;
						spi_err = 1;
						return;
					}
					if ( pos < NUM_BYTES ){
						rx[pos++] = c;
						SPDR = cntrl;
						
						// If buffer is full set process_it flag
						if ( pos >= NUM_BYTES - 1) {    
							process_flag = true; 
						}
					}
					break;
			case 0:
					// Transmit SPI Packet to Master
					if (rx_pos == 0 && c != SPI_WR_CNTRL) {
						SPDR = SPI_ERR_CNTRL;
						spi_rw = 1;
						rx_pos = 0;
						spi_err = 1;
						return;
					}
					if (pos < NUM_BYTES ) {
						SPDR = rx[rx_pos++];
						
						// If buffer is de-assert spi_active and SLAVE_READY, return to
						// read state
						if ( rx_pos >= NUM_BYTES - 1){
							digitalWrite(SLAVE_READY, HIGH);
							spi_active = false;
							rx_pos = 0;
							spi_rw = 1;
						}
					}
					break;
			default:
				rx_pos = 0;
				spi_rw = 1;
				spi_err = 1;
				break;  
		}
	
	#if DEBUG
		Serial.write(c);
	#endif

	}  // End of Interupt Service Routine
#endif


#ifdef DUE

void spi_sck() {
	if (spi_setup) return;
	switch(digitalRead(SCK)) {
		case LOW:
			digitalWrite(MISO, ((spi_outbyte >> 7) & 0x1));
			spi_outbyte = ((spi_outbyte << 1) & 0xFF);
			if (++bitptr >= 8) {
				spi_rx[byteptr++] = spi_inbyte;
				spi_outbyte = byteptr > SPI_MAX_BUFFER ? spi_tx[byteptr] : 0x0;
				bitptr = 0;
				spi_inbyte = 0;
			}	
			break;
		case HIGH:
			byte spi_bit = digitalRead(MOSI);
			spi_inbyte = (spi_inbyte << 1) | spi_bit;
			break;
	}
}

void spi_enable() {
	if (spi_setup) {
		memset(spi_rx, 0x00, SPI_MAX_BUFFER);
		memset(spi_tx, SPI_RD_CNTRL, SPI_MAX_BUFFER);
		bitptr = 0;
		byteptr = 0;
		spi_inbyte = 0;
		spi_outbyte = 0;
		digitalWrite(MISO, LOW);
		return;
	}
	switch(digitalRead(CE)) {
		case LOW:
			digitalWrite(MISO, ((spi_outbyte >> 7) & 0x1));
			spi_setup = true;
			attachInterrupt(SCK, spi_sck, CHANGE);
			spi_setup = false;
			spi_outbyte = ((spi_outbyte << 1) & 0xFF);
			break;
		case HIGH:
			detachInterrupt(SCK);
			//memcpy(rx, spi_rx, NUM_BYTES);
			memset(spi_tx, SPI_RD_CNTRL, SPI_MAX_BUFFER);
			//memset(spi_rx, 0x00, SPI_MAX_BUFFER);
			process_flag = true;
			bitptr = 0;
			byteptr = 0;
			spi_inbyte = 0;
			spi_outbyte = 0;
			digitalWrite(MISO, LOW);
			break;
	}
}

void spi_enable_falling () {
	if (spi_setup) {
		return;
	}
	spi_setup = true;
	attachInterrupt(SCK, spi_sck, CHANGE);
	spi_setup = false;
}

void spi_enable_rising() {
	if (spi_setup) Serial.println("SPI INIT Complete");
	detachInterrupt(SCK);
	memcpy(rx, spi_rx, NUM_BYTES);
	memset(spi_tx, SPI_RD_CNTRL, SPI_MAX_BUFFER);
	memset(spi_rx, 0x00, SPI_MAX_BUFFER);
	process_flag = spi_setup ? false: true;
	bitptr = 0;
	byteptr = 0;
	spi_inbyte = 0;
	spi_outbyte = 0;
	digitalWrite(MISO, LOW);
}



void spi_sck_rising() {
	if (spi_setup) return;
	byte spi_bit = digitalRead(MOSI);
	spi_inbyte = (spi_inbyte << 1) | spi_bit;
}

void spi_sck_falling(){
	if (spi_setup) return;
	digitalWrite(MISO, ((spi_outbyte >> 7) & 0x1));
	spi_outbyte = ((spi_outbyte << 1) & 0xFF);
	if (++bitptr >= 8) {
		spi_rx[byteptr++] = spi_inbyte;
		spi_outbyte = (byteptr = SPI_MAX_BUFFER ? spi_tx[byteptr] : 0x0);
		bitptr = 0;
		spi_inbyte = 0;
	}	
}
#endif

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
