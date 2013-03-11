#include <stdint.h>
#include <string.h>
#include "handlers.h"
#include "../../payloads.h"
#include "../routing/routing.h"

int routerHandleHatching(RoutingTable *routing_table,
		uint8_t serialdata[sizeof(SansgridHatching)]) {
	// Handle a Hatching data type
	// 1. Set radio IP address
	
	// TODO: Implement me
	return -1;
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



// vim: ft=c ts=4 noet sw=4:
