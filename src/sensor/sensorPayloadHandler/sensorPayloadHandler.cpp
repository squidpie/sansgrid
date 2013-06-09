/* Sensor Payload Handler Implementation
 * Specific to the Arduino Platform
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 */
 
#include <Arduino.h>
#include "sensorPayloadHandler.h"

//#define PUSH_BUTTON 1
 
// Payload Handler - Processes a SansgridSerial Struct. Will
// parse all inbound SPI packets in the form of a SansgridSerial, and 
// transmit all outboud SPI packets in the form of a SansgridSerial.
void payloadHandler( SensorConfig *sg_config , SansgridSerial *sg_serial){
    // Read in data type from first position of payload
    // to determine what to do with packet
    uint8_t command = sg_serial->payload[0];
	// Counter for Verifying Router IP and Forgotten Server. 
	// If old Router IP address does'nt match peck Forget Sensor.
	int val = 0;
	int val2 = 16;
	// Memory Test to see how much SRAM left
    switch ( command ){ 
        case 0x01 :
            // Peck - Initial server response
            SansgridPeck sg_peck;
            // Parse SansgridSerial into a peck payload struct
            parsePeck( sg_serial , &sg_peck );
			// Check to see if payload was meant for this sensor
			for( int i = 0 ; i < SN ; i++){
				if( sg_config->sn[i] != sg_peck.sn[i] )
					break;
			}
			// Check to see if router IP is the same as stored
			// if it is not then set forget flag to notify server
			// in a squawk that sensor has forgotten it, please
			// forget the sensor and proceed to sing.
			for( int i = 0 ; i < SERVER_ID ; i++){
				if( sg_config->server_id[i] == sg_peck.server_id[i] ){
					val++;
				}
			}
			// Set forget if count doesn't match IP_ADDRESS
			if( val != SERVER_ID )
				sg_config->forget = true;
			else
			    sg_config->forget = false;
			// Copy Router IP and assigned sensor IP into SensorConfig struct
			memcpy( sg_config->server_id , sg_peck.server_id , SERVER_ID );
			memcpy( sg_config->router_ip , sg_peck.router_ip , IP_ADDRESS );
			memcpy( sg_config->ip_address , sg_peck.ip_address , IP_ADDRESS );
            break;        
        case 0x02 :
            // Sing - The server has a public key to share with 
            // the sensor for future authentication challenges.        
        case 0x03 :
            // Sing - Server does not require authentication, 
            // ready to accept sensor key if needed.
            SansgridSing sg_sing;
            // Parse SansgridSerial into Sing payload
            parseSing( sg_serial , &sg_sing );
			// If sing has a Sensor Public Key, store in SensorConfig
			// struct.
			if( sg_sing.dt[0] == 0x02 ){
				memcpy( sg_config->server_public_key , sg_sing.server_public_key , SERVER_KEY );
			}
            // Set Sing flag to true to intiate sending a 
            // Mock payload.
            sg_config->sing = true;
            break;
        case 0x07 :
            // Mock - The sensor has a public key to share with 
            // the sensor for future authentication challenges.
            SansgridMock sg_mock_key;
            // Set datatype of Mock to be sent
            sg_mock_key.dt[0] = (uint8_t) 0x07;
            // Transmit Mock payload
            transmitMock( sg_serial , &sg_mock_key );
            sg_config->mock = true;
            sg_config->sing = false;
            break;
        case 0x08 :
            // Mock - Sensor does not require authentication, 
            // ready to accept sensor key if needed.
            SansgridMock sg_mock_nokey;
            // Set datatype of Mock to be sent
            sg_mock_key.dt[0] = (uint8_t) 0x08;
            // Transmit Mock Payload
            transmitMock( sg_serial , &sg_mock_nokey );
            sg_config->mock = true;
            sg_config->sing = false;
            break;
        case 0x0C :
		    delay(1000);
            // Peacock - Sensor share's capabilities with server.
            SansgridPeacock sg_peacock;
            // Transmit Peacock, if more than two signals are
            // associated with sensor, set sg_peacock.additional
            // to 0x01 before transmitting
            //Serial.println( "Sending Peacock" );
            transmitPeacock( sg_serial , &sg_peacock );
            sg_config->mock = false;
            break;
        case 0x10 :
            // Nest - Server accepts sensor into network.
            // Sensor is nested, set flag
            sg_config->nest = true;
			sg_config->connecting = false;
            // If mating is push button initiated
            // clear mate flag once nested to allow
            // for push button mate again.
            #ifdef PUSH_BUTTON
            sg_config->mate = false;
            #endif // PUSH_BUTTON
            break;
        case 0x11 :    
            // Squawk - Server challenge
            // Check to see if Sensor forgot Server
            // if so, set datatype to 0x27, set Squawk
            // flag to true and initiate Squawk forget
            // sensor payload.
            if( sg_config->forget == true ){
                sg_serial->payload[0] = (uint8_t) 0x27;
                sg_config->squawk = true;
                break;
            }
            SansgridSquawk sg_squawk_noauth;
            // Copy SansgridSerial struct into Squawk
            // struct
            parseSquawk( sg_serial , &sg_squawk_noauth );
			for( int i = 0 ; i < SERVER_ID ; i++ )
			    sg_config->server_challenge[i] = sg_squawk_noauth.data[ i ];
            // If sensor requires a challenge and flag
            // is set to true, send datatype 0x16, otherwise
            // sent datatype 0x15
            if( sg_config->nokey == false )
                sg_serial->payload[0] = (uint8_t) 0x16;
            else
                sg_serial->payload[0] = (uint8_t) 0x15;
            // Set squawk flag to true to intitiate
            // sending reply squawk of data type
            // 0x15 or 0x16.
            sg_config->squawk = true;
			sg_config->challenge = true;
            break;
        case 0x12 :    
            // Squawk - Server doesn't need challenge
            // Check to see if Sensor forgot Server
            // if so, set datatype to 0x27, set Squawk
            // flag to true and initiate Squawk forget
            // sensor payload.
            if( sg_config->forget == true ){
                sg_serial->payload[0] = (uint8_t) 0x27;
                sg_config->squawk = true;
                break;
            }
            SansgridSquawk sg_squawk_auth;
            // Copy SansgridSerial struct into Squawk
            // struct
            parseSquawk( sg_serial , &sg_squawk_auth);
            // Server requires authentication, call function
            // to authenticate Server and Sensor Public Key
            // store authentication in Squawk payload
            if( sg_config->nokey == false )
                sg_serial->payload[0] = (uint8_t) 0x16;
            else
                sg_serial->payload[0] = (uint8_t) 0x15;
            // Set squawk flag to true to intitiate
            // sending reply squawk of data type
            // 0x15 or 0x16.
            sg_config->squawk = true;
			sg_config->challenge = false;
            break;
		case 0x15 :    
            // Squawk - Sensor response to server squawk no 
            // challenge needed.
            SansgridSquawk sg_squawk_sensor_noauth;
            // Copy SansgridSerial struct into Squawk
            // struct
            parseSquawk( sg_serial , &sg_squawk_sensor_noauth );
            // Server requires authentication, call function
            // to authenticate Server and Sensor Public Key
            // store authentication in Squawk payload
            if( sg_config->challenge == true ){
			    authenticateServer( sg_config , &sg_squawk_sensor_noauth );
				sg_config->challenge = false;
			}
            // Transmit Squawk payload
			transmitSquawk ( sg_serial , &sg_squawk_sensor_noauth );
            // Set squawk flag to false, wait for another Squawk or Nest
            // payload to be received.
            sg_config->squawk = false;
            break;
        case 0x16 :    
            // Squawk - Sensor acknowledge.
            // Squawk - Sensor response to challenge.
            SansgridSquawk sg_squawk_acknowledge;
            // Copy SansgridSerial struct into Squawk
            // struct
            parseSquawk( sg_serial , &sg_squawk_acknowledge );
			// Sensor Challenge
			for( int i = 0 ; i < SERVER_KEY ; i++ )
			    sg_squawk_acknowledge.data[ i + 1 ] = sg_config->sensor_challenge[i];
            // Transmit Squawk payload
            transmitSquawk( sg_serial , &sg_squawk_acknowledge );
			sg_serial->payload[0] = (uint8_t) 0x17;
			break;
		case 0x17 :    
            // Delay
			delay(1000);
			// Squawk - Sensor response to challenge.
            SansgridSquawk sg_squawk_response;
			// Copy SansgridSerial struct into Squawk
            // struct
            parseSquawk( sg_serial , &sg_squawk_response );
            // Server requires authentication, call function
            // to authenticate Server and Sensor Public Key
            // store authentication in Squawk payload
            authenticateServer( sg_config , &sg_squawk_response );
            // Transmit Squawk payload
			transmitSquawk( sg_serial , &sg_squawk_response );
            // Set squawk flag to false, wait for another Squawk or Nest
            // payload to be received.
            sg_config->squawk = false;
            break;
        case 0x1b :    
            // Squawk - Server denies sensor's challenge response.
            // Server denied challenge, attempt to mate again by
            // setting fly flag to false and wait for fly payload again
            sg_config->fly = false;
            sg_config->squawk = false;
			sg_config->connecting = false;
            break;
		default : 
            break;
    }
}

// Payload Handler - Processes a SansgridSerial Struct. Will
// parse all inbound SPI packets in the form of a SansgridSerial, and 
// transmit all outboud SPI packets in the form of a SansgridSerial.
void payloadHandlerB( SensorConfig *sg_config , SansgridSerial *sg_serial){
    // Read in data type from first position of payload
    // to determine what to do with packet
    uint8_t command = sg_serial->payload[0];
    switch ( command ){ 
        case 0x1c :    
            // Squawk - Server response to challenge.
        case 0x1d :    
            // Squawk - Sensor accepts server's response
            SansgridSquawk sg_squawk_accept_response;
            // Copy SansgridSerial struct into Squawk
            // struct
            parseSquawk( sg_serial , &sg_squawk_accept_response );
            // Check to see if Server response matches Sensor
            // Authentication, if so respond with squawk accepting
            // response from server
            if( authenticateSensor( sg_config , &sg_squawk_accept_response ) ){
                sg_squawk_accept_response.dt[0] = (uint8_t) 0x1d;
                transmitSquawk( sg_serial , &sg_squawk_accept_response );
            }
			
            // If not, Sensor denied challenge, attempt to mate again by
            // setting fly and squawk flag to false and wait for fly payload
            // to begin mate process again.
            else{
                sg_config->fly = false;
				sg_config->connecting = false;
				sg_config->challenge = false;
            }
            sg_config->squawk = false;
            break;
        case 0x1e :    
            // Heartbeat - Router pulse to sensor
        case 0x1f :    
            // Heartbeat - Sensor's response to router's pulse
            break;
        case 0x20 :    
            // Chirp - Command sent from server to sensor.
            SansgridChirp sg_chirp_in;
            parseChirp( sg_serial , &sg_chirp_in );
            sg_config->chirp = true;
            break;
        case 0x21 :    
            // Chirp - Chirp sent from sensor to server.
            SansgridChirp sg_chirp_out;
            transmitChirp( sg_serial , &sg_chirp_out );
            break;
        case 0x25 :    
            // Chirp - Network is disconnecting sensor.
        case 0x26 :    
            // Chirp - Sensor is disconnecting from the network.
            SansgridChirp sg_chirp_disconnect;
            parseChirp( sg_serial , &sg_chirp_disconnect );
            sg_chirp_disconnect.dt[0] = (uint8_t) 0x26;
            transmitChirp( sg_serial , &sg_chirp_disconnect );
            sg_config->fly = false;
			sg_config->nest = false;
			sg_config->connecting = false;
            break;
        case 0x27 :    
            // Squawk - Sensor has forgotten Server, 
            // Server forget Sensor.
            SansgridSquawk sg_squawk_forget;
            sg_squawk_forget.dt[0] = (uint8_t) 0x27;
            transmitSquawk( sg_serial , &sg_squawk_forget );
            sg_config->fly = false;
			sg_config->squawk = false;
			sg_config->connecting = false;
            break;
        case 0xEB :
			// Eyeball - Sensor entering network.
            SansgridEyeball sg_eyeball;
            // Set MODE in Eyeball payload. If mate is
            // true then MODE will equal 0x01, if false
            // push button has not been pressed and therefore
            // sensor not ready to mate, 0x00.
            if( sg_config->mate == false )
                sg_eyeball.mode[0] = (uint8_t) 0x00;
            else
                sg_eyeball.mode[0] = (uint8_t) 0x01;
            // Copy Eyeball payload into SansgridSerial
            // struct and send over SPI.
            transmitEyeball( sg_serial , &sg_eyeball );
			// Set FLY flag back to false, wait for Peck
            // packet to arrive.
            sg_config->fly = false;
            break;
		case 0xF0 :    
            // Flying - Broadcast from router identifying the network
            if( sg_config->connecting == true )
			    break;
			SansgridFly sg_fly;
            parseFly( sg_serial , &sg_fly );
            memcpy( sg_config->network_name , sg_fly.network_name , DATA );
			if ( sg_config->nest == false ){
                sg_config->fly = true;
				sg_config->connecting == true;
			}
            break;
        case 0xFE :    
            // - Reserved for future expansion
            break;
        case 0xFF :    
            // - Reserved for future expansion
            break;
        default : 
            break;
    }  
    Serial.println( freeRam() );	
}

// Authenticate Public Keys from Sensor and Server
// by XOR each bit and count the ones. Store 16 bit value split
// between high and low into last two bytes of payload.
void authenticateServer( SensorConfig *sg_config , SansgridSquawk *sg_squawk ){
    uint16_t key_count = 0;
    // Count all ones
    for( int i = 0 ; i < SERVER_KEY ; i++ ){
        key_count = key_count + ( countBits( sg_config->server_public_key[i] ^ sg_config->server_challenge[i] ));
    }
    // Parse 16 bit value into two 8 bit bytes
    uint8_t hi_lo[2] = { (uint8_t)( key_count >> 8 ), (uint8_t)key_count };
    for( int i = 0 ; i < DATA - 2 ; i++ )
        sg_squawk->data[i] = (uint8_t) 0x00;
    // Store two 8 bit bytes into last two positions of payload
    sg_squawk->data[0] = (uint8_t) hi_lo[0];
    sg_squawk->data[1] = (uint8_t) hi_lo[1];
}

// Compare response from Server to challenge, return value true if 
// it is the same, and false if it doesn't match.
bool authenticateSensor( SensorConfig *sg_config , SansgridSquawk *sg_squawk ){
    bool correct = false;
    uint16_t count = 0;
    // Add up all the ones
    for( int i = 0 ; i < SENSOR_KEY ; i++ ){
        count = count + ( countBits( sg_config->sensor_public_key[i] ^ sg_config->sensor_challenge[i] ));
    }
    // Parse 16 bit value into two 8 bit bytes
    uint8_t hi_lo[2] = { (uint8_t)( count >> 8 ), (uint8_t)count };
    // Check if bytes match what is sent from Server
    if(( sg_squawk->data[0] == hi_lo[0] ) && ( sg_squawk->data[1] == hi_lo[1] ))
        correct = true;
    return correct;
}

// Connect sensor to network. Wait in while loop until nested.
void sensorConnect( SensorConfig *sg_config , SansgridSerial *sg_serial ){
    while( sg_config->nest == false ){
        // Received flag is set indicating Master received 
        // a packet from Slave over SPI, process packet
        if ( sg_config->received == true ){
            // Received Packet
            sgSerialReceive( sg_serial , 1 );
			if( sg_serial->payload[0] < 0x1C )
                payloadHandler( sg_config , sg_serial );
            else
			    payloadHandlerB( sg_config , sg_serial );
			sg_config->received = false;
		}
        // Sent a Mock packet, now send a Peacock packet
        else if ( sg_config->mock == true ){
            // Peacock
            sg_serial->control[0] = (uint8_t) 0xAD;
            memcpy( sg_serial->ip_addr , sg_config->router_ip , IP_ADDRESS );
            sg_serial->payload[0] = (uint8_t) 0x0C;
            if( sg_serial->payload[0] < 0x1C )
                payloadHandler( sg_config , sg_serial );
            else
			    payloadHandlerB( sg_config , sg_serial );
        }
        // Received a Sing packet, send a Mock packet
        else if ( sg_config->sing == true ){
            // Mock
            sg_serial->control[0] = (uint8_t) 0xAD;
            memcpy( sg_serial->ip_addr , sg_config->router_ip , IP_ADDRESS );
            if( sg_config->nokey == false )
                sg_serial->payload[0] = (uint8_t) 0x07;
            else
                sg_serial->payload[0] = (uint8_t) 0x08;
            if( sg_serial->payload[0] < 0x1C )
                payloadHandler( sg_config , sg_serial );
            else
			    payloadHandlerB( sg_config , sg_serial );
        }
        // Received Fly packet, send an Eyeball packet
        else if( sg_config->fly == true ){
			// Eyeball
            sg_serial->control[0] = (uint8_t) 0xAD;
            memcpy( sg_serial->ip_addr , sg_config->router_ip , IP_ADDRESS );
            sg_serial->payload[0] = (uint8_t) 0xEB ;
            if( sg_serial->payload[0] < 0x1C )
                payloadHandler( sg_config , sg_serial );
            else
			    payloadHandlerB( sg_config , sg_serial );
        }
        else if( sg_config->squawk == true ){
            // Received a Squawk packet, now send a Squawk back
            // Set control byte to valid data
            sg_serial->control[0] = (uint8_t) 0xAD;
            // Set IP address to router ip
            memcpy( sg_serial->ip_addr , sg_config->router_ip , IP_ADDRESS );
            // Call Payload Handler to send return squawk
            if( sg_serial->payload[0] < 0x1C )
                payloadHandler( sg_config , sg_serial );
            else
			    payloadHandlerB( sg_config , sg_serial );
			if( sg_serial->payload[0] == 0x16 )
				sg_serial->payload[0] = (uint8_t) 0x17;
        }
    }
}

// Function is called to verify space left in SRAM (Stack)
int freeRam( void ){ 
	// Value of start of Stack and current end of Stack
    extern int __heap_start, *__brkval; 
	// Value of SRAM left
    int v; 
	// Return value left of SRAM
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

int16_t countBits(uint8_t value){
    int16_t  count = 0;
    for(int i = 0; i < 8; i++)
        count += (value >> i) & 0x01; // Shift bit[i] to the first position, and mask off the remaining bits.
    return count;
}