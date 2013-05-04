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




int8_t sgTCPSend(SansgridSerial *sg_serial, uint32_t size) {
	// Send size bytes of serialdata
	if (!size) 
		return -1;
	char cmd[2000];
	char payload[size*5];
	char config_path[150];
	FILE *FPTR = NULL;
	char key[100],
		 url[50];
	char *buffer = NULL;
	int buff_size = 1000;
	int exit_code;
	syslog(LOG_INFO, "Sending packet over TCP");

	// get the configuration path
	getSansgridDir(config_path);
	snprintf(config_path, 150, "%s/sansgrid.conf", config_path);
	if ((FPTR = fopen(config_path, "r")) == NULL) {
		syslog(LOG_DEBUG, "Couldn't find path %s", config_path);
		return -1;
	} else {
		buffer = (char*)malloc(buff_size*sizeof(char));
		if (buffer == NULL) {
			syslog(LOG_ERR, "Couldn't allocate buffer!");
			return -1;
		}
		while (getline(&buffer, &size, FPTR) != -1) {
			if (strstr(buffer, "key")) {
				sscanf(buffer, "key = '%s'", key);
			} else if (strstr(buffer, "url")) {
				sscanf(buffer, "url = '%s'", url);
			}
		}
		free(buffer);
		fclose(FPTR);
	}

	if (sgRouterToServerConvert(sg_serial, payload) == -1) {
		syslog(LOG_DEBUG, "Router-->Server conversion failed");
		return -1;
	} else {
		snprintf(cmd, 2000, "curl -s --data-urlencode --payload=\"%s\"", payload);
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

