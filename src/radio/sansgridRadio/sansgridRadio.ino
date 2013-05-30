//#define DUE

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

#define SPI_NO_DATA 0xfd
#define SPI_YES_DATA 0xad
#define SPI_ERR_DATA 0xfe

#define XB_BAUD 115200

void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp);

SansgridSerial SpiData;
SnIpTable RouteTable;

//const int ledPin = 13;
//int length;
//HardwareSerial * test;

SansgridRadio sgRadio;

// SPI Setup 
#define SLAVE_READY 8
#define NUM_BYTES 98
#define SPI_MAX_BUFFER 100

#ifdef DUE
	#define CE 		10
	#define MOSI 	11
	#define MISO 	12
	#define SCK 	13

	uint8_t bitptr;
	uint8_t spi_inbyte;
	uint8_t spi_outbyte;
	uint8_t next_bit;
	unsigned int byteptr;
#endif

int rx_pos;
int tx_pos;
volatile bool process_flag;
volatile bool spi_active;
volatile bool spi_err;
volatile bool spi_setup;
uint8_t spi_rw;
uint8_t cmd;

uint8_t spi_rx[SPI_MAX_BUFFER];
uint8_t spi_tx[SPI_MAX_BUFFER];
/* = { 0xAD,// Control Byte 1 BYTE
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0xA8,
        0x00,0x01, // IP Address 16 BYTES
        0xF0, // Payload (Data Type) 1 BYTE 18b
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // 56B + 18B
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	// + 14
        0x00,0x00,0x53,0x61,0x6E,0x73,0x67,0x72,0x69,0x64 };

*/
int freeRam( void ){
    // Value of start of Stack and current end of Stack
    extern int __heap_start, *__brkval;
    // Value of SRAM left
    int v;
    // Return value left of SRAM
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

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
	cmd = 0;
	spi_rw = 1;
	process_flag = false;
	spi_active = false;
	spi_err = false;
	memset(spi_tx, 0, SPI_MAX_BUFFER);
	memset(spi_rx, 0, SPI_MAX_BUFFER);
  	
  //SPCR |= _BV(SPIE);
	
	// Flag to dump false spi interrupts
	spi_setup = true;	
	#ifdef DUE
		attachInterrupt(CE, spi_enable, CHANGE);
	#else
		SPI.attachInterrupt();
	#endif
	// Allow spi interrupts to operate as normal
	spi_setup = false;
	
	#if DEBUG
		Serial.println("Setup Complete");
		delay(50);
	#endif
}

void loop() {   		
	// Check for spi errors and resolve
	if (spi_err) {
		digitalWrite(SLAVE_READY, HIGH);
		process_flag = false;
		spi_active = false;
		spi_err = false;
		spi_rw = 1;
		cmd = 0;
		tx_pos = 0;
		rx_pos = 0;
	}
	
	// Check for completion of Spi transfer, process packet and send out over
	// radio
	if (process_flag) {
		Serial.println("processing spi completion");
		cmd = 0;
		spi_rw = 1;
		if (spi_active) {
			spi_active = false;
			tx_pos = 0;
			memset(spi_tx, 0, SPI_MAX_BUFFER);
			digitalWrite(SLAVE_READY, HIGH);	
		}
		else {
			rx_pos = 0;
			memcpy(&SpiData, spi_rx, sizeof(SpiData));
			memset(spi_rx, 0, SPI_MAX_BUFFER);
			sgRadio.processSpi();
			sgRadio.loadFrame(0);
			sgRadio.write();
			sgRadio.loadFrame(1);
			sgRadio.write();
			
			#if DEBUG
				Serial.println("Packet written");
				delay(50);
			#endif
		}
		process_flag = false;
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
				// This packet is not addressed to this XBee, disregard
				return;
			}
      
			// process packet as fragment, if fragment 2, send over spi to host
			// device
			if(sgRadio.defrag()) {
				sgRadio.processPacket();
				memcpy(spi_tx,&SpiData,sizeof(SpiData));
				spi_rw = 0;
				spi_active = true;
				//#if DEBUG
					Serial.println("Sending SPI");
					delay(50);
					//Serial.write(spi_tx,sizeof(SpiData));
					//delay(1000);
				//#endif
				// Initiate Spi Slave write
				#ifdef DUE
					spi_outbyte = spi_tx[0];
				#endif

	



				spi_active = true;
				digitalWrite(SLAVE_READY, LOW);
			}
		}
	}
}

#ifndef DUE

	// SPI Interrupt Service Routine
	ISR (SPI_STC_vect) {
		// Read byte from SPI SPDR register	
		uint8_t c = SPDR;
		switch(cmd) {
			case 0x00:
				cmd = c;
				break;
			case 0xAD:
				if (rx_pos < NUM_BYTES) {
					spi_rx[rx_pos++] = c;
				}
				if (rx_pos >= NUM_BYTES - 1) {
					process_flag = true;
				}
				break;
			case 0xFD:
				SPDR = spi_tx[tx_pos++];
				if (tx_pos >= NUM_BYTES) {
					process_flag = true;
				}
				break;
			default:
				spi_err = true;
				break;
			}
		// Call Command Byte
		/*
		switch(spi_rw){
			case 1:
					// Receive SPI Packet from Master
					if ( rx_pos == 0 && !good_frame) {
						if (c != SPI_YES_DATA ) {
							Serial.print("FAILED");
							SPDR = SPI_ERR_DATA;
							rx_pos = 0;
							spi_err = 1;
						}
						SPDR = cntrl;
						good_frame = true;
						return;
					}
					if ( rx_pos < NUM_BYTES ){
						spi_rx[rx_pos++] = c;
						SPDR = cntrl;
						
						// If buffer is full set process_it flag
						if ( rx_pos >= NUM_BYTES) {    
							process_flag = true; 
							good_frame = false;
						}
					}
					break;
			case 0:
					// Transmit SPI Packet to Master
					if (tx_pos == 0 && !good_frame) {
						if (c != SPI_NO_DATA) {
							SPDR = SPI_ERR_DATA;
							spi_rw = 1;
							tx_pos = 0;
							spi_err = 1;
						}
						SPDR = cntrl;
						good_frame = true;
						return;
					}
					if (tx_pos < NUM_BYTES ) {
						SPDR = spi_tx[tx_pos++];
						
						// If buffer is de-assert spi_active and SLAVE_READY, return to
						// read state
						if ( tx_pos >= NUM_BYTES - 1){
							digitalWrite(SLAVE_READY, HIGH);
							spi_active = false;
							tx_pos = 0;
							spi_rw = 1;
							good_frame = false;
						}
					}
					break;
			default:
				rx_pos = 0;
				tx_pos = 0;
				spi_rw = 1;
				spi_err = 1;
				break;  
		}
	*/
	}  // End of Interupt Service Routine
#endif


#ifdef DUE

void spi_sck() {
	if (spi_setup) return;
	switch(digitalRead(SCK)) {
		case LOW:
			digitalWrite(MISO, next_bit);
			bitptr++;
			if (bitptr >= 8) {
				spi_rx[byteptr++] = spi_inbyte;
				if (byteptr < SPI_MAX_BUFFER) {
					spi_outbyte = spi_tx[byteptr];
					next_bit = ((spi_outbyte >> 7) & 0x1);
					digitalWrite(MISO, next_bit);
				}
				else {
					spi_outbyte = 0x0;
				}
				bitptr = 0;
				spi_inbyte = 0;
			}	
			break;
		case HIGH:
			byte spi_bit = digitalRead(MOSI);
			spi_inbyte = (spi_inbyte << 1) | spi_bit;
			spi_outbyte = ((spi_outbyte << 1) & 0xFF);
			next_bit = ((spi_outbyte >> 7) & 0x1);
			break;
	}
}

void spi_enable() {
	if (spi_setup) {
		memset(spi_rx, 0x00, SPI_MAX_BUFFER);
		memset(spi_tx, SPI_NO_DATA, SPI_MAX_BUFFER);
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
//			spi_outbyte = ((spi_outbyte << 1) & 0xFF);
			spi_setup = true;
			attachInterrupt(SCK, spi_sck, CHANGE);
			spi_setup = false;
			spi_active = true;
			break;
		case HIGH:
			detachInterrupt(SCK);
			//memcpy(rx, spi_rx, NUM_BYTES);
			memset(spi_tx, SPI_NO_DATA, SPI_MAX_BUFFER);
			//memset(spi_rx, 0x00, SPI_MAX_BUFFER);
			if (spi_rw) process_flag = true;
			bitptr = 0;
			byteptr = 0;
			spi_inbyte = 0;
			spi_outbyte = 0;
			spi_active = false;
			digitalWrite(MISO, LOW);
			digitalWrite(SLAVE_READY, HIGH);
			break;
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
