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

#define _POSIX_C_SOURCE 200809L		///< Required for nanosleep()

#include <syslog.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "sg_tcp.h"
#include "../payload_handlers/payload_handlers.h"
#include "../sansgrid_router.h"
/** \file */




/**
 * \brief Extract a keyvalue from a string
 *
 * \param[in]	str		Null-terminated string to parse
 * \param[out]	key		Extracted Key
 * \param[out]	value	Extracted Value
 * \param[out]	saved	Used to make the function Re-entrant/thread-safe
 *
 * \returns
 * This function only returns 0 when both the key and the value are null.
 * This is to catch empty values but still return the key. \n
 * Notes:
 * No checks are done for empty keys. \n
 * The saved argument should start initialized as a pointer to NULL.
 */
int extract_keyvalue(char *str, char **key, char **value, char **saved) {
	// Extract the next keyvalue from str
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




/**
 * \brief Convert the full string of hex values into an array
 *
 * \param[out]	hexarray	An array of 8-bit values
 * \param[in]	str			A null-terminated string of ASCII hex values
 * \param[in]	hexsize		Number of hex values (not used)
 */
void atox(uint8_t *hexarray, char *str, uint32_t hexsize) {
	// convert the full string of hex values into an array
	uint32_t i_str, i_hex = 0;
	int increment = 0;
	char chunk[3];
	uint32_t length;
	uint32_t uval;
	int32_t offset;

	for (i_hex=0; i_hex<hexsize; i_hex++)
		hexarray[i_hex] = 0x0;
	i_hex = 0;
	if (str == NULL)
		return;
	length = strlen(str);
	if ((length+1)/2 > hexsize) {
		syslog(LOG_DEBUG, "atox: truncating string %s", str);
		length = hexsize*2;
	} else if ((length+1)/2 < hexsize) {
		syslog(LOG_DEBUG, "atox: left-padding string %s", str);
		offset = hexsize - (length+1)/2;
		return atox(hexarray+offset, str, hexsize-offset);
	}


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



/**
 * \brief Get a value from a dictionary
 *
 * \param[in]	dict	A populated dictionary
 * \param[in]	size	Size of the dictionary
 * \param[in]	key		Key to match against
 * \returns
 * A match is returned if found. \n
 * If no match is found, NULL is returned.
 */
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
		} else if (!strncmp(key, dict[i].key, strlen(key))) {
			// matched
			return dict[i].value;
		}
	}
	return NULL;
}


/**
 * \brief Find the rdid from the dictionary
 * \param[in]	dict	Populated Dictionary Structure
 * \param[in]	size	Number of entries in the Dictionary
 * \returns
 * If the Device Identifier rdid could not be found, -1 is returned. \n
 * On success, 0 is returned.
 */
static int32_t translateRdid(Dictionary dict[], int size) {
	// Find the rdid from the dictionary
	uint32_t rdid_return;
	char *rdid_str = match(dict, size, "rdid");

	//rdid_return = atoi(match(dict, size, "rdid"));
	rdid_return = (rdid_str != NULL ? atoi(rdid_str) : -1);
	return rdid_return;
}



/**
 * \brief Build a SansgridEyeball from entries in the dictionary
 * \param[in]	dict		Populated Dictionary Structure
 * \param[in]	size		Number of entries in the Dictionary
 * \param[out]	sg_serial	Outputted SansgridSerial Structure
 *
 * Note that sg_serial must point to a valid chunk of memory.
 * \returns
 * The Device Identifier rdid is returned when found. \n
 * If the rdid is missing, return -1.
 */
static int32_t convertEyeball(Dictionary dict[], int size, SansgridSerial *sg_serial) {
	// Get an eyeball datatype from the payload
	SansgridEyeball sg_eyeball;

	atox(&sg_eyeball.datatype,		match(dict, size, "dt"), 	1*sizeof(uint8_t));
	atox(sg_eyeball.manid, 			match(dict, size, "manid"),	4*sizeof(uint8_t));
	atox(sg_eyeball.modnum, 		match(dict, size, "modnum"),4*sizeof(uint8_t));
	atox(sg_eyeball.serial_number,	match(dict, size, "sn"), 	8*sizeof(uint8_t));
	atox(&sg_eyeball.profile, 		match(dict, size, "profile"),1*sizeof(uint8_t));
	atox(&sg_eyeball.mode, 			match(dict, size, "mode"), 	1*sizeof(uint8_t));

	memset(sg_eyeball.padding, 0x0, 62*sizeof(uint8_t));

	memcpy(sg_serial->payload, &sg_eyeball, sizeof(SansgridEyeball));

	return translateRdid(dict, size);
}



/**
 * \brief Build a SansgridPeck from entries in the dictionary
 * \param[in]	dict		Populated Dictionary Structure
 * \param[in]	size		Number of entries in the Dictionary
 * \param[out]	sg_serial	Outputted SansgridSerial Structure
 *
 * Note that sg_serial must point to a valid chunk of memory.
 * \returns
 * The Device Identifier rdid is returned when found. \n
 * If the rdid is missing, return -1.
 */
static int8_t convertPeck(Dictionary dict[], int size, SansgridSerial *sg_serial) {
	// Get a peck datatype from the payload
	SansgridPeck sg_peck;
	uint32_t rdid;

	atox(&sg_peck.datatype, 	match(dict, size, "dt"),		1*sizeof(uint8_t));
	atox(sg_peck.assigned_ip,	match(dict, size, "server_ip"),	IP_SIZE);
	atox(sg_peck.server_id,		match(dict, size, "sid"),		16*sizeof(uint8_t));
	atox(&sg_peck.recognition,	match(dict, size,"recognition"),1*sizeof(uint8_t));
	atox(sg_peck.manid,			match(dict, size, "manid"),		4*sizeof(uint8_t));
	atox(sg_peck.modnum,		match(dict, size, "modnum"),	4*sizeof(uint8_t));
	atox(sg_peck.serial_number,	match(dict, size, "sn"),		8*sizeof(uint8_t));

	memset(sg_peck.padding, 0x0, 15*sizeof(uint8_t));
	routingTableGetRouterIP(routing_table, sg_peck.router_ip);
	rdid = atoi(match(dict, size, "rdid"));
	routingTableRDIDToIP(routing_table, rdid, sg_peck.assigned_ip);

	routingTableGetBroadcast(routing_table, sg_serial->ip_addr);
	memcpy(sg_serial->payload, &sg_peck, sizeof(SansgridPeck));
	
	return translateRdid(dict, size);
}



/**
 * \brief Build a SansgridSing from entries in the dictionary
 * \param[in]	dict		Populated Dictionary Structure
 * \param[in]	size		Number of entries in the Dictionary
 * \param[out]	sg_serial	Outputted SansgridSerial Structure
 *
 * Note that sg_serial must point to a valid chunk of memory.
 * \returns
 * The Device Identifier rdid is returned when found. \n
 * If the rdid is missing, return -1.
 */
static int8_t convertSing(Dictionary dict[], int size, SansgridSerial *sg_serial) {
	// Get a sing datatype from the payload
	SansgridSing sg_sing;

	atox(&sg_sing.datatype,		match(dict, size, "dt"),		1*sizeof(uint8_t));
	atox(sg_sing.pubkey, 		match(dict, size, "servpubkey"),80*sizeof(uint8_t));

	memcpy(sg_serial->payload, &sg_sing, sizeof(SansgridSing));
	
	return translateRdid(dict, size);
}



/**
 * \brief Build a SansgridMock from entries in the dictionary
 * \param[in]	dict		Populated Dictionary Structure
 * \param[in]	size		Number of entries in the Dictionary
 * \param[out]	sg_serial	Outputted SansgridSerial Structure
 *
 * Note that sg_serial must point to a valid chunk of memory.
 * \returns
 * The Device Identifier rdid is returned when found. \n
 * If the rdid is missing, return -1.
 */
static int8_t convertMock(Dictionary dict[], int size, SansgridSerial *sg_serial) {
	// Get a mock datatype from the payload
	SansgridMock sg_mock;

	atox(&sg_mock.datatype, 	match(dict, size, "dt"),		1*sizeof(uint8_t));
	atox(sg_mock.pubkey,		match(dict, size, "senspubkey"),80*sizeof(uint8_t));

	memcpy(sg_serial->payload, &sg_mock, sizeof(SansgridMock));
	
	return translateRdid(dict, size);
}



/**
 * \brief Build a SansgridPeacock from entries in the dictionary
 * \param[in]	dict		Populated Dictionary Structure
 * \param[in]	size		Number of entries in the Dictionary
 * \param[out]	sg_serial	Outputted SansgridSerial Structure
 *
 * Note that sg_serial must point to a valid chunk of memory.
 * \returns
 * The Device Identifier rdid is returned when found. \n
 * If the rdid is missing, return -1.
 */
static int8_t convertPeacock(Dictionary dict[], int size, SansgridSerial *sg_serial) {
	// Get a peacock datatype from the payload
	char *label = NULL;
	SansgridPeacock sg_peacock;

	atox(&sg_peacock.datatype,	match(dict, size, "dt"),		1*sizeof(uint8_t));

	atox(&sg_peacock.IO_A_id,	match(dict, size, "sida"),		1*sizeof(uint8_t));
	atox(&sg_peacock.IO_A_class,match(dict, size, "classa"),	1*sizeof(uint8_t));
	atox(&sg_peacock.IO_A_direc,match(dict, size, "dira"),		1*sizeof(uint8_t));
	if ((label = match(dict, size, "labela")) == NULL) {
		memset(sg_peacock.IO_A_label, 0x0,						30*sizeof(char));
	} else {
		strncpy(sg_peacock.IO_A_label, label, 30);
	}
	if ((label = match(dict, size, "unitsa")) == NULL) {
		memset(sg_peacock.IO_A_units, 0x0,						6*sizeof(char));
	} else {
		strncpy(sg_peacock.IO_A_units, label, 6);
	}
	

	atox(&sg_peacock.IO_B_id,	match(dict, size, "sidb"),		1*sizeof(uint8_t));
	atox(&sg_peacock.IO_B_class,match(dict, size, "classb"),	1*sizeof(uint8_t));
	atox(&sg_peacock.IO_B_direc,match(dict, size, "dirb"),		1*sizeof(uint8_t));
	if ((label = match(dict, size, "labelb")) == NULL) {
		memset(sg_peacock.IO_B_label, 0x0,						30*sizeof(char));
	} else {
		strncpy(sg_peacock.IO_B_label, label, 30);
	}
	if ((label = match(dict, size, "unitsb")) == NULL) {
		memset(sg_peacock.IO_B_units, 0x0,						6*sizeof(char));
	} else {
		strncpy(sg_peacock.IO_B_units, label, 6);
	}

	atox(&sg_peacock.additional_IO_needed, 
			match(dict, size, "additional"), 1*sizeof(uint8_t));
	sg_peacock.padding = 0x0;

	memcpy(sg_serial->payload, &sg_peacock, sizeof(SansgridPeacock));

	return translateRdid(dict, size);
}



/**
 * \brief Build a SansgridNest from entries in the dictionary
 * \param[in]	dict		Populated Dictionary Structure
 * \param[in]	size		Number of entries in the Dictionary
 * \param[out]	sg_serial	Outputted SansgridSerial Structure
 *
 * Note that sg_serial must point to a valid chunk of memory.
 * \returns
 * The Device Identifier rdid is returned when found. \n
 * If the rdid is missing, return -1.
 */
static int8_t convertNest(Dictionary dict[], int size, SansgridSerial *sg_serial) {
	// Get a nest datatype from the payload
	SansgridNest sg_nest;

	atox(&sg_nest.datatype, 	match(dict, size, "dt"),		1*sizeof(uint8_t));
	
	memset(sg_nest.padding, 0x0, 80*sizeof(uint8_t));

	memcpy(sg_serial->payload, &sg_nest, sizeof(SansgridNest));
	
	return translateRdid(dict, size);
}



/**
 * \brief Build a SansgridSquawk from entries in the dictionary
 * \param[in]	dict		Populated Dictionary Structure
 * \param[in]	size		Number of entries in the Dictionary
 * \param[out]	sg_serial	Outputted SansgridSerial Structure
 *
 * Note that sg_serial must point to a valid chunk of memory.
 * \returns
 * The Device Identifier rdid is returned when found. \n
 * If the rdid is missing, return -1.
 */
static int8_t convertSquawk(Dictionary dict[], int size, SansgridSerial *sg_serial) {
	// Get a squawk datatype from the payload
	SansgridSquawk sg_squawk;
	
	atox(&sg_squawk.datatype, 	match(dict, size, "dt"),		1*sizeof(uint8_t));
	atox(sg_squawk.data,		match(dict, size, "data"),		80*sizeof(uint8_t));

	memcpy(sg_serial->payload, &sg_squawk, sizeof(SansgridSquawk));

	return translateRdid(dict, size);
}



/**
 * \brief Build a SansgridChirp from entries in the dictionary
 * \param[in]	dict		Populated Dictionary Structure
 * \param[in]	size		Number of entries in the Dictionary
 * \param[out]	sg_serial	Outputted SansgridSerial Structure
 *
 * Note that sg_serial must point to a valid chunk of memory.
 * \returns
 * The Device Identifier rdid is returned when found. \n
 * If the rdid is missing, return -1.
 */
static int8_t convertChirp(Dictionary dict[], int size, SansgridSerial *sg_serial) {
	// Get a chirp datatype from the payload
	SansgridChirp sg_chirp;

	atox(&sg_chirp.datatype,	match(dict, size, "dt"),		1*sizeof(uint8_t));
	atox(&sg_chirp.sid,			match(dict, size, "sid"),	1*sizeof(uint8_t));
	strncpy((char*)sg_chirp.data, 		match(dict, size, "data"),	79);
	//atox(sg_chirp.data,		match(dict, size, "data"),		79*sizeof(uint8_t));
	
	memcpy(sg_serial->payload, &sg_chirp, sizeof(SansgridChirp));

	return translateRdid(dict, size);
}



/**
 * \brief Build a SansgridIRStatus from entries in the dictionary
 * \param[in]	dict		Populated Dictionary Structure
 * \param[in]	size		Number of entries in the Dictionary
 * \param[out]	sg_serial	Outputted SansgridSerial Structure
 *
 * Note that sg_serial must point to a valid chunk of memory.
 * \returns
 * The Device Identifier rdid is returned when found. \n
 * If the rdid is missing, return -1.
 */
static int8_t convertIRStatus(
		Dictionary dict[], 
		int size, 
		SansgridSerial *sg_serial) {
	// Get an router<-->server datatype from the payload
	SansgridIRStatus sg_irstatus;
	char *status = NULL;

	memset(&sg_irstatus, 0x0, sizeof(SansgridIRStatus));
	atox(&sg_irstatus.datatype,	match(dict, size, "dt"),		1*sizeof(uint8_t));
	atox(sg_irstatus.rdid,		match(dict, size, "rdid"),		1*sizeof(uint8_t));
	if ((status = match(dict, size, "status")) != NULL) {
		strcpy(sg_irstatus.status, status);
	}

	memcpy(sg_serial->payload, &sg_irstatus, sizeof(SansgridIRStatus));
	return translateRdid(dict, size);
}



/**
 * \brief Convert an intrarouter null-terminated key-value string 
 * into formatted data
 *
 * \param[in]	payload		Null-terminated key-value string
 * \param[out]	sg_serial	Sansgrid Serial Formatted String
 * \returns
 * On success, return 0 \n
 * On failure, return -1
 */
int8_t sgServerToRouterConvert(char *payload, SansgridSerial *sg_serial) {
	// Take a payload, 	return the translated serial packet,
	// 					return the device identifier (rdid)
	uint8_t datatype = ~0x0;
	enum SansgridDeviceStatusEnum dev_datatype;
	Dictionary dict[30];
	int32_t size = 0;
	int8_t exit_code = 0;
	uint32_t rdid_32;
	char *rdid_str;
	char *saved 	= NULL,
		 *key 		= NULL,
		 *value 	= NULL;

	sg_serial->control = SG_SERIAL_CTRL_VALID_DATA;
	memset(sg_serial->ip_addr, 0x0, sizeof(sg_serial->ip_addr));
	syslog(LOG_DEBUG, "processing packet %s", payload);
	do {
		if (extract_keyvalue(payload, &key, &value, &saved) == 1) {
			dict[size].key = key+sizeof(DELIM_KEY)-2;
			dict[size].value = (value == NULL ? NULL : value+sizeof(DELIM_VAL)-2);
			size++;
		} else
			break;
	} while (1);
	dict[size].key = NULL;
	dict[size].value = NULL;

	// Find payload type
	atox(&datatype, match(dict, size, "dt"), 1*sizeof(uint8_t));
	//atox(rdid, 		match(dict, size, "rdid"), 4*sizeof(uint8_t));
	rdid_str = match(dict, size, "rdid");
	if (rdid_str == NULL) {
		syslog(LOG_WARNING, "Couldn't find rdid in payload %s", payload);
		rdid_32 = 0;
	} else {
		rdid_32 = atoi(rdid_str);
	}
	if (datatype == ~0x0) {
		return -1;
	}
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
		case SG_DEVSTATUS_HEARTBEAT:
			exit_code = convertIRStatus(dict, size, sg_serial);
			break;
		default:
			printf("Type not found: %u\n", dev_datatype);
			exit_code = -1;
			break;
	}
	//printf("Exiting with %i\n", exit_code);

	return exit_code;
}



/**
 * \brief Add a Hex field to a null-terminated key-value string.
 *
 * \param[in]	key		A key to add to the null-terminated string
 * \param[in]	value	A value to add to the null-terminated string
 * \param[in]	size	Size of the key field
 * \param[out]	payload	The null-terminated key-value string
 * \returns
 * 0 always
 */
int addHexField(const char *key, uint8_t *value, uint32_t size, char *payload) {
	// Add a field to the payload
	int i;
	int cap = 0;
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




/**
 * \brief Add an integer value to the null-terminated key-value string
 *
 * \param[in]	key		The key match
 * \param[in]	value	An integer stored in value
 * \param[in]	size	The number of entries in value
 * \param[out]	payload	A null-terminated key-value string
 */
int addIntField(const char *key, uint8_t *value, uint32_t size, char *payload) {
	// Add a field to the payload
	uint32_t i;
	uint32_t value_32 = 0;
	const char *delim_key = DELIM_KEY;
	const char *delim_val = DELIM_VAL;

	sprintf(payload, "%s%s%s%s", payload, delim_key, key, delim_val);
	for (i=0; i<size; i++) {
		value_32 = value_32 << 8;
		value_32 |= value[i];
	}
	sprintf(payload, "%s%u", payload, value_32);

	return 0;
}



/**
 * \brief Add a value directly to the null-terminated key-value string
 *
 *
 * \param[in]	key		The key match
 * \param[in]	value	A null-terminated string stored in value
 * \param[in]	size	The size of the string in value
 * \param[out]	payload	A null-terminated key-value string
 */
int addCharField(const char *key, char *value, uint32_t size, char *payload) {
	// Add a field to the payload
	const char *delim_key = DELIM_KEY;
	const char *delim_val = DELIM_VAL;
	sprintf(payload, "%s%s%s%s", payload, delim_key, key, delim_val);
	strncat(payload, value, size*sizeof(char));
	return 0;
}



/**
 * \brief Translate a SansgridSerial Packet into a null-terminated key-value
 * intrarouter payload
 *
 * \param[in]	sg_serial	A SansgridSerial Data Structure with data to send
 * \param[out]	payload		A null-terminated key-value string
 * \returns
 * On Error, returns -1
 * Errors will be noted in the log if verbosity is allows LOG_WARNING
 * if the conversion was successful, returns 0
 */
int sgRouterToServerConvert(SansgridSerial *sg_serial, char *payload) {
	// translate the SansgridSerial packet into an intrarouter payload
	SansgridEyeball 	sg_eyeball;
	SansgridPeck 		sg_peck;
	SansgridSing 		sg_sing;
	SansgridMock 		sg_mock;
	SansgridPeacock 	sg_peacock;
	SansgridNest 		sg_nest;
	SansgridSquawk 		sg_squawk;
	SansgridIRStatus 	sg_irstatus;
	//SansgridHeartbeat 	sg_heartbeat;
	SansgridChirp 		sg_chirp;
	uint32_t 			rdid_32 = 0;
	uint8_t 			rdid[4];
	uint8_t 			broadcast[IP_SIZE];
	

	if (!sg_serial || !sg_serial->payload) {
		syslog(LOG_WARNING, "Router-->Server: container or payload is NULL!");
		return -1;
	}
	payload[0] = '\0';
	uint8_t payload_type = sg_serial->payload[0];
	enum SansgridDeviceStatusEnum datatype = sgPayloadGetType(payload_type);

	memset(rdid, 0x0, sizeof(rdid));
	routingTableGetBroadcast(routing_table, broadcast);
	if (datatype == SG_DEVSTATUS_PECKING) {
		// Peck is a special case because the IP field of sg_serial is broadcast.
		// So we need to grab the IP from the payload
		memcpy(&sg_peck, sg_serial->payload, sizeof(SansgridPeck));
		if ((rdid_32 = routingTableIPToRDID(routing_table, sg_peck.assigned_ip)) == 0) {
			syslog(LOG_NOTICE, "No device found: %u", 
					routingTableIPToRDID(routing_table, sg_peck.assigned_ip));
			return -1;
		} else {
			wordToByte(rdid, &rdid_32, 4);
			addIntField("rdid", rdid, 4, payload);
		}
	} else if (!memcmp(sg_serial->ip_addr, broadcast, IP_SIZE)) {
		// broadcast address
		addHexField("rdid", rdid, 4, payload);
	} else if ((rdid_32 = routingTableIPToRDID(routing_table, sg_serial->ip_addr)) == 0) {
		// no match found: this really shouldn't happen
		syslog(LOG_NOTICE, "No device found: %u", 
				routingTableIPToRDID(routing_table, sg_serial->ip_addr));
		return -1;
	} else {
		// match found; continue
		//memcpy(rdid, &rdid_32, 4*sizeof(uint8_t));
		wordToByte(rdid, &rdid_32, 4);
		//addHexField("rdid", rdid, 4, payload);
		addIntField("rdid", rdid, 4, payload);
	}


	if (payload_type == SG_HEARTBEAT_SENSOR_TO_ROUTER)
		payload_type = 0xfd;
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
			// Heartbeat
			memcpy(&sg_irstatus, sg_serial->payload, sizeof(SansgridIRStatus));
			addCharField("status", sg_irstatus.status, sizeof(sg_irstatus.status), payload);
			break;
		case SG_DEVSTATUS_CHIRPING:
			memcpy(&sg_chirp, sg_serial->payload, sizeof(SansgridChirp));
			addHexField("sid", &sg_chirp.sid, 1, payload);
			addCharField("data",		(char*)sg_chirp.data, 79, payload);
			break;
		default:
			// error
			syslog(LOG_WARNING, "Router-->Server: Unknown datatype: %u", datatype);
			return -1;
	}
	strcat(payload, DELIM_KEY);

	return 0;
}


// vim: ft=c ts=4 noet sw=4:

