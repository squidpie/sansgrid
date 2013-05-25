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


/// Test a nesting payload
int testNestPayload(PayloadTestStruct *test_struct) {
	// Call Nest tests with all valid options
	int exit_code;
	PayloadTestStruct test_struct_copy;
	PayloadTestNode nest = { SG_TEST_COMM_WRITE_SPI, SG_DEVSTATUS_LEASED, 0};
	test_struct->nest = &nest;
	test_struct->nest_mode = SG_NEST;
	// Non-recognition path
	memcpy(&test_struct_copy, test_struct, sizeof(PayloadTestStruct));
	exit_code = testPeacockPayload(test_struct);
	if (exit_code)
		return exit_code;
	// Recognition, both auth path
	memcpy(test_struct, &test_struct_copy, sizeof(PayloadTestStruct));
	exit_code = testSquawkPayloadAuthBoth(test_struct);
	if (exit_code)
		return exit_code;
	// Recognition, Server auth path
	memcpy(test_struct, &test_struct_copy, sizeof(PayloadTestStruct));
	exit_code = testSquawkPayloadAuthServer(test_struct);
	if (exit_code)
		return exit_code;
	// Recognition, Sensor auth path
	memcpy(test_struct, &test_struct_copy, sizeof(PayloadTestStruct));
	exit_code = testSquawkPayloadAuthSensor(test_struct);
	if (exit_code)
		return exit_code;
	// Recognition, No auth path
	memcpy(test_struct, &test_struct_copy, sizeof(PayloadTestStruct));
	exit_code = testSquawkPayloadNoAuth(test_struct);

	return exit_code;
}



/// Unit testing for nest payloads
START_TEST (testNest) {
#if TESTS_DEBUG_LEVEL > 0
	printf("\n\nTesting Nesting\n");
#endif
	PayloadTestStruct test_struct;
	testStructInit(&test_struct);
	testNestPayload(&test_struct);
#if TESTS_DEBUG_LEVEL > 0
	printf("Successfully Nested\n");
#endif
}
END_TEST;


/// Nest payload unit testing
Suite *payloadTestNest(void) {
	Suite *s = suite_create("Nest Payload Tests");
	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, testNest);

	suite_add_tcase(s, tc_core);

	return s;
}




// vim: ft=c ts=4 noet sw=4:


