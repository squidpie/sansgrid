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
#include <sensorParse.h>
#include <sensorPayloads.h>
#include <sgSerial.h>

//#define PUSH_BUTTON 1

void payloadHandler( SensorConfig *sg_config , SansgridSerial *sg_serial );
void peck( SensorConfig *sg_config , SansgridPeck *sg_peck );
void sing( SensorConfig *sg_config , SansgridSing *sg_sing );
void peacock( SensorConfig *sg_config , SansgridPeacock *sg_sing);
void authenticateKey( SensorConfig *sg_config , SansgridSquawk *sg_squawk );
void sensorConnect( SensorConfig *sg_config , SansgridSerial *sg_serial );

// Payload Handler
void payloadHandler( SensorConfig *sg_config , SansgridSerial *sg_serial){
    uint8_t command = sg_serial->payload[0];
    switch ( command ){ 
        case 0x00 :	// Eyeball - Sensor entering network.
            SansgridEyeball sg_eyeball;
            if( sg_config->mate == false )
                sg_eyeball.mode[0] = (uint8_t) 0x00;
			else
			    sg_eyeball.mode[0] = (uint8_t) 0x01;
            transmitEyeball( sg_serial , &sg_eyeball );
			sg_config->fly = false;
			break;
        case 0x01 :	// Peck - Initial server response
            SansgridPeck sg_peck;
            parsePeck( sg_serial , &sg_peck );
            peck( sg_config , &sg_peck );
            break;		
        case 0x02 :	// Sing - The server has a public key to share with the sensor for future authentication challenges.		
        case 0x03 :	// Sing - Server does not require authentication, ready to accept sensor key if needed.
            SansgridSing sg_sing;
            parseSing( sg_serial , &sg_sing );
            sing( sg_config , &sg_sing );
			sg_config->sing = true;
            break;
        case 0x07 :	// Mock - The sensor has a public key to share with the sensor for future authentication challenges.
            SansgridMock sg_mock_key;
			sg_mock_key.dt[0] = (uint8_t) 0x07;
            delay(1000);
            transmitMock( sg_serial , &sg_mock_key );
            sg_config->mock = true;
			sg_config->sing = false;
			break;
        case 0x08 :	// Mock - Sensor does not require authentication, ready to accept sensor key if needed.
            SansgridMock sg_mock_nokey;
			sg_mock_key.dt[0] = (uint8_t) 0x08;
            delay(1000);
            transmitMock( sg_serial , &sg_mock_nokey );
			sg_config->mock = true;
			sg_config->sing = false;
			break;
        case 0x0C :	// Peacock - Sensor share's capabilities with server.
            SansgridPeacock sg_peacock;
            transmitPeacock( sg_serial , &sg_peacock );
			if( sg_peacock.additional[0] == 0x01 ){
			    SansgridPeacock sg_peacock_add;
                peacock( sg_config , &sg_peacock_add );
                transmitPeacock( sg_serial , &sg_peacock_add );
			}
			sg_config->mock = false;
            break;
        case 0x10 :	// Nest - Server accepts sensor into network.
            sg_config->nest = true;
            #ifdef PUSH_BUTTON
            sg_config->mate = false;
            #endif // PUSH_BUTTON
            break;
        case 0x11 :	// Squawk - Server challenge
        case 0x12 :	// Squawk - Server doesn't need challenge
        case 0x15 :	// Squawk - Sensor response to server challenge, sensor challenge coming.
            SansgridSquawk sg_squawk;
            parseSquawk( sg_serial , &sg_squawk );
            authenticateKey( sg_config , &sg_squawk);
            transmitSquawk ( sg_serial , &sg_squawk );
            break;
        case 0x17 :	// Squawk - Sensor challenge.
			break;
		case 0x1b :	// Squawk - Server denies sensor's challenge response.
			sg_config->nest = false;
			break;
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
			sg_config->nest = false;
			break;
		case 0x27 :	// Squawk - Sensor has forgotten Server, Server please forget Sensor.
			break;
		case 0xF0 :	// Flying - Broadcast from router identifying the network
            SansgridFly sg_fly;
            parseFly( sg_serial , &sg_fly );
			memcpy( sg_config->network_name , sg_fly.network_name , DATA );
			sg_config->fly = true;
			break;
        case 0xFE :	// - Reserved for future expansion
            break;
        case 0xFF :	// - Reserved for future expansion
            break;
        default : 
            break;
    }		 
}

void peck( SensorConfig *sg_config , SansgridPeck *sg_peck ){
    memcpy( sg_config->router_ip , sg_peck->router_ip , IP_ADDRESS );
    memcpy( sg_config->ip_address , sg_peck->ip_address , IP_ADDRESS );
    if( sg_peck->recognition == 0x00 || 0x02 || 0x03 ){
        sg_config->nest = false;
		sg_config->fly = false;
	}
}

void sing( SensorConfig *sg_config , SansgridSing *sg_sing ){
    if( sg_sing->dt[0] == 0x02 ){
	    memcpy( sg_config->server_public_key , sg_sing->server_public_key , SERVER_KEY );
    }
}

void peacock( SensorConfig *sg_config , SansgridPeacock *sg_sing){
    //TBD
}

void authenticateKey( SensorConfig *sg_config , SansgridSquawk *sg_squawk ){
    uint16_t count = 0;
    for( int i = 0 ; i < DATA ; i++ ){
        if( sg_config->server_public_key[i] ^ sg_config->sensor_public_key[i] )
            count++;
    }
    uint8_t hi_lo[2] = { (uint8_t)( count >> 8 ), (uint8_t)count };
	for( int i = 0 ; i < DATA - 2 ; i++ )
	    sg_squawk->data[i] = 0x00;
    sg_squawk->data[78] = hi_lo[1];
    sg_squawk->data[79] = hi_lo[2];
}

void sensorConnect( SensorConfig *sg_config , SansgridSerial *sg_serial ){
    // Received a Squawk packet, now send a Squawk back
    if ( sg_config->squawk ){
        //squawk 
        SansgridSquawk sg_squawk;
    }
	// Sent a Mock packet, now send a Peacock packet
	else if ( sg_config->mock ){
		//peacock
		sg_serial->payload[0] = 0x0C;
	}
	// Received a Sing packet, send a Mock packet
	else if ( sg_config->sing ){
		//mock
		if( sg_config->sensor_public_key )
			sg_serial->payload[0] = 0x08;
		else
			sg_serial->payload[0] = 0x07;            
	}
	// Received Fly packet, send an Eyeball packet
	else if( sg_config->fly ){
		//eyeball
		sg_serial->payload[0] = 0x00;
	}
	// Delay 1 second between packets
	// during mating 
	delay(1000);
}

#endif // __SENSOR_PAYLOAD_HANDLER_H__
