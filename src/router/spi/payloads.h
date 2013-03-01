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

// TODO: Which is better, using all uint8_t, 
// 				or using largest divisible register? (e.g. uint32_t)
// 		Depends on how complicated endian encoding is

#include <stdint.h>

#define SANSGRID_UNION(type, name) union name { \
	type formdata; \
	uint8_t serialdata[sizeof(type)]; \
};

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

SANSGRID_UNION(struct SansgridFly, SansgridFlyConv)


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

/*
union SansgridEyeballConv {
	struct SansgridEyeball formdata;
	uint8_t serialdata[sizeof(struct SansgridEyeball)];
};
*/

SANSGRID_UNION(struct SansgridEyeball, SansgridEyeballConv)


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


SANSGRID_UNION(struct SansgridPeck, SansgridPeckConv)

/*
union SansgridPeckConv {
	struct SansgridPeck formdata;
	uint8_t serialdata[sizeof(struct SansgridPeck)];
};
*/


struct SansgridSing {
	// Datatype: 	0x02	- Ack, valid pubkey
	// 				0x03	- Ack, no pubkey
	uint8_t datatype;
	uint8_t pubkey[80];
};

SANSGRID_UNION(struct SansgridSing, SansgridSingConv)


struct SansgridMock {
	// Datatype:	0x07	- Ack, sensor gives pubkey
	// 				0x08	- Ack, no pubkey
	uint8_t datatype;
	uint8_t pubkey[80];
};

SANSGRID_UNION(struct SansgridMock, SansgridMockConv)

struct SansgridPeacock {
	// Datatype:	0x0C
	uint8_t datatype;
	uint8_t capabilities[80];
};

SANSGRID_UNION(struct SansgridPeacock, SansgridPeacockConv)

struct SansgridNest {
	// Datatype:	0x10
	uint8_t datatype;
	uint8_t padding[80];
};

SANSGRID_UNION(struct SansgridNest, SansgridNestConv)

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


SANSGRID_UNION(struct SansgridSquawk, SansgridSquawkConv)


struct SansgridHeartbeat {
	// Datatypes:
	// 		0x1E: Router Pings Sensor
	// 		0x1F: Sensor responds to ping
	uint8_t datatype;
	uint8_t padding[80];
};


SANSGRID_UNION(struct SansgridHeartbeat, SansgridHeartbeatConv)

struct SansgridChirp {
	// Datatypes
	// 		0x20: Command sent from server to sensor
	// 		0x21: Data sent from sensor to server
	// 		0x22: Start of data stream
	// 		0x23: Continued data stream
	// 		0x24: End of data stream
	uint8_t datatype;
	uint8_t data[80];
};

SANSGRID_UNION(struct SansgridChirp, SansgridChirpConv)

// vim: ft=c ts=4 noet sw=4:
