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


Suite *intraRouterTestAtox(void) {
	Suite *s = suite_create("Hex String Conversion Tests");
	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, testAtoxOneByte);

	suite_add_tcase(s, tc_core);

	return s;
}


// vim: ft=c ts=4 noet sw=4:

