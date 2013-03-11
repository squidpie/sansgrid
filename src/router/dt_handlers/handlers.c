#include <stdint.h>
#include <string.h>
#include "handlers.h"
#include "../../payloads.h"
#include "../routing/routing.h"

int router_handle_eyeball(RoutingTable *routing_table, 
		uint8_t serialdata[sizeof(SansgridEyeball)]) {
	// Handle an Eyeball data type
	// 1. Assign IP Address
	// 2. Transmit properties to server
	SansgridEyeball *SGEyeball;
	SANSGRID_UNION(SansgridEyeball, SansgridEyeballConv) SGEyeballUnion;
	uint8_t ip_addr[IP_SIZE];

	// Convert serial data to formatted data
	SGEyeballUnion.serialdata = serialdata;
	SGEyeball = SGEyeballUnion.formdata;

	// Store in the routing table
	routingTableAssignIP(routing_table, ip_addr, SGEyeball);

	// TODO: Send data to server

	return 0;
}


// vim: ft=c ts=4 noet sw=4:
