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


#include <stdint.h>

#define SANSGRID_UNION(type, name) union name { \
	type formdata; \
	uint8_t serialdata[sizeof(type)]; \
}



struct SansgridHatching {
	// Datatype: 0xF1
	uint8_t datatype;
	uint8_t ip[16];
	uint8_t padding[64];
};

SANSGRID_UNION(struct SansgridHatching, SansgridHatchingConv);



struct SansgridFly {
	// Datatype: 0xF0
	uint8_t datatype;
	char network_name[80];
};

/*
union SansgridFlyConv {
	struct SansgridFly formdata;
	uint8_t serialdata[sizeof(struct SansgridFly)];
};
*/

SANSGRID_UNION(struct SansgridFly, SansgridFlyConv);



struct SansgridEyeball {
	// Datatype: 0x00
	uint8_t datatype;
	uint8_t manid[4];
	uint8_t modnum[4];
	uint8_t serial_number[8];
	uint8_t profile;

	// Modes:
	// 	00 - Standard mode
	// 	01 - Sensor Ready to mate
	unsigned mode:4;
};

SANSGRID_UNION(struct SansgridEyeball, SansgridEyeballConv);



struct SansgridPeck {
	// Datatype: 0x01
	uint8_t datatype;
	uint8_t ip[16];
	uint8_t server_id[16];
	// Recognition:
	// 	0x00	- Recognized
	// 	0x01	- Not Recognized, server will mate
	// 	0x02	- Not Recognized, server refuses mate
	// 	0x03	- Not Recognized, sensor refuses mate
	uint8_t recognition;
	uint8_t padding[47];
};

SANSGRID_UNION(struct SansgridPeck, SansgridPeckConv);



struct SansgridSing {
	// Datatype: 	0x02	- Ack, valid pubkey
	// 				0x03	- Ack, no pubkey
	uint8_t datatype;
	uint8_t pubkey[80];
};

SANSGRID_UNION(struct SansgridSing, SansgridSingConv);



struct SansgridMock {
	// Datatype:	0x07	- Ack, sensor gives pubkey
	// 				0x08	- Ack, no pubkey
	uint8_t datatype;
	uint8_t pubkey[80];
};

SANSGRID_UNION(struct SansgridMock, SansgridMockConv);



struct SansgridPeacock {
	// Datatype:	0x0C
	uint8_t datatype;
	uint8_t capabilities[80];
};

SANSGRID_UNION(struct SansgridPeacock, SansgridPeacockConv);



struct SansgridNest {
	// Datatype:	0x10
	uint8_t datatype;
	uint8_t padding[80];
};

SANSGRID_UNION(struct SansgridNest, SansgridNestConv);



struct SansgridSquawk {
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
};

SANSGRID_UNION(struct SansgridSquawk, SansgridSquawkConv);



struct SansgridHeartbeat {
	// Datatypes:
	// 		0x1E: Router Pings Sensor
	// 		0x1F: Sensor responds to ping
	uint8_t datatype;
	uint8_t padding[80];
};

SANSGRID_UNION(struct SansgridHeartbeat, SansgridHeartbeatConv);



struct SansgridChirp {
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
	uint8_t data[80];
};

SANSGRID_UNION(struct SansgridChirp, SansgridChirpConv);



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
	SG_CHIRP_DATA_STREAM_START = 0x22,
	SG_CHIRP_DATA_STREAM_CONTINUE = 0x23,
	SG_CHIRP_DATA_STREAM_END = 0x24,
	SG_CHIRP_NETWORK_DISCONNECTS_SENSOR = 0x25,
	SG_CHIRP_SENSOR_DISCONNECT = 0x26
};

#undef SANSGRID_UNION
// vim: ft=c ts=4 noet sw=4:
