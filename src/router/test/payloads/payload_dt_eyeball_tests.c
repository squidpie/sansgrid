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
#include "payload_tests.h"

int testEyeballPayload(PayloadTestStruct *test_struct) {
	// Call Eyeball tests with all options
	// Mate, NoMate
	// Next payload is always pecking
	int exit_code;
	PayloadTestNode eyeball = { SG_TEST_COMM_WRITE_TCP, SG_DEVSTATUS_PECKING };
	test_struct->eyeball = &eyeball;
	test_struct->eyeball_mode = SG_EYEBALL_MATE;
	exit_code = testPayload(test_struct);
	if (exit_code)
		return exit_code;
	// FIXME: This path is not implemented yet
	//test_struct->eyeball_mode = SG_EYEBALL_NOMATE;
	//testPayload(test_struct);
	return 0;
}



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
END_TEST



Suite *payloadTestEyeball(void) {
	Suite *s = suite_create("Eyeball Payload Tests");
	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, testEyeball);

	suite_add_tcase(s, tc_core);

	return s;
}


// vim: ft=c ts=4 noet sw=4:


