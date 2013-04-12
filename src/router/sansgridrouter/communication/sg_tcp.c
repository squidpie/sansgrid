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
#include "sg_tcp.h"
#include "../../../sg_serial.h"
#include "../../../payloads.h"
#include "../payload_handlers/payload_handlers.h"

// Delimiters
// Note: These are wide chars. They take up 2 bytes
#define DELIM_VAL "α"
#define DELIM_KEY "β"


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

	*key = (*saved == NULL) ? strtok_r(&str[1],	DELIM_VAL,	saved) : 
				  strtok_r(NULL,	DELIM_VAL,	saved);

	if (*saved == NULL) {
		*value = NULL;
		return 1;
	} else if (!strcmp(*saved, DELIM_KEY)) {
		*value = NULL;
		*saved = *saved+1;
		return 1;
	} else {
		*value = strtok_r(NULL, DELIM_KEY, saved);
	}


	if (*key == NULL && *value == NULL) {
		return 0;
	} else {
		return 1;
	}
}




void atox(uint8_t *hexarray, char *str, uint32_t hexsize) {
	// convert the full string of hex values into an array
	int i;
	char chunk[3];
	int offset; 
	uint32_t length;
	uint32_t uval;

	for (i=0; i<hexsize; i++)
		hexarray[i] = 0x0;
	if (str == NULL)
		return;
	length = strlen(str);
	offset = hexsize - ((length+1)/2);

	for (i=0; i<length;) {
		if (((length ^ i) & 0x1) == 0x1) {
			// make sure we catch case of 0x123
			// where we should parse as 0x1  0x23
			sscanf(&str[i], "%1s", chunk);
			i++;
		} else {
			sscanf(&str[i], "%2s", chunk);
			i += 2;
		}
		sscanf(chunk, "%x", &uval);
		hexarray[i+offset] = (uval & 0xff);
	}
	return;
}



char *match(Dictionary dict[], int size, char *key) {
	// Return the value at key.
	int i;
	for (i=0; i<size; i++) {
		if (!strcmp(key, dict[i].key)) {
			return dict[i].value;
		}
	}
	return NULL;
}



int8_t convertEyeball(Dictionary dict[], int size, SansgridSerial *sg_serial) {
	// Get an eyeball datatype from the payload
	SansgridEyeball sg_eyeball;

	atox(&sg_eyeball.datatype,		match(dict, size, "dt"), 		1*sizeof(uint8_t));
	atox(sg_eyeball.manid, 			match(dict, size, "manid"),		4*sizeof(uint8_t));
	atox(sg_eyeball.modnum, 		match(dict, size, "modnum"),	4*sizeof(uint8_t));
	atox(sg_eyeball.serial_number,	match(dict, size, "sn"), 		8*sizeof(uint8_t));
	atox(&sg_eyeball.profile, 		match(dict, size, "profile"), 	1*sizeof(uint8_t));
	atox(&sg_eyeball.mode, 			match(dict, size, "mode"), 		1*sizeof(uint8_t));

	memset(sg_eyeball.padding, 0x0, 62*sizeof(uint8_t));

	memcpy(sg_serial->payload, &sg_eyeball, sizeof(SansgridEyeball));

	return 0;
}

int8_t convertPeck(Dictionary dict[], int size, SansgridSerial *sg_serial) {
	// Get a peck datatype from the payload
	SansgridPeck sg_peck;

	atox(&sg_peck.datatype, 		match(dict, size, "dt"),		1*sizeof(uint8_t));
	atox(sg_peck.router_ip, 		match(dict, size, "router_ip"),	IP_SIZE);
	atox(sg_peck.assigned_ip,		match(dict, size, "server_ip"),	IP_SIZE);
	atox(sg_peck.server_id,			match(dict, size, "sid"),		16*sizeof(uint8_t));
	atox(&sg_peck.recognition,		match(dict, size,"recognition"),1*sizeof(uint8_t));
	atox(sg_peck.manid,				match(dict, size, "manid"),		4*sizeof(uint8_t));
	atox(sg_peck.modnum,			match(dict, size, "modnum"),	4*sizeof(uint8_t));
	atox(sg_peck.serial_number,		match(dict, size, "sn"),		8*sizeof(uint8_t));

	memset(sg_peck.padding, 0x0, 15*sizeof(uint8_t));

	memcpy(sg_serial->payload, &sg_peck, sizeof(SansgridPeck));
	
	return 0;
}

int8_t convertSing(Dictionary dict[], int size, SansgridSerial *sg_serial) {
	// Get a sing datatype from the payload
	SansgridSing sg_sing;

	atox(&sg_sing.datatype,			match(dict, size, "dt"),		1*sizeof(uint8_t));
	atox(sg_sing.pubkey, 			match(dict, size, "servpubkey"),80*sizeof(uint8_t));

	memcpy(sg_serial->payload, &sg_sing, sizeof(SansgridSing));
	
	return 0;
}

int8_t convertMock(Dictionary dict[], int size, SansgridSerial *sg_serial) {
	// Get a mock datatype from the payload
	SansgridMock sg_mock;

	atox(&sg_mock.datatype, 		match(dict, size, "dt"),		1*sizeof(uint8_t));
	atox(sg_mock.pubkey,			match(dict, size, "senspubkey"),80*sizeof(uint8_t));

	memcpy(sg_serial->payload, &sg_mock, sizeof(SansgridMock));
	
	return 0;
}

int8_t convertPeacock(Dictionary dict[], int size, SansgridSerial *sg_serial) {
	// Get a peacock datatype from the payload
	SansgridPeacock sg_peacock;

	atox(&sg_peacock.datatype,		match(dict, size, "dt"),		1*sizeof(uint8_t));

	atox(&sg_peacock.IO_A_id,		match(dict, size, "?"),			1*sizeof(uint8_t));
	atox(&sg_peacock.IO_A_class,	match(dict, size, "?"),			1*sizeof(uint8_t));
	atox(&sg_peacock.IO_A_direc,	match(dict, size, "?"),			1*sizeof(uint8_t));
	memcpy(sg_peacock.IO_A_label, 	match(dict, size, "?"), 		30*sizeof(char));
	atox(sg_peacock.IO_A_units,		match(dict, size, "?"),			6*sizeof(uint8_t));
	
	atox(&sg_peacock.IO_B_id,		match(dict, size, "?"),			1*sizeof(uint8_t));
	atox(&sg_peacock.IO_B_class,	match(dict, size, "?"),			1*sizeof(uint8_t));
	atox(&sg_peacock.IO_B_direc,	match(dict, size, "?"),			1*sizeof(uint8_t));
	memcpy(sg_peacock.IO_B_label, 	match(dict, size, "?"),			30*sizeof(char));
	atox(sg_peacock.IO_B_units,		match(dict, size, "?"),			6*sizeof(uint8_t));

	sg_peacock.padding = 0x0;

	memcpy(sg_serial->payload, &sg_peacock, sizeof(SansgridPeacock));

	return 0;
}

int8_t convertNest(Dictionary dict[], int size, SansgridSerial *sg_serial) {
	// Get a nest datatype from the payload
	SansgridNest sg_nest;

	atox(&sg_nest.datatype, 		match(dict, size, "dt"),		1*sizeof(uint8_t));
	
	memset(sg_nest.padding, 0x0, 80*sizeof(uint8_t));

	memcpy(sg_serial->payload, &sg_nest, sizeof(SansgridNest));
	
	return 0;
}

int8_t convertSquawk(Dictionary dict[], int size, SansgridSerial *sg_serial) {
	// Get a squawk datatype from the payload
	SansgridSquawk sg_squawk;
	
	atox(&sg_squawk.datatype, 		match(dict, size, "dt"),		1*sizeof(uint8_t));
	atox(sg_squawk.data,			match(dict, size, "data"),		80*sizeof(uint8_t));

	memcpy(sg_serial->payload, &sg_squawk, sizeof(SansgridSquawk));

	return 0;
}

int8_t convertChirp(Dictionary dict[], int size, SansgridSerial *sg_serial) {
	// Get a squawk datatype from the payload
	SansgridChirp sg_chirp;

	atox(&sg_chirp.datatype,		match(dict, size, "dt"),		1*sizeof(uint8_t));
	atox(&sg_chirp.datasize,		match(dict, size, "?"),			1*sizeof(uint8_t));
	atox(sg_chirp.data,				match(dict, size, "data"),		79*sizeof(uint8_t));
	
	memcpy(sg_serial->payload, &sg_chirp, sizeof(SansgridChirp));

	return 0;
}



int8_t sgTCPHandle(char *payload, SansgridSerial *sg_serial) {
	int i;
	uint8_t datatype = ~0x0;
	Dictionary dict[30];
	int32_t size = 0;
	int8_t exit_code = 0;
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
	dict[size].key = NULL;
	dict[size].value = NULL;

	// Find payload type
	for (i=0; i<size; i++) {
		key = dict[i].key;
		if (!strcmp(key, "dt")) {
			// found the datatype
			datatype = atoi(key);
		}
	}
	if (datatype == ~0x0) {
		return -1;
	}
	datatype = sgPayloadGetType(datatype);
	switch(datatype) {
		case SG_DEVSTATUS_EYEBALLING:
			exit_code = convertEyeball(dict, size, sg_serial);
			break;
		case SG_DEVSTATUS_PECKING:
			exit_code = convertPeck(dict, size, sg_serial);
			break;
		case SG_DEVSTATUS_SINGING:
			exit_code = convertSing(dict, size, sg_serial);
			break;
		case SG_DEVSTATUS_MOCKING:
			exit_code = convertMock(dict, size, sg_serial);
			break;
		case SG_DEVSTATUS_PEACOCKING:
			exit_code = convertPeacock(dict, size, sg_serial);
			break;
		case SG_DEVSTATUS_NESTING:
			exit_code = convertNest(dict, size, sg_serial);
			break;
		case SG_DEVSTATUS_SQUAWKING:
			exit_code = convertSquawk(dict, size, sg_serial);
			break;
		case SG_DEVSTATUS_CHIRPING:
			exit_code = convertChirp(dict, size, sg_serial);
			break;
		default:
			exit_code = -1;
			break;
	}

	return exit_code;
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

