/* Core Test File
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

#include <stdio.h>
#include <check.h>
#include <stdlib.h>
#include <stdint.h>

#include "tests.h"
#include "../routing/routing.h"


void routingTablePrint(uint8_t ip_addr[IP_SIZE]) {
	// Print the IP address like an IPv6 address
	int i;
	for (i=0; i<IP_SIZE; i++) {
		printf("%.2X", ip_addr[i]);
		if (i < IP_SIZE-1)
			printf(":");
	}
	printf("\n");
	return;
}



Suite *makeMasterSuite (void) {
	Suite *s = suite_create("Master Testing Suite");


	return s;
}



int main(void) {
	int number_failed;


	SRunner *sr;
   	sr = srunner_create(makeMasterSuite());
	srunner_add_suite(sr, routingBasicTestSuite());
	srunner_add_suite(sr, dispatchBasicTesting());
	srunner_add_suite(sr, dispatchAdvancedTesting());
	srunner_add_suite(sr, payloadSizeTesting());
	srunner_add_suite(sr, payloadTesting());

	// Uncomment to better debug segfaults
	srunner_set_fork_status(sr, CK_NOFORK);
	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}



// vim: ft=c ts=4 noet sw=4:

