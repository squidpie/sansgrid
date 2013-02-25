#define _POSIX_C_SOURCE 200809L		// Required for nanosleep()

#include <stdint.h>
#include <time.h>

#include "routing.h"


struct DeviceIP {
	uint32_t ip[4];		// IP address
	uint32_t clkdiff;	// ping offset
};

struct DeviceIP *routing_table[ROUTING_ARRAYSIZE];
//uint32_t *routing_table[ROUTING_ARRAYSIZE];

