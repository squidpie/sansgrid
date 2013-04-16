/* Definitions for communication functions
 * Specific to the Raspberry Pi Platform
 *
 * Copyright (C) 2013 SansGrid
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 *
 */
 
#include <SPI.h> 

#define SLAVE_SELECT  10        // SS pin 10
#define SLAVE_READY  9          // Hand shake pin identifying Slave has data to send
#define LED 13                  // LED for indicating Master recieved data from Slave

byte control = 0xFD;            // Byte sent when from Master when receiving data from Slave
char tx[] = "Hello World";      // Transmit Buffer
char rx[] = { 0 , 0 };          // Receive Buffer

void setup(){
    // Initialize SPI Master
    spiMasterInit( SLAVE_SELECT );
    
    // Initialize Hand Shake Pin
    pinMode( SLAVE_READY , INPUT );
}

void loop(){
    // If Slave asserts Hand Shake Pin for Data ready to transmit to Master
    if (digitalRead( SLAVE_READY ) == LOW ){
        // Call function for Master to receive data from Slave      
        spiMasterReceive( rx , SLAVE_SELECT );
        
        // If first byte in Receive Buffer is a 1 turn on LED connected to Pin 13
        if ( rx[0] == 1 )
            digitalWrite( LED , HIGH );
        // If first byte in Receive Buffer is a 0 turn off LED connected to Pin 13
        else
            digitalWrite( LED , LOW );
        
        // Insert delay to visibly see the effects of Receiving data from Slave    
        delay(1000);
    }
    // If Master has data to transmit to Slave
    else
        spiMasterTransmit( tx , SLAVE_SELECT );
}

// Initialize SPI Master
void spiMasterInit( int ss ){
    // Initialize the SPI bus
    SPI.begin(ss);
    
    // Set order bits are shifted onto the SPI bus
    SPI.setBitOrder( ss , MSBFIRST ); 
    
    // Set SPI Baud Rate to 2MHz
    // 84 MHz / 84 = 2MHz
    SPI.setClockDivider( ss , 42 );
    
    // Set SPI Mode 0-3
    SPI.setDataMode( ss , SPI_MODE0 );
    
    // Close SPI bus
    SPI.end( ss ); 
}

// Transmit data from Master to Slave
void spiMasterTransmit( char * data , int ss ){
    // Enable SPI bus
    SPI.begin( ss );
    
    // Transmit data from Master to Slave    
    for( int i = 0; i < sizeof(data) - 1 ; i++){
        if( i == ( sizeof( data ) - 2 ) )
            SPI.transfer( ss , data[i] , SPI_LAST );
        else
            SPI.transfer( ss , data[i] , SPI_CONTINUE );
        delayMicroseconds( 20 );
    }
    
    // Close SPI bus
    SPI.end( ss );
}

// Receive data to Master from Slave
void spiMasterReceive( char * data, int ss ){
    // Enable SPI bus
    SPI.begin( ss );
    
    // Loop through untill all characters recieved
    for( int i = 0 ; i < 81 ; i++ ){
        data[i] = SPI.transfer( ss, 0xFD );
        delayMicroseconds( 20 );
        if( data[i] == '\n' )
            break;
    }  
    
    // Close SPI bus
    SPI.end( ss );
}
