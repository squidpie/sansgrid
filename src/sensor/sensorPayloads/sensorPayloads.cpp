/* Sensor Payload Structs and Definitions
 * specific to the Arduino DUE Platform
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
 
#include <Arduino.h>
#include "sensorPayloads.h"

// Initialize SensorConfig static members
bool SensorConfig::connected = false;
byte SensorConfig::manid[ MANID ] = { 0x01, 0x00, 0x00, 0x00 };
byte SensorConfig::modnum[ MODNUM ] = { 0x01, 0x00, 0x00, 0x00 };
char SensorConfig::sn[ SN + 1 ] = "00112233";
char SensorConfig::network_name[ DATA + 1 ] = "Sansgrid";
byte SensorConfig::ip_address[ IP_ADDRESS ] = { 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
byte SensorConfig::router_ip[ IP_ADDRESS ] = { 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };			 
byte SensorConfig::server_public_key[ SERVER_KEY ] = { 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00 };
byte SensorConfig::sensor_public_key[ SENSOR_KEY ] = { 0x09,0xC4,0x59,0xDC,
	0xA6,0xF5,0xF0,0x16,0x54,0xE5,0xC0,0x87,0xB0,0x94,0xBA,0x89,0x16,0xFF,
	0x0C,0x26,0xEF,0xE9,0xAF,0x53,0x02,0x83,0x7D,0x1B,0xF9,0x6A,0x94,0x61,
	0x57,0x27,0xDF,0xDE,0xB7,0x0B,0x2D,0xA4,0xC6,0x23,0x8D,0x66,0x15,0x38,
	0x4A,0x28,0xE3,0x3F,0x29,0xB1,0x2F,0xBD,0xB4,0xED,0x28,0xF1,0x3D,0xED,
	0x25,0xB3,0xC1,0xE2 }; 
byte SensorConfig::control[ CONTROL ] = { 0xFD };
byte SensorConfig::padding[ CONTROL ] = { 0x00 };

// Initialize SansgridEyeball static members
byte SansgridEyeball::dt[ DT ] = { 0x00 };
byte SansgridEyeball::manid[ MANID ] = { 0x00, 0x00, 0x00, 0x01 };
byte SansgridEyeball::modnum[ MODNUM ] = { 0x00, 0x00, 0x00, 0x01 };
char SansgridEyeball::sn[ SN + 1 ] = "00112233";
byte SansgridEyeball::profile[ PROFILE ] = { 0x00 };
byte SansgridEyeball::mode[ MODE ] = { 0x00,0x00,0x00,0x01 } ;

// Initialize SansgridMock static members
byte SansgridMock::dt[ DT ] = { 0x07 };
byte SansgridMock::sensor_public_key[ SENSOR_KEY ] = { 0x09,0xC4,0x59,0xDC,
	0xA6,0xF5,0xF0,0x16,0x54,0xE5,0xC0,0x87,0xB0,0x94,0xBA,0x89,0x16,0xFF,
	0x0C,0x26,0xEF,0xE9,0xAF,0x53,0x02,0x83,0x7D,0x1B,0xF9,0x6A,0x94,0x61,
	0x57,0x27,0xDF,0xDE,0xB7,0x0B,0x2D,0xA4,0xC6,0x23,0x8D,0x66,0x15,0x38,
	0x4A,0x28,0xE3,0x3F,0x29,0xB1,0x2F,0xBD,0xB4,0xED,0x28,0xF1,0x3D,0xED,
	0x25,0xB3,0xC1,0xE2 };

// Initialize SansgridMock static members
byte SansgridPeacock::dt[ DT ] = { 0x0C };
byte SansgridPeacock::additional[ ADDITIONAL ] = { 0x00 };

// Initialize Heartbeat static members
byte SansgridHeartbeat::dt[ DT ] = { 0x1F };