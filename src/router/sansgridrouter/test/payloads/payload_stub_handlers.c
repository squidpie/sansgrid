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

#include "payload_tests.h"

void payloadMkSerial(SansgridSerial *sg_serial) {
	for (int i=0; i<IP_SIZE; i++) {
		sg_serial->dest_ip[i] = 0x0;
		sg_serial->origin_ip[i] = 0x0;
	}
	for (int i=0; i<81; i++)
		sg_serial->payload[i] = 0x0;
	return;
}

void payloadMkEyeball(SansgridEyeball *sg_eyeball, PayloadTestStruct *test_specs) {
	int i;

	sg_eyeball->datatype = test_specs->eyeball_mode;
	for (i=0; i<4; i++) {
		sg_eyeball->manid[i] = 0x0;
		sg_eyeball->modnum[i] = 0x0;
		sg_eyeball->serial_number[i] = 0x0;
	}
	for (i=4; i<8; i++) {
		sg_eyeball->serial_number[i] = 0x0;
	}
	sg_eyeball->profile = 0x0;
	sg_eyeball->mode = test_specs->eyeball_mode;
	for (i=0; i<62; i++)
		sg_eyeball->padding[i] = 0x0;

	return;
}


void payloadMkPeck(SansgridPeck *sg_peck, PayloadTestStruct *test_specs) {
	int i;
	SansgridEyeball sg_eyeball;

	sg_peck->datatype = SG_PECK;
	for (i=0; i<IP_SIZE; i++) {
		sg_peck->router_ip[i] = 0x0;
		sg_peck->assigned_ip[i] = 0x0;
	}
	for (i=0; i<16; i++)
		sg_peck->server_id[i] = 0x0;
	sg_peck->recognition = test_specs->peck_mode;
	payloadMkEyeball(&sg_eyeball, test_specs);
	memcpy(&sg_peck->manid, &sg_eyeball.manid, 4*sizeof(uint8_t));
	memcpy(&sg_peck->modnum, &sg_eyeball.modnum, 4*sizeof(uint8_t));
	memcpy(&sg_peck->serial_number, &sg_eyeball.serial_number, 8*sizeof(uint8_t));
	for (i=0; i<31; i++)
		sg_peck->padding[i] = 0x0;

	return;
}


void payloadMkSing(SansgridSing *sg_sing, PayloadTestStruct *test_specs) {
	int i;

	sg_sing->datatype = test_specs->sing_mode;
	for (i=0; i<80; i++)
		sg_sing->pubkey[i] = 0x0;

	return;
}


void payloadMkMock(SansgridMock *sg_mock, PayloadTestStruct *test_specs) {
	int i;

	sg_mock->datatype = test_specs->mock_mode;
	for (i=0; i<80; i++)
		sg_mock->pubkey[i] = 0x0;

	return;
}


void payloadMkPeacock(SansgridPeacock *sg_peacock, PayloadTestStruct *test_specs) {
	int i;
	sg_peacock->datatype = test_specs->peacock_mode;
	sg_peacock->IO_A_id = 0x0;
	sg_peacock->IO_A_classification = 0x0;
	sg_peacock->IO_A_direction = 0x0;
	for (i=0; i<30; i++)
		sg_peacock->IO_A_label[i] = 0x0;
	for (i=0; i<6; i++)
		sg_peacock->IO_A_units[i] = 0x0;

	sg_peacock->IO_B_id = 0x0;
	sg_peacock->IO_B_classification = 0x0;
	sg_peacock->IO_B_direction = 0x0;
	for (i=0; i<30; i++)
		sg_peacock->IO_B_label[i] = 0x0;
	for (i=0; i<6; i++)
		sg_peacock->IO_B_units[i] = 0x0;

	sg_peacock->additional_IO_needed = 0x0;

	sg_peacock->padding = 0x0;

	return;
}


void payloadMkNest(SansgridNest *sg_nest, PayloadTestStruct *test_specs) {
	int i;
	sg_nest->datatype = test_specs->nest_mode;
	for (i=0; i<80; i++)
		sg_nest->padding[i] = 0x0;

	return;
}


void payloadMkSquawkServer(SansgridSquawk *sg_squawk, PayloadTestStruct *test_specs) {
	int i;
	sg_squawk->datatype = test_specs->squawk_server_mode;
	for (i=0; i<80; i++)
		sg_squawk->data[i] = 0x0;

	return;
}


void payloadMkSquawkSensor(SansgridSquawk *sg_squawk, PayloadTestStruct *test_specs) {
	int i;
	sg_squawk->datatype = test_specs->squawk_sensor_mode;
	for (i=0; i<80; i++)
		sg_squawk->data[i] = 0x0;

	return;
}


void payloadMkChirp(SansgridChirp *sg_chirp, PayloadTestStruct *test_specs) {
	sg_chirp->datatype = test_specs->chirp_mode;
	sg_chirp->datasize = 79;
	for (int i=0; i<79; i++)
		sg_chirp->data[i] = 0x0;
}
	

// vim: ft=c ts=4 noet sw=4:

