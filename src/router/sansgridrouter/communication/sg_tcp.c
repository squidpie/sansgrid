/* Definitions for server communication functions
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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <syslog.h>
#include "sg_tcp.h"
#include "../payload_handlers/payload_handlers.h"




int8_t sgTCPSend(SansgridSerial *sg_serial, uint32_t size) {
	// Send size bytes of serialdata
	if (!size) 
		return -1;
	char cmd[1000];
	char payload[size*5];
	FILE *FPTR = NULL;
	int exit_code;
	syslog(LOG_INFO, "Sending packet over TCP");

	if (sgRouterToServerConvert(sg_serial, payload) == -1) {
		syslog(LOG_DEBUG, "Router-->Server conversion failed");
		return -1;
	} else {
		snprintf(cmd, 1000, "sansrts.pl \"%s\"", payload);
		if ((FPTR = popen(cmd, "r")) == NULL) {
			syslog(LOG_DEBUG, "Router-->Server send failed");
			return -1;
		}
		exit_code = pclose(FPTR);
		if (exit_code > 0) {
			syslog(LOG_INFO, "send command exited successfully");
		} else {
			syslog(LOG_INFO, "send command exited with exit code %i", exit_code);
		}
	}

	return 0;
}

int8_t sgTCPReceive(SansgridSerial **sg_serial, uint32_t *size) {
	// Receive serialdata, size of packet stored in size
	sem_t blocker;

	sem_init(&blocker, 0, 0);
	sem_wait(&blocker);

	sem_destroy(&blocker);

	return -1;
}



// vim: ft=c ts=4 noet sw=4:

