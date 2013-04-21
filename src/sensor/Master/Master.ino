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
#include <sgSerial.h>
#include <sgSerial_due.h>

void setup(){
    // Initialize Sensor SPI communication
    sgSerialSetAsSensor();
}

void loop(){
    SansgridSerial PayloadIn;
    PayloadIn.control = { 0x00 };
    PayloadIn.ip_addr = { 0x01 };
    PayloadIn.payload = { 0xFF };
    SansgridSerial PayloadOut;
    
    int num_bytes = 98;
    char value = 0;
    
    sgSerialOpen();
    sgSerialSend( &PayloadIn , &PayloadIn , num_bytes );
    sgSerialReceive( &PayloadIn , num_bytes );
    sgSerialClose();
}

