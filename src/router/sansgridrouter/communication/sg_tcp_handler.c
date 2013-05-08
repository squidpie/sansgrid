/* Implementation for server communication
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

#include <syslog.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "sg_tcp.h"
#include "../payload_handlers/payload_handlers.h"
#include "../sansgrid_router.h"



int extract_keyvalue(char *str, char **key, char **value, char **saved) {
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
	uint32_t i_str, i_hex = 0;
	int increment = 0;
	char chunk[3];
	uint32_t length;
	uint32_t uval;

	for (i_hex=0; i_hex<hexsize; i_hex++)
		hexarray[i_hex] = 0x0;
	i_hex = 0;
	if (str == NULL)
		return;
	length = strlen(str);
	//offset = hexsize - ((length+1)/2);

	for (i_str=0; i_str<length;) {
		if (((length ^ i_str) & 0x1) == 0x1) {
			// make sure we catch case of 0x123
			// where we should parse as 0x1  0x23
			chunk[0] = str[i_str];
			chunk[1] = '\0';
			//sscanf(&str[i], "%1s", chunk);
			increment = 1;
		} else {
			chunk[0] = str[i_str];
			chunk[1] = str[i_str+1];
			chunk[2] = '\0';
			//sscanf(&str[i], "%2s", chunk);
			increment = 2;
		}
		sscanf(chunk, "%x", &uval);
		hexarray[i_hex++] = (uval & 0xff);
		i_str+=increment;
	}
	return;
}



char *match(Dictionary dict[], int size, char *key) {
	// Return the value at key.
	int i;
	if (key == NULL) {
		// Guard against garbage data in key
		return NULL;
	}
	for (i=0; i<size; i++) {
		if (dict[i].key == NULL) {
			// Guard against garbage data in dict
			return NULL;
		} else if (!strcmp(key, dict[i].key)) {
			// matched
			return dict[i].value;
		}
	}
	return NULL;
}


static int32_t translateRdid(Dictionary dict[], int size) {
	uint8_t rdid[4];
	uint32_t rdid_return;
	atox(rdid,					match(dict, size, "rdid"),		4*sizeof(uint8_t));
	memcpy(&rdid_return, rdid, 4*sizeof(uint8_t));
	return rdid_return;
}


static int32_t convertEyeball(Dictionary dict[], int size, SansgridSerial *sg_serial) {
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

	return translateRdid(dict, size);
}

static int8_t convertPeck(Dictionary dict[], int size, SansgridSerial *sg_serial) {
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
	
	return translateRdid(dict, size);
}

static int8_t convertSing(Dictionary dict[], int size, SansgridSerial *sg_serial) {
	// Get a sing datatype from the payload
	SansgridSing sg_sing;

	atox(&sg_sing.datatype,			match(dict, size, "dt"),		1*sizeof(uint8_t));
	atox(sg_sing.pubkey, 			match(dict, size, "servpubkey"),80*sizeof(uint8_t));

	memcpy(sg_serial->payload, &sg_sing, sizeof(SansgridSing));
	
	return translateRdid(dict, size);
}

static int8_t convertMock(Dictionary dict[], int size, SansgridSerial *sg_serial) {
	// Get a mock datatype from the payload
	SansgridMock sg_mock;

	atox(&sg_mock.datatype, 		match(dict, size, "dt"),		1*sizeof(uint8_t));
	atox(sg_mock.pubkey,			match(dict, size, "senspubkey"),80*sizeof(uint8_t));

	memcpy(sg_serial->payload, &sg_mock, sizeof(SansgridMock));
	
	return translateRdid(dict, size);
}

static int8_t convertPeacock(Dictionary dict[], int size, SansgridSerial *sg_serial) {
	// Get a peacock datatype from the payload
	char *label = NULL;
	SansgridPeacock sg_peacock;

	atox(&sg_peacock.datatype,		match(dict, size, "dt"),		1*sizeof(uint8_t));

	atox(&sg_peacock.IO_A_id,		match(dict, size, "sida"),		1*sizeof(uint8_t));
	atox(&sg_peacock.IO_A_class,	match(dict, size, "classa"),	1*sizeof(uint8_t));
	atox(&sg_peacock.IO_A_direc,	match(dict, size, "dira"),		1*sizeof(uint8_t));
	if ((label = match(dict, size, "labela")) == NULL) {
		memset(sg_peacock.IO_A_label, 0x0,							30*sizeof(char));
	} else {
		strncpy(sg_peacock.IO_A_label, label, 30);
	}
	if ((label = match(dict, size, "unitsa")) == NULL) {
		memset(sg_peacock.IO_A_units, 0x0,							6*sizeof(char));
	} else {
		strncpy(sg_peacock.IO_A_units, label, 6);
	}
	

	atox(&sg_peacock.IO_B_id,		match(dict, size, "sidb"),		1*sizeof(uint8_t));
	atox(&sg_peacock.IO_B_class,	match(dict, size, "classb"),	1*sizeof(uint8_t));
	atox(&sg_peacock.IO_B_direc,	match(dict, size, "dirb"),		1*sizeof(uint8_t));
	if ((label = match(dict, size, "labelb")) == NULL) {
		memset(sg_peacock.IO_B_label, 0x0,							30*sizeof(char));
	} else {
		strncpy(sg_peacock.IO_B_label, label, 30);
	}
	if ((label = match(dict, size, "unitsb")) == NULL) {
		memset(sg_peacock.IO_B_units, 0x0,							6*sizeof(char));
	} else {
		strncpy(sg_peacock.IO_B_units, label, 6);
	}

	atox(&sg_peacock.additional_IO_needed, match(dict, size, "additional"), 1*sizeof(uint8_t));
	sg_peacock.padding = 0x0;

	memcpy(sg_serial->payload, &sg_peacock, sizeof(SansgridPeacock));

	return translateRdid(dict, size);
}



static int8_t convertNest(Dictionary dict[], int size, SansgridSerial *sg_serial) {
	// Get a nest datatype from the payload
	SansgridNest sg_nest;

	atox(&sg_nest.datatype, 		match(dict, size, "dt"),		1*sizeof(uint8_t));
	
	memset(sg_nest.padding, 0x0, 80*sizeof(uint8_t));

	memcpy(sg_serial->payload, &sg_nest, sizeof(SansgridNest));
	
	return translateRdid(dict, size);
}

static int8_t convertSquawk(Dictionary dict[], int size, SansgridSerial *sg_serial) {
	// Get a squawk datatype from the payload
	SansgridSquawk sg_squawk;
	
	atox(&sg_squawk.datatype, 		match(dict, size, "dt"),		1*sizeof(uint8_t));
	atox(sg_squawk.data,			match(dict, size, "data"),		80*sizeof(uint8_t));

	memcpy(sg_serial->payload, &sg_squawk, sizeof(SansgridSquawk));

	return translateRdid(dict, size);
}

static int8_t convertChirp(Dictionary dict[], int size, SansgridSerial *sg_serial) {
	// Get a squawk datatype from the payload
	SansgridChirp sg_chirp;

	atox(&sg_chirp.datatype,		match(dict, size, "dt"),		1*sizeof(uint8_t));
	atox(&sg_chirp.datasize,		match(dict, size, "?"),			1*sizeof(uint8_t));
	atox(sg_chirp.data,				match(dict, size, "data"),		79*sizeof(uint8_t));
	
	memcpy(sg_serial->payload, &sg_chirp, sizeof(SansgridChirp));

	return translateRdid(dict, size);
}



int8_t sgServerToRouterConvert(char *payload, SansgridSerial *sg_serial) {
	// Take a payload, 	return the translated serial packet,
	// 					return the device identifier (rdid)
	uint8_t datatype = ~0x0;
	enum SansgridDeviceStatusEnum dev_datatype;
	Dictionary dict[30];
	int32_t size = 0;
	int8_t exit_code = 0;
	uint8_t rdid[4];
	int rdid_size;
	uint32_t rdid_32;
	char *saved 	= NULL,
		 *key 		= NULL,
		 *value 	= NULL;

	memset(sg_serial->ip_addr, 0x0, sizeof(sg_serial->ip_addr));
	syslog(LOG_DEBUG, "processing packet %s", payload);
	do {
		if (extract_keyvalue(payload, &key, &value, &saved) == 1) {
			dict[size].key = &key[sizeof(DELIM_KEY)-2];
			dict[size].value = (value == NULL ? NULL : &value[sizeof(DELIM_VAL)-2]);
			size++;
		} else
			break;
	} while (1);
	dict[size].key = NULL;
	dict[size].value = NULL;

	// Find payload type
	atox(&datatype, match(dict, size, "dt"), 1*sizeof(uint8_t));
	atox(rdid, 		match(dict, size, "rdid"), 4*sizeof(uint8_t));
	if (datatype == ~0x0) {
		return -1;
	}
	rdid_size = (strlen(match(dict, size, "rdid"))+1)/2;
	byteToWord(&rdid_32, rdid, 4*sizeof(uint8_t));
	rdid_32 = rdid_32 >> ((4-rdid_size)*8);
	printf("rdid = %u\n", rdid_32);
	if (!rdid_32) {
		// broadcast
		//routingTableGetBroadcast(routing_table, sg_serial->ip_addr);
	} else {
		routingTableRDIDToIP(routing_table, rdid_32, sg_serial->ip_addr);
	}
	dev_datatype = sgPayloadGetType(datatype);
	switch(dev_datatype) {
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

int addHexField(const char *key, uint8_t *value, uint32_t size, char *payload) {
	// Add a field to the payload
	int i;
	uint32_t cap = 0;
	int field_not_zero = 0;
	int first_byte = 0;
	const char *delim_key = DELIM_KEY;
	const char *delim_val = DELIM_VAL;
	sprintf(payload, "%s%s%s%s", payload, delim_key, key, delim_val);
	if (size == 0) {
		field_not_zero = 0;
	} else {
		for (i=size-1; i>=0; i--) {
			if (value[i] != 0x0) {
				field_not_zero = 1;
				cap = i+1;
				break;
			}
		}
	}
	if (field_not_zero) {
		for (i=0; i<cap; i++) {
			if (value[i] == 0x0)
				continue;
			else {
				first_byte = i;
				break;
			}
		}
		sprintf(payload, "%s%x", payload, value[i]);
		for (i=first_byte+1; i<cap; i++) {
			sprintf(payload, "%s%.2x", payload, value[i]);
		}
	} else {
		sprintf(payload, "%s%x", payload, 0x0);
	}
	return 0;
}


int addCharField(const char *key, char *value, uint32_t size, char *payload) {
	// Add a field to the payload
	const char *delim_key = DELIM_KEY;
	const char *delim_val = DELIM_VAL;
	sprintf(payload, "%s%s%s%s", payload, delim_key, key, delim_val);
	strncat(payload, value, size*sizeof(char));
	return 0;
}



int sgRouterToServerConvert(SansgridSerial *sg_serial, char *payload) {
	// translate the SansgridSerial packet into an intrarouter payload
	SansgridEyeball 	sg_eyeball;
	SansgridPeck 		sg_peck;
	SansgridSing 		sg_sing;
	SansgridMock 		sg_mock;
	SansgridPeacock 	sg_peacock;
	SansgridNest 		sg_nest;
	SansgridSquawk 		sg_squawk;
	//SansgridHeartbeat 	sg_heartbeat;
	SansgridChirp 		sg_chirp;
	uint32_t 			rdid_32 = 0;
	uint8_t 			rdid[4];
	uint8_t 			broadcast[IP_SIZE];
	

	if (!sg_serial || !sg_serial->payload) {
		return -1;
	}
	payload[0] = '\0';
	uint8_t payload_type = sg_serial->payload[0];
	uint8_t datatype = sgPayloadGetType(payload_type);

	memset(rdid, 0x0, 4*sizeof(uint8_t));
	routingTableGetBroadcast(routing_table, broadcast);
	if (!memcmp(sg_serial->ip_addr, broadcast, IP_SIZE)) {
		// broadcast address
		addHexField("rdid", rdid, 4, payload);
	} else if ((rdid_32 = routingTableIPToRDID(routing_table, sg_serial->ip_addr)) == 0) {
		// no match found: this really shouldn't happen
		syslog(LOG_DEBUG, "No device found");
		return -1;
	} else {
		// match found; continue
		memcpy(rdid, &rdid_32, 4*sizeof(uint8_t));
		addHexField("rdid", rdid, 4, payload);
	}


	addHexField("dt", &payload_type, 1, payload);
	switch (datatype) {
		case SG_DEVSTATUS_EYEBALLING:
			memcpy(&sg_eyeball, sg_serial->payload, sizeof(SansgridEyeball));
			addHexField("manid",	sg_eyeball.manid,	4,	payload);
			addHexField("modnum", 	sg_eyeball.modnum, 	4,	payload);
			addHexField("sn",		sg_eyeball.serial_number, 8, payload);
			addHexField("profile",	&sg_eyeball.profile,1,	payload);
			addHexField("mode",		&sg_eyeball.mode,	1,	payload);
			break;
		case SG_DEVSTATUS_PECKING:
			memcpy(&sg_peck, sg_serial->payload, sizeof(SansgridPeck));
			addHexField("ip",		sg_peck.assigned_ip, IP_SIZE, payload);
			addHexField("sid",		sg_peck.server_id,	16,	payload);
			addHexField("recognition",&sg_peck.recognition, 1,payload);
			addHexField("manid",	sg_peck.manid,		4,	payload);
			addHexField("modnum",	sg_peck.modnum,		4,	payload);
			addHexField("sn",		sg_peck.serial_number,8,payload);
			break;
		case SG_DEVSTATUS_SINGING:
			memcpy(&sg_sing, sg_serial->payload, sizeof(SansgridSing));
			addHexField("servpubkey", sg_sing.pubkey, 64, payload);
			break;
		case SG_DEVSTATUS_MOCKING:
			memcpy(&sg_mock, sg_serial->payload, sizeof(SansgridMock));
			addHexField("senspubkey", sg_mock.pubkey, 64, payload);
			break;
		case SG_DEVSTATUS_PEACOCKING:
			memcpy(&sg_peacock, sg_serial->payload, sizeof(SansgridPeacock));
			addHexField("sida",		&sg_peacock.IO_A_id,1, payload);
			addHexField("classa",	&sg_peacock.IO_A_class,1, payload);
			addHexField("dira",	&sg_peacock.IO_A_direc,1, payload);
			addCharField("labela", sg_peacock.IO_A_label, 30, payload);
			addCharField("unitsa", sg_peacock.IO_A_units, 6, payload);

			addHexField("sidb",		&sg_peacock.IO_B_id,1, payload);
			addHexField("classb",	&sg_peacock.IO_B_class,1, payload);
			addHexField("dirb",	&sg_peacock.IO_B_direc,1, payload);
			addCharField("labelb", sg_peacock.IO_B_label, 30, payload);
			addCharField("unitsb", sg_peacock.IO_B_units, 6, payload);

			addHexField("additional",	&sg_peacock.additional_IO_needed, 1, payload);
			break;
		case SG_DEVSTATUS_NESTING:
			memcpy(&sg_nest, sg_serial->payload, sizeof(SansgridNest));
			break;
		case SG_DEVSTATUS_SQUAWKING:
			memcpy(&sg_squawk, sg_serial->payload, sizeof(SansgridSquawk));
			addHexField("data", sg_squawk.data, 64, payload);
			break;
		case SG_DEVSTATUS_HEARTBEAT:
			// Nothing here
			break;
		case SG_DEVSTATUS_CHIRPING:
			memcpy(&sg_chirp, sg_serial->payload, sizeof(SansgridChirp));
			addHexField("datasize", &sg_chirp.datasize, 1, payload);
			addHexField("data",		sg_chirp.data, 79, payload);
			break;
		default:
			// error
			return -1;
	}
	strcat(payload, DELIM_KEY);

	return 0;
}


// vim: ft=c ts=4 noet sw=4:

