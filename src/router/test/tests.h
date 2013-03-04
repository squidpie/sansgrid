#include <check.h>
#include <stdint.h>

#define TESTS_DEBUG_LEVEL 0

// General Debug
void routingTablePrint(uint8_t*);

// Routing
Suite *routingBasicTestSuite (void);
// Dispatch
Suite *dispatchBasicTesting (void);
Suite *dispatchAdvancedTesting (void);


// vim: ft=c ts=4 noet sw=4:
