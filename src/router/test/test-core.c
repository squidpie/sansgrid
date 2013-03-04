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

	//TCase *tc_core = tcase_create("Core");
	//tcase_add_test(tc_core, testAdvancedDispatch);

	//suite_add_tcase(s, tc_core);

	return s;
}



int main(void) {
	int number_failed;

	//Suite *s = routingBasicTestSuite();
	//Suite *dispatch_advanced_s = dispatchAdvancedTesting();
	SRunner *sr = srunner_create(makeMasterSuite());
	srunner_add_suite(sr, routingBasicTestSuite());
	srunner_add_suite(sr, dispatchBasicTesting());
	srunner_add_suite(sr, dispatchAdvancedTesting());
	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
	/*
	int i;
	uint8_t ip_addr[IP_SIZE];

	routingTableInit();


	for (i=0; i<32; i++) {
		routingTableAssignIP(ip_addr);
		routingTablePrint(ip_addr);
	}

	if (routingTableLookup(ip_addr)) {
		printf("IP Address is resident: ");
		routingTablePrint(ip_addr);
	}

	if (routingTableFreeIP(ip_addr))
		printf("Oops!\n");


	routingTableDestroy();
	return 0;
	*/
}



// vim: ft=c ts=4 noet sw=4:

