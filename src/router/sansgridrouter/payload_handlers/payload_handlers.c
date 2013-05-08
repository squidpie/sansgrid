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
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <syslog.h>
#include "payload_handlers.h"
#include "../routing_table/routing_table.h"
#include "../communication/sg_tcp.h"


int32_t routerFreeDevice(RoutingTable *routing_table, uint8_t ip_addr[IP_SIZE]) {
	// Something went wrong. Transmit NETWORK_DISCONNECT_SENSOR to ip_addr
	SansgridSerial sg_serial;
	SansgridChirp sg_chirp;

	if ((ip_addr[IP_SIZE-1] & ~1) == 0x0) {
		// router or server. Don't free device
		return -1;
	}

	sg_chirp.datatype = SG_CHIRP_NETWORK_DISCONNECTS_SENSOR;
	memcpy(&sg_serial, &sg_chirp, sizeof(SansgridChirp));

	// Signal to server disconnecting
	sgTCPSend(&sg_serial, sizeof(SansgridSerial));

	// Signal to sensor disconnecting
	sgSerialSend(&sg_serial, sizeof(SansgridSerial));

	routingTableFreeIP(routing_table, ip_addr);	

	return 0;
}


enum SansgridDeviceStatusEnum sgPayloadGetType(enum SansgridDataTypeEnum dt) {
	// Return the generic data type of the payload
	// e.g SG_DEVSTATUS_EYEBALLING, SG_DEVSTATUS_PECKING, etc
	switch (dt) {
		case SG_HATCH:
			// Hatching
			return SG_DEVSTATUS_HATCHING;
			break;
		case SG_FLY:
			// Flying
			return SG_DEVSTATUS_FLYING;
			break;
		case SG_EYEBALL:
			// Eyeballing
			return SG_DEVSTATUS_EYEBALLING;
			break;
		case SG_PECK:
			// Pecking
			return SG_DEVSTATUS_PECKING;
			break;
		case SG_SING_WITH_KEY:
		case SG_SING_WITHOUT_KEY:
			// Singing
			return SG_DEVSTATUS_SINGING;
			break;
		case SG_MOCK_WITH_KEY:
		case SG_MOCK_WITHOUT_KEY:
			// Mocking
			return SG_DEVSTATUS_MOCKING;
			break;
		case SG_PEACOCK:
			// Peacocking
			return SG_DEVSTATUS_PEACOCKING;
			break;
		case SG_NEST:
			return SG_DEVSTATUS_NESTING;
			break;
		case SG_SQUAWK_SERVER_CHALLENGE_SENSOR:
		case SG_SQUAWK_SENSOR_RESPOND_NO_REQUIRE_CHALLENGE:
		case SG_SQUAWK_SENSOR_RESPOND_REQUIRE_CHALLENGE:
		case SG_SQUAWK_SENSOR_CHALLENGE_SERVER:
		case SG_SQUAWK_SERVER_DENY_SENSOR:
		case SG_SQUAWK_SERVER_RESPOND:
		case SG_SQUAWK_SENSOR_ACCEPT_RESPONSE:
			return SG_DEVSTATUS_SQUAWKING;
			break;
		case SG_HEARTBEAT_ROUTER_TO_SENSOR:
		case SG_HEARTBEAT_SENSOR_TO_ROUTER:
			return SG_DEVSTATUS_HEARTBEAT;
			break;
		case SG_CHIRP_COMMAND_SERVER_TO_SENSOR:
		case SG_CHIRP_DATA_SENSOR_TO_SERVER:
		case SG_CHIRP_DATA_STREAM_START:
		case SG_CHIRP_DATA_STREAM_CONTINUE:
		case SG_CHIRP_DATA_STREAM_END:
		case SG_CHIRP_NETWORK_DISCONNECTS_SENSOR:
		case SG_CHIRP_SENSOR_DISCONNECT:
			return SG_DEVSTATUS_CHIRPING;
			break;
		default:
			return SG_DEVSTATUS_NULL;
			break;
	}
	return SG_DEVSTATUS_NULL;
}


int routerHandleHatching(RoutingTable *routing_table, SansgridSerial *sg_serial) {
	// Handle a Hatching data type
	// 1. Set radio IP address
	SANSGRID_UNION(SansgridHatching, SansgridHatchingConv) sg_hatching_union;
	SansgridHatching *sg_hatching;
	DeviceProperties *dev_prop;
	SansgridEyeball *sg_eyeball;

	sg_hatching_union.serialdata = sg_serial->payload;
	sg_hatching = sg_hatching_union.formdata;

	dev_prop = (DeviceProperties*)malloc(sizeof(DeviceProperties));
	dev_prop->dev_status = SG_DEVSTATUS_EYEBALLING;
	sg_eyeball = &dev_prop->dev_attr;
	memset(sg_eyeball->manid, 0x0, sizeof(sg_eyeball->manid));
	memset(sg_eyeball->modnum, 0x0, sizeof(sg_eyeball->modnum));
	memset(sg_eyeball->serial_number, 0x0, sizeof(sg_eyeball->serial_number));
	sg_eyeball->datatype = SG_EYEBALL;

	sg_eyeball->profile = 0x0;
	sg_eyeball->mode = SG_EYEBALL_MATE;
	
	routingTableAssignIPStatic(routing_table, sg_hatching->ip, dev_prop);
	sgSerialSend(sg_serial, sizeof(SansgridSerial));

	return 0;
}


int routerHandleFly(RoutingTable *routing_table, SansgridSerial *sg_serial) {
	// Handle a Fly data type
	// Send a SansgridFly from router to radio
	
	SANSGRID_UNION(SansgridFly, SansgridFlyConv) sg_fly_union;
	char essid[80];
	SansgridFly *sg_fly;

	sg_fly_union.serialdata = sg_serial->payload;
	sg_fly = sg_fly_union.formdata;

	routingTableGetEssid(routing_table, essid);
	if (strcmp(essid, sg_fly->network_name)) {
		// essids don't match, so that's weird
		syslog(LOG_DEBUG, "Sansgrid Network name doesn't match internal network name");
	}
	sgSerialSend(sg_serial, sizeof(SansgridSerial));
	
	return 0;
}


int routerHandleEyeball(RoutingTable *routing_table, SansgridSerial *sg_serial) {
	// Handle an Eyeball data type
	// Assign tentative IP Address 
	// Send SansgridEyeball from sensor to server
	SansgridEyeball *sg_eyeball;
	DeviceProperties *dev_prop;
	SANSGRID_UNION(SansgridEyeball, SansgridEyeballConv) sg_eyeball_union;
	uint8_t ip_addr[IP_SIZE];

	// Convert serial data to formatted data
	sg_eyeball_union.serialdata = sg_serial->payload;
	sg_eyeball = sg_eyeball_union.formdata;

	dev_prop = (DeviceProperties*)malloc(sizeof(DeviceProperties));
	dev_prop->dev_status = SG_DEVSTATUS_EYEBALLING;
	dev_prop->next_expected_packet = SG_DEVSTATUS_PECKING;
	memcpy(&dev_prop->dev_attr, sg_eyeball, sizeof(SansgridEyeball));

	memset(ip_addr, 0x0, sizeof(ip_addr));
	// Store IP in the routing table
	if (sg_eyeball->mode == SG_EYEBALL_MATE) {
		if (!memcmp(sg_serial->ip_addr, ip_addr, sizeof(ip_addr))) {
			// no IP address given
			// Assign an IP address
			routingTableAssignIP(routing_table, ip_addr, dev_prop);
			memcpy(&sg_serial->ip_addr, ip_addr, IP_SIZE);
		} else {
			// IP address given
			if (routingTableAssignIPStatic(routing_table, sg_serial->ip_addr, dev_prop) == 1) {
				syslog(LOG_INFO, "Couldn't statically assign IP");
				routingTableAssignIP(routing_table, ip_addr, dev_prop);
				memcpy(&sg_serial->ip_addr, ip_addr, IP_SIZE);
			}
		}

		// Send packet to the server
		sgTCPSend(sg_serial, sizeof(SansgridSerial));
	} else {
		// TODO: Not implemented yet
		// Have to send a refusal back to sensor
		return 1;
	}

	free(dev_prop);
	return 0;
}


int routerHandlePeck(RoutingTable *routing_table, SansgridSerial *sg_serial) {
	// Handle a Peck data type
	// Send SansgridPeck from server to sensor
	SansgridEyeball sg_eyeball;
	SansgridPeck *sg_peck;
	SANSGRID_UNION(SansgridPeck, SansgridPeckConv) sg_peck_union;
	DeviceProperties dev_prop;
	uint8_t ip_addr[IP_SIZE];

	
	// Convert serial data to formatted data
	sg_peck_union.serialdata = sg_serial->payload;
	sg_peck = sg_peck_union.formdata;

	memcpy(&sg_eyeball.manid, sg_peck->manid, 4);
	memcpy(&sg_eyeball.modnum, sg_peck->modnum, 4);
	memcpy(&sg_eyeball.serial_number, sg_peck->serial_number, 8);

	memcpy(&dev_prop.dev_attr, &sg_eyeball, sizeof(SansgridEyeball));
	if (routingTableFindByAttr(routing_table, &dev_prop, ip_addr) != 1) {
		// error
		return -1;
	}
	memcpy(&sg_serial->ip_addr, ip_addr, IP_SIZE);
	//routingTableGetRouterIP(routing_table, sg_serial->origin_ip);

	switch (sg_peck->recognition) {
		case SG_PECK_RECOGNIZED:
			// Sensor Recognized
			// Next packet: Squawk
			routingTableSetNextExpectedPacket(routing_table, sg_serial->ip_addr,
					SG_DEVSTATUS_SQUAWKING);
			sgSerialSend(sg_serial, sizeof(SansgridSerial));
			break;
		case SG_PECK_MATE:
			// Sensor Not Recognized;
			// Server will mate
			// Next packet: Mate
			routingTableSetNextExpectedPacket(routing_table, sg_serial->ip_addr,
					SG_DEVSTATUS_SINGING);
			sgSerialSend(sg_serial, sizeof(SansgridSerial));
			break;
		case SG_PECK_SERVER_REFUSES_MATE:
			// Sensor Not Recognized;
			// Server refuses mate
		case SG_PECK_SENSOR_REFUSES_MATE:
			// Sensor Not Recognized;
			// Sensor refuses mate
		default:
			// fallthrough/error
			routerFreeDevice(routing_table, sg_serial->ip_addr);
			return 1;
			break;
	}

	return 0;
}


int routerHandleSing(RoutingTable *routing_table, SansgridSerial *sg_serial) {
	// Handle a Sing data type
	// Send a SansgridSing payload from server to sensor
	SansgridSing *sg_sing;
	SANSGRID_UNION(SansgridSing, SansgridSingConv) sg_sing_union;

	// Convert serial data to formatted data
	sg_sing_union.serialdata = sg_serial->payload;
	sg_sing = sg_sing_union.formdata;

	routingTableSetNextExpectedPacket(routing_table, sg_serial->ip_addr,
			SG_DEVSTATUS_MOCKING);

	switch (sg_sing->datatype) {
		case SG_SING_WITH_KEY:
			// Acknowledgement and server's public key
		case SG_SING_WITHOUT_KEY:
			// Acknowledgement, no public key
			sgSerialSend(sg_serial, sizeof(SansgridSerial));
			break;
		default:
			// error
			routerFreeDevice(routing_table, sg_serial->ip_addr);
			return 1;
			break;
	}

	return 0;
}


int routerHandleMock(RoutingTable *routing_table, SansgridSerial *sg_serial) {
	// Handle a Mock data type
	// Send a SansgridMock payload from server to sensor
	SansgridMock *sg_mock;
	SANSGRID_UNION(SansgridMock, SansgridMockConv) sg_mock_union;

	// Convert serial data to formatted data
	sg_mock_union.serialdata = sg_serial->payload;
	sg_mock = sg_mock_union.formdata;

	routingTableSetNextExpectedPacket(routing_table, sg_serial->ip_addr,
			SG_DEVSTATUS_PEACOCKING);


	switch (sg_mock->datatype) {
		case SG_MOCK_WITH_KEY:
			// Acknowledgement and sensor's public key
		case SG_MOCK_WITHOUT_KEY:
			// Acknowledgement, no sensor key
			sgTCPSend(sg_serial, sizeof(SansgridSerial));
			break;
		default:
			routerFreeDevice(routing_table, sg_serial->ip_addr);
			return 1;
			break;
	}

	return 0;
}


int routerHandlePeacock(RoutingTable *routing_table, SansgridSerial *sg_serial) {
	// Handle a Peacock data type
	// Send a SansgridPeacock from sensor to server
	SansgridPeacock *sg_peacock;
	SANSGRID_UNION(SansgridPeacock, SansgridPeacockConv) sg_peacock_union;

	// Convert serial data to formatted data
	sg_peacock_union.serialdata = sg_serial->payload;
	sg_peacock = sg_peacock_union.formdata;

	if (sg_peacock->additional_IO_needed == 1) {
		routingTableSetNextExpectedPacket(routing_table, sg_serial->ip_addr,
				SG_DEVSTATUS_PEACOCKING);
	} else {
		routingTableSetNextExpectedPacket(routing_table, sg_serial->ip_addr,
				SG_DEVSTATUS_NESTING);
	}
	sgTCPSend(sg_serial, sizeof(SansgridSerial));

	return 0;
}


int routerHandleNest(RoutingTable *routing_table, SansgridSerial *sg_serial) {
	// Handle a Nest data type
	// Send a SansgridNest from server to sensor
	routingTableSetNextExpectedPacket(routing_table, sg_serial->ip_addr,
			SG_DEVSTATUS_LEASED);
	sgSerialSend(sg_serial, sizeof(SansgridSerial));

	return 0;
}


int routerHandleSquawk(RoutingTable *routing_table, SansgridSerial *sg_serial) {
	// Handle a Squawk data type
	// Send a SansgridSquawk 	from server to sensor
	// 						 or from sensor to server
	SansgridSquawk *sg_squawk;
	SANSGRID_UNION(SansgridSquawk, SansgridSquawkConv) sg_squawk_union;

	// Convert serial data to formatted data
	sg_squawk_union.serialdata = sg_serial->payload;
	sg_squawk = sg_squawk_union.formdata;

	switch (sg_squawk->datatype) {
		case SG_SQUAWK_SERVER_CHALLENGE_SENSOR:
			// Server Challenges sensor
			routingTableSetNextExpectedPacket(routing_table, sg_serial->ip_addr,
					SG_DEVSTATUS_SQUAWKING);
			sgSerialSend(sg_serial, sizeof(SansgridSerial));
			break;
		case SG_SQUAWK_SERVER_NOCHALLENGE_SENSOR:
			// Server Responds without challenge to sensor
			// TODO: Confirm datatype addition
			routingTableSetNextExpectedPacket(routing_table, sg_serial->ip_addr,
					SG_DEVSTATUS_SQUAWKING);
			sgSerialSend(sg_serial, sizeof(SansgridSerial));
			break;
		case SG_SQUAWK_SENSOR_RESPOND_NO_REQUIRE_CHALLENGE:
			// Sensor respond to server challenge,
			// no sensor challenge needed
			routingTableSetNextExpectedPacket(routing_table, sg_serial->ip_addr,
					SG_DEVSTATUS_SQUAWKING);
			sgTCPSend(sg_serial, sizeof(SansgridSerial));
			break;
		case SG_SQUAWK_SENSOR_RESPOND_REQUIRE_CHALLENGE:
			// Sensor respond to server challenge,
			// sensor challenge coming
			routingTableSetNextExpectedPacket(routing_table, sg_serial->ip_addr,
					SG_DEVSTATUS_SQUAWKING);
			sgTCPSend(sg_serial, sizeof(SansgridSerial));
			break;
		case SG_SQUAWK_SENSOR_CHALLENGE_SERVER:
			// Sensor challenges server
			routingTableSetNextExpectedPacket(routing_table, sg_serial->ip_addr,
					SG_DEVSTATUS_SQUAWKING);
			sgTCPSend(sg_serial, sizeof(SansgridSerial));
			break;
		case SG_SQUAWK_SERVER_DENY_SENSOR:
			// Server denies sensor challenge request
			routerFreeDevice(routing_table, sg_serial->ip_addr);
			break;
		case SG_SQUAWK_SERVER_RESPOND:
			// Server Responds to challenge
			routingTableSetNextExpectedPacket(routing_table, sg_serial->ip_addr,
					SG_DEVSTATUS_SQUAWKING);
			sgSerialSend(sg_serial, sizeof(SansgridSerial));
			break;
		case SG_SQUAWK_SENSOR_ACCEPT_RESPONSE:
			// Sensor accepts server's response
			routingTableSetNextExpectedPacket(routing_table, sg_serial->ip_addr,
					SG_DEVSTATUS_NESTING);
			sgTCPSend(sg_serial, sizeof(SansgridSerial));
			break;
		default:
			// error
			routerFreeDevice(routing_table, sg_serial->ip_addr);
			return 1;
			break;
	}
	return 0;
}

int routerHandleHeartbeat(RoutingTable *routing_table, SansgridSerial *sg_serial) {
	// Handle a Heartbeat data type
	// Send a SansgridHeartbeat from router to sensor
	// Receive a SansgridHeartbeat from sensor

	SansgridHeartbeat *sg_heartbeat;
	SANSGRID_UNION(SansgridHeartbeat, SGHB) sansgrid_heartbeat_union;

	sansgrid_heartbeat_union.serialdata = sg_serial->payload;
	sg_heartbeat = sansgrid_heartbeat_union.formdata;

	switch (sg_heartbeat->datatype) {
		case SG_HEARTBEAT_ROUTER_TO_SENSOR:
			// Heartbeat from router to sensor
			routingTableSetHeartbeatStatus(routing_table, sg_serial->ip_addr,
					SG_DEVICE_PINGING);
			sgSerialSend(sg_serial, sizeof(SansgridSerial));
			break;
		case SG_HEARTBEAT_SENSOR_TO_ROUTER:
			// Heartbeat response from sensor
			routingTableSetHeartbeatStatus(routing_table, sg_serial->ip_addr,
					SG_DEVICE_PRESENT);
			break;
		default:
			routerFreeDevice(routing_table, sg_serial->ip_addr);
			return 1;
			break;
	}

	return 0;
}

int routerHandleChirp(RoutingTable *routing_table, SansgridSerial *sg_serial) {
	// Handle a Chirp data type
	// Send a SansgridChirp from server to sensor
	// Send a SansgridChirp from sensor to server
	
	SansgridChirp *sg_chirp;
	SANSGRID_UNION(SansgridChirp, SansgridChirpConv) sg_chirp_union;

	// Convert serial data to formatted data
	sg_chirp_union.serialdata = sg_serial->payload;
	sg_chirp = sg_chirp_union.formdata;
	

	switch (sg_chirp->datatype) {
		case SG_CHIRP_COMMAND_SERVER_TO_SENSOR:
			// Command sent from server to sensor
			routingTableSetNextExpectedPacket(routing_table, sg_serial->ip_addr,
					SG_DEVSTATUS_LEASED);
			sgSerialSend(sg_serial, sizeof(SansgridSerial));
			break;
		case SG_CHIRP_DATA_SENSOR_TO_SERVER:
			// Data sent from server to sensor
			routingTableSetNextExpectedPacket(routing_table, sg_serial->ip_addr,
					SG_DEVSTATUS_LEASED);
			sgTCPSend(sg_serial, sizeof(SansgridSerial));
			break;
		case SG_CHIRP_DATA_STREAM_START:
			// Start of data stream
			// Currently undefined
		case SG_CHIRP_DATA_STREAM_CONTINUE:
			// Continued stream of data
			// Currently undefined
		case SG_CHIRP_DATA_STREAM_END:
			// End of data stream
			// Currently undefined
		case SG_CHIRP_NETWORK_DISCONNECTS_SENSOR:
			// Network is disconnecting sensor
			routerFreeDevice(routing_table, sg_serial->ip_addr);
			break;
		case SG_CHIRP_SENSOR_DISCONNECT:
			// Sensor is disconnecting from network
			routerFreeDevice(routing_table, sg_serial->ip_addr);
			break;
		default:
			routerFreeDevice(routing_table, sg_serial->ip_addr);
			return -1;
			break;
	}

	return 0;
}


// vim: ft=c ts=4 noet sw=4:
