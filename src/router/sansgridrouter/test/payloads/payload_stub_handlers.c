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
/// \file

#include "payload_tests.h"

/// Create a serial packet with sane options
void payloadMkSerial(SansgridSerial *sg_serial) {
	memset(sg_serial, 0x0, sizeof(SansgridSerial));
	sg_serial->control = SG_SERIAL_CTRL_VALID_DATA;
	sg_serial->ip_addr[IP_SIZE-1] = 0x2;
	return;
}

/// Make an eyeball using specs from test_specs
void payloadMkEyeball(SansgridEyeball *sg_eyeball, PayloadTestStruct *test_specs) {
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
	sg_eyeball->mode = test_specs->eyeball_mode;
	for (i=0; i<62; i++)
		sg_eyeball->padding[i] = 0x0;

	return;
}


/// Make a peck using specs from test_specs
void payloadMkPeck(SansgridPeck *sg_peck, PayloadTestStruct *test_specs) {
	int i;
	SansgridEyeball sg_eyeball;

	sg_peck->datatype = SG_PECK;
	for (i=0; i<IP_SIZE; i++) {
		sg_peck->router_ip[i] = 0x0;
		sg_peck->assigned_ip[i] = 0x0;
	}
	sg_peck->router_ip[IP_SIZE-1] = 0x1;
	for (i=0; i<16; i++)
		sg_peck->server_id[i] = 0x0;
	sg_peck->recognition = test_specs->peck_mode;
	payloadMkEyeball(&sg_eyeball, test_specs);
	memcpy(&sg_peck->manid, &sg_eyeball.manid, 4*sizeof(uint8_t));
	memcpy(&sg_peck->modnum, &sg_eyeball.modnum, 4*sizeof(uint8_t));
	memcpy(&sg_peck->serial_number, &sg_eyeball.serial_number, 8*sizeof(uint8_t));
	memset(sg_peck->padding, 0x0, 15*sizeof(uint8_t));

	return;
}


/// Make a sing using specs from test_specs
void payloadMkSing(SansgridSing *sg_sing, PayloadTestStruct *test_specs) {
	int i;

	sg_sing->datatype = test_specs->sing_mode;
	for (i=0; i<80; i++)
		sg_sing->pubkey[i] = 0x0;

	return;
}


/// Make a mock using specs from test_specs
void payloadMkMock(SansgridMock *sg_mock, PayloadTestStruct *test_specs) {
	int i;

	sg_mock->datatype = test_specs->mock_mode;
	for (i=0; i<80; i++)
		sg_mock->pubkey[i] = 0x0;

	return;
}


/// Make a peacock using specs from test_specs
void payloadMkPeacock(SansgridPeacock *sg_peacock, PayloadTestStruct *test_specs) {
	sg_peacock->datatype = test_specs->peacock_mode;
	sg_peacock->IO_A_id = 0x0;
	sg_peacock->IO_A_class= 0x0;
	sg_peacock->IO_A_direc= 0x0;
	memset(sg_peacock->IO_A_label, 0x0, 30*sizeof(char));
	memset(sg_peacock->IO_A_units, 0x0, 6*sizeof(char));
	strcpy(sg_peacock->IO_A_label, "Label A");
	strcpy(sg_peacock->IO_A_units, "Aunit");

	sg_peacock->IO_B_id = 0x0;
	sg_peacock->IO_B_class= 0x0;
	sg_peacock->IO_B_direc= 0x0;
	memset(sg_peacock->IO_B_label, 0x0, 30*sizeof(char));
	memset(sg_peacock->IO_B_units, 0x0, 6*sizeof(char));
	strcpy(sg_peacock->IO_B_label, "Label B");
	strcpy(sg_peacock->IO_B_units, "Bunit");

	sg_peacock->additional_IO_needed = 0x0;

	sg_peacock->padding = 0x0;

	return;
}


/// Make a nest using specs from test_specs
void payloadMkNest(SansgridNest *sg_nest, PayloadTestStruct *test_specs) {
	sg_nest->datatype = test_specs->nest_mode;
	memset(sg_nest->padding, 0x0, 80*sizeof(uint8_t));

	return;
}


/// Make a squawk from server using specs from test_specs
void payloadMkSquawkServer(SansgridSquawk *sg_squawk, PayloadTestStruct *test_specs) {
	int i;
	sg_squawk->datatype = test_specs->squawk_server_mode;
	for (i=0; i<80; i++)
		sg_squawk->data[i] = 0x0;

	return;
}


/// Make a squawk from sensor using specs from test_specs
void payloadMkSquawkSensor(SansgridSquawk *sg_squawk, PayloadTestStruct *test_specs) {
	int i;
	sg_squawk->datatype = test_specs->squawk_sensor_mode;
	for (i=0; i<80; i++)
		sg_squawk->data[i] = 0x0;

	return;
}


/// Make a chirp using specs from test_specs
void payloadMkChirp(SansgridChirp *sg_chirp, PayloadTestStruct *test_specs) {
	sg_chirp->datatype = test_specs->chirp_mode;
	sg_chirp->sid = 79;
	for (int i=0; i<79; i++)
		sg_chirp->data[i] = 0x0;
}
	

// vim: ft=c ts=4 noet sw=4:

