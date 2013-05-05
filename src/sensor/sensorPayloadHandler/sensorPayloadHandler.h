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
#include <sensorConnect.h>
#include <sensorParse.h>
#include <sensorPayloads.h>
#include <spiMaster.h>
#include <sgSerial.h>

void payloadHandler( SensorConfig *sg_config );
void receivePayload( SensorConfig *sg_config , SansgridSerial * rx );
void peck( SensorConfig *sg_config , SansgridPeck *sg_peck );
void sing( SensorConfig *sg_config , SansgridSing *sg_sing );

// Payload Handler
void payloadHandler( SensorConfig *sg_config ){
	SansgridSerial payload;
	receivePayload( sg_config , &payload );
	byte command = payload.payload[0];
	switch ( command ){
		case 0x00 :	// Eyeball - Sensor entering network.
					break;
		case 0x01 :	// Peck - Initial server response
					SansgridPeck sg_peck;
					parsePeck( &payload , &sg_peck );
					peck( sg_config , &sg_peck );
					break;		
		case 0x02 :	// Sing - The server has a public key to share with the sensor for future authentication challenges.		
		case 0x03 :	// Sing - Server does not require authentication, ready to accept sensor key if needed.
					SansgridSing sg_sing;
					parseSing( &payload , &sg_sing );
					sing( sg_config , &sg_sing );
					SansgridMock sg_mock;
					transmitMock( sg_config , &sg_mock );
					// TBD 
					// Need to determine wait time, testing with one second between mock and peacock
					delay(1000);
					SansgridPeacock sg_peacock;
					transmitPeacock( sg_config , &sg_peacock );
					break;
		case 0x07 :	// Mock - The sensor has a public key to  with the sensor for future authentication challenges.
					break;
		case 0x08 :	// Mock - Sensor does not require authentication, ready to accept sensor key if needed.
					break;
		case 0x0C :	// Peacock - Sensor share's capabilities with server.
					break;
		case 0x10 :	// Nest - Server accepts sensor into network.
					SansgridNest sg_nest;
					parseNest( &payload , &sg_nest );
					sg_config->connected = true;
					break;
		case 0x11 :	// Squawk - Server challenge
		case 0x12 :	// Squawk - Server doesn't need challenge
		case 0x15 :	// Squawk - Sensor response to server challenge, sensor challenge coming.
		case 0x17 :	// Squawk - Sensor challenge.
		case 0x1b :	// Squawk - Server denies sensor's challenge response.
		case 0x1c :	// Squawk - Server response to challenge.
		case 0x1d :	// Squawk - Sensor accepts server's response
					break;
		case 0x1e :	// Heartbeat - Router pulse to sensor
					break;
		case 0x1f :	// Heartbeat - Sensor's response to router's pulse
					break;
		case 0x20 :	// Chirp - Command sent from server to sensor.
		case 0x21 :	// Chirp - Chirp sent from sensor to server.
					break;
		case 0x22 :	// Chirp - Start of data stream.
					break;
		case 0x23 :	// Chirp - Continued stream of data.
					break;
		case 0x24 :	// Chirp - End of data stream.
					break;
		case 0x25 :	// Chirp - Network is disconnecting sensor.
		case 0x26 :	// Chirp - Sensor is disconnecting from the network.
					break;
		case 0xF0 :	// Flying - Broadcast from router identifying the network
					SansgridFly sg_fly;
					parseFly( &payload , &sg_fly );
					SansgridEyeball sg_eyeball;
					transmitEyeball( sg_config , &sg_eyeball );
					break;
		case 0xFE :	// - Reserved for future expansion
					break;
		case 0xFF :	// - Reserved for future expansion
					break;
		default : 
					break;
	}		 
}

void receivePayload( SensorConfig *sg_config , SansgridSerial * rx ){
	spiMasterTransmit( sg_config->control , CONTROL , SLAVE_SELECT );
	spiMasterReceive( sg_config->control[0] , rx->control , CONTROL , SLAVE_SELECT );
	spiMasterReceive( sg_config->control[0] , rx->ip_addr , IP_ADDRESS , SLAVE_SELECT );
	spiMasterReceive( sg_config->control[0] , rx->payload , PAYLOAD , SLAVE_SELECT );
}

void peck( SensorConfig *sg_config , SansgridPeck *sg_peck ){
	memcpy( sg_config->router_ip , sg_peck->router_ip , IP_ADDRESS );
	memcpy( sg_config->ip_address , sg_peck->ip_address , IP_ADDRESS );
	// TBD
	// Disconnect start again if sg->recognition == 0x02 or 0x03
}

void sing( SensorConfig *sg_config , SansgridSing *sg_sing ){
	memcpy( sg_config->server_public_key , sg_sing->server_public_key , SERVER_KEY );
	// TBD
	// Server doesn't require a key for authentication
}

#endif // __SENSOR_PAYLOAD_HANDLER_H__