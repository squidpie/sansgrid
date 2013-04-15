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

#include <stdint.h>
#include "../../../lib/sgSerial.h"

// Delimiters
// Note: These are wide chars. They take up 2 bytes
#define DELIM_VAL "α"
#define DELIM_KEY "β"

typedef struct Dictionary {
	char *key;
	char *value;
} Dictionary;

void atox(uint8_t *hexarray, char *str, uint32_t hexsize);
int8_t sgTCPHandle(char *payload, SansgridSerial *sg_serial);


// Low-level sending/receiving
int8_t sgTCPSend(SansgridSerial *sg_serial, uint32_t size);
int8_t sgTCPReceive(SansgridSerial **sg_serial, uint32_t *size);

#endif

// vim: ft=c ts=4 noet sw=4:

