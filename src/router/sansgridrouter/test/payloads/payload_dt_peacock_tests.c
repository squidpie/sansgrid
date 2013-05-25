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

/// Peacock payload testing
int testPeacockPayload(PayloadTestStruct *test_struct) {
	// Call peacock tests with all valid options
	int exit_code;
	PayloadTestNode peacock = { SG_TEST_COMM_WRITE_TCP, SG_DEVSTATUS_NESTING, 0};
	test_struct->peacock = &peacock;
	test_struct->peacock_mode = SG_PEACOCK;
	exit_code = testMockPayload(test_struct);
	return exit_code;
}


/// Unit test for testing peacock payloads
START_TEST (testPeacock) {
#if TESTS_DEBUG_LEVEL > 0
	printf("\n\nTesting Peacocking\n");
#endif
	PayloadTestStruct test_struct;
	testStructInit(&test_struct);
	testPeacockPayload(&test_struct);
#if TESTS_DEBUG_LEVEL > 0
	printf("Successfully Peacocked\n");
#endif
}
END_TEST;


/// Unit tests for peacock payloads
Suite *payloadTestPeacock(void) {
	Suite *s = suite_create("Peacock Payload Tests");
	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, testPeacock);

	suite_add_tcase(s, tc_core);

	return s;
}




// vim: ft=c ts=4 noet sw=4:


