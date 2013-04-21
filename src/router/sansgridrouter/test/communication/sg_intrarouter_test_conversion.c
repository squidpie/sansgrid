/* Tests for router<-->server communication (conversion)
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
 */

#include "sg_tcp_test.h"
#include "../payloads/payload_tests.h"
#include "../../communication/sg_tcp.h"


START_TEST(testEyeballConversion) {
	SansgridSerial sg_serial;
	SansgridSerial sg_serial_back;
	SansgridEyeball sg_eyeball;
	PayloadTestStruct test_struct;
	char payload[100] = "";
	uint32_t rdid = ~0;

	mark_point();
	testStructInit(&test_struct);
	PayloadTestNode eyeball = { SG_TEST_COMM_WRITE_TCP, SG_DEVSTATUS_PECKING, 0 };
	test_struct.eyeball = &eyeball;
	test_struct.eyeball_mode = SG_EYEBALL_MATE;
	payloadMkSerial(&sg_serial);
	payloadMkEyeball(&sg_eyeball, &test_struct);

	memcpy(&sg_serial_back, &sg_serial, sizeof(SansgridSerial));

	mark_point();
	sgRouterToServerConvert(&sg_serial, payload);
	mark_point();
	rdid = sgServerToRouterConvert(payload, &sg_serial_back);

	mark_point();
	fail_if(memcmp(&sg_serial, &sg_serial_back, sizeof(SansgridSerial)),
			"Converted serial packet doesn't match original!");
}
END_TEST


START_TEST(testPeckConversion) {
	SansgridSerial sg_serial,
				   sg_serial_back;
}
END_TEST


Suite *intraRouterTestConversion(void) {
	Suite *s = suite_create("Intrarouter Conversion Tests");
	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, testEyeballConversion);

	suite_add_tcase(s, tc_core);

	return s;
}


// vim: ft=c ts=4 noet sw=4:

