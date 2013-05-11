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
#define SENSOR_A 39

typedef struct SansgridSensor{
	uint8_t id[ SENSOR_ID ];
	uint8_t classification[ CLASSIFICATION ];
	uint8_t direction[ DIRECTION ];
	char label[ LABEL + 1 ];
	char units[ UNITS + 1 ];
} SansgridSensor;

typedef struct SensorConfig{
	static bool connected;
	static uint8_t manid[ MANID ];
	static uint8_t modnum[ MODNUM ];
	static char sn[ SN + 1 ];
	static char network_name[ DATA + 1 ];
	static uint8_t ip_address[ IP_ADDRESS ];
	static uint8_t router_ip[ IP_ADDRESS ];
	static uint8_t server_public_key[ SERVER_KEY ];
	static uint8_t sensor_public_key[ SENSOR_KEY ]; 
	static uint8_t control[ CONTROL ];
	static uint8_t padding[ CONTROL ];
	SansgridSensor a;
	SansgridSensor b;
} SensorConfig;

typedef struct SansgridFly{
	uint8_t dt[ DT ];
	char network_name[ DATA + 1 ];
} SansgridFly;
	
typedef struct SansgridEyeball{
    static uint8_t dt[ DT ];
	static uint8_t manid[ MANID ];
	static uint8_t modnum[ MODNUM ];
	static char sn[ SN + 1 ];
	static uint8_t profile[ PROFILE ];
	static uint8_t mode[ MODE ];
} SansgridEyeball;

typedef struct SansgridPeck{
    uint8_t dt[ DT ];
	uint8_t router_ip[ IP_ADDRESS ];
	uint8_t ip_address[ IP_ADDRESS ];
	uint8_t server_id[ SERVER_ID ];
	uint8_t recognition[ RECOGNITION ];
	uint8_t manid[ MANID ];
	uint8_t modnum[ MODNUM ];
	char sn[ SN + 1 ];
} SansgridPeck;

typedef struct SansgridSing{
	uint8_t dt[ DT ];
	uint8_t server_public_key[ SERVER_KEY ];
} SansgridSing;

typedef struct SansgridMock{
	static uint8_t dt[ DT ];
	static uint8_t sensor_public_key[ SENSOR_KEY ];
} SansgridMock;

typedef struct SansgridPeacock{
	static uint8_t dt[ DT ];
	SansgridSensor a;
	SansgridSensor b;
	static uint8_t additional[ ADDITIONAL ];
} SansgridPeacock;

typedef struct SansgridNest{
	uint8_t dt[ DT ];
} SansgridNest;

typedef struct SansgridSquawk{
	uint8_t dt[ DT ];
	uint8_t data[ DATA ];
} SansgridSquawk;

typedef struct SansgridHeartbeat{
	static uint8_t dt[ DT ];
} SansgridHeartbeat;

typedef struct SansgridChirp{
	uint8_t dt[ DT ];
	uint8_t data[ DATA];
} SansgridChirp;

#endif // __SENSOR_PAYLOADS_H__
