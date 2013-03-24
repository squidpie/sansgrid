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

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <check.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../../../sg_serial.h"
#include "../../../payloads.h"
#include "../../routing/routing.h"

#include "../../dispatch/dispatch.h"
#include "../tests.h"


void checkSize(const char *pkname, size_t pksize) {
	fail_unless((pksize == PAYLOAD_SIZE), 
			"\n%s is wrong size: \
			\n\tExpected: %i\
			\n\tGot: %i", pkname, PAYLOAD_SIZE, pksize);
}


START_TEST (testPayloadSize) {
	// unit test code for making sure all payloads are the same size
	checkSize("SansgridHatching", sizeof(SansgridHatching));
	checkSize("SansgridFly", sizeof(SansgridFly));
	checkSize("SansgridEyeball", sizeof(SansgridEyeball));
	checkSize("SansgridPeck", sizeof(SansgridPeck));
	checkSize("SansgridSing", sizeof(SansgridSing));
	checkSize("SansgridMock", sizeof(SansgridMock));
	checkSize("SansgridPeacock", sizeof(SansgridPeacock));
	checkSize("SansgridNest", sizeof(SansgridNest));
	checkSize("SansgridSquawk", sizeof(SansgridSquawk));
	checkSize("SansgridHeartbeat", sizeof(SansgridHeartbeat));
	checkSize("SansgridChirp", sizeof(SansgridChirp));
}
END_TEST


START_TEST (testEyeball) {
	// unit test code to test the Eyeball data type
	int i;
	SansgridEyeball sg_eyeball;
	SansgridSerial sg_serial;
	RoutingTable *routing_table;
	uint8_t base[IP_SIZE];

	sg_eyeball.datatype = SG_EYEBALL;
	for (i=0; i<4; i++) {
		sg_eyeball.manid[i] = 0x0;
		sg_eyeball.modnum[i] = 0x0;
		sg_eyeball.serial_number[i] = 0x0;
	}
	for (i=4; i<8; i++) {
		sg_eyeball.serial_number[i] = 0x0;
	}
	for (i=0; i<IP_SIZE; i++)
		base[i] = 0x0;
	base[0] = 128;
	sg_eyeball.profile = 0x0;


	routing_table = routingTableInit(base);
	fail_if((routing_table == NULL), "Error: routing table not initialized!");


	sg_eyeball.mode = SG_EYEBALL_MATE;


	

}
END_TEST



Suite *payloadTesting (void) {
	Suite *s = suite_create("Payload Tests");

	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, testPayloadSize);

	suite_add_tcase(s, tc_core);

	return s;
}


// vim: ft=c ts=4 noet sw=4:
