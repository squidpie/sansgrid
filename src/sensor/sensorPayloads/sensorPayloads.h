/* Payload Handlers
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
#include <sensorPayloadHandler.h>

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

typedef struct SensorConfig{
	int8_t manid[ MANID ];
	int8_t modnum[ MODNUM ];
	int8_t sn[ SN ];
	int8_t ip_address[ IP_ADDRESS ];
	int8_t router_ip[ IP_ADDRESS ];
	int8_t server_public_key[ SERVER_KEY ];
	int8_t sensor_public_key[ SENSOR_KEY ];
} SensorConfig;

typedef struct SansgridEyeball{
    int8_t dt[ DT ];
	int8_t manid[ MANID ];
	int8_t modnum[ MODNUM ];
	int8_t sn[ SN ];
	int8_t profile[ PROFILE ];
	int8_t mode[ MODE ];
} SansgridEyeball;

typedef struct SansgridPeck{
    int8_t dt[ DT ];
	int8_t router_ip[ IP_ADDRESS ];
	int8_t ip_address[ IP_ADDRESS ];
	int8_t server_id[ SERVER_ID ];
	int8_t recognition[ RECOGNITION ];
	int8_t manid[ MANID ];
	int8_t modnum[ MODNUM ];
	int8_t sn[ SN ];
} SansgridPeck;

typedef struct SansgridSing{
	int8_t dt[ DT ];
	int8_t server_public_key[ SERVER_KEY ];
} SansgridSing;

typedef struct SansgridMock{
	int8_t dt[ DT ];
	int8_t sensor_public_key[ SENSOR_KEY ];
} SansgridMock;

typedef struct SansgridSensor{
	int8_t id[ SENSOR_ID + 1 ];
	int8_t classification[ CLASSIFICATION ];
	int8_t direction[ DIRECTION ];
	int8_t label[ LABEL ];
	int8_t units[ UNITS ];
} SansgridSensor;

typedef struct SansgridPeacock{
	int8_t dt[ DT ];
	SansgridSensor a;
	SansgridSensor b;
	int8_t additional[ ADDITIONAL ];
} SansgridPeacock;

typedef struct SansgridNest{
	int8_t dt[ DT ];
} SansgridNest;

typedef struct SansgridSquawk{
	int8_t dt[ DT ];
	int8_t data[ DATA ];
} SansgridSquawk;

#endif // __SENSOR_PAYLOADS_H__
