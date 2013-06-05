/* Sensor Payload Structs and Definitions
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
    
#ifndef __SENSOR_PAYLOADS_H__
#define __SENSOR_PAYLOADS_H__

#include <Arduino.h>
#include <sgSerial.h>

#define DT 1
#define MANID 4
#define MODNUM 4
#define SN 8
#define PROFILE 1
#define MODE 1
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
#define DELAY 6
#define EYEBALL_PADDING 62
#define MOCK_PADDING 16
#define PECK_PADDING 15
#define PEACOCK_PADDING 1
#define NEST_PADDING 80
#define HEARTBEAT_PADDING 80
#define SID 1
#define DATA_SIZE 79

typedef struct SensorConfig{
    static bool mate;
    static bool fly;
    static bool sing;
    static bool mock;
    static bool squawk;
    static bool nest;
    static bool chirp;
    static bool nokey;
    static bool challenge;
    static bool forget;
	static bool received;
	static bool connecting;
    static uint8_t manid[ MANID ];
    static uint8_t modnum[ MODNUM ];
    static uint8_t sn[ SN ];
    static int8_t network_name[ DATA ];
	static uint8_t server_id[ SERVER_ID ];
    static uint8_t ip_address[ IP_ADDRESS ];
    static uint8_t router_ip[ IP_ADDRESS ];
    static uint8_t server_public_key[ SERVER_KEY ];
	static uint8_t server_challenge[ SERVER_KEY ];
    static uint8_t sensor_public_key[ SENSOR_KEY ];
    static uint8_t sensor_challenge[ SENSOR_KEY ];	
} SensorConfig;

typedef struct SansgridFly{
    uint8_t dt[ DT ];
    int8_t network_name[ DATA ];
} SansgridFly;
    
typedef struct SansgridEyeball{
    static uint8_t dt[ DT ];
    static uint8_t manid[ MANID ];
    static uint8_t modnum[ MODNUM ];
    static uint8_t sn[ SN ];
    static uint8_t profile[ PROFILE ];
    static uint8_t mode[ MODE ];
    static uint8_t padding[ EYEBALL_PADDING ];
} SansgridEyeball;

typedef struct SansgridPeck{
    uint8_t dt[ DT ];
    uint8_t router_ip[ IP_ADDRESS ];
    uint8_t ip_address[ IP_ADDRESS ];
    uint8_t server_id[ SERVER_ID ];
    uint8_t recognition[ RECOGNITION ];
    uint8_t manid[ MANID ];
    uint8_t modnum[ MODNUM ];
    uint8_t sn[ SN ];
    uint8_t padding[ PECK_PADDING ];
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
    static uint8_t id_a[ SENSOR_ID ];
    static uint8_t classification_a[ CLASSIFICATION ];
    static uint8_t direction_a[ DIRECTION ];
    static int8_t label_a[ LABEL ];
    static int8_t units_a[ UNITS ];    
    static uint8_t id_b[ SENSOR_ID ];
    static uint8_t classification_b[ CLASSIFICATION ];
    static uint8_t direction_b[ DIRECTION ];
    static int8_t label_b[ LABEL ];
    static int8_t units_b[ UNITS ];
    static uint8_t additional[ ADDITIONAL ];
    static uint8_t padding[ PEACOCK_PADDING ];
} SansgridPeacock;

typedef struct SansgridNest{
    uint8_t dt[ DT ];
    uint8_t padding[ NEST_PADDING ];
} SansgridNest;

typedef struct SansgridSquawk{
    uint8_t dt[ DT ];
    uint8_t data[ DATA ];
} SansgridSquawk;

typedef struct SansgridHeartbeat{
    uint8_t dt[ DT ];
    uint8_t padding[ HEARTBEAT_PADDING ];
} SansgridHeartbeat;

typedef struct SansgridChirp{
    uint8_t dt[ DT ];
	uint8_t sid[ SID ];
    uint8_t data[ DATA];
} SansgridChirp;

#endif // __SENSOR_PAYLOADS_H__
