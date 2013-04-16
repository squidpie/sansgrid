#include <SPI.h>

#define LED 7

char buf [10];
volatile byte pos;
volatile boolean process_it;


void setup (void)
{
  // Initialize SPI Slave
  spiSlaveInit();
  // Initialize LED
  ledInit();

}  // end of setup


// SPI interrupt routine
ISR (SPI_STC_vect)
{
  // SPI Slave receive
  spiSlaveReceive();
  spiSlaveSend();
}

// main loop - wait for flag set in interrupt routine
void loop (void)
{
  if (process_it)
  {
    // Set last byte to 0 in buffer
    buf [pos] = 0;  
    // Turn on LED
    digitalWrite(LED, LOW);
    // Reset buffer position to 0
    pos = 0;
    // Rest Flag to to process payload
    process_it = false;
  }  // end of flag set  
}  // end of loop

// Initialize SPI Slave
void spiSlaveInit( void )
{
  // Turn on SPI in Slave Mode
  SPCR |= _BV(SPE);
  SPCR |= _BV(SPR1);
  SPCR |= _BV(SPR0);
  
  // Set SPI to Mode 0
  SPI.setDataMode (SPI_MODE0);
  
  // Set SPI Baud Rate to 1MHz
  SPI.setClockDivider(SPI_CLOCK_DIV64);
  
  // Set Buffer to Empty
  pos = 0;   
  process_it = false;

  // Turn Interrupts
  SPI.attachInterrupt();
}

// Receive byte on SPI
void spiSlaveReceive( void )
{
  // Set pin to LOW, LED off
  digitalWrite(LED, HIGH);
  
  byte c = SPDR;  // grab byte from SPI Data Register
  
  // add to buffer if room
  if (pos < sizeof buf)
  {
    buf [pos++] = c;
    
    // example: newline means time to process buffer
    if (c == '\n')
      process_it = true;
      
  }  // end of room available
}

// Send byte on SPI
void spiSlaveSend( void )
{
  // Acknowledge Received Byte over SPI
  SPI.transfer(0xFF);
}

// Initialize LED
void ledInit( void )
{
  // Set LED pin to outpu
  pinMode(7, OUTPUT); 
  // Set pin to LOW, LED off
  digitalWrite(LED, LOW);
}
