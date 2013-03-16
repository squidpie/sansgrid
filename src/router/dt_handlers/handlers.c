#include <stdint.h>
#include <string.h>
#include "handlers.h"
#include "../../payloads.h"
#include "../routing/routing.h"

int routerHandleHatching(RoutingTable *routing_table,
		uint8_t serialdata[sizeof(SansgridHatching)]) {
	// Handle a Hatching data type
	// 1. Set radio IP address
	int i;
	SANSGRID_UNION(SansgridHatching, SansgridHatchingConv) sg_hatching_union;
	SansgridHatching *sg_hatching;
	SansgridEyeball *sg_eyeball;

	sg_hatching_union.serialdata = serialdata;
	sg_hatching = sg_hatching_union.formdata;

	sg_eyeball = (SansgridEyeball*)malloc(sizeof(SansgridEyeball));
	sg_eyeball->datatype = SG_EYEBALL;
	for (i=0; i<4; i++) {
		sg_eyeball->manid[i] = 0x0;
		sg_eyeball->modnum[i] = 0x0;
	}

	for (i=0; i<8; i++) {
		sg_eyeball->serial_number[i] = 0x0;
	}
	sg_eyeball->profile = 0x0;
	sg_eyeball->mode = SG_EYEBALL_MATE;
	
	routingTableAssignIPStatic(routing_table, sg_hatching->ip, sg_eyeball);

	return -1;
}


int routerHandleFly(RoutingTable *routing_table,
		uint8_t serialdata[sizeof(SansgridFly)]) {
	// Handle a Fly data type
	
	// TODO: multicast network existence
}


int routerHandleEyeball(RoutingTable *routing_table, 
		uint8_t serialdata[sizeof(SansgridEyeball)]) {
	// Handle an Eyeball data type
	// 1. Assign IP Address
	// 2. Transmit properties to server
	SansgridEyeball *sg_eyeball;
	SANSGRID_UNION(SansgridEyeball, SansgridEyeballConv) sg_eyeball_union;
	uint8_t ip_addr[IP_SIZE];

	// Convert serial data to formatted data
	sg_eyeball_union.serialdata = serialdata;
	sg_eyeball = sg_eyeball_union.formdata;

	// Store in the routing table
	routingTableAssignIP(routing_table, ip_addr, sg_eyeball);

	// TODO: Send data to server

	return 0;
}


int routerHandlePeck(RoutingTable *routing_table,
		uint8_t serialdata[sizeof(SansgridPeck)]) {
	// Handle a Peck data type
	SansgridPeck *sg_peck;
	SANSGRID_UNION(SansgridPeck, SansgridPeckConv) sg_peck_union;
	
	// Convert serial data to formatted data
	sg_peck_union.serialdata = serialdata;
	sg_peck = sg_peck_union.formdata;

	switch (sg_peck->recognition) {
		case SG_PECK_RECOGNIZED:
			// Sensor Recognized
			break;
		case SG_PECK_MATE:
			// Sensor Not Recognized;
			// Server will mate
			break;
		case SG_PECK_SERVER_REFUSES_MATE:
			// Sensor Not Recognized;
			// Server refuses mate
			break;
		case SG_PECK_SENSOR_REFUSES_MATE:
			// Sensor Not Recognized;
			// Sensor refuses mate
			break;
		default:
			// error
			break;
	}
	return -1;
}


int routerHandleSing(RoutingTable *routing_table,
		uint8_t serialdata[sizeof(SansgridSing)]) {
	// Handle a Sing data type
	SansgridSing *sg_sing;
	SANSGRID_UNION(SansgridSing, SansgridSingConv) sg_sing_union;

	// Convert serial data to formatted data
	sg_sing_union.serialdata = serialdata;
	sg_sing = sg_sing_union.formdata;

	switch (sg_sing->datatype) {
		case SG_SING_WITH_KEY:
			// Acknowledgement and server's public key
			break;
		case SG_SING_WITHOUT_KEY:
			// Acknowledgement, no public key
			break;
		default:
			// error
			break;
	}
	return -1;
}


int routerHandleMock(RoutingTable *routing_table,
		uint8_t serialdata[sizeof(SansgridMock)]) {
	// Handle a Mock data type
	SansgridMock *sg_mock;
	SANSGRID_UNION(SansgridMock, SansgridMockConv) sg_mock_union;

	// Convert serial data to formatted data
	sg_mock_union.serialdata = serialdata;
	sg_mock = sg_mock_union.formdata;

	switch (sg_mock->datatype) {
		case SG_MOCK_WITH_KEY:
			// Acknowledgement and sensor's public key
			break;
		case SG_MOCK_WITHOUT_KEY:
			// Acknowledgement, no sensor key
			break;
		default:
			break;
	}
	return -1;
}


int routerHandlePeacock(RoutingTable *routing_table,
		uint8_t serialdata[sizeof(SansgridPeacock)]) {
	// Handle a Peacock data type
	SansgridPeacock *sg_peacock;
	SANSGRID_UNION(SansgridPeacock, SansgridPeacockConv) sg_peacock_union;

	// Convert serial data to formatted data
	sg_peacock_union.serialdata = serialdata;
	sg_peacock = sg_peacock_union.formdata;

	return -1;
}


int routerHandleNest(RoutingTable *routing_table,
		uint8_t serialdata[sizeof(SansgridNest)]) {
	// Handle a Nest data type
	SansgridNest *sg_nest;
	SANSGRID_UNION(SansgridNest, SansgridNestConv) sg_nest_union;

	// Convert serial data to formatted data
	sg_nest_union.serialdata = serialdata;
	sg_nest = sg_nest_union.formdata;

	return -1;
}


int routerHandleSquawk(RoutingTable *routing_table,
		uint8_t serialdata[sizeof(SansgridSquawk)]) {
	// Handle a Squawk data type
	SansgridSquawk *sg_squawk;
	SANSGRID_UNION(SansgridSquawk, SansgridSquawkConv) sg_squawk_union;

	// Convert serial data to formatted data
	sg_squawk_union.serialdata = serialdata;
	sg_squawk = sg_squawk_union.formdata;

	switch (sg_squawk->datatype) {
		case SG_SQUAWK_SERVER_CHALLENGE_SENSOR:
			// Server Challenges sensor
			break;
		case SG_SQUAWK_SENSOR_RESPOND_NO_REQUIRE_CHALLENGE:
			// Sensor respond to server challenge,
			// no sensor challenge needed
			break;
		case SG_SQUAWK_SENSOR_RESPOND_REQUIRE_CHALLENGE:
			// Sensor respond to server challenge,
			// sensor challenge coming
			break;
		case SG_SQUAWK_SENSOR_CHALLENGE_SERVER:
			// Sensor challenges server
			break;
		case SG_SQUAWK_SERVER_DENY_SENSOR:
			// Server denies sensor challenge request
			break;
		case SG_SQUAWK_SERVER_RESPOND:
			// Server Responds to challenge
			break;
		case SG_SQUAWK_SENSOR_ACCEPT_RESPONSE:
			// Sensor accepts server's response
			break;
		default:
			break;
	}
	return -1;
}

int routerHandleHeartbeat(RoutingTable *routing_table,
		uint8_t serialdata[sizeof(SansgridHeartbeat)]) {
	// Handle a Heartbeat data type
	return -1;
}

int routerHandleChirp(RoutingTable *routing_table,
		uint8_t serialdata[sizeof(SansgridChirp)]) {
	// Handle a Chirp data type
	
	SansgridChirp *sg_chirp;
	SANSGRID_UNION(SansgridChirp, SansgridChirpConv) sg_chirp_union;

	// Convert serial data to formatted data
	sg_chirp_union.serialdata = serialdata;
	sg_chirp = sg_chirp_union.formdata;

	switch (sg_chirp->datatype) {
		case SG_CHIRP_COMMAND_SERVER_TO_SENSOR:
			// Command sent from server to sensor
			break;
		case SG_CHIRP_DATA_SENSOR_TO_SERVER:
			// Data sent from server to sensor
			break;
		case SG_CHIRP_DATA_STREAM_START:
			// Start of data stream
			break;
		case SG_CHIRP_DATA_STREAM_CONTINUE:
			// Continued stream of data
			break;
		case SG_CHIRP_DATA_STREAM_END:
			// End of data stream
			break;
		case SG_CHIRP_NETWORK_DISCONNECTS_SENSOR:
			// Network is disconnecting sensor
			break;
		case SG_CHIRP_SENSOR_DISCONNECT:
			// Sensor is disconnecting from network
			break;
		default:
			break;
	}
	return -1;
}


// vim: ft=c ts=4 noet sw=4:
