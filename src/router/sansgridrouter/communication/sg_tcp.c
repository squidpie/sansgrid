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
#include "../sansgrid_router.h"


#define USE_SANSRTS 1


int8_t sgTCPSend(SansgridSerial *sg_serial, uint32_t size) {
	// Send size bytes of serialdata
	if (!size) 
		return -1;
	char cmd[2000];
	char payload[size*5];
	char config_path[300];
	FILE *FPTR = NULL;
#ifndef USE_SANSRTS
	char key[100],
		 url[50];
	char sansgrid_path[300];
#endif
	char *buffer = NULL;
	int buff_size = size;
	int exit_code;
	syslog(LOG_INFO, "Sending packet over TCP");

	// get the configuration path
#ifdef USE_SANSRTS
	snprintf(config_path, 300, "sansrts.pl");
#endif

	if (sgRouterToServerConvert(sg_serial, payload) == -1) {
		syslog(LOG_WARNING, "Router-->Server conversion failed");
		return -1;
	} else {
		syslog(LOG_DEBUG, "Sending packet %s to server", payload);
#ifdef USE_SANSRTS
		snprintf(cmd, 2000, "%s \"%s\"", config_path, payload);
#else
		snprintf(cmd, 2000, "curl -s %s/API.php --data-urlencode --key=%s --data-urlencode --payload=\"%s\"", 
				router_opts.url, router_opts.key, payload);
#endif
		if ((FPTR = popen(cmd, "r")) == NULL) {
			syslog(LOG_WARNING, "Router-->Server send failed");
			return -1;
		}
		buff_size = size;
		buffer = (char*)malloc(buff_size*sizeof(char));
		while (getline(&buffer, &size, FPTR) != -1) {
		}
		free(buffer);
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

