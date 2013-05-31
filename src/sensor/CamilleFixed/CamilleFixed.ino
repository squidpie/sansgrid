#include <Arduino.h>
#include <SPI.h>
#include <sgSerial.h>

#define __ASSERT_USE_STDERR
#define DEBUG false
#define DEBUG_LEVEL 1

#define SPI_NO_DATA 0xfd
#define SPI_YES_DATA 0xad
#define SPI_ERR_DATA 0xfe

#define XB_BAUD 9600

//int length;
//uint8_t spi_rw = 1;

SansgridSerial SpiData;

// SPI Setup 
#define SLAVE_READY 8
#define NUM_BYTES 98
#define SPI_MAX_BUFFER 100

uint8_t spi_rx[SPI_MAX_BUFFER];
uint8_t spi_tx[SPI_MAX_BUFFER] = { 0xAD,// Control Byte 1 BYTE
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0xA8,
        0x00,0x01, // IP Address 16 BYTES
        0xF0, // Payload (Data Type) 1 BYTE
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	
        0x00,0x00,0x53,0x61,0x6E,0x73,0x67,0x72,0x69,0x64 }; // Payload 
        // Network ID 80 BYTES
        // Payload 81 BYTES
uint8_t cmd = 0;

volatile uint8_t rx_pos;
volatile uint8_t tx_pos;
volatile boolean process_flag;
volatile boolean spi_active;
volatile boolean spi_err;
volatile boolean spi_setup;
volatile boolean good_frame;

void setup() {
    // Open serial com with XBee
    Serial.begin(XB_BAUD);

    // Initialize SPI Ports for Uno
    pinMode(MISO, OUTPUT);
    digitalWrite(MISO, LOW);
    
    // Initialize Slave Ready Handshake Port
    pinMode(SLAVE_READY, OUTPUT);
    digitalWrite(SLAVE_READY, HIGH);
    
    // Enable Uno as SPI slave 
    SPCR |= _BV(SPE);
    
    // Prepare spi buffers and flags
    rx_pos = 0;
    tx_pos = 0;
    process_flag = false;
    spi_active = false;
    
    SPI.attachInterrupt();
}

void loop() {
    // Check for completion of Spi transfer, process packet and send out over
    // radio
    uint8_t buff[98];
    if (process_flag) {
        if(spi_active){
            memcpy(buff , spi_tx , NUM_BYTES );
            Serial.println( "tx buffer");
            for( int i = 0 ; i < NUM_BYTES ; i++ ){
                Serial.println( buff[i] , HEX );
            }  
            tx_pos = 0;  
        }
        else{
            memcpy(&SpiData, spi_rx, sizeof(SpiData));
            memcpy(buff , spi_rx , NUM_BYTES );
            Serial.println( "rx buffer");
            for( int i = 0 ; i < NUM_BYTES ; i++ ){
                Serial.println( buff[i] , HEX );
            }
            memset(spi_rx,0, SPI_MAX_BUFFER);
            rx_pos = 0;
        }    
        cmd = 0;
        process_flag = false;
        spi_active = false;
    }
    delay(1000);
    digitalWrite( SLAVE_READY , LOW );
    delayMicroseconds(6);
    digitalWrite( SLAVE_READY , HIGH );
    delay(1000);
}

// SPI Interrupt Service Routine
ISR (SPI_STC_vect) {
    uint8_t c = SPDR;
    switch(cmd) {
        case 0x00:
            cmd = c;
            break;
        case 0xAD:
            if (rx_pos < NUM_BYTES) {
                spi_rx[rx_pos++] = c;
            }
            if (rx_pos == NUM_BYTES - 1) {
                process_flag = true;
                spi_active = false;
            }
            break;
        case 0xFD:
            SPDR = spi_tx[tx_pos++];
            if ( tx_pos == NUM_BYTES ) {
                process_flag = true;
                spi_active = true;
            }
            break;
        default:
            break;
    }
}  // End of Interupt Service Routine

