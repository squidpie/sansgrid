/* Payload Specifications
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

#ifndef __SG_PAYLOADS_H__
#define __SG_PAYLOADS_H__

#include <stdint.h>


#define IP_SIZE 16
#define PAYLOAD_SIZE 81


#define SANSGRID_UNION(type, name) union name { \
	type *formdata; \
	uint8_t *serialdata; \
}



typedef struct SansgridHatching {
	// Datatype: 0xF1
	uint8_t datatype;
	uint8_t ip[16];
	uint8_t padding[64];
} SansgridHatching;



typedef struct SansgridFly {
	// Datatype: 0xF0
	uint8_t datatype;
	char network_name[80];
} SansgridFly;




typedef struct SansgridEyeball {
	// Datatype: 0x00
	uint8_t datatype;
	uint8_t manid[4];
	uint8_t modnum[4];
	uint8_t serial_number[8];
	uint8_t profile;

	// Modes:
	// 	00 - Standard mode
	// 	01 - Sensor Ready to mate
	//unsigned mode:4;
	uint8_t mode;
	uint8_t padding[62];
} SansgridEyeball;




typedef struct SansgridPeck {
	// Datatype: 0x01
	uint8_t datatype;
	uint8_t router_ip[IP_SIZE];
	uint8_t assigned_ip[IP_SIZE];
	uint8_t server_id[16];
	// Recognition:
	// 	0x00	- Recognized
	// 	0x01	- Not Recognized, server will mate
	// 	0x02	- Not Recognized, server refuses mate
	// 	0x03	- Not Recognized, sensor refuses mate
	uint8_t recognition;
	uint8_t manid[4];
	uint8_t modnum[4];
	uint8_t serial_number[8];
	uint8_t padding[15];
} SansgridPeck;




typedef struct SansgridSing {
	// Datatype: 	0x02	- Ack, valid pubkey
	// 				0x03	- Ack, no pubkey
	uint8_t datatype;
	uint8_t pubkey[80];
} SansgridSing;




typedef struct SansgridMock {
	// Datatype:	0x07	- Ack, sensor gives pubkey
	// 				0x08	- Ack, no pubkey
	uint8_t datatype;
	uint8_t pubkey[80];
} SansgridMock;




typedef struct SansgridPeacock {
	// Datatype:	0x0C
	uint8_t		datatype;
	// I/O(A) capabilities
	uint8_t		IO_A_id;
	uint8_t		IO_A_class;
	uint8_t		IO_A_direc;
	char 		IO_A_label[30];
	uint8_t 	IO_A_units[6];

	// I/O(B) capabilities
	uint8_t		IO_B_id;
	uint8_t		IO_B_class;
	uint8_t		IO_B_direc;
	char 		IO_B_label[30];
	uint8_t 	IO_B_units[6];

	// If more I/O types are coming
	uint8_t 	additional_IO_needed;

	uint8_t 	padding;
} SansgridPeacock;




typedef struct SansgridNest {
	// Datatype:	0x10
	uint8_t datatype;
	uint8_t padding[80];
} SansgridNest;




typedef struct SansgridSquawk {
	// Datatypes:
	// 		0x11: Server Challenge to sensor
	// 		0x15: Sensor completes server challenge. No sensor challenge
	// 		0x16: Sensor completes server challenge. Sensor challenge coming
	// 		0x17: Sensor sends challenge to server
	// 		0x1B: Server denies sensor challenge response
	// 		0x1C: Server responds to sensor challenge
	// 		0x1D: Sensor accepts server's response
	uint8_t datatype;
	uint8_t data[80];
} SansgridSquawk;




typedef struct SansgridHeartbeat {
	// Datatypes:
	// 		0x1E: Router Pings Sensor
	// 		0x1F: Sensor responds to ping
	uint8_t datatype;
	uint8_t padding[80];
} SansgridHeartbeat;




typedef struct SansgridChirp {
	// Datatypes
	// 		0x20: Command sent from server to sensor
	// 		0x21: Data sent from sensor to server
	// 		0x22: Start of data stream
	// 		0x23: Continued data stream
	// 		0x24: End of data stream
	// 		0x25: Network Disconnecting Sensor
	// 		0x26: Sensor Disconnecting from Network
	uint8_t datatype;
	uint8_t datasize;	// Used to truncate data
	uint8_t data[79];
} SansgridChirp;




enum SansgridDataTypeEnum {
	// Initialization packets
	SG_HATCH = 0xF1,
	SG_FLY = 0xF0,
	// Eyeballing
	SG_EYEBALL = 0x00,
	// Pecking
	SG_PECK = 0x01,
	// Singing
	SG_SING_WITH_KEY = 0x02,
	SG_SING_WITHOUT_KEY = 0x03,
	// Mocking
	SG_MOCK_WITH_KEY = 0x07,
	SG_MOCK_WITHOUT_KEY = 0x08,
	// Peacocking
	SG_PEACOCK = 0x0C,
	// Nesting
	SG_NEST = 0x10,
	// Squawking
	SG_SQUAWK_SERVER_CHALLENGE_SENSOR = 0x11,
	SG_SQUAWK_SERVER_NOCHALLENGE_SENSOR = 0x12,
	SG_SQUAWK_SENSOR_RESPOND_NO_REQUIRE_CHALLENGE = 0x15,
	SG_SQUAWK_SENSOR_RESPOND_REQUIRE_CHALLENGE = 0x16,
	SG_SQUAWK_SENSOR_CHALLENGE_SERVER = 0x17,
	SG_SQUAWK_SERVER_DENY_SENSOR = 0x1B,
	SG_SQUAWK_SERVER_RESPOND = 0x1C,
	SG_SQUAWK_SENSOR_ACCEPT_RESPONSE = 0x1D,
	// Heartbeat
	SG_HEARTBEAT_ROUTER_TO_SENSOR = 0x1E,
	SG_HEARTBEAT_SENSOR_TO_ROUTER = 0x1F,
	// Chirp
	SG_CHIRP_COMMAND_SERVER_TO_SENSOR = 0x20,
	SG_CHIRP_DATA_SENSOR_TO_SERVER = 0x21,
	SG_CHIRP_NETWORK_DISCONNECTS_SENSOR = 0x25,
	SG_CHIRP_SENSOR_DISCONNECT = 0x26,
	// Internal
	SG_SERVSTATUS = 0xfd,
};

enum SansgridEyeballModeEnum {
	SG_EYEBALL_NOMATE = 0x0,
	SG_EYEBALL_MATE = 0x01
};

enum SansgridPeckRecognitionEnum {
	SG_PECK_RECOGNIZED = 0x0,
	SG_PECK_MATE = 0x01,
	SG_PECK_SERVER_REFUSES_MATE = 0x02,
	SG_PECK_SENSOR_REFUSES_MATE = 0x03
};

enum SansgridDeviceStatusEnum {
	SG_DEVSTATUS_NULL,
	SG_DEVSTATUS_HATCHING,			// Hatching status
	SG_DEVSTATUS_FLYING,			// Flying status
	SG_DEVSTATUS_EYEBALLING,		// Eyeballing Status
	SG_DEVSTATUS_PECKING,			// Pecking Status
	SG_DEVSTATUS_SINGING,			// Singing Status
	SG_DEVSTATUS_MOCKING,			// Mocking Status
	SG_DEVSTATUS_PEACOCKING,		// Peacocking Status
	SG_DEVSTATUS_NESTING,			// Nesting Status
	SG_DEVSTATUS_SQUAWKING,			// Squawking Status
	SG_DEVSTATUS_HEARTBEAT,
	SG_DEVSTATUS_CHIRPING,
	
	// Compound types
	SG_DEVSTATUS_LEASED 			// Device Associated with Network
};

#endif

// vim: ft=c ts=4 noet sw=4:
