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
/// \file
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <syslog.h>
#include "payload_handlers.h"
#include "../sansgrid_router.h"
#include "../routing_table/routing_table.h"
#include "../communication/sg_tcp.h"


/// Check for a valid control path
static int32_t chkValidCTRLPath(RoutingTable *routing_table, 
		uint8_t ip_addr[IP_SIZE], uint8_t dt, const char *pktname) {
	// check to see if this is a valid control path for this device
	if (routingTableCheckValidPacket(routing_table, ip_addr, dt) <= 0) {
		// Not a valid control path
		syslog(LOG_NOTICE, "%s is not a valid payload for this device: %x", 
				pktname, ip_addr[IP_SIZE-1]);
		return -1;
	} else {
		return 0;
	}
}


/**
 * \brief Send a disconnect signal and deallocate a device
 *
 * This function sends a NETWORK_DISCONNECT_SENSOR signal 
 * and frees the device with IP address ip_addr
 */
int32_t routerFreeDevice(RoutingTable *routing_table, uint8_t ip_addr[IP_SIZE]) {
	// Something went wrong. Transmit NETWORK_DISCONNECT_SENSOR to ip_addr
	SansgridSerial sg_serial;
	SansgridChirp sg_chirp;

	if ((ip_addr[IP_SIZE-1] & ~1) == 0x0) {
		// router or server. Don't free device
		syslog(LOG_WARNING, "Can't free reserved devices!");
		return -1;
	} else if (!routingTableLookup(routing_table, ip_addr)) {
		syslog(LOG_DEBUG, "Trying to free device that doesn't exist");
		return 0;
	}

	memset(&sg_chirp, 0x0, sizeof(SansgridChirp));
	sg_chirp.datatype = SG_CHIRP_NETWORK_DISCONNECTS_SENSOR;
	memcpy(&sg_serial.payload, &sg_chirp, sizeof(SansgridChirp));
	memcpy(sg_serial.ip_addr, ip_addr, IP_SIZE);

	// Signal to server disconnecting
	sgTCPSend(&sg_serial, sizeof(SansgridSerial));

	// Signal to sensor disconnecting
	sgSerialSend(&sg_serial, sizeof(SansgridSerial));

	routingTableFreeIP(routing_table, ip_addr);	

	return 0;
}



/**
 * \brief Disconnect all devices
 *
 * This function sends a NETWORK_DISCONNECT_SENSOR signal
 * to all devices, then frees all devices (except the router)
 */
int32_t routerFreeAllDevices(RoutingTable *routing_table) {
	// Transmit NETWORK_DISCONNECT_SENSOR to all devices on network
	uint8_t ip_addr[IP_SIZE];
	uint8_t router_ip[IP_SIZE];
	routingTableGetRouterIP(routing_table, router_ip);
	routingTableForEachDevice(routing_table, ip_addr);
	do {
		// free all devices
		if (memcmp(ip_addr, router_ip, IP_SIZE)) {
			routerFreeDevice(routing_table, ip_addr);
		}
		if (routingTableGetDeviceCount(routing_table) < 2)
			break;
	} while (routingTableStepNextDevice(routing_table, ip_addr));

	return 0;
}



/**
 * \brief Send a message to the server that device has come back online
 *
 * Used when a stale device is heard from.
 */
static int32_t routerRefreshDevice(uint8_t ip_addr[IP_SIZE]) {
	// Send a message to the server that device has come back online
	//printf("Refreshing\n");
	SansgridIRStatus sg_irstatus;
	SansgridSerial *sg_serial = NULL;
	sg_serial = (SansgridSerial*)malloc(sizeof(SansgridSerial));
	memset(sg_serial, 0x0, sizeof(SansgridSerial));
	memset(&sg_irstatus, 0x0, sizeof(SansgridIRStatus));
	sg_irstatus.datatype = SG_SERVSTATUS;
	strcpy(sg_irstatus.status, "online");
	memcpy(sg_serial->ip_addr, ip_addr, IP_SIZE);
	memcpy(sg_serial->payload, &sg_irstatus, sizeof(SansgridIRStatus));
	sg_serial->control = 0xad;
	queueEnqueue(dispatch, sg_serial);
	return 0;
}



/**
 * \brief Translate a Sansgrid datatype into a generic datatype
 *
 * The Sansgrid datatypes are verbose; sometimes it's more desirable
 * to reduce the many types for each payload into one parameter to check.
 * That is the idea of this function. For instance, a Chirp has many
 * different types. This function reduces that to SG_DEVSTATUS_CHIRPING.
 * \param	dt		A Sansgrid Payload Datatype
 * \returns
 * A Generic Payload type is returned
 */
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
			// Nesting
			return SG_DEVSTATUS_NESTING;
			break;
		case SG_SQUAWK_SERVER_CHALLENGE_SENSOR:
		case SG_SQUAWK_SERVER_NOCHALLENGE_SENSOR:
		case SG_SQUAWK_SENSOR_RESPOND_NO_REQUIRE_CHALLENGE:
		case SG_SQUAWK_SENSOR_RESPOND_REQUIRE_CHALLENGE:
		case SG_SQUAWK_SENSOR_CHALLENGE_SERVER:
		case SG_SQUAWK_SERVER_DENY_SENSOR:
		case SG_SQUAWK_SERVER_RESPOND:
		case SG_SQUAWK_SENSOR_ACCEPT_RESPONSE:
		case SG_SQUAWK_FORGET_ME:
			// Squawking
			return SG_DEVSTATUS_SQUAWKING;
			break;
		case SG_HEARTBEAT_ROUTER_TO_SENSOR:
		case SG_HEARTBEAT_SENSOR_TO_ROUTER:
		case SG_SERVSTATUS:
			// Heartbeating
			return SG_DEVSTATUS_HEARTBEAT;
			break;
		case SG_CHIRP_COMMAND_SERVER_TO_SENSOR:
		case SG_CHIRP_DATA_SENSOR_TO_SERVER:
		case SG_CHIRP_NETWORK_DISCONNECTS_SENSOR:
		case SG_CHIRP_SENSOR_DISCONNECT:
			// Chirping
			return SG_DEVSTATUS_CHIRPING;
			break;
		default:
			// Couldn't find datatype
			syslog(LOG_DEBUG, "payload conversion: Don't know datatype %x", dt);
			return SG_DEVSTATUS_NULL;
			break;
	}
	return SG_DEVSTATUS_NULL;
}



/**
 * \brief returns the name of a generic data type in a null-terminated string
 *
 * \param[in]	devstatus		A General payload type
 * \param[out]	name			A null-terminated string the name is returned in
 * \returns
 * 0 when the datatype was found \n
 * -1 on error
 */
int32_t sgPayloadGetPayloadName(enum SansgridDeviceStatusEnum devstatus, char *name) {
	// Get the name of a payload
	switch(devstatus) {
		case SG_DEVSTATUS_NULL:
			strcpy(name, "Unknown");
			break;
		case SG_DEVSTATUS_HATCHING:
			strcpy(name, "Hatching");
			break;
		case SG_DEVSTATUS_FLYING:
			strcpy(name, "Flying");
			break;
		case SG_DEVSTATUS_EYEBALLING:
			strcpy(name, "Eyeballing");
			break;
		case SG_DEVSTATUS_PECKING:
			strcpy(name, "Pecking");
			break;
		case SG_DEVSTATUS_SINGING:
			strcpy(name, "Singing");
			break;
		case SG_DEVSTATUS_MOCKING:
			strcpy(name, "Mocking");
			break;
		case SG_DEVSTATUS_PEACOCKING:
			strcpy(name, "Peacocking");
			break;
		case SG_DEVSTATUS_NESTING:
			strcpy(name, "Nesting");
			break;
		case SG_DEVSTATUS_SQUAWKING:
			strcpy(name, "Squawking");
			break;
		case SG_DEVSTATUS_HEARTBEAT:
			strcpy(name, "Heartbeating");
			break;
		case SG_DEVSTATUS_CHIRPING:
			strcpy(name, "Chirping");
			break;
		default:
			strcpy(name, "Bad Datatype");
			return -1;
			break;
	}
	return 0;
}


/**
 * \brief Handle a SansgridHatching payload
 *
 * Sends a Hatch to the radio
 */
int routerHandleHatching(RoutingTable *routing_table, SansgridSerial *sg_serial) {
	// Handle a Hatching data type
	syslog(LOG_DEBUG, "Handling Hatch");
	SANSGRID_UNION(SansgridHatching, SansgridHatchingConv) sg_hatching_union;
	SansgridHatching *sg_hatching;

	sg_hatching_union.serialdata = sg_serial->payload;
	sg_hatching = sg_hatching_union.formdata;
	
	routingTableAssignIPStatic(routing_table, sg_hatching->ip);
	sgSerialSend(sg_serial, sizeof(SansgridSerial));

	return 0;
}



/**
 * \brief Handle a SansgridFly payload
 *
 * Sends a Fly to the radio to be broadcast on the network
 */
int routerHandleFly(RoutingTable *routing_table, SansgridSerial *sg_serial) {
	// Handle a Fly data type
	// Send a SansgridFly from router to radio
	
	SANSGRID_UNION(SansgridFly, SansgridFlyConv) sg_fly_union;
	char essid[80];
	SansgridFly *sg_fly;

	syslog(LOG_DEBUG, "Handling Fly Packet");
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



/**
 * \brief Handle a SansgridEyeball payload
 *
 * Sends an Eyeball to the server
 */
int routerHandleEyeball(RoutingTable *routing_table, SansgridSerial *sg_serial) {
	// Handle an Eyeball data type
	// Assign tentative IP Address 
	// Send SansgridEyeball from sensor to server
	SansgridEyeball *sg_eyeball;
	SANSGRID_UNION(SansgridEyeball, SansgridEyeballConv) sg_eyeball_union;
	uint8_t ip_addr[IP_SIZE];

	if (sg_serial->ip_addr[IP_SIZE-1])
		syslog(LOG_INFO, "Handling Eyeball packet: device wants IP that ends with %u", 
				sg_serial->ip_addr[IP_SIZE-1]);
	else
		syslog(LOG_INFO, "Handling Eyeball packet: device wants dynamic IP");
	// Convert serial data to formatted data
	sg_eyeball_union.serialdata = sg_serial->payload;
	sg_eyeball = sg_eyeball_union.formdata;

	//memset(ip_addr, 0x0, sizeof(ip_addr));
	memcpy(ip_addr, sg_serial->ip_addr, IP_SIZE);
	// Store IP in the routing table
	if (sg_eyeball->mode == SG_EYEBALL_MATE) {
		syslog(LOG_DEBUG, "New device wishes to mate");
	} else {
		syslog(LOG_DEBUG, "New device doesn't wish to mate");
	}

	if (!memcmp(sg_serial->ip_addr, ip_addr, sizeof(ip_addr))) {
		// no IP address given
		// Assign an IP address
		syslog(LOG_INFO, "Assigning IP dynamically for new device");
		routingTableAssignIP(routing_table, ip_addr);
		memcpy(&sg_serial->ip_addr, ip_addr, IP_SIZE);
	} else {
		// IP address given
		if (routingTableAssignIPStatic(routing_table, sg_serial->ip_addr) == 1) {
			syslog(LOG_INFO, "Couldn't statically assign IP for new device");
			routingTableAssignIP(routing_table, ip_addr);
			memcpy(&sg_serial->ip_addr, ip_addr, IP_SIZE);
		}
	}
	routingTableSetCurrentPacket(routing_table, 
			sg_serial->ip_addr, SG_DEVSTATUS_EYEBALLING);
	routingTableSetNextExpectedPacket(routing_table, sg_serial->ip_addr, SG_DEVSTATUS_PECKING);

	// Send packet to the server
	syslog(LOG_DEBUG, "Sending Eyeball to server");
	sgTCPSend(sg_serial, sizeof(SansgridSerial));

	return 0;
}



/**
 * \brief Handle a SansgridPeck payload
 *
 * Sends a Peck over the radio to a sensor
 */
int routerHandlePeck(RoutingTable *routing_table, SansgridSerial *sg_serial) {
	// Handle a Peck data type
	// Send SansgridPeck from server to sensor
	SansgridPeck *sg_peck;
	SANSGRID_UNION(SansgridPeck, SansgridPeckConv) sg_peck_union;

	
	// Convert serial data to formatted data
	sg_peck_union.serialdata = sg_serial->payload;
	sg_peck = sg_peck_union.formdata;

	// NOTE: Peck sends out a broadcast, so you need to check assigned
	// 	IP instead
	syslog(LOG_INFO, "Handling Peck packet: device %u",
			routingTableIPToRDID(routing_table, sg_peck->assigned_ip));
	if (chkValidCTRLPath(routing_table, sg_peck->assigned_ip,
				sg_peck->datatype, "Peck") < 0)
		return -1;
	routingTableSetCurrentPacket(routing_table, 
			sg_peck->assigned_ip, SG_DEVSTATUS_PECKING);

	switch (sg_peck->recognition) {
		case SG_PECK_RECOGNIZED:
			// Sensor Recognized
			// Next packet: Squawk
			routingTableSetNextExpectedPacket(routing_table, 
					sg_peck->assigned_ip,
					SG_DEVSTATUS_SQUAWKING);
			sgSerialSend(sg_serial, sizeof(SansgridSerial));
			break;
		case SG_PECK_MATE:
		case SG_PECK_SERVER_WAIT:
			// Sensor Not Recognized;
			// Server will mate
			// Next packet: Mate
			routingTableSetNextExpectedPacket(routing_table, 
					sg_peck->assigned_ip,
					SG_DEVSTATUS_SINGING);
			sgSerialSend(sg_serial, sizeof(SansgridSerial));
			break;
		case SG_PECK_SERVER_REFUSES_MATE:
			// Sensor Not Recognized;
			// Server refuses mate
		case SG_PECK_SENSOR_REFUSES_MATE:
			// Sensor Not Recognized;
			// Sensor refuses mate
			sgSerialSend(sg_serial, sizeof(SansgridSerial));
			routerFreeDevice(routing_table,
					sg_peck->assigned_ip);
			break;
		default:
			// fallthrough/error
			syslog(LOG_WARNING, "Peck: Recognition is not recognized: %x\n",
					sg_peck->recognition);
			routerFreeDevice(routing_table, 
					sg_peck->assigned_ip);
			return 1;
			break;
	}

	return 0;
}



/**
 * \brief Handle a SansgridSing Payload
 *
 * Sends a Sing to the radio
 */
int routerHandleSing(RoutingTable *routing_table, SansgridSerial *sg_serial) {
	// Handle a Sing data type
	// Send a SansgridSing payload from server to sensor
	SansgridSing *sg_sing;
	SANSGRID_UNION(SansgridSing, SansgridSingConv) sg_sing_union;

	// Convert serial data to formatted data
	sg_sing_union.serialdata = sg_serial->payload;
	sg_sing = sg_sing_union.formdata;

	syslog(LOG_INFO, "Handling Sing packet: device %u",
			routingTableIPToRDID(routing_table, sg_serial->ip_addr));
	if (chkValidCTRLPath(routing_table, sg_serial->ip_addr, 
				sg_sing->datatype, "Sing") < 0)
		return -1;

	routingTableSetCurrentPacket(routing_table, 
			sg_serial->ip_addr, SG_DEVSTATUS_SINGING);
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
			syslog(LOG_WARNING, "Sing: datatype not recognized: %x\n",
					sg_sing->datatype);
			routerFreeDevice(routing_table, sg_serial->ip_addr);
			return 1;
			break;
	}

	return 0;
}



/**
 * \brief Handle a SansgridMock payload
 *
 * Sends a Mock to the server
 */
int routerHandleMock(RoutingTable *routing_table, SansgridSerial *sg_serial) {
	// Handle a Mock data type
	// Send a SansgridMock payload from server to sensor
	SansgridMock *sg_mock;
	SANSGRID_UNION(SansgridMock, SansgridMockConv) sg_mock_union;

	// Convert serial data to formatted data
	sg_mock_union.serialdata = sg_serial->payload;
	sg_mock = sg_mock_union.formdata;

	syslog(LOG_INFO, "Handling Mock packet: device %u",
			routingTableIPToRDID(routing_table, sg_serial->ip_addr));

	if (chkValidCTRLPath(routing_table, sg_serial->ip_addr, 
				sg_mock->datatype, "Mock") < 0)
		return -1;
	routingTableSetCurrentPacket(routing_table, 
			sg_serial->ip_addr, SG_DEVSTATUS_MOCKING);
	routingTableSetNextExpectedPacket(routing_table, sg_serial->ip_addr,
			SG_DEVSTATUS_PEACOCKING);
	if (routingTableHeardDevice(routing_table, sg_serial->ip_addr) > 0) {
		routerRefreshDevice(sg_serial->ip_addr);
	}



	switch (sg_mock->datatype) {
		case SG_MOCK_WITH_KEY:
			// Acknowledgement and sensor's public key
		case SG_MOCK_WITHOUT_KEY:
			// Acknowledgement, no sensor key
			sgTCPSend(sg_serial, sizeof(SansgridSerial));
			break;
		default:
			syslog(LOG_WARNING, "mock: datatype not recognized: %x\n",
					sg_mock->datatype);
			routerFreeDevice(routing_table, sg_serial->ip_addr);
			return 1;
			break;
	}

	return 0;
}



/**
 * \brief Handle a SansgridPeacock payload
 *
 * Sends a Peacock to the server
 */
int routerHandlePeacock(RoutingTable *routing_table, SansgridSerial *sg_serial) {
	// Handle a Peacock data type
	// Send a SansgridPeacock from sensor to server
	SansgridPeacock *sg_peacock;
	SANSGRID_UNION(SansgridPeacock, SansgridPeacockConv) sg_peacock_union;

	// Convert serial data to formatted data
	sg_peacock_union.serialdata = sg_serial->payload;
	sg_peacock = sg_peacock_union.formdata;

	syslog(LOG_INFO, "Handling Peacock packet: device %u",
			routingTableIPToRDID(routing_table, sg_serial->ip_addr));
	if (chkValidCTRLPath(routing_table, sg_serial->ip_addr, 
				sg_peacock->datatype, "Peacock") < 0)
		return -1;
	routingTableSetCurrentPacket(routing_table, 
			sg_serial->ip_addr, SG_DEVSTATUS_PEACOCKING);
	if (routingTableHeardDevice(routing_table, sg_serial->ip_addr) > 0) {
		routerRefreshDevice(sg_serial->ip_addr);
	}
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



/**
 * \brief Handle a SansgridNest payload
 *
 * Sends a Nest over the radio to the sensor
 */
int routerHandleNest(RoutingTable *routing_table, SansgridSerial *sg_serial) {
	// Handle a Nest data type
	// Send a SansgridNest from server to sensor
	syslog(LOG_INFO, "Handling Nest packet: device %u",
			routingTableIPToRDID(routing_table, sg_serial->ip_addr));
	if (chkValidCTRLPath(routing_table, sg_serial->ip_addr, 
				sg_serial->payload[0], "Nest") < 0)
		return -1;
	routingTableSetCurrentPacket(routing_table, 
			sg_serial->ip_addr, SG_DEVSTATUS_NESTING);
	if (routingTableHeardDevice(routing_table, sg_serial->ip_addr) > 0) {
		routerRefreshDevice(sg_serial->ip_addr);
	}
	routingTableSetNextExpectedPacket(routing_table, sg_serial->ip_addr,
			SG_DEVSTATUS_LEASED);
	sgSerialSend(sg_serial, sizeof(SansgridSerial));

	return 0;
}


/**
 * \brief Handle a SansgridSquawk payload
 *
 * Sends a Squawk either to the server, or to the sensor
 */
int routerHandleSquawk(RoutingTable *routing_table, SansgridSerial *sg_serial) {
	// Handle a Squawk data type
	// Send a SansgridSquawk 	from server to sensor
	// 						 or from sensor to server
	SansgridSquawk *sg_squawk;
	SANSGRID_UNION(SansgridSquawk, SansgridSquawkConv) sg_squawk_union;

	// Convert serial data to formatted data
	sg_squawk_union.serialdata = sg_serial->payload;
	sg_squawk = sg_squawk_union.formdata;

	syslog(LOG_INFO, "Handling Squawk packet: device %u",
			routingTableIPToRDID(routing_table, sg_serial->ip_addr));

	if (chkValidCTRLPath(routing_table, sg_serial->ip_addr, 
				sg_squawk->datatype, "Squawk") < 0)
		return -1;

	routingTableSetCurrentPacket(routing_table, 
			sg_serial->ip_addr, SG_DEVSTATUS_SQUAWKING);
	if (routingTableHeardDevice(routing_table, sg_serial->ip_addr) > 0) {
		routerRefreshDevice(sg_serial->ip_addr);
	}

	switch (sg_squawk->datatype) {
		case SG_SQUAWK_SERVER_CHALLENGE_SENSOR:
			// Server Challenges sensor
			routingTableSetNextExpectedPacket(routing_table, sg_serial->ip_addr,
					SG_DEVSTATUS_SQUAWKING);
			sgSerialSend(sg_serial, sizeof(SansgridSerial));
			break;
		case SG_SQUAWK_SERVER_NOCHALLENGE_SENSOR:
			// Server Responds without challenge to sensor
			routingTableSetNextExpectedPacket(routing_table, sg_serial->ip_addr,
					SG_DEVSTATUS_SQUAWKING);
			sgSerialSend(sg_serial, sizeof(SansgridSerial));
			break;
		case SG_SQUAWK_SENSOR_RESPOND_NO_REQUIRE_CHALLENGE:
			// Sensor respond to server challenge,
			// no sensor challenge needed
			routingTableSetNextExpectedPacket(routing_table, sg_serial->ip_addr,
					SG_DEVSTATUS_SQUAWKING | SG_DEVSTATUS_NESTING);
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
			routingTableSetNextExpectedPacket(routing_table, sg_serial->ip_addr,
					SG_DEVSTATUS_SQUAWKING);
			sgSerialSend(sg_serial, sizeof(SansgridSerial));
			//routerFreeDevice(routing_table, sg_serial->ip_addr);
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
		case SG_SQUAWK_FORGET_ME:
			// Sensor requires server to forget it
			routingTableSetNextExpectedPacket(routing_table, sg_serial->ip_addr,
					SG_DEVSTATUS_SQUAWKING);
			sgTCPSend(sg_serial, sizeof(SansgridSerial));
			//routerFreeDevice(routing_table, sg_serial->ip_addr);
			break;
		default:
			// error
			syslog(LOG_WARNING, "Squawk: datatype not recognized: %x\n",
					sg_squawk->datatype);
			routerFreeDevice(routing_table, sg_serial->ip_addr);
			return 1;
			break;
	}
	return 0;
}


/**
 * \brief Handle a SansgridHeartbeat payload
 *
 * Sends a Heartbeat to the radio, which will forward the payload
 * to a sensor radio \n
 * Or refresh a device if a heartbeat response has been received
 * from a sensor radio
 */
int routerHandleHeartbeat(RoutingTable *routing_table, SansgridSerial *sg_serial) {
	// Handle a Heartbeat data type
	// Send a SansgridHeartbeat from router to sensor
	// Receive a SansgridHeartbeat from sensor

	SansgridHeartbeat *sg_heartbeat;
	SANSGRID_UNION(SansgridHeartbeat, SGHB) sansgrid_heartbeat_union;

	sansgrid_heartbeat_union.serialdata = sg_serial->payload;
	sg_heartbeat = sansgrid_heartbeat_union.formdata;

	if (sg_heartbeat->datatype != SG_SERVSTATUS) {
		syslog(LOG_INFO, "Handling Heartbeat packet: device %u",
				routingTableIPToRDID(routing_table, sg_serial->ip_addr));
	} else {
		syslog(LOG_INFO, "Handling Server Command: device %u",
				routingTableIPToRDID(routing_table, sg_serial->ip_addr));
	}



	switch (sg_heartbeat->datatype) {
		case SG_HEARTBEAT_ROUTER_TO_SENSOR:
			// Heartbeat from router to sensor
			if(!routingTableIsDeviceLost(routing_table, sg_serial->ip_addr))
				sgSerialSend(sg_serial, sizeof(SansgridSerial));
			break;
		case SG_HEARTBEAT_SENSOR_TO_ROUTER:
			// Heartbeat response from sensor
			if (routingTableHeardDevice(routing_table, sg_serial->ip_addr) > 0) {
				routerRefreshDevice(sg_serial->ip_addr);
			}
			break;
		case SG_SERVSTATUS:
			routerHandleServerStatus(routing_table, sg_serial);
			break;
		default:
			syslog(LOG_WARNING, "Heartbeat: datatype not recognized: %x\n",
					sg_heartbeat->datatype);
			routerFreeDevice(routing_table, sg_serial->ip_addr);
			return 1;
			break;
	}

	return 0;
}


/** 
 * \brief Handle a SansgridChirp payload
 *
 * Forwards a chirp either from the sensor to server,
 * or from server to sensor
 */
int routerHandleChirp(RoutingTable *routing_table, SansgridSerial *sg_serial) {
	// Handle a Chirp data type
	// Send a SansgridChirp from server to sensor
	// Send a SansgridChirp from sensor to server
	
	SansgridChirp *sg_chirp;
	SANSGRID_UNION(SansgridChirp, SansgridChirpConv) sg_chirp_union;

	// Convert serial data to formatted data
	sg_chirp_union.serialdata = sg_serial->payload;
	sg_chirp = sg_chirp_union.formdata;
	
	// Make sure device is allowed to send this packet.
	// Make an exception if it's a disconnection request
	if (sg_chirp->datatype != SG_CHIRP_NETWORK_DISCONNECTS_SENSOR
			&& sg_chirp->datatype == SG_CHIRP_SENSOR_DISCONNECT) {
		if (chkValidCTRLPath(routing_table, sg_serial->ip_addr, 
					sg_chirp->datatype, "Chirp") < 0)
			return -1;
	} 

	syslog(LOG_INFO, "Handling Chirp packet: device %u",
			routingTableIPToRDID(routing_table, sg_serial->ip_addr));
	routingTableSetCurrentPacket(routing_table, 
			sg_serial->ip_addr, SG_DEVSTATUS_CHIRPING);

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
			if (routingTableHeardDevice(routing_table, sg_serial->ip_addr) > 0) {
				routerRefreshDevice(sg_serial->ip_addr);
			}
			sgTCPSend(sg_serial, sizeof(SansgridSerial));
			break;
		case SG_CHIRP_NETWORK_DISCONNECTS_SENSOR:
			// Network is disconnecting sensor
			routerFreeDevice(routing_table, sg_serial->ip_addr);
			break;
		case SG_CHIRP_SENSOR_DISCONNECT:
			// Sensor is disconnecting from network
			routerFreeDevice(routing_table, sg_serial->ip_addr);
			break;
		default:
			syslog(LOG_WARNING, "Chirp: datatype not recognized: %x\n",
					sg_chirp->datatype);
			routerFreeDevice(routing_table, sg_serial->ip_addr);
			return -1;
			break;
	}

	return 0;
}



/**
 * \brief Handle a Server Status payload
 *
 * Used for status updates between the router and server
 */
int routerHandleServerStatus(RoutingTable *routing_table, SansgridSerial *sg_serial) {
	// Handle a server<-->router status command
	SansgridIRStatus *sg_irstatus;
	SANSGRID_UNION(SansgridIRStatus, SGIR_un) sg_ir_union;
	sg_ir_union.serialdata = sg_serial->payload; 
	sg_irstatus = sg_ir_union.formdata;
	syslog(LOG_INFO, "Handling Router<-->Server packet: device %u",
			routingTableIPToRDID(routing_table, sg_serial->ip_addr));
	// FIXME: Get rid of magic payload type number 0xfd
	if (sg_irstatus->datatype == SG_SERVSTATUS) {
		if (!strcmp(sg_irstatus->status, "stale")
				|| !strcmp(sg_irstatus->status, "lost"))
			sgTCPSend(sg_serial, sizeof(SansgridSerial));
		if (!strcmp(sg_irstatus->status, "online"))
			sgTCPSend(sg_serial, sizeof(SansgridSerial));	
	} else {
		return 1;
	}

	return 0;
}



// vim: ft=c ts=4 noet sw=4:
