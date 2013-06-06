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
/// \file


#include "routing_table.h"
#include "auth_status.h"
#include "../sansgrid_router.h"
#include "../payload_handlers/payload_handlers.h"
#include <payloads.h>
#include <stdlib.h>
#include <syslog.h>

/// Default strictness
static int global_strictness = DEV_AUTH_LOOSE;

/**
 * \brief Storage for Device Authentication Status
 *
 * This structure stores the authentication status
 * for a device that is tracked in the Routing Table.
 */
struct DeviceAuth {
	/// The last general payload received from the device
	enum SansgridDeviceStatusEnum auth_place;
	/// The next expected general payload to be received from the device
	enum SansgridDeviceStatusEnum next_expected_packet;	  
	/// The authentication strictness of the device
	int strictness;
};


/// Make sure the device authentication is non-null
static int devauthAssertValid(DeviceAuth *dev_auth) {
	// Make sure device authentication structure is non-null
	if (dev_auth == NULL) {
		syslog(LOG_WARNING, "DeviceAuth is NULL when it shouldn't be");
		return -1;
	} else {
		return 0;
	}
}



/**
 * \brief Enable Loose device authentication for device
 *
 * Loose authentication means an eyeball must be received
 * from a device and then no more authentication checks
 * are done.
 */
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



/**
 * \brief Enable Filtered device authentication for device
 *
 * Filtered authentication means that any unexpected
 * packet received is dropped; the device is kept on
 * the network.
 */
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



/**
 * \brief Enable Strict device authentication for device
 *
 * Strict authentication means that any unexpected
 * packet received is dropped. The offending device
 * is then disconnected from the network.
 */
int deviceAuthEnable(DeviceAuth *dev_auth) {
	// Enable strict authentication
	if (devauthAssertValid(dev_auth) == -1) {
		syslog(LOG_INFO, "NULL in deviceAuthEnable");
		return -1;
	}
	int old_strictness = dev_auth->strictness;
	dev_auth->strictness = DEV_AUTH_STRICT;
	return old_strictness;
}



/**
 * \brief Get authentication level for device
 *
 * Returns the strictness level for a device
 */
int deviceAuthIsEnabled(DeviceAuth *dev_auth) {
	// check to see if strict authentication is enabled
	if (devauthAssertValid(dev_auth) == -1) {
		syslog(LOG_INFO, "NULL in deviceAuthIsEnabled");
		return -1;
	}
	return dev_auth->strictness;
}


/**
 * \brief Initialize a data structure for device authentication
 *
 * \param	strictness		Device strictness
 * \returns
 * On success, a pointer to the structure is returned. \n
 * On failure, NULL is returned and a warning is logged
 */
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



/**
 * \brief Destroy a device authentication structure
 */
void deviceAuthDestroy(DeviceAuth *dev_auth) {
	// Destroy authentication tracking object
	if (devauthAssertValid(dev_auth) == -1) {
		syslog(LOG_INFO, "NULL in deviceAuthDestroy");
		return;
	}
	free(dev_auth);
	return;
}


/**
 * \brief Check whether a general payload is valid input from device
 *
 * \param		gdt		A general payload type
 * \returns
 * If the general payload is a valid payload, return 1 \n
 * If the general payload is not a valid payload, return 0 \n
 * On error, return -1
 */
int deviceAuthIsGeneralPayloadTypeValid(DeviceAuth *dev_auth, uint32_t gdt) {
	// Check whether a general payload is valid input from device
	if (devauthAssertValid(dev_auth) == -1) {
		syslog(LOG_INFO, "NULL in deviceAuthIsPayloadTypeValid");
		return -1;
	}
	if (dev_auth->strictness == DEV_AUTH_ERROR) {
		// internal error
		// always invalid
		return -1;
	} else if (dev_auth->strictness == DEV_AUTH_LOOSE) {
		// only valid if device has eyeballed
		if ((dev_auth->auth_place & SG_DEVSTATUS_NULL) == 0) {
			return 1;
		} else {
			return 0;
		}
	} else if ((dev_auth->next_expected_packet == SG_DEVSTATUS_LEASED) &&
			((gdt & SG_DEVSTATUS_CHIRPING) > 0 || (gdt & SG_DEVSTATUS_HEARTBEAT) > 0)) {
		return 1;
	} else if ((dev_auth->next_expected_packet & gdt) > 0) {
		return 1;
	} else {
		return 0;
	}
}



/**
 * \brief Check whether a Sansgrid Payload is valid input from device
 *
 * \param		dt		A Sansgrid Payload type
 * \returns
 * If the Sansgrid Payload is a valid payload, return 1 \n
 * If the Sansgrid Payload is not a valid payload, return 0 \n
 * On error, return -1
 */
int deviceAuthIsSGPayloadTypeValid(DeviceAuth *dev_auth, uint8_t dt) {
	// check a payload type against the next expected packet
	// return 1 if it is valid
	// return 0 if it is invalid
	// return -1 on error
	
	uint32_t gdt;
	if (global_strictness == 0)
		return 1;
	if (devauthAssertValid(dev_auth) == -1) {
		syslog(LOG_INFO, "NULL in deviceAuthIsPayloadTypeValid");
		return -1;
	}
	gdt = sgPayloadGetType(dt);
	return deviceAuthIsGeneralPayloadTypeValid(dev_auth, gdt);
}



/**
 * \brief Set the next expected general payload type
 *
 * \param		gdt		A general payload type
 * \returns
 * 0 on success \n
 * -1 on error
 */
int deviceAuthSetNextGeneralPayload(DeviceAuth *dev_auth, uint32_t gdt) {
	// Set the next expected payload 
	if (devauthAssertValid(dev_auth) == -1) {
		syslog(LOG_INFO, "NULL in deviceAuthSetNextGeneralPayload");
		return -1;
	}
	dev_auth->next_expected_packet = gdt;
	return 0;
}



/**
 * \brief Get the next expected general payload type
 */
enum SansgridDeviceStatusEnum deviceAuthGetNextGeneralPayload(DeviceAuth *dev_auth) {
	// Get the next expected payload
	if (devauthAssertValid(dev_auth) == -1) {
		syslog(LOG_INFO, "NULL in deviceAuthSetNextGeneralPayload");
		return -1;
	}
	return dev_auth->next_expected_packet;
}



/**
 * \brief Get the current general payload for device
 */
enum SansgridDeviceStatusEnum deviceAuthGetCurrentGeneralPayload(DeviceAuth *dev_auth) {
	// Get the current general payload
	if (devauthAssertValid(dev_auth) == -1) {
		syslog(LOG_INFO, "NULL in deviceAuthGetCurrentGeneralPayload");
		return -1;
	}
	return dev_auth->auth_place;
}



/**
 * \brief Set the current general payload for device
 */
int32_t deviceAuthSetCurrentGeneralPayload(DeviceAuth *dev_auth, enum SansgridDeviceStatusEnum gdt) {
	// Set the current general payload
	if (devauthAssertValid(dev_auth) == -1) {
		syslog(LOG_INFO, "NULL in deviceAuthGetCurrentGeneralPayload");
		return -1;
	}
	dev_auth->auth_place = gdt;

	return 0;
}
	


// vim: ft=c ts=4 noet sw=4:
