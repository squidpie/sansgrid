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
#include <syslog.h>
#include <getopt.h>

#include "tests.h"
#include "../routing_table/routing_table.h"
#include "communication/sg_communication_stubs.h"
/// \file

int usage(int exit_code);


/**
 * \brief Pretty Print the IP address
 */
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



/**
 * \brief Setup check unit testing
 */
Suite *makeMasterSuite (void) {
	Suite *s = suite_create("Master Testing Suite");


	return s;
}



/**
 * \brief check.h unit test runtime
 */
int main(int argc, char *argv[]) {
	int number_failed;
	int c;
	int verbosity = LOG_ERR;

	SRunner *sr;
   	sr = srunner_create(makeMasterSuite());
	// routing tests
	srunner_add_suite(sr, routingBasicTestSuite());
	srunner_add_suite(sr, routingAuthBasicTestSuite());
	// dispatch tests
	srunner_add_suite(sr, dispatchBasicTesting());
	srunner_add_suite(sr, dispatchAdvancedTesting());
	// payload tests
	srunner_add_suite(sr, payloadSizeTesting());
	srunner_add_suite(sr, payloadTestEyeball());
	srunner_add_suite(sr, payloadTestPeck());
	srunner_add_suite(sr, payloadTestSing());
	srunner_add_suite(sr, payloadTestMock());
	srunner_add_suite(sr, payloadTestPeacock());
	srunner_add_suite(sr, payloadTestSquawk());
	srunner_add_suite(sr, payloadTestNest());
	srunner_add_suite(sr, payloadTestChirp());
	// intrarouter tests
	srunner_add_suite(sr, intraRouterTestAtox());
	srunner_add_suite(sr, intraRouterTestConversion());

	num_devices = 0;

	while (1) {
		const struct option long_options[] = {
			{"nofork",    no_argument,     0,        'n'},
			{"verbose",   no_argument,     0,        'v'},
			{"help",      no_argument,     0,        'h'},
			{0, 0, 0, 0}
		};
		int option_index = 0;

		c = getopt_long(argc, argv, "nvh", long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
			case '0':
				if (long_options[option_index].flag != 0)
					break;
				printf("option %s ", long_options[option_index].name);
				if (optarg)
					printf("With arg %s", optarg);
				printf("\n");
				break;
			case 'n':
				// Don't fork off
				srunner_set_fork_status(sr, CK_NOFORK);
				break;
			case 'v':
				verbosity++;
				break;
			case 'h':
				usage(EXIT_SUCCESS);
				break;
			case '?':
				// getopt_long already printed an error message
				exit(EXIT_FAILURE);
				break;
			default: 
				abort();
		}
	}



	setlogmask(LOG_UPTO(verbosity));

	// Uncomment to better debug segfaults
	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/**
 * \brief check.h unit testing usage
 */
int usage(int status) {
	if (status != EXIT_SUCCESS)
		printf("Try sgtest -h\n");
	else {
		printf("Usage: sgtest [OPTION]\n");
		printf("\
  -n  --nofork               don't fork off (good for debugging segfaults\n\
  -v  --verbose              Be verbose (warnings)\n\
  -vv                        Be more verbose (notices)\n\
  -vvv                       Be even more verbose (info)\n\
  -vvvv                      Be very very verbose (debug)\n\
  -h, --help                 display this help and exit\n");
	}
	exit(status);
}


// vim: ft=c ts=4 noet sw=4:

