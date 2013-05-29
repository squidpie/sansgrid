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
uint8_t spi_tx[SPI_MAX_BUFFER];
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
    if (process_flag) {
        uint8_t buff[98];
        memcpy(&SpiData, spi_rx, sizeof(SpiData));
        memcpy(buff , spi_rx , NUM_BYTES );
        for( int i = 0 ; i < NUM_BYTES ; i++ ){
            Serial.println( buff[i] );
        }
        memset(spi_rx,0, SPI_MAX_BUFFER);
        rx_pos = 0;
        process_flag = false;
        spi_active = false;
    }
}

// SPI Interrupt Service Routine
ISR (SPI_STC_vect) {
    //Serial.println( "ISR" );
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
                rx_pos = 0;
                cmd = 0;
                process_flag = true;
            }
            break;
        case 0xFD:
            SPDR = spi_tx[tx_pos++];
            if ( tx_pos >= NUM_BYTES ) {
                tx_pos = 0;
                cmd = 0;
                spi_active = false;
            }
            break;
        default:
            break;
    }
}  // End of Interupt Service Routine

