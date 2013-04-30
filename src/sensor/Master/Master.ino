/* SPI Master Test Code
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
#include <sgSerial.h>
#include <spiMaster.h>
#include <string.h>

int8_t control_test[ 1 ] = { 0xAD };
int8_t iptest[ 16 ] = {};
int8_t payload_test[ 81 ] = { 0x00 };

void setup(){
    // Initialize Sensor SPI communication
    //sgSerialSetAsSensor();
    sgSerialOpen();
    attachInterrupt( SLAVE_READY , receive , LOW );
}

void loop(){
    SansgridSerial PayloadOut;
    memcpy(PayloadOut.control, control_test , sizeof control_test );
    memcpy(PayloadOut.ip_addr, iptest , sizeof iptest );
    memcpy(PayloadOut.payload, payload_test , sizeof payload_test );
    sgSerialSend( &PayloadOut , NUM_BYTES ); 
    delayMicroseconds(6);   
}

void receive(){
    // Create empty Payload Struct to receive payload 
    SansgridSerial PayloadIn;
    // Read payload
    sgSerialReceive( &PayloadIn , NUM_BYTES );
    // Process Payload
}
