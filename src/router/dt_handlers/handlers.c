#include <stdint.h>
#include <string.h>
#include "handlers.h"
#include "../../payloads.h"
#include "../routing/routing.h"

int routerHandleHatching(RoutingTable *routing_table,
		uint8_t serialdata[sizeof(SansgridHatching)]) {
	// Handle a Hatching data type
	// 1. Set radio IP address
	SANSGRID_UNION(SansgridHatching, SansgridHatchingConv) sg_hatching_union;
	SansgridHatching *sg_hatching;
	SansgridEyeball *sg_eyeball;

	sg_hatching_union.serialdata = serialdata;
	sg_hatching = sg_hatching_union.formdata;

	// TODO: Add information to eyeball structure
	sg_eyeball = (SansgridEyeball*)malloc(sizeof(SansgridEyeball));
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

// vim: ft=c ts=4 noet sw=4:
