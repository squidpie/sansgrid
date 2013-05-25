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


/// Test a mock payload
int testMockPayload(PayloadTestStruct *test_struct) {
	// Call mock tests with all options
	int exit_code;
	PayloadTestNode mock = { SG_TEST_COMM_WRITE_TCP, SG_DEVSTATUS_PEACOCKING, 0 };
	test_struct->mock = &mock;

	// Test with key
	test_struct->mock_mode = SG_MOCK_WITH_KEY;
	exit_code = testSingPayload(test_struct);
	if (exit_code)
		return exit_code;

	// Test without key
	test_struct->mock_mode = SG_MOCK_WITHOUT_KEY;
	exit_code = testSingPayload(test_struct);
	return exit_code;
}



/// Unit test for a mock payload
START_TEST (testMock) {
#if TESTS_DEBUG_LEVEL > 0
	printf("\n\nTesting Mocking\n");
#endif
	PayloadTestStruct test_struct;
	testStructInit(&test_struct);
	testMockPayload(&test_struct);
#if TESTS_DEBUG_LEVEL > 0
	printf("Successfully Mocked\n");
#endif
}
END_TEST;


/// Mock unit testing
Suite *payloadTestMock(void) {
	Suite *s = suite_create("Mock Payload Tests");
	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, testMock);

	suite_add_tcase(s, tc_core);

	return s;
}




// vim: ft=c ts=4 noet sw=4:


