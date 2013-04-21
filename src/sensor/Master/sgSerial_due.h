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
#ifndef __SG_SERIAL_DUE_H__
#define __SG_SERIAL_DUE_H__

// Define ARCH_DUE which will be used elsewhere when instructions
// specific to this board type are needed
#define ARCH_DUE

// Initialize SPI Master
void spiMasterInit(){
    // Initialize the SPI bus
    SPI.begin();
    // Set order bits are shifted onto the SPI bus
    SPI.setBitOrder( MSBFIRST ); 
    // Set SPI Baud Rate to 500 KHz
    // 84 MHz / 168 = 500 KHz
    SPI.setClockDivider( 168 );
    // Set SPI Mode 0-3
    SPI.setDataMode( SPI_MODE0 );
}

// Receive data to Master from Slave
void spiMasterReceive( int data_out , char * data_in , int size ){
    // Loop through untill all characters received
    for( int i = 0 ; i < size ; i++ ){
        data_in[i] = SPI.transfer( data_out );
    }
}

// Transmit data to Slave from Master
void spiMasterTransmit( char * data_out , char * data_in , int size ){
    for( int i = 0; i < size ; i++){
        data_in[i] = SPI.transfer( data_out[i] );
    } 
}

// Open SPI bus for Slave Select pin
void spiMasterOpen( int ss ){
    // Open SPI bus
    digitalWrite( ss , LOW );
}

// Close SPI bus for Slave Select pin
void spiMasterClose( int ss ){
    // Close SPI bus
    digitalWrite( ss , HIGH );
}

#endif // __SG_SERIAL_DUE_H
