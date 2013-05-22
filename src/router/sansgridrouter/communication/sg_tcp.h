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
#ifndef __SG_ROUTER_TCP_H__
#define __SG_ROUTER_TCP_H__
/** \file */

#include <stdint.h>
#include <sgSerial.h>

// Delimiters
// Note: These are wide chars. They take up 2 bytes
/**
 * \brief Value Delimiter
 *
 * This delimiter separates a key from a value.
 * Note that this is probably a wide char.
 * Take care when doing comparisons to match against the entire character,
 * and not just one byte.
 */
#define DELIM_VAL "α"
/**
 * \brief Key Delimiter
 *
 * This delimiter separates key-value pairs.
 * Note that this is probably a wide char.
 * Take care when doing comparisons to match against the entire character,
 * and not just one byte.
 */
#define DELIM_KEY "β"

/**
 * \brief Router Status, used for heartbeat status updates
 *
 * When a status update is transmitted to the server, these
 * are used to indicate a device's status. Later on, before they are sent,
 * they are converted to strings.
 */
enum SansgridIRStatusEnum {
	/// Device is online
	SG_IR_STATUS_ONLINE,
	/// Device is stale
	SG_IR_STATUS_STALE,
	/// Server Acknowledge
	SG_IR_STATUS_ACK,
};


/**
 * \brief Intrarouter Status Updates Data Structure
 *
 * Used to send status updates to and from the server
 */
typedef struct SansgridIRStatus {
	/// Sansgrid Payload Datatype
	uint8_t datatype;
	/// Device Unique Identifier
	uint8_t rdid[4];
	/// Null-Terminated status string
	char status[76];
} SansgridIRStatus;


/**
 * \brief Dictionary Implementation for key-value matching
 *
 * When a command is received from the server, it is in a 
 * null-terminated string with key-value pair delimiters.
 * This needs to be efficiently parsed and sorted into
 * Sansgrid Payload Structures. This dictionary aids that process
 * by storing key-value pairs.
 */
typedef struct Dictionary {
	/// The key from the key field
	char *key;
	/// The value from the value field
	char *value;
} Dictionary;

void atox(uint8_t *hexarray, char *str, uint32_t hexsize);
int8_t sgServerToRouterConvert(char *payload, SansgridSerial *sg_serial);
int sgRouterToServerConvert(SansgridSerial *sg_serial, char *payload);


// Low-level sending
int8_t sgTCPSend(SansgridSerial *sg_serial, uint32_t size);

#endif

// vim: ft=c ts=4 noet sw=4:

