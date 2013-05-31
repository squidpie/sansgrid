/* Definitions for heartbeats
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


#include "heartbeat.h"

#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <syslog.h>
/// \file


/**
 * \brief Data structure for managing device heartbeats
 *
 * Keeps track of when the device was last heard,
 * when to consider the device stale, and when to drop
 * the device
 */
struct HeartbeatStatus {
	/**
	 * \brief Current status of device
	 *
	 * starts at ping_thres, 
	 * drops every time we ping
	 * goes back up to ping_thres
	 * when we hear from device
	 */
	int32_t device_health;	
	/// when a device goes stale
	int32_t stale_thres;
	/**
	 * how many lost pings we can have 
	 * before the device is considered lost
	 */
	int32_t ping_thres;
};


/// Default Stale Threshold
static int32_t stale_thres_default = STALE_THRESHOLD;
/// Default Refresh Threshold
static int32_t ping_thres_default = PING_THRESHOLD;

/// Make sure HeartbeatStatus Structure is non-null
static int hbAssertValid(HeartbeatStatus *hb) {
	// Make sure Heartbeat Status Structure is non-null
	if (hb == NULL) {
		syslog(LOG_WARNING, "Heartbeat Status struct is NULL!");
		return -1;
	}
	return 0;
}



/**
 * \brief Return whether or not the device is stale
 *
 * also signal device stale if device is lost
 */
int32_t hbIsDeviceStale(HeartbeatStatus *hb) {
	// Return whether or not device is stale
	if (hbAssertValid(hb) == -1) {
		syslog(LOG_INFO, "NULL in hbIsDeviceStale");
		return -1;
	}

	if (hb->device_health > (hb->ping_thres - hb->stale_thres))
		return 0;
	return 1;
}



/**
 * \brief Return whether or not the device is lost
 */
int32_t hbIsDeviceLost(HeartbeatStatus *hb) {
	// Return whether or not the device has been lost
	if (hbAssertValid(hb) == -1) {
		syslog(LOG_INFO, "NULL in hbIsDeviceLost");
		return -1;
	}

	return hb->device_health <= 0;
}


/**
 * \brief Initialize Heartbeat Data Structure
 *
 * Initializes to start at the ping_thres,
 * \param	ping_thres		How many pings can pass before device is lost
 * \param	stale_thres		At what device_health level the device is stale
 * \returns
 * A pointer to a block of memory containing the heartbeat data structure
 */
HeartbeatStatus *hbInit(int32_t ping_thres, int32_t stale_thres) {
	// init with a nonstandard ping threshold for the device
	HeartbeatStatus *hb;

	if ((hb = malloc(sizeof(HeartbeatStatus))) == NULL) {
		syslog(LOG_WARNING, "Couldn't allocate HeartbeatStatus");
		return NULL;
	}
	hb->ping_thres = ping_thres;
	hb->stale_thres = stale_thres;
	hb->device_health = hb->ping_thres;

	return hb;
}



/**
 * \brief Initialize Heartbeat Data Structure to Default values
 *
 * Use defined values for ping threshold and stale threshold
 * \returns
 * A pointer to a block of memory containing the heartbeat data structure
 */
HeartbeatStatus *hbInitDefault(void) {
	// Init with standard ping threshold for the device
	return hbInit(ping_thres_default, stale_thres_default);
}



/**
 * \brief Free allocated resources contained in Heartbeat Data structure
 */
void hbDestroy(HeartbeatStatus *hb) {
	// Free allocated resources contained in heartbeat data structure
	free(hb);
	return;
}



/**
 * \brief Decrement the device health and return if status changed
 *
 * Note that this doesn't send a status update
 * \returns
 * return -1 on error
 * return 1 if device just went stale
 * return 3 if device was just lost
 * otherwise return 0 (state didn't change)
 */
int32_t hbDecrement(HeartbeatStatus *hb) {
	// Decrement device health, return if status changed
	int32_t device_before = 0;
	int32_t device_after = 0;

	if (hbAssertValid(hb) == -1) {
		syslog(LOG_INFO, "NULL in hbDecrement");
		return -1;
	}
	device_before =  hbIsDeviceStale(hb);
	device_before |= (hbIsDeviceLost(hb) << 1);
	if (hb->device_health > 0)
		hb->device_health--;
	device_after =  hbIsDeviceStale(hb);
	device_after |= (hbIsDeviceLost(hb) << 1);

	return (device_before ^ device_after);
}



/**
 * \brief Device was just heard from
 *
 * Note that this doesn't send a status update
 * \returns
 * return -1 on error
 * return 1 if device was stale
 * return 2 if device was lost
 * otherwise return 0 (state didn't change)
 */
int32_t hbRefresh(HeartbeatStatus *hb) {
	// Device was just heard from
	int32_t device_before = 0;
	int32_t device_after = 0;

	if (hbAssertValid(hb) == -1) {
		syslog(LOG_INFO, "NULL in hbRefresh");
		return -1;
	}
	device_before = hbIsDeviceStale(hb);
	device_before |= (hbIsDeviceLost(hb) << 1);
	hb->device_health = hb->ping_thres;
	device_after = hbIsDeviceStale(hb);
	device_after |= (hbIsDeviceLost(hb) << 1);

	return (device_before ^ device_after);
}



// vim: ft=c ts=4 noet sw=4:
