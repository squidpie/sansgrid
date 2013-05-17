/* Device Authentication status
 *
 * Copyright (C) 2013 SansGrid
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 *
 */


#include "routing_table.h"
#include "auth_status.h"
#include "../sansgrid_router.h"
#include "../payload_handlers/payload_handlers.h"
#include <payloads.h>
#include <stdlib.h>
#include <syslog.h>

static int global_strictness = DEV_AUTH_LOOSE;

struct DeviceAuth {
	enum SansgridDeviceStatusEnum auth_place,
							  next_expected_packet;	  
	int strictness;
};

static int devauthAssertValid(DeviceAuth *dev_auth) {
	if (dev_auth == NULL) {
		syslog(LOG_WARNING, "DeviceAuth is NULL when it shouldn't be");
		return -1;
	} else {
		return 0;
	}
}

int deviceAuthDisable(DeviceAuth *dev_auth) {
	// don't authenticate devices
	if (devauthAssertValid(dev_auth) == -1) {
		syslog(LOG_INFO, "NULL in deviceAuthDisable");
		return -1;
	}
	int old_strictness = dev_auth->strictness;
	dev_auth->strictness = DEV_AUTH_NONE;
	return old_strictness;
}

int deviceAuthEnableLoosely(DeviceAuth *dev_auth) {
	// make sure devices at least eyeball first
	if (devauthAssertValid(dev_auth) == -1) {
		syslog(LOG_INFO, "NULL in deviceAuthEnable");
		return -1;
	}
	int old_strictness = dev_auth->strictness;
	dev_auth->strictness = DEV_AUTH_LOOSE;
	return old_strictness;
}


int deviceAuthEnableFiltered(DeviceAuth *dev_auth) {
	// If we get an unexpected packet, drop it
	// don't drop the offending device though
	if (devauthAssertValid(dev_auth) == -1) {
		syslog(LOG_INFO, "NULL in deviceAuthEnable");
		return -1;
	}
	int old_strictness = dev_auth->strictness;
	dev_auth->strictness = DEV_AUTH_FILTERED;
	return old_strictness;
}
	
int deviceAuthEnable(DeviceAuth *dev_auth) {
	// authenticate devices
	if (devauthAssertValid(dev_auth) == -1) {
		syslog(LOG_INFO, "NULL in deviceAuthEnable");
		return -1;
	}
	int old_strictness = dev_auth->strictness;
	dev_auth->strictness = DEV_AUTH_STRICT;
	return old_strictness;
}

int deviceAuthIsEnabled(DeviceAuth *dev_auth) {
	// check to see if strict authentication is enabled
	if (devauthAssertValid(dev_auth) == -1) {
		syslog(LOG_INFO, "NULL in deviceAuthIsEnabled");
		return -1;
	}
	return dev_auth->strictness;
}

DeviceAuth *deviceAuthInit(int strictness) {
	// Initialize authentication tracking with strictness
	DeviceAuth *dev_auth;

	if ((dev_auth = (DeviceAuth*)malloc(sizeof(DeviceAuth))) == NULL) {
		syslog(LOG_WARNING, "malloc failed in deviceAuthInit");
		return NULL;
	}
	dev_auth->strictness = strictness;
	dev_auth->auth_place = SG_DEVSTATUS_NULL;
	dev_auth->next_expected_packet = SG_DEVSTATUS_NULL;

	return dev_auth;
}



void deviceAuthDestroy(DeviceAuth *dev_auth) {
	// Destroy authentication tracking object
	if (devauthAssertValid(dev_auth) == -1) {
		syslog(LOG_INFO, "NULL in deviceAuthDestroy");
		return;
	}
	free(dev_auth);
	return;
}


int deviceAuthIsGeneralPayloadTypeValid(DeviceAuth *dev_auth, uint8_t gdt) {
	if (devauthAssertValid(dev_auth) == -1) {
		syslog(LOG_INFO, "NULL in deviceAuthIsPayloadTypeValid");
		return -1;
	}
	if (dev_auth->strictness == DEV_AUTH_NONE) {
		// always valid
		return 1;
	} else if (dev_auth->strictness == DEV_AUTH_LOOSE) {
		// only valid if device has eyeballed
		if (dev_auth->auth_place != SG_DEVSTATUS_NULL) {
			return 1;
		} else {
			return 0;
		}
	} else if ((dev_auth->next_expected_packet == SG_DEVSTATUS_LEASED) &&
			(gdt == SG_DEVSTATUS_CHIRPING || gdt == SG_DEVSTATUS_HEARTBEAT)) {
		return 1;
	} else if (dev_auth->next_expected_packet == gdt) {
		return 1;
	} else {
		return 0;
	}
}


int deviceAuthIsSGPayloadTypeValid(DeviceAuth *dev_auth, uint8_t dt) {
	// check a payload type against the next expected packet
	// return 1 if it is valid
	// return 0 if it is invalid
	// return -1 on error
	
	uint8_t gdt;
	if (global_strictness == 0)
		return 1;
	if (devauthAssertValid(dev_auth) == -1) {
		syslog(LOG_INFO, "NULL in deviceAuthIsPayloadTypeValid");
		return -1;
	}
	gdt = sgPayloadGetType(dt);
	return deviceAuthIsGeneralPayloadTypeValid(dev_auth, gdt);
}

int deviceAuthSetNextGeneralPayload(DeviceAuth *dev_auth, uint8_t gdt) {
	// Set the next expected payload 
	if (devauthAssertValid(dev_auth) == -1) {
		syslog(LOG_INFO, "NULL in deviceAuthSetNextGeneralPayload");
		return -1;
	}
	dev_auth->next_expected_packet = gdt;
	return 0;
}

uint8_t deviceAuthGetNextGeneralPayload(DeviceAuth *dev_auth) {
	// Get the next expected payload
	if (devauthAssertValid(dev_auth) == -1) {
		syslog(LOG_INFO, "NULL in deviceAuthSetNextGeneralPayload");
		return -1;
	}
	return dev_auth->next_expected_packet;
}


uint8_t deviceAuthGetCurrentGeneralPayload(DeviceAuth *dev_auth) {
	// Get the current general payload
	if (devauthAssertValid(dev_auth) == -1) {
		syslog(LOG_INFO, "NULL in deviceAuthGetCurrentGeneralPayload");
		return -1;
	}
	return dev_auth->auth_place;
}


int32_t deviceAuthSetCurrentGeneralPayload(DeviceAuth *dev_auth, uint8_t gdt) {
	// Set the current general payload
	if (devauthAssertValid(dev_auth) == -1) {
		syslog(LOG_INFO, "NULL in deviceAuthGetCurrentGeneralPayload");
		return -1;
	}
	dev_auth->auth_place = gdt;

	return 0;
}
	


// vim: ft=c ts=4 noet sw=4:
