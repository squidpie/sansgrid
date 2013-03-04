/* A quick implementation for a statically-defined table.
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
 * This implementation defines the table size at compile-time instead of at run-time,
 * making the system less prone to bugs. The table is an array of pointers, which
 * facilitates creating/moving/deleting IP addresses.
 */

#include <stdio.h>
#include <stdint.h>
#include <check.h>
#include <stdlib.h>

#include "../../routing/routing.h"
#include "../tests.h"




START_TEST (testEndianness) {
	// unit test code for checking endianness of the machine
#if TESTS_DEBUG_LEVEL > 0
	if (littleEndian()) {
		printf("Machine is Little Endian\n");
		printf("Conversion is required.\n");
	} else {
		printf("Machine is Big Endian\n");
		printf("No Conversion is required.\n");
	}
#endif
}
END_TEST


START_TEST (testRoutingTableAdd) {
	int i;
	uint8_t ip_addr[IP_SIZE];
	routingTableInit();


	for (i=0; i<32; i++) {
		routingTableAssignIP(ip_addr);
#if TESTS_DEBUG_LEVEL > 0
		routingTablePrint(ip_addr);
#endif
	}
	routingTableDestroy();
}
END_TEST

START_TEST (testRoutingTableLookup) {
	int i;
	uint8_t ip_addr[IP_SIZE];
	routingTableInit();


	for (i=0; i<32; i++) {
		routingTableAssignIP(ip_addr);
#if TESTS_DEBUG_LEVEL > 0
		routingTablePrint(ip_addr);
#endif
	}
	fail_unless((routingTableLookup(ip_addr)),
			"Error: IP Address not Resident!");
#if TESTS_DEBUG_LEVEL > 0
	if (routingTableLookup(ip_addr)) {
		printf("IP Address is resident: ");
		routingTablePrint(ip_addr);
	}
#endif
	routingTableDestroy();
}
END_TEST



Suite *routingBasicTestSuite (void) {
	Suite *s = suite_create("Basic route testing");

	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, testEndianness);
	tcase_add_test(tc_core, testRoutingTableAdd);
	tcase_add_test(tc_core, testRoutingTableLookup);

	suite_add_tcase(s, tc_core);

	return s;
}



// vim: ft=c ts=4 noet sw=4:

