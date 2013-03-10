#include <stdint.h>
#include <string.h>
#include "../../payloads.h"

int router_handle_eyeball(uint8_t serialdata[sizeof(SansgridEyeball)])
{
	// Handle an Eyeball data type
	SansgridEyeball *sg_eyeball;
	SANSGRID_UNION(SansgridEyeball, SansgridEyeballConv) sg_eb_un;

	// Convert serial data to formatted data
	sg_eb_un.serialdata = serialdata;
	sg_eyeball = sg_eb_un.formdata;

	return 0;
}


// vim: ft=c ts=4 noet sw=4:
