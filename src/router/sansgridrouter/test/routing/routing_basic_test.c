/* Tests for the routing table
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
/// \file

#include <stdio.h>
#include <stdint.h>
#include <check.h>
#include <stdlib.h>

#include "../../routing_table/routing_table.h"
#include "../tests.h"


/**
 * Tests endianness
 * Returns 1 if little endian
 * Returns 0 if big endian
 */
int32_t littleEndian(void) {
	// Tests endianness
	int i = 1;
	char *p = (char*)&i;

	return (p[0] == 1);
}



/**
 * \brief unit test code for checking endianness of the machine
 *
 * this is a quick test to check for endianness
 */
START_TEST (testEndianness) {
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


/**
 * Test adding a device to the routing table
 */
START_TEST (testRoutingTableAdd) {
	int i;
	uint8_t ip_addr[IP_SIZE];
	uint8_t base[IP_SIZE];
	RoutingTable *table;
	
	for (i=0; i<IP_SIZE; i++)
		base[i] = 0x0;

	table = routingTableInit(base, "Testing ESSID");


	for (i=0; i<32; i++) {
		routingTableAssignIP(table, ip_addr);
#if TESTS_DEBUG_LEVEL > 0
		routingTablePrint(ip_addr);
#endif
	}
	table = routingTableDestroy(table);
}
END_TEST




/**
 * Test looking up a device in the routing table
 */
START_TEST (testRoutingTableLookup) {
	int i;
	uint8_t ip_addr[IP_SIZE];
	uint8_t base[IP_SIZE];
	RoutingTable *table;

	for (i=0; i<IP_SIZE; i++)
		base[i] = 0x0;

	table = routingTableInit(base, "Testing ESSID");


	for (i=0; i<32; i++) {
		routingTableAssignIP(table, ip_addr);
#if TESTS_DEBUG_LEVEL > 0
		routingTablePrint(ip_addr);
#endif
	}
	fail_unless((routingTableLookup(table, ip_addr)),
			"Error: IP Address not Resident!");
#if TESTS_DEBUG_LEVEL > 0
	if (routingTableLookup(table, ip_addr)) {
		printf("IP Address is resident: ");
		routingTablePrint(ip_addr);
	}
#endif
	table = routingTableDestroy(table);
}
END_TEST



/**
 * Test the routing table
 */
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

