/* Payload Tests
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
 * This dispatch test uses a named pipe as a stub to read from.
 * The data from the stub is enqueued, and the dispatch thread dequeues the data.
 */

#include "../../../payloads.h"
#include "../../../sg_serial.h"
#include "../../communication/sg_tcp.h"

void payloadMkSerial(SansgridSerial *sg_serial) {
	for (int i=0; i<IP_SIZE; i++) {
		sg_serial->dest_ip[i] = 0x0;
		sg_serial->origin_ip[i] = 0x0;
	}
	for (int i=0; i<81; i++)
		sg_serial->payload[i] = 0x0;
	sg_serial->timestamp = 0x0;
	sg_serial->origin = 0x0;
	return;
}

void payloadMkEyeball(SansgridEyeball *sg_eyeball, enum SansgridEyeballModeEnum ebmate_type) {
	int i;

	sg_eyeball->datatype = SG_EYEBALL;
	for (i=0; i<4; i++) {
		sg_eyeball->manid[i] = 0x0;
		sg_eyeball->modnum[i] = 0x0;
		sg_eyeball->serial_number[i] = 0x0;
	}
	for (i=4; i<8; i++) {
		sg_eyeball->serial_number[i] = 0x0;
	}
	sg_eyeball->profile = 0x0;
	sg_eyeball->mode = ebmate_type;
	for (i=0; i<62; i++)
		sg_eyeball->padding[i] = 0x0;

	return;
}


// vim: ft=c ts=4 noet sw=4:

