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
bool compareResponse( SensorConfig *sg_config , SansgridSquawk *sg_squawk );
void sensorConnect( SensorConfig *sg_config , SansgridSerial *sg_serial );

// Payload Handler
void payloadHandler( SensorConfig *sg_config , SansgridSerial *sg_serial){
    Serial.println( "payloadHandler");
    uint8_t command = sg_serial->payload[0];
	Serial.println( command );
    switch ( command ){ 
        case 0x00 :	// Eyeball - Sensor entering network.
            SansgridEyeball sg_eyeball;
            if( sg_config->mate == false )
                sg_eyeball.mode[0] = (uint8_t) 0x00;
			else
			    sg_eyeball.mode[0] = (uint8_t) 0x01;
			Serial.println( "Sending Eyeball");
            transmitEyeball( sg_serial , &sg_eyeball );
			//sg_config->fly = false;
			break;
        case 0x01 :	// Peck - Initial server response
            SansgridPeck sg_peck;
            parsePeck( sg_serial , &sg_peck );
            peck( sg_config , &sg_peck );
            break;		
        case 0x02 :	// Sing - The server has a public key to share with 
		    // the sensor for future authentication challenges.		
        case 0x03 :	// Sing - Server does not require authentication, 
		    // ready to accept sensor key if needed.
            SansgridSing sg_sing;
            parseSing( sg_serial , &sg_sing );
            sing( sg_config , &sg_sing );
			//sg_config->sing = true;
            break;
        case 0x07 :	// Mock - The sensor has a public key to share with 
		    // the sensor for future authentication challenges.
            SansgridMock sg_mock_key;
			sg_mock_key.dt[0] = (uint8_t) 0x07;
			Serial.println( "Sending Mock 07" );
            transmitMock( sg_serial , &sg_mock_key );
            //sg_config->mock = true;
			//sg_config->sing = false;
			break;
        case 0x08 :	// Mock - Sensor does not require authentication, 
		    // ready to accept sensor key if needed.
			SansgridMock sg_mock_nokey;
			sg_mock_key.dt[0] = (uint8_t) 0x08;
			Serial.println( "Sending Mock 08" );
            transmitMock( sg_serial , &sg_mock_nokey );
			//sg_config->mock = true;
			//sg_config->sing = false;
			break;
        case 0x0C :	// Peacock - Sensor share's capabilities with server.
            SansgridPeacock sg_peacock;
			Serial.println( "Sending Peacock" );
			memcpy( &sg_peacock.a , &sg_config->a , sizeof(SansgridSensor));
			memcpy( &sg_peacock.a , &sg_config->b , sizeof(SansgridSensor));
			sg_peacock.additional[0] = (uint8_t) 0x00;
			transmitPeacock( sg_serial , &sg_peacock );
			if( sg_peacock.additional[0] == 0x01 ){
			    SansgridPeacock sg_peacock_add;
                peacock( sg_config , &sg_peacock_add );
                transmitPeacock( sg_serial , &sg_peacock_add );
			}
			//sg_config->mock = false;
            break;
        case 0x10 :	// Nest - Server accepts sensor into network.
            sg_config->nest = true;
            #ifdef PUSH_BUTTON
            sg_config->mate = false;
            #endif // PUSH_BUTTON
            break;
        case 0x11 :	// Squawk - Server challenge
			SansgridSquawk sg_squawk_noauth;
            parseSquawk( sg_serial , &sg_squawk_noauth );
			if( sg_config->challenge )
			    sg_serial->payload[0] = (uint8_t) 0x16;
			else
			    sg_serial->payload[0] = (uint8_t) 0x15;
			//sg_config->squawk = true;
			break;
        case 0x12 :	// Squawk - Server doesn't need challenge
			SansgridSquawk sg_squawk_auth;
            parseSquawk( sg_serial , &sg_squawk_auth);
            authenticateKey( sg_config , &sg_squawk_auth );
			if( sg_config->challenge )
			    sg_serial->payload[0] = (uint8_t) 0x16;
			else
			    sg_serial->payload[0] = (uint8_t) 0x15;
			//sg_config->squawk = true;
			break;
        case 0x15 :	// Squawk - Sensor response to server squawk no 
		    // challenge needed.
            SansgridSquawk sg_squawk_sensor_noauth;
            parseSquawk( sg_serial , &sg_squawk_sensor_noauth );
            transmitSquawk ( sg_serial , &sg_squawk_sensor_noauth );
			//sg_config->squawk = false;
            break;
        case 0x16 :	// Squawk - Sensor acknowledge.
			SansgridSquawk sg_squawk_acknowledge;
            parseSquawk( sg_serial , &sg_squawk_acknowledge );
            transmitSquawk( sg_serial , &sg_squawk_acknowledge );
			sg_serial->payload[0] = (uint8_t) 0x17;
			break;
		case 0x17 :	// Squawk - Sensor response to challenge.
			SansgridSquawk sg_squawk_sensor_response;
            parseSquawk( sg_serial , &sg_squawk_sensor_response );
            transmitSquawk( sg_serial , &sg_squawk_sensor_response );
			//sg_config->squawk = false;
			break;
		case 0x1b :	// Squawk - Server denies sensor's challenge response.
			break;
		case 0x1c :	// Squawk - Server response to challenge.
			SansgridSquawk sg_squawk_response;
            parseSquawk( sg_serial , &sg_squawk_response );
            if( compareResponse( sg_config , &sg_squawk_response ) )
			    sg_serial->payload[0] = (uint8_t) 0x1d;
			break;
		case 0x1d :	// Squawk - Sensor accepts server's response
		    SansgridSquawk sg_squawk_accept;
            parseSquawk( sg_serial , &sg_squawk_accept );
			break;
		case 0x1e :	// Heartbeat - Router pulse to sensor
			break;
		case 0x1f :	// Heartbeat - Sensor's response to router's pulse
			break;
		case 0x20 :	// Chirp - Command sent from server to sensor.
		    SansgridChirp sg_chirp_in;
			parseChirp( sg_serial , &sg_chirp_in );
			sg_config->chirp = true;
			break;
		case 0x21 :	// Chirp - Chirp sent from sensor to server.
		    SansgridChirp sg_chirp_out;
			transmitChirp( sg_serial , &sg_chirp_out );
			break;
		case 0x22 :	// Chirp - Start of data stream.
		case 0x23 :	// Chirp - Continued stream of data.
		case 0x24 :	// Chirp - End of data stream.
			break;
		case 0x25 :	// Chirp - Network is disconnecting sensor.
		case 0x26 :	// Chirp - Sensor is disconnecting from the network.
			SansgridChirp sg_chirp_disconnect;
			parseChirp( sg_serial , &sg_chirp_disconnect );
			transmitChirp( sg_serial , &sg_chirp_disconnect );
			//sg_config->nest = false;
			break;
		case 0x27 :	// Squawk - Sensor has forgotten Server, 
		    // Server forget Sensor.
			SansgridSquawk sg_squawk_forget;
			sg_squawk_forget.dt[0] = (uint8_t) 0x27;
			transmitSquawk( sg_serial , &sg_squawk_forget );
			//sg_config->fly = true;
			break;
		case 0xF0 :	// Flying - Broadcast from router identifying the network
            SansgridFly sg_fly;
            parseFly( sg_serial , &sg_fly );
			memcpy( sg_config->network_name , sg_fly.network_name , DATA );
			//sg_config->fly = true;
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
        //sg_config->nest = false;
		//sg_config->fly = false;
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
	    sg_squawk->data[i] = (uint8_t) 0x00;
    sg_squawk->data[78] = hi_lo[1];
    sg_squawk->data[79] = hi_lo[2];
}

bool compareResponse( SensorConfig *sg_config , SansgridSquawk *sg_squawk ){
    bool correct = false;
	uint16_t count = 0;
    for( int i = 0 ; i < DATA ; i++ ){
        if( sg_config->server_public_key[i] ^ sg_config->sensor_public_key[i] )
            count++;
    }
    uint8_t hi_lo[2] = { (uint8_t)( count >> 8 ), (uint8_t)count };
	if(( sg_squawk->data[78] == hi_lo[0] ) && ( sg_squawk->data[79] = hi_lo[1] ))
	    correct = true;
	return correct;
}

void sensorConnect( SensorConfig *sg_config , SansgridSerial *sg_serial ){
    while( sg_config->nest == false ){    
	// Received a Squawk packet, now send a Squawk back
    if ( sg_config->squawk == true ){
        // Squawk 
        Serial.println( "SQUAWKING" );
		sg_serial->control[0] = (uint8_t) 0xAD;
		memcpy( sg_serial->ip_addr , sg_config->router_ip , IP_ADDRESS );
		payloadHandler( sg_config , sg_serial);
    }
	// Sent a Mock packet, now send a Peacock packet
	else if ( sg_config->mock == true ){
		// Peacock
		Serial.println( "PEACOCKING" );
		sg_serial->control[0] = (uint8_t) 0xAD;
		memcpy( sg_serial->ip_addr , sg_config->router_ip , IP_ADDRESS );
		sg_serial->payload[0] = (uint8_t) 0x0C;
		payloadHandler( sg_config , sg_serial);
	}
	// Received a Sing packet, send a Mock packet
	else if ( sg_config->sing == true ){
		// Mock
		Serial.println( "MOCKING" );
		sg_serial->control[0] = (uint8_t) 0xAD;
		memcpy( sg_serial->ip_addr , sg_config->router_ip , IP_ADDRESS );
		if( sg_config->sensor_public_key > 0 )
			sg_serial->payload[0] = (uint8_t) 0x07;
		else
			sg_serial->payload[0] = (uint8_t) 0x08;   
        payloadHandler( sg_config , sg_serial);			
	}
	// Received Fly packet, send an Eyeball packet
	else if( sg_config->fly == true ){
		// Eyeball
		Serial.println( "EYEBALLING");
		sg_serial->control[0] = (uint8_t) 0xAD;
		memcpy( sg_serial->ip_addr , sg_config->router_ip , IP_ADDRESS );
		sg_serial->payload[0] = (uint8_t) 0x00;
		payloadHandler( sg_config , sg_serial);
	}
	Serial.println("LOOPING");
	// Delay 1 second between packets
	// during mating 
	while(1){
	delay(1000);
    }
	}
}

#endif // __SENSOR_PAYLOAD_HANDLER_H__
