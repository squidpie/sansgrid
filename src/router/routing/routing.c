#include <stdint.h>
#include <time.h>

#include "routing.h"


struct RoutingTable {
	uint32_t ip[4];
	struct timespec clkdiff;
};

uint32_t *routing_table[ROUTING_ARRAYSIZE];

