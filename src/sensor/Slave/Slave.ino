#include <SPI.h>

//#define LED 7

char buf [40];
volatile byte pos;
volatile boolean process_it;
volatile int LED = 0;
const int slaveSelectPin = 10;
const int slaveReady = 9;

void setup (void)
{
  // Initialize SPI Slave
  spiSlaveInit();
  pinMode( slaveReady , OUTPUT );
  digitalWrite( slaveReady, HIGH );
  // Initialize LED
  //ledInit();

}  // end of setup

// main loop - wait for flag set in interrupt routine
void loop (void)
{
  digitalWrite( slaveReady , LOW);
  delayMicroseconds(20);
  digitalWrite( slaveReady , HIGH);
  if (process_it)
  {  
    buf [pos] = 0;
    pos = 0;
    process_it = false;
  } // end of flag set
} // end of loop

// Initialize SPI Slave
void spiSlaveInit( void )
{
  // have to send on master in, *slave out*
  pinMode( MISO, OUTPUT );
  // turn on SPI in slave mode
  SPCR |= _BV( SPE );
  // get ready for an interrupt
  pos = 0; // buffer empty
  process_it = false;
 
  // now turn on interrupts
  SPI.attachInterrupt();
}

// SPI interrupt routine
ISR (SPI_STC_vect)
{
  byte c = SPDR; // grab byte from SPI Data Register
  if ( c == 0xFD )
  {
    SPDR = LED;
    LED++;
    if ( LED > 1 )
      LED = 0;
  }
  // add to buffer if room
  else if ( pos < sizeof buf )
  {
    SPDR = 0xFD;
    buf [pos++] = c;  
    // example: newline means time to process buffer
    if (c == '\n')
      process_it = true;
  } // end of room available
} // end of interrupt routine SPI_STC_vect
