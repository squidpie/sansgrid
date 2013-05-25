/* Tests for router<-->server communication
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
/// \file

/**
 * Test atox() one byte at a time
 */
START_TEST (testAtoxOneByte) {
	uint16_t expected;
	uint8_t hexarray;
	char str[4];

	// Test 0x00 .. 0xff
	for (expected = 0x00; expected <= 0xff; expected++) {
		snprintf(str, 3, "%.2x", expected);
		atox(&hexarray, str, 1);
		fail_unless((hexarray == expected), "Conversion Mismatch: Expected 0x%.2x\tGot 0x%.2x", expected, hexarray);
	}
}
END_TEST


/**
 * test atox() with multiple types
 */
START_TEST (testAtoxMulti) {
	int i, j;
	uint16_t incrementor;
	uint16_t expected[16];
	uint16_t expected_flipped[16];
	uint8_t expected_short[16];
	uint8_t expected_short_flipped[16];
	char str[34];
	char strin[3];
	char str_flipped[34];
	char strin_flipped[3];
	uint8_t hexarray[16];
	uint8_t hexarray_flipped[16];

	memset(expected, 0x0, sizeof(expected));
	memset(expected_short, 0x0, sizeof(expected_short));
	memset(expected_flipped, 0x0, sizeof(expected_flipped));
	memset(expected_short_flipped, 0x0, sizeof(expected_short_flipped));
	memset(hexarray_flipped, 0x0, sizeof(hexarray_flipped));

	for (incrementor = 0x0; incrementor < 0x100; incrementor++) {
		expected[0] = incrementor;
		expected_flipped[15] = incrementor;

		for (i=0; i<16; i++) {
			expected_short[i] = (expected[i] & 0xff);
			expected_short_flipped[i] = (expected_flipped[i] & 0xff);

			snprintf(strin, 3, "%.2x", expected_short[i]);
			snprintf(strin_flipped, 3, "%.2x", expected_short_flipped[i]);

			str[2*i] = strin[0];
			str[2*i+1] = strin[1];
			str[2*(i+1)] = '\0';
			str_flipped[2*i] = strin_flipped[0];
			str_flipped[2*i+1] = strin_flipped[1];
			str_flipped[2*(i+1)] = '\0';
		}
		atox(hexarray, str, 16*sizeof(uint8_t));
		atox(hexarray_flipped, str_flipped, 16*sizeof(uint8_t));

		str[32] = '\0';
		str_flipped[32] = '\0';

		for (j=0; j<16; j++) {
			fail_unless((hexarray[j] == expected_short[j]),
					"Conversion Mismatch at increment %i: index %i of %s: Expected 0x%x, Got 0x%x", 
					incrementor, j, str, expected_short[j], hexarray[j]);
			fail_unless((hexarray_flipped[j] == expected_short_flipped[j]),
					"Conversion Mismatch at increment %i: index %i of %s: Expected 0x%x, Got 0x%x", 
					incrementor, j, str_flipped, expected_short_flipped[j], hexarray_flipped[j]);
		}

		for (i=15; i>= 1; i--) {
			expected[i] = expected[i-1];
		}
		for (i=0; i<14; i++) {
			expected_flipped[i] = expected_flipped[i+1];
		}
#if TESTS_DEBUG_LEVEL > 0
		for (i=15; i>=1; i--) {
			printf("%.2x", expected[i]);
		}
		printf(": 0x%.2x\n", incrementor);
#endif
	}
}
END_TEST



/**
 * \brief atox() function testing
 *
 * atox() is a complex function. Thus, there is a testsuite specifically
 * dedicated to it. The tests make sure the conversions are correct.
 */
Suite *intraRouterTestAtox(void) {
	Suite *s = suite_create("Hex String Conversion Tests");
	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, testAtoxOneByte);
	tcase_add_test(tc_core, testAtoxMulti);

	suite_add_tcase(s, tc_core);

	return s;
}


// vim: ft=c ts=4 noet sw=4:

