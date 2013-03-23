#ifndef __SG_ROUTER_HANDLERS_H__
#define __SG_ROUTER_HANDLERS_H__

#include <stdint.h>
#include "../../payloads.h"
#include "../routing/routing.h"
#include "../../sg_serial.h"




int routerHandleHatching(RoutingTable *routing_table, SansgridSerial *sg_serial);
int routerHandleFly(RoutingTable *routing_table, SansgridSerial *sg_serial);
int routerHandleEyeball(RoutingTable *routing_table, SansgridSerial *sg_serial);
int routerHandlePeck(RoutingTable *routing_table, SansgridSerial *sg_serial);
int routerHandleSing(RoutingTable *routing_table, SansgridSerial *sg_serial);
int routerHandleMock(RoutingTable *routing_table, SansgridSerial *sg_serial);
int routerHandlePeacock(RoutingTable *routing_table, SansgridSerial *sg_serial);
int routerHandleNest(RoutingTable *routing_table, SansgridSerial *sg_serial);
int routerHandleSquawk(RoutingTable *routing_table, SansgridSerial *sg_serial);
int routerHandleHeartbeat(RoutingTable *routing_table, SansgridSerial *sg_serial);
int routerHandleChirp(RoutingTable *routing_table, SansgridSerial *sg_serial);


#endif
