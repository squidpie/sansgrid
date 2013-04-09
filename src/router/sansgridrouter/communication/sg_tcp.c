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

#define _POSIX_C_SOURCE 1		// required for strtok_r

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "sg_tcp.h"
#include "../../../sg_serial.h"
#include "../../../payloads.h"


typedef struct Dictionary {
	char *key;
	char *value;
} Dictionary;


int handle_payload(char *str, char **key, char **value, char **saved) {
	// Get key and value from string
	// Notes:
	// 	Function returns 0 only if both the key and value are null.
	// 		This is to catch empty values but still return the key
	// 	No checks are done for empty keys
	// 	saved should start initialized as a ptr to NULL

	*key = (*saved == NULL) ? strtok_r(&str[1],	"!",	saved) : 
				  strtok_r(NULL,	"!",	saved);

	if (*saved == NULL) {
		*value = NULL;
		return 1;
	} else if (*saved[0] == '|') {
		*value = NULL;
		*saved = *saved+1;
		return 1;
	} else {
		*value = strtok_r(NULL, "|", saved);
	}


	if (*key == NULL && *value == NULL) {
		return 0;
	} else {
		return 1;
	}
}



int8_t sgTCPGetEyeball(const char *payload, SansgridEyeball *eyeball) {
	// Get an eyeball datatype from the payload

	return -1;
}



int8_t sgTCPHandle(char *payload, SansgridSerial **sg_serial) {
	uint32_t identifier = 0x0;
	uint8_t datatype = ~0x0;
	Dictionary dict[30];
	int32_t size = 0;
	char *saved 	= NULL,
		 *key 		= NULL,
		 *value 	= NULL;
	do {
		if (handle_payload(payload, &key, &value, &saved) == 1) {
			dict[size].key = key;
			dict[size].value = value;
			size++;
		} else
			break;
	} while (1);

	// Find payload type
	if (!strcmp(key, "rdid")) {
		// found the identifier
		identifier = atoi(key);
	} else if (!strcmp(key, "dt")) {
		// found the datatype
		datatype = atoi(key);
	}

	return 0;
}



int8_t sgTCPSend(SansgridSerial *sg_serial, uint32_t size) {
	// Send size bytes of serialdata

	return -1;
}

int8_t sgTCPReceive(SansgridSerial **sg_serial, uint32_t *size) {
	// Receive serialdata, size of packet stored in size
	return -1;
}



// vim: ft=c ts=4 noet sw=4:

