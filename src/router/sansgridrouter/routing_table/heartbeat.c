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


#define _POSIX_C_SOURCE 200809L		// Required for nanosleep()
									// used in a deprecated function
#include "heartbeat.h"

#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <syslog.h>

struct HeartbeatStatus {
	int32_t device_health;	// starts at ping_thres, 
							// drops every time we ping
							// goes back up to ping_thres
							// when we hear from device
	int32_t stale_thres;	// when a device goes stale

	int32_t ping_thres;		// how many lost pings we can get
							// before the device is lost
};


static int32_t stale_thres_default = STALE_THRESHOLD;
static int32_t ping_thres_default = PING_THRESHOLD;

static int hbAssertValid(HeartbeatStatus *hb) {
	if (hb == NULL) {
		syslog(LOG_WARNING, "Heartbeat Status struct is NULL!");
		return -1;
	}
	return 0;
}

int32_t hbIsDeviceStale(HeartbeatStatus *hb) {
	// return whether or not the device is stale
	// also signal device stale if device is lost
	if (hbAssertValid(hb) == -1) {
		syslog(LOG_INFO, "NULL in hbIsDeviceStale");
		return -1;
	}

	if (hb->device_health > (hb->ping_thres - hb->stale_thres))
		return 0;
	return 1;
}


int32_t hbIsDeviceLost(HeartbeatStatus *hb) {
	// return whether or not the device is lost
	if (hbAssertValid(hb) == -1) {
		syslog(LOG_INFO, "NULL in hbIsDeviceLost");
		return -1;
	}

	return hb->device_health > 0;
}

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

HeartbeatStatus *hbInitDefault(void) {
	// Init with standard ping threshold for the device
	return hbInit(ping_thres_default, stale_thres_default);
}


void hbDestroy(HeartbeatStatus *hb) {
	free(hb);
	return;
}



int32_t hbDecrement(HeartbeatStatus *hb) {
	// Decrement the device health
	// return -1 on error
	// return 1 if device just went stale
	// return 2 if device was just lost
	// otherwise return 0 (state didn't change)
	int32_t device_before = 0;
	int32_t device_after = 0;

	if (hbAssertValid(hb) == -1) {
		syslog(LOG_INFO, "NULL in hbDecrement");
		return -1;
	}
	device_before = hbIsDeviceStale(hb);
	device_before = (hbIsDeviceLost(hb) << 1);
	if (hb->device_health > 0)
		hb->device_health--;
	device_after = hbIsDeviceStale(hb);
	device_after = (hbIsDeviceLost(hb) << 1);

	return (device_before ^ device_after);
}


int32_t hbRefresh(HeartbeatStatus *hb) {
	// Device was just heard from
	// return -1 on error
	// return 1 if device was stale
	// return 2 if device was lost
	// otherwise return 0 (state didn't change)
	int32_t device_before = 0;
	int32_t device_after = 0;

	if (hbAssertValid(hb) == -1) {
		syslog(LOG_INFO, "NULL in hbRefresh");
		return -1;
	}
	device_before = hbIsDeviceStale(hb);
	device_before = (hbIsDeviceLost(hb) << 1);
	hb->device_health = hb->ping_thres;
	device_after = hbIsDeviceStale(hb);
	device_after = (hbIsDeviceLost(hb) << 1);

	return (device_before ^ device_after);
}



// vim: ft=c ts=4 noet sw=4:
