#include <SPI.h>  // include the SPI library:

const int slaveSelectPin = 10;

void setup() 
{
  // Initialize SPI Master
  spiMasterInit();
}

void loop() 
{
  char message[3] = "Hi";
  
  // send test string
  for ( int i = 0; i < 3; i++)
  {
    spiMasterSend(message[i]);
    spiMasterReceive();
  }
}

// Initialize SPI Master
void spiMasterInit( void )
{
  // Set the slaveSelectPin High
  digitalWrite(slaveSelectPin, HIGH);  
  // Start SPI:
  SPI.begin(); 
  // Set SPI to Mode 0
  SPI.setDataMode (SPI_MODE0);
  // Set SPI Baud Rate to 1MHz
  SPI.setClockDivider(SPI_CLOCK_DIV64);
}

void spiMasterSend(byte ab) 
{
  // take the SS pin low to select the chip:
  digitalWrite(slaveSelectPin,LOW);
  //  send in the address and value via SPI:
  SPI.transfer(ab);
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH); 
}

void spiMasterReceive( void )
{
  byte c = SPDR;
}
