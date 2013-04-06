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
#include "../../../payloads.h"

#include <stdint.h>
#include <stdlib.h>
#include <time.h>

int sleepMicro(uint32_t usecs) {
	// Sleep a specified number of microseconds

	int excode;
	struct timespec required, remaining;

	// Set sleep time in microseconds
	required.tv_sec = 0;
	required.tv_nsec = 1000L*usecs;

	do {
		// sleep the full amount of time
		excode = nanosleep(&required, &remaining);
		required.tv_nsec = remaining.tv_nsec;
	} while (excode);

	return 0;
}




// vim: ft=c ts=4 noet sw=4:
