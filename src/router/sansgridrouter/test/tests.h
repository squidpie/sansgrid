/* Tests
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

#ifndef __SANSGRID_ROUTER_TESTS_H__
#define __SANSGRID_ROUTER_TESTS_H__

#include <check.h>
#include <stdint.h>

#define TESTS_DEBUG_LEVEL 0
uint32_t num_devices;				// number of devices being tested


// General Debug
void routingTablePrint(uint8_t*);

// Routing
Suite *routingBasicTestSuite (void);
// Dispatch
Suite *dispatchBasicTesting (void);
Suite *dispatchAdvancedTesting (void);
// Payload size
Suite *payloadSizeTesting (void);
// Payload tests
Suite *payloadTestEyeball(void);
Suite *payloadTestPeck(void);
Suite *payloadTestSing(void);
Suite *payloadTestMock(void);
Suite *payloadTestPeacock(void);
Suite *payloadTestSquawk(void);
Suite *payloadTestNest(void);
Suite *payloadTestChirp(void);
// intrarouter tests
Suite *intraRouterTestAtox(void);
Suite *intraRouterTestConversion(void);


#endif

// vim: ft=c ts=4 noet sw=4:
