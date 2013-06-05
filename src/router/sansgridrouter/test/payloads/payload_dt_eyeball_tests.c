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
 */
/// \file
#include "payload_tests.h"


/// Test an eyeball payload
int testEyeballPayload(PayloadTestStruct *test_struct) {
	// Call Eyeball tests with all options
	// Mate, NoMate
	// Next payload could be pecking, or no packet
	int exit_code;
	PayloadTestNode eyeball = { SG_TEST_COMM_WRITE_TCP, SG_DEVSTATUS_PECKING, 0 };
	test_struct->eyeball = &eyeball;
	test_struct->eyeball_mode = SG_EYEBALL_MATE;
	exit_code = testPayload(test_struct);
	if (exit_code)
		return exit_code;
	test_struct->eyeball_mode = SG_EYEBALL_NOMATE;
	eyeball.read_dir = SG_TEST_COMM_WRITE_TCP;
	eyeball.expected_exit_code = 0;
	test_struct->eyeball = &eyeball;
	return testPayload(test_struct);
}




/// Test an eyeball payload
START_TEST (testEyeball) {
#if TESTS_DEBUG_LEVEL > 0
	printf("\n\nTesting Eyeball\n");
#endif
	PayloadTestStruct test_struct;
	testStructInit(&test_struct);
	testEyeballPayload(&test_struct);
#if TESTS_DEBUG_LEVEL > 0
	printf("Successfully Eyeballed\n");
#endif
}
END_TEST;


/// Test multiple eyeball payloads
START_TEST (testMultEyeballs) {
	int num_defined_here = 0;
	int old_num_devices;
	if (num_devices == 0) {
		old_num_devices = num_devices;
		num_devices = 20;
		num_defined_here = 1;
	}
#if TESTS_DEBUG_LEVEL > 0
	printf("\n\nTesting Multiple Eyeballs\n");
#endif
	PayloadTestStruct test_struct[num_devices];
	for (uint32_t i=0; i<num_devices; i++) {
		testStructInit(&test_struct[i]);
		testEyeballPayload(&test_struct[i]);
	}
	if (num_defined_here)
		num_devices = old_num_devices;
} END_TEST;


/// Eyeball tests
Suite *payloadTestEyeball(void) {
	Suite *s = suite_create("Eyeball Payload Tests");
	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, testEyeball);

	TCase *tc_mult = tcase_create("Multiple");
	tcase_add_test(tc_mult, testMultEyeballs);

	suite_add_tcase(s, tc_core);
	suite_add_tcase(s, tc_mult);

	return s;
}


// vim: ft=c ts=4 noet sw=4:


