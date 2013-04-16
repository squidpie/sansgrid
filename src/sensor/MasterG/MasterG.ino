#include <SPI.h>  // include the SPI library:

const int slaveSelectPin = 10;

void setup() 
{
  // Initialize SPI Master
  spiMasterInit();
}

void loop() 
{
  
}

// Initialize SPI Master
void spiMasterInit( void )
{
  // Set SPI to Master 
  // default is zero which is Slave
  SPCR |= _BV(MSTR);
  
  // Set SPI Baud Rate to 1MHz
  SPI.setClockDivider(SPI_CLOCK_DIV16);
}

void spiMasterOpen( int ss )
{
  SPI.begin(ss);
}

void spiMasterClose( int ss )
{
  SPI.end(ss);
}

void spiMasterSend( char * ab ) 
{
  // Enable SPI bus
  spiMasterOpen();
  // take the SS pin low to select the chip:
  digitalWrite(slaveSelectPin,LOW);
  //  send in the char arra via SPI:
  for( int i = 0; i < sizeof(ab) - 1; i++)
  {
    if( i == (sizeof(ab)- 2) )
      SPI.transfer( slaveSelectPin, ab[i] , SPI_LAST );
    else
      SPI.transfer( slaveSelectPin, ab[i] , SPI_CONTINUE );
  }
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH);
  // Close SPI bus
  spiMasterClose();
}

void spiMasterReceive( char * ab )
{
  // Enable SPI bus
  spiMasterOpen(slaveSelectPi);
  // take the SS pin low to select the chip:
  digitalWrite(slaveSelectPin,LOW);
  control = SPI.transfer(slaveSelectPin, 0xFD);
  if( control == 0xFD);
  {
    for( int i = 0; i < 81 ; i++)
    {
      cd[i] = SPI.transfer(0xFD);
      if( cd == '\0' )
      break;
    }
    processPacket(cd); 
  } 
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH);
  // Close SPI bus
  spiMasterClose(slaveSelectPin);
}
