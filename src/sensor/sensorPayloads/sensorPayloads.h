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
 
#ifndef __SENSOR_PAYLOADS_H__
#define __SENSOR_PAYLOADS_H__

#include <Arduino.h>
#include <sgSerial.h>

#define DT 1
#define MANID 4
#define MODNUM 4
#define SN 8
#define PROFILE 1
#define MODE 4
#define SERVER_ID 16
#define RECOGNITION 1
#define SERVER_KEY 64
#define SENSOR_KEY 64
#define SENSOR_ID 1
#define CLASSIFICATION 1
#define DIRECTION 1
#define LABEL 30
#define UNITS 6
#define ADDITIONAL 1
#define DATA 80
#define PAYLOAD 81
#define IP_ADDRESS 16
#define CONTROL 1

typedef struct SansgridSensor{
	byte id[ SENSOR_ID ];
	byte classification[ CLASSIFICATION ];
	byte direction[ DIRECTION ];
	char label[ LABEL + 1 ];
	char units[ UNITS + 1 ];
} SansgridSensor;

typedef struct SensorConfig{
	static byte manid[ MANID ];
	static byte modnum[ MODNUM ];
	static char sn[ SN + 1 ];
	static byte ip_address[ IP_ADDRESS ];
	static byte router_ip[ IP_ADDRESS ];
	static byte server_public_key[ SERVER_KEY ];
	static byte sensor_public_key[ SENSOR_KEY ]; 
	static byte padding[ CONTROL ];
	SansgridSensor a;
	SansgridSensor b;
} SensorConfig;

typedef struct SansgridFly{
	byte dt[ DT ];
	char network_name[ DATA + 1 ];
} SansgridFly;
	
typedef struct SansgridEyeball{
    static byte dt[ DT ];
	static byte manid[ MANID ];
	static byte modnum[ MODNUM ];
	static char sn[ SN + 1 ];
	static byte profile[ PROFILE ];
	static byte mode[ MODE ];
} SansgridEyeball;

typedef struct SansgridPeck{
    byte dt[ DT ];
	byte router_ip[ IP_ADDRESS ];
	byte ip_address[ IP_ADDRESS ];
	byte server_id[ SERVER_ID ];
	byte recognition[ RECOGNITION ];
	byte manid[ MANID ];
	byte modnum[ MODNUM ];
	char sn[ SN + 1 ];
} SansgridPeck;

typedef struct SansgridSing{
	byte dt[ DT ];
	byte server_public_key[ SERVER_KEY ];
} SansgridSing;

typedef struct SansgridMock{
	static byte dt[ DT ];
	static byte sensor_public_key[ SENSOR_KEY ];
} SansgridMock;

typedef struct SansgridPeacock{
	static byte dt[ DT ];
	SansgridSensor a;
	SansgridSensor b;
	static byte additional[ ADDITIONAL ];
} SansgridPeacock;

typedef struct SansgridNest{
	byte dt[ DT ];
} SansgridNest;

typedef struct SansgridSquawk{
	byte dt[ DT ];
	byte data[ DATA ];
} SansgridSquawk;

typedef struct SansgridHeartbeat{
	byte dt[ DT ];
	byte data[ DATA];
} SansgridHeartbeat;

typedef struct SansgridChirp{
	byte dt[ DT ];
	byte data[ DATA];
} SansgridChirp;

#endif // __SENSOR_PAYLOADS_H__
