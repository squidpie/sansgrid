 /* Definitions for communication functions
 * Specific to the Arduino DUE Platform
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
#include <Arduino.h>
#include "spiMaster.h"

// Initialize SPI Master
void spiMasterInit( int8_t ss , int8_t sr ){
	// have to send on master in, *slave out*
    pinMode( sr , INPUT );
    // Initialize the SPI bus
    SPI.begin( ss );
    // Set order bits are shifted onto the SPI bus
    SPI.setBitOrder( ss , MSBFIRST ); 
    // Set SPI Baud Rate to 333 KHz
    // 84 MHz / 168 = 333 KHz
    SPI.setClockDivider( ss, 252 );
    // Set SPI Mode 0-3
    SPI.setDataMode( ss , SPI_MODE0 );
}

// Receive data to Master from Slave
int8_t spiMasterReceive( int8_t data_out , int8_t * data_in , int8_t size , int8_t ss ){
    // Initiate receive with a single byte transfer
	SPI.transfer( ss , data_out );
	// Loop through untill all characters received
    for( int i = 0 ; i < size ; i++ ){
	    if( i == ( size - 1 ) )
		    data_in[i] = SPI.transfer( ss , data_out , SPI_LAST );
		else
			data_in[i] = SPI.transfer( ss , data_out , SPI_CONTINUE );
		delayMicroseconds( DELAY );
    }
	return 0;
}

// Transmit data to Slave from Master
int8_t spiMasterTransmit( int8_t * data_out , int8_t size , int8_t ss ){
	// Loop through untill all characters transmitted
	for( int i = 0; i < size ; i++){
        if( i == ( size - 1 ) )
		    SPI.transfer( ss , data_out[i] , SPI_LAST );
		else
			SPI.transfer( ss , data_out[i] , SPI_CONTINUE );
		delayMicroseconds( DELAY );
	}
	return 0;
}

// Open SPI bus for Slave Select pin
void spiMasterOpen( int8_t ss ){
    // Open SPI bus ( not needed for Arduino DUE extended )
	// uncomment for UNO
    // digitalWrite( ss , LOW );
	delayMicroseconds( DELAY );
}

// Close SPI bus for Slave Select pin
void spiMasterClose( int8_t ss ){
    // Close SPI bus ( not needed for Arduino DUE extended )
	// uncomment for UNO
    // digitalWrite( ss , HIGH );
	delayMicroseconds( DELAY );
}
