#include <stdint.h>
#include "../../payloads.h"
#include "../routing/routing.h"

int routerHandleEyeball(RoutingTable *routing_table, uint8_t serialdata[sizeof(SansgridEyeball)]);
