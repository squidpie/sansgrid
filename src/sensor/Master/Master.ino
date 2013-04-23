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
#include <spiMaster.h>
#include <string.h>

void setup(){
    // Initialize Sensor SPI communication
    //sgSerialSetAsSensor();
    sgSerialOpen();
}

void loop(){
    const char control_test[2] = { 0xFF };
    const char iptest[4] = "HIH";
    const char payload_test[5] = "HIHI";
    SansgridSerial PayloadIn;
    strcpy(PayloadIn.control , control_test);
    strcpy(PayloadIn.ip_addr, iptest);
    strcpy(PayloadIn.payload, payload_test);
    SansgridSerial PayloadOut;
    
    uint32_t num_bytes = 8;

    sgSerialSend( &PayloadIn , num_bytes ); 
    delayMicroseconds(6);   
}

