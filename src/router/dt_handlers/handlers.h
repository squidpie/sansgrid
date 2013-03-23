#include <stdint.h>
#include "../../payloads.h"
#include "../routing/routing.h"

int routerHandleHatching(RoutingTable *routing_table,
		uint8_t serial_data[sizeof(SansgridHatching)]);
int routerHandleFly(RoutingTable *routing_table,
		uint8_t serial_data[sizeof(SansgridFly)]);
int routerHandleEyeball(RoutingTable *routing_table, 
		uint8_t serial_data[sizeof(SansgridEyeball)]);
int routerHandlePeck(RoutingTable *routing_table,
		uint8_t serial_data[sizeof(SansgridPeck)]);
int routerHandleSing(RoutingTable *routing_table,
		uint8_t serial_data[sizeof(SansgridSing)]);
int routerHandleMock(RoutingTable *routing_table,
		uint8_t serial_data[sizeof(SansgridMock)]);
int routerHandlePeacock(RoutingTable *routing_table,
		uint8_t serial_data[sizeof(SansgridPeacock)]);
int routerHandleNest(RoutingTable *routing_table,
		uint8_t serial_data[sizeof(SansgridNest)]);
int routerHandleSquawk(RoutingTable *routing_table,
		uint8_t serial_data[sizeof(SansgridSquawk)]);
int routerHandleHeartbeat(RoutingTable *routing_table,
		uint8_t serial_data[sizeof(SansgridHeartbeat)]);
int routerHandleChirp(RoutingTable *routing_table,
		uint8_t serial_data[sizeof(SansgridChirp)]);

