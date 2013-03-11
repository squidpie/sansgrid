#include <stdint.h>
#include "../../payloads.h"
#include "../routing/routing.h"

int router_handle_eyeball(RoutingTable *routing_table, uint8_t serialdata[sizeof(SansgridEyeball)]);
