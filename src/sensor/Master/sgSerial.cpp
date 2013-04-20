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
 
#ifndef __SG_SERIAL_H__
#define __SG_SERIAL_H__

#include "sgSerial.h"

/* Define ARCH_DUE which will be used elsewhere when instructions
specific to this board type are needed */
#ifdef __MCU__
#define ARCH_DUE
#include <Arduino.h>

// initialize serial connection
int8_t sgSerialOpen(void){
    spiMasterInit();
	return 0;
}

// Configure radio as a router radio
// Radio will be configured as a sensor radio by default.
int8_t sgSerialSetAsRouter(void){
    // TBD
	return -1;
}

int8_t sgSerialSetAsSensor(void){
	//TBD
	return -1;
}

// Send size bytes of serialdata serially
int8_t sgSerialSend(SansgridSerial *tx, SansgridSerial **rx, uint32_t size){
	spiMasterOpen( SLAVE_SELECT );
	spiMasterTransmit( tx.control , &rx.control , SLAVE_SELECT );
	spiMasterTransmit( tx.ip_address , &rx.ip_address , SLAVE_SELECT );
	spiMasterTransmit( tx.payload , &rx.payload , SLAVE_SELECT );
	spiMasterClose( SLAVE_SELECT );
	return 0;
}

// Get data from serial in. Data size will be in size.
int8_t sgSerialReceive(SansgridSerial **rx, uint32_t *size){
	spiMasterOpen( SLAVE_SELECT );
	spiMasterReceive( RECEIVE , &rx.control , CONTROL , SLAVE_SELECT );
	spiMasterReceive( RECEIVE , &rx.ip_address , IP_ADDRESS , SLAVE_SELECT );
	spiMasterReceive( RECEIVE , &rx.payload , PAYLOAD , SLAVE_SELECT );
	spiMasterClose( SLAVE_SELECT );
	return 0;
}

// Finish serial connection
// WARNING: This function will change
int8_t sgSerialClose(void){
	SPI.end();
	return 0;
}

#else
#define ARCH_PI

#endif // __MCU__
