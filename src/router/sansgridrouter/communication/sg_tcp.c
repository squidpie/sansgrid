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

#define _POSIX_C_SOURCE 200809L		///< Required for nanosleep()

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
/** \file */



/**
 * \brief Send a Sansgrid Payload to the Server
 *
 * Translate a SansgridPayload into a null-terminated key-value string
 * and send it to the server.
 * \param[in]	sg_serial	data to send
 * \param[in]	size		size of structure. Not currently used.
 */
int8_t sgTCPSend(SansgridSerial *sg_serial, uint32_t size) {
	// Send size bytes of serialdata
	if (!size) 
		return -1;
	char cmd[2000];
	char payload[size*5];
	FILE *FPTR = NULL;
	int buff_size = size;
	int exit_code;
	char *buffer;
	syslog(LOG_INFO, "Sending packet over TCP");

	// get the configuration path

	if (sgRouterToServerConvert(sg_serial, payload) == -1) {
		syslog(LOG_WARNING, "Router-->Server conversion failed");
		return -1;
	} else {
		syslog(LOG_DEBUG, "Sending packet %s to server", payload);
		snprintf(cmd, 2000, "curl -m 2 -s %s/API.php --data-urlencode key='%s' --data-urlencode payload='%s'", 
				router_opts.serverip, router_opts.serverkey, payload);
		printf("%s\n", cmd);
		if ((FPTR = popen(cmd, "r")) == NULL) {
			syslog(LOG_WARNING, "Router-->Server send failed");
			return -1;
		}
		// Eat stdout
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



// vim: ft=c ts=4 noet sw=4:

