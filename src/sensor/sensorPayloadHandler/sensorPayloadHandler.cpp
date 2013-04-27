/* Sensor Payload Handler
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

#ifndef __SENSOR_PAYLOAD_HANDLER_H__
#define __SENSOR_PAYLOAD_HANDLER_H__

#include <Arduino.h>
#include <sgSerial.h> 
#include <sensorPayloads.h>
#indlude "sensorPayloadHandler.h"

// Payload Handler
int8_t payloadHandler( SansgridSerial *sg_serial, SensorConfig *sensor_config ){
    int8_t command = sg_serial->payload[0];
	switch ( command ){
		case: 0x00 	// Eyeball Sensor entering network.
			eyeball( sg_serial , sensor_config );
			sensor_config->eyeball = true;
			break;
		case: 0x01 	// Peck 	Initial server response
			if( sensor_config->eyeball == true )
			    peck( sg_serial , sensor_config );
			sensor_config->eyeball = false;
			break;		
		case: 0x02 	// Sing 	The server has a public key to share with the sensor for future authentication challenges.
			if( sensor_config->peck == true )
			    sing( sg_serial , sensor_config );
			sensor_config->peck = false;
			break;
		case: 0x03 	// Sing 	Server does not require authentication, ready to accept sensor key if needed.
			if( sensor_config->peck == true )
			    sing( sg_serial , sensor_config );
			sensor_config->peck = false;			
			break;
		case: 0x07 	// Mock 	The sensor has a public key to share with the sensor for future authentication challenges.
			if( sensor_config->sing == true )
				mock( sg_serial , sensor_config );
			sensor_config->sing = false;
			break;
		case: 0x08 	// Mock 	Sensor does not require authentication, ready to accept sensor key if needed.
			if( sensor_config->sing == true )
				mock( sg_serial , sensor_config );
			sensor_config->sing = false;
			break;
		case: 0x0C 	// Peacock 	Sensor share's capabilities with server.
			if( sensor_config->mock == true )
				peacock( sg_serial , sensor_config );
			sensor_config->mock = false;
			break;
		case: 0x10 	// Nest 	Server accepts sensor into network.
					 break;
		case: 0x11 	// Squawk 	Server challenge
					 break;
		case: 0x12 	// Squawk 	Server doesn't need challenge
					 break;
		case: 0x15 	// Squawk 	Sensor response to server challenge, no sensor challenge needed.
					 break;
		case: 0x16 	// Squawk 	Sensor response to server challenge, sensor challenge coming.
					 break;
		case: 0x17 	// Squawk 	Sensor challenge.
					 break;
		case: 0x1b 	// Squawk 	Server denies sensor's challenge response.
					 break;
		case: 0x1c 	// Squawk 	Server response to challenge.
					 break;
		case: 0x1d 	// Squawk 	Sensor accepts server's response
					 break;
		case: 0x1e 	// Heartbeat 	Router pulse to sensor
					 break;
		case: 0x1f 	// Heartbeat 	Sensor's response to router's pulse
					 break;
		case: 0x20 	// Chirp 	Command sent from server to sensor.
					 break;
		case: 0x21 	// Chirp 	Chirp sent from sensor to server.
					 break;
		case: 0x22 	// Chirp 	Start of data stream.
					 break;
		case: 0x23 	// Chirp 	Continued stream of data.
					 break;
		case: 0x24 	// Chirp 	End of data stream.
					 break;
		case: 0x25 	// Chirp 	Network is disconnecting sensor.
					 break;
		case: 0x26 	// Chirp 	Sensor is disconnecting from the network.
					 break;
		case: 0xF0 	// Flying 	Broadcast from router identifying the network
					 break;
		case: 0xFE 	// -- 	Reserved for future expansion
					 break;
		case: 0xFF 	// -- 	Reserved for future expansion 
					 break;
		default: 
					 break;

	}				 
}
					 
// Payloads sent from Sensor to Router 
int8_t eyeball( SansgridSerial *tx , SansgridEyeball *sg_eyeball );
int8_t mock( SansgridSerial *tx , SansgridMock *sg_mock );
int8_t peacock( SansgridSerial *tx , SansgridPeacock *sg_peacock );
int8_t squawk( SansgridSerial *tx , SansgridSquawk *sg_squawk ); 

// Payloads recieved at Sensor from Router
int8_t peck( SansgridSerial *rx , SansgridPeck *sg_peck );
int8_t sing( SansgridSerial *rx , SansgridSing *sg_sing );
int8_t nest( SansgridSerial *rx , SansgridNest *sg_nest );

#endif // __SENSOR_PAYLOAD_HANDLER_H__