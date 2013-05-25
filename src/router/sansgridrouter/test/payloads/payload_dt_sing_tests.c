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

/// Sing payload testing
int testSingPayload(PayloadTestStruct *test_struct) {
	// Call Sing tests with all options
	int exit_code;
	PayloadTestNode eyeball = { SG_TEST_COMM_WRITE_TCP, SG_DEVSTATUS_PECKING, 0};
	PayloadTestNode peck = { SG_TEST_COMM_WRITE_SPI, SG_DEVSTATUS_SINGING, 0};
	PayloadTestNode sing = { SG_TEST_COMM_WRITE_SPI, SG_DEVSTATUS_MOCKING, 0};

	// Set defaults
	sing.read_dir = SG_TEST_COMM_WRITE_SPI;
	sing.next_packet = SG_DEVSTATUS_MOCKING;
	sing.expected_exit_code = 0;
	test_struct->eyeball_mode = SG_EYEBALL_MATE;
	test_struct->peck_mode = SG_PECK_MATE;
	// Assign nodes
	test_struct->eyeball = &eyeball;
	test_struct->peck = &peck;
	test_struct->sing = &sing;

	// Test server authenticating with key
	test_struct->sing_mode = SG_SING_WITH_KEY;
	exit_code = testPayload(test_struct);
	if (exit_code)
		return exit_code;

	// Test server authenticating without key
	test_struct->sing_mode = SG_SING_WITHOUT_KEY;
	exit_code = testPayload(test_struct);

	return exit_code;
}


/// Unit test for sing payloads
START_TEST (testSing) {
#if TESTS_DEBUG_LEVEL > 0
	printf("\n\nTesting Singing\n");
#endif
	PayloadTestStruct test_struct;
	testStructInit(&test_struct);
	testSingPayload(&test_struct);
#if TESTS_DEBUG_LEVEL > 0
	printf("Successfully Sung\n");
#endif
}
END_TEST;


/// Unit tests for sing payloads
Suite *payloadTestSing(void) {
	Suite *s = suite_create("Sing Payload Tests");
	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, testSing);

	suite_add_tcase(s, tc_core);

	return s;
}


// vim: ft=c ts=4 noet sw=4:


