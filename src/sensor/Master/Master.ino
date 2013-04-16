#include <SPI.h> 

const int slaveSelectPin = 10;
const int slaveReady = 9;
const int LED = 13;
byte control = 0xFD;
char tx[] = "Hello World";
char rx[] = { 0 , 0 };

void setup() 
{
  // Initialize SPI Master
  spiMasterInit( slaveSelectPin );
  pinMode( slaveReady , INPUT );
}

void loop() 
{
  if (digitalRead( slaveReady ) == LOW ){
    spiMasterReceive( rx , slaveSelectPin );
    if ( rx[0] == 1 )
      digitalWrite( LED , HIGH );
    else
      digitalWrite( LED , LOW );
    delay(1000);
  }
  else
    spiMasterSend( tx , slaveSelectPin );
  
}

// Initialize SPI Master
void spiMasterInit( int ss )
{
  // Initialize the SPI bus
  SPI.begin(ss);
  
  // Set order bits are shifted onto the SPI bus
  SPI.setBitOrder( ss , MSBFIRST ); 
  
  // Set SPI Baud Rate to 2MHz
  // 84 MHz / 84 = 2MHz
  SPI.setClockDivider( ss , 42 );
  
  // Set SPI Mode 0-3
  SPI.setDataMode( ss , SPI_MODE0 );
 
  SPI.end( ss ); 
}

void spiMasterSend( char * ab , int ss ) 
{
  // Enable SPI bus
  SPI.begin( ss );
  //  send in the char arra via SPI:
  for( int i = 0; i < sizeof(ab) - 1; i++)
  {
    if( i == (sizeof(ab)- 2) )
      SPI.transfer( slaveSelectPin, ab[i] , SPI_LAST );
    else
      SPI.transfer( slaveSelectPin, ab[i] , SPI_CONTINUE );
    delayMicroseconds(20);
  }
  // Close SPI bus
  SPI.end( ss );
}

void spiMasterReceive( char * ab, int ss )
{
  // Enable SPI bus
  SPI.begin( ss );
  //byte response = SPI.transfer( ss , control );
  //delayMicroseconds( 20 );
  //if( response == control );
  //{
    for( int i = 0 ; i < 81 ; i++ )
    {
      ab[i] = SPI.transfer( ss, 0xFD );
      delayMicroseconds( 20 );
      if( ab[i] == '\0' )
        break;
    } 
  //} 
  // Close SPI bus
  SPI.end( ss );
}
