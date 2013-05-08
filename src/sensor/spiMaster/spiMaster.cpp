/* SPI Master implementation
 * specific to the Arduino DUE Platform
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
 
#include <Arduino.h>
#include "spiMaster.h"

//#define DUE 1

// Initialize SPI Master
void spiMasterInit( int ss , int sr ){
	// Have to send on Master In Slave Out
	pinMode( sr , INPUT );
	#ifdef DUE
	// Initialize the SPI bus
	SPI.begin( ss );
	// Set order bits are shifted onto the SPI bus
	SPI.setBitOrder( ss , MSBFIRST ); 
	// Set SPI Baud Rate to 500 KHz
	// 84 MHz / 252 = 500 KHz
	SPI.setClockDivider( ss, 168 );
	// Set SPI Mode 0-3
	SPI.setDataMode( ss , SPI_MODE0 );
	#else // End DUE code Begin UNO
	// Initialize the SPI bus
	SPI.begin();
	// Set order bits are shifted onto the SPI bus
	SPI.setBitOrder( MSBFIRST ); 
	// Set SPI Baud Rate to 500 KHz
	// 16 MHz / 168 = 333 KHz
	SPI.setClockDivider( SPI_CLOCK_DIV32 );
	// Set SPI Mode 0-3
	SPI.setDataMode( SPI_MODE0 );
	#endif // DUE
}

// Receive ASCII char (int8_t) to Master from Slave
void spiMasterReceive( byte  data_out , char * data_in , int size , int ss ){
	// Loop through untill all characters received
	for( int i = 0 ; i < size ; i++ ){
		if( i == ( size - 1 ) ){
			#ifdef DUE
			data_in[i] = SPI.transfer( ss , data_out , SPI_LAST );
			#else // End Due code Begin Uno
			spiMasterOpen( ss );
			data_in[i] = SPI.transfer( data_out );
			spiMasterClose( ss );
			#endif // DUE
		}
		else{
			#ifdef DUE
			data_in[i] = SPI.transfer( ss , data_out , SPI_CONTINUE );
			#else // End Due code Begin Uno
			spiMasterOpen( ss );
			data_in[i] = SPI.transfer( data_out );
			spiMasterClose( ss );
			#endif // DUE
		}
		delayMicroseconds( DELAY );
	}
}

// Receive BYTE (uint8_t) to Master from Slave
void spiMasterReceive( byte  data_out , byte * data_in , int size , int ss ){
	// Loop through untill all characters received
	for( int i = 0 ; i < size ; i++ ){
		if( i == ( size - 1 ) ){
			#ifdef DUE
			data_in[i] = SPI.transfer( ss , data_out , SPI_LAST );
			#else // End Due code Begin Uno
			spiMasterOpen( ss );
			data_in[i] = SPI.transfer( data_out );
			spiMasterClose( ss );
			#endif // DUE
		}
		else{
			#ifdef DUE
			data_in[i] = SPI.transfer( ss , data_out , SPI_CONTINUE );
			#else // End Due code Begin Uno
			spiMasterOpen( ss );
			data_in[i] = SPI.transfer( data_out );
			spiMasterClose( ss );
			#endif // DUE
		}
		delayMicroseconds( DELAY );
	}
}

// Transmit ASCII char (int8_t) to Slave from Master
void spiMasterTransmit( char * data_out , int size , int ss ){
	// Loop through untill all characters transmitted
	for( int i = 0; i < size ; i++){
		if( i == ( size - 1 ) ){
			#ifdef DUE
			SPI.transfer( ss , data_out[i] , SPI_LAST );
			#else // End Due code Begin Uno
			spiMasterOpen( ss );
			SPI.transfer( data_out[i] );
			spiMasterClose( ss );
			#endif // DUE
		}
		else{
			#ifdef DUE
			SPI.transfer( ss , data_out[i] , SPI_CONTINUE );
			#else // End Due code Begin Uno
			spiMasterOpen( ss );
			SPI.transfer( data_out[i] );
			spiMasterClose( ss );
			#endif // DUE
		}
		delayMicroseconds( DELAY );
	}
}

// Transmit BYTE (uint8_t) to Slave from Master
void spiMasterTransmit( byte * data_out , int size , int ss ){
	// Loop through untill all characters transmitted
	for( int i = 0; i < size ; i++){
		if( i == ( size - 1 ) ){
			#ifdef DUE
			SPI.transfer( ss , data_out[i] , SPI_LAST );
			#else // End Due code Begin Uno
			spiMasterOpen( ss );
			SPI.transfer( data_out[i] );
			spiMasterClose( ss );
			#endif // DUE
		}
		else{
			#ifdef DUE
			SPI.transfer( ss , data_out[i] , SPI_CONTINUE );
			#else // End Due code Begin Uno
			spiMasterOpen( ss );
			SPI.transfer( data_out[i] );
			spiMasterClose( ss );
			#endif // DUE
		}
		delayMicroseconds( DELAY );
	}
}

// Transmit Padding as a BYTE
void spiMasterPadding( byte data_out , int size , int ss ){
	for( int i = size ; i < NUM_BYTES ; i++){
		if( i == ( NUM_BYTES - 1 ) ){
			#ifdef DUE
			SPI.transfer( ss , data_out , SPI_LAST );
			#else // End Due code Begin Uno
			spiMasterOpen( ss );
			SPI.transfer( data_out );
			spiMasterClose( ss );
			#endif // DUE
		}
		else{
			#ifdef DUE
			SPI.transfer( ss , data_out , SPI_CONTINUE );
			#else // End Due code Begin Uno
			spiMasterOpen( ss );
			SPI.transfer( data_out );
			spiMasterClose( ss );
			#endif // DUE
		}
		delayMicroseconds( DELAY );
	}
}

// Open SPI bus for Slave Select pin, only used on UNO
// Extended functionality of DUE does this automatically
void spiMasterOpen( int ss ){
	// Open SPI bus ( not needed for Arduino DUE extended )
	digitalWrite( ss , LOW );
	delayMicroseconds( DELAY );
}

// Close SPI bus for Slave Select pin , only used on UNO
// Extended functionality of DUE does this automatically
void spiMasterClose( int ss ){
	// Close SPI bus ( not needed for Arduino DUE extended )
	digitalWrite( ss , HIGH );
	delayMicroseconds( DELAY );
}
