/* Serial Communication implementation
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

 #include "sgSerial.h"
 #include <Arduino.h>
 #include <SPI.h>
 #include <spiMaster.h>
 
// Opens serial device for reading/writing, configures ports, sets order data 
// bits  are shifted in as MSB or LSB, and sets the clock frequency. Function 
// is called prior to sending or receiving any data over the serial line. 
int sgSerialOpen(void){
    spiMasterInit( SLAVE_SELECT , SLAVE_READY );
	return 0;
}

// Configure radio as a router radio
// Radio will be configured as a sensor radio by default.
int sgSerialSetAsRouter(void){
    // TBD
	return -1;
}

int sgSerialSetAsSensor(void){
	//TBD
	return -1;
}

// Send size bytes of serialdata serially
int sgSerialSend(SansgridSerial *sg_serial, int size ){
	byte padding = 0x00;
	spiMasterTransmit( sg_serial->control ,  CONTROL , SLAVE_SELECT );
	spiMasterTransmit( sg_serial->ip_addr , IP_ADDRESS , SLAVE_SELECT );
	spiMasterTransmit( sg_serial->payload ,  PAYLOAD , SLAVE_SELECT );
	// Pad Packet to necessary size
	if( size > CONTROL + IP_ADDRESS + PAYLOAD ){
	    for( int i = CONTROL + IP_ADDRESS + PAYLOAD ; i < size ; i++ ){ 	
            if( i == ( size - 1 ) )
		        SPI.transfer( SLAVE_SELECT , padding , SPI_LAST );
			else
				SPI.transfer( SLAVE_SELECT , padding , SPI_CONTINUE );
			delayMicroseconds( DELAY );
		}
	}
	return 0;
}

// Get data from serial in. Data size will be in size.
int sgSerialReceive(SansgridSerial *sg_serial, int size){
	byte padding = 0x00;
	SPI.transfer( SLAVE_SELECT , RECEIVE , SPI_LAST  );
	spiMasterReceive( RECEIVE , sg_serial->control , CONTROL , SLAVE_SELECT );
	spiMasterReceive( RECEIVE , sg_serial->ip_addr , IP_ADDRESS , SLAVE_SELECT );
	spiMasterReceive( RECEIVE , sg_serial->payload , PAYLOAD , SLAVE_SELECT );
	// Pad Packet to necessary size
	if( size > CONTROL + IP_ADDRESS + PAYLOAD ){
	    for( int i = CONTROL + IP_ADDRESS + PAYLOAD ; i < size ; i++ ){ 	
			if( i == ( size - 1 ) )
		        SPI.transfer( SLAVE_SELECT , padding , SPI_LAST );
			else
				SPI.transfer( SLAVE_SELECT , padding , SPI_CONTINUE );
			delayMicroseconds( DELAY );
		}
	}
	return 0;
}

// Function is called when all SPI input and output is completed. Stops SPI 
// from being transmitted and received. 
int sgSerialClose(void){
	SPI.end();
	return 0;
}
