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

static int global_strictness = 1;

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

int deviceAuthDisable(void) {
	// don't authenticate devices
	int old_global_strictness = global_strictness;
	global_strictness = 0;
	return old_global_strictness;
}

int deviceAuthEnable(void) {
	// authenticate devices
	int old_global_strictness = global_strictness;
	global_strictness = 1;
	return old_global_strictness;
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



int deviceAuthIsSGPayloadTypeValid(DeviceAuth *dev_auth, uint8_t dt) {
	// check a payload type against the next expected packet
	// return 1 if it is valid
	// return 0 if it is invalid
	// return -1 on error
	
	if (global_strictness == 0)
		return 1;
	if (devauthAssertValid(dev_auth) == -1) {
		syslog(LOG_INFO, "NULL in deviceAuthIsPayloadTypeValid");
		return -1;
	}
	if (dev_auth->strictness == 0) {
		// always valid
		return 1;
	} else if (dev_auth->next_expected_packet == sgPayloadGetType(dt)) {
		return 1;
	} else {
		return 0;
	}
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



// vim: ft=c ts=4 noet sw=4:
