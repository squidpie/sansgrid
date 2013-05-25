/* Tests for the routing table's authentication system
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
#include <stdlib.h>
#include <stdint.h>
#include <check.h>

#include <payloads.h>
#include "../../routing_table/routing_table.h"
#include "../../routing_table/auth_status.h"
#include "../../payload_handlers/payload_handlers.h"
#include "../tests.h"


/**
 * Test the authentication strictness
 */
START_TEST (testAuthStrictness) {
	// Test loose/strict authentication
	DeviceAuth *dev_auth = NULL;

	// init as loose
	dev_auth = deviceAuthInit(DEV_AUTH_LOOSE);
	fail_if((dev_auth == NULL), "Device Auth wasn't initialized");
	fail_if((deviceAuthIsEnabled(dev_auth) != DEV_AUTH_LOOSE), "Device Auth wasn't initialized to \"loose\"");
	// Set to strict
	deviceAuthEnable(dev_auth);
	fail_if((deviceAuthIsEnabled(dev_auth) != DEV_AUTH_STRICT), "Device Auth wasn't set to \"strict\"");
	// Set to filtered
	deviceAuthEnableFiltered(dev_auth);
	fail_if((deviceAuthIsEnabled(dev_auth) != DEV_AUTH_FILTERED), "Device Auth wasn't set to \"filtered\"");
	// Finally, set back to loose
	deviceAuthEnableLoosely(dev_auth);
	fail_if((deviceAuthIsEnabled(dev_auth) != DEV_AUTH_LOOSE), "Device Auth wasn't set to \"loose\"");

	deviceAuthDestroy(dev_auth);
} END_TEST


/**
 * Do a basic test to make sure the auth order is valid
 */
int testAuthOrder(enum SansgridDataTypeEnum dt[], enum SansgridDeviceStatusEnum gdt[], uint32_t size) {
	DeviceAuth *dev_auth = NULL;
	uint32_t i;

	char payload_got[30],
		 payload_expected[30];

	dev_auth = deviceAuthInit(1);
	fail_if((dev_auth == NULL), "Device Auth wasn't initialized");
	fail_if((deviceAuthIsEnabled(dev_auth) != 1), "Device Auth wasn't initialized to \"strict\"");

	for (i=1; i<size; i++) {
		sgPayloadGetPayloadName(gdt[i], payload_expected);
		sgPayloadGetPayloadName(sgPayloadGetType(dt[i]), payload_got);
		deviceAuthSetCurrentGeneralPayload(dev_auth, gdt[i-1]);
		deviceAuthSetNextGeneralPayload(dev_auth, gdt[i]);
		fail_if((deviceAuthIsSGPayloadTypeValid(dev_auth, dt[i]) == 0),
				"Control path is invalid! Getting: %s\tShould have gotten %s",
				payload_got, payload_expected);

	}
	return 0;
}

/**
 * This doesn't actually test the valid control flow
 * it just walks through what an authentication would look like,
 * making sure that the authentication syncs correctly
 */
START_TEST (testAuthOrderNoRecognize) {
	enum SansgridDataTypeEnum dt[] = {
		SG_EYEBALL,
		SG_PECK,
		SG_SING_WITH_KEY,
		SG_MOCK_WITH_KEY,
		SG_PEACOCK,
		SG_NEST,
		SG_CHIRP_DATA_SENSOR_TO_SERVER,
		SG_CHIRP_DATA_SENSOR_TO_SERVER,
	};

	enum SansgridDeviceStatusEnum gdt[] = {
		SG_DEVSTATUS_EYEBALLING,
		SG_DEVSTATUS_PECKING,
		SG_DEVSTATUS_SINGING,
		SG_DEVSTATUS_MOCKING,
		SG_DEVSTATUS_PEACOCKING,
		SG_DEVSTATUS_NESTING,
		SG_DEVSTATUS_CHIRPING,
		SG_DEVSTATUS_CHIRPING,
	};
	fail_if((sizeof(gdt)/sizeof(gdt[0]) != sizeof(dt)/sizeof(dt[0])), "Test specs problem: count(dt) != count(gdt)");
	testAuthOrder(dt, gdt, sizeof(gdt)/sizeof(gdt[0]));
	

} END_TEST;


/**
 * This doesn't actually test the valid control flow
 * it just walks through what an authentication would look like
 * making sure that the authentication syncs correctly
 */
START_TEST (testAuthOrderRecognized) {
	enum SansgridDataTypeEnum dt[] = {
		SG_EYEBALL,
		SG_PECK,
		SG_SQUAWK_SERVER_NOCHALLENGE_SENSOR,
		SG_SQUAWK_SENSOR_RESPOND_REQUIRE_CHALLENGE,
		SG_SQUAWK_SERVER_RESPOND,
		SG_SQUAWK_SENSOR_ACCEPT_RESPONSE,
		SG_NEST,
		SG_CHIRP_DATA_SENSOR_TO_SERVER,
		SG_CHIRP_DATA_SENSOR_TO_SERVER,
	};
	enum SansgridDeviceStatusEnum gdt[] = {
		SG_DEVSTATUS_EYEBALLING,
		SG_DEVSTATUS_PECKING,
		SG_DEVSTATUS_SQUAWKING,
		SG_DEVSTATUS_SQUAWKING,
		SG_DEVSTATUS_SQUAWKING,
		SG_DEVSTATUS_SQUAWKING,
		SG_DEVSTATUS_NESTING,
		SG_DEVSTATUS_CHIRPING,
		SG_DEVSTATUS_CHIRPING,
	};

	fail_if((sizeof(gdt)/sizeof(gdt[0]) != sizeof(dt)/sizeof(dt[0])), "Test specs problem: count(dt) != count(gdt)");
	testAuthOrder(dt, gdt, sizeof(gdt)/sizeof(gdt[0]));

} END_TEST;



/**
 * Routing Authentication Testing
 */
Suite *routingAuthBasicTestSuite (void) {
	Suite *s = suite_create("Basic route authentication testing");

	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, testAuthStrictness);
	tcase_add_test(tc_core, testAuthOrderNoRecognize);
	tcase_add_test(tc_core, testAuthOrderRecognized);

	suite_add_tcase(s, tc_core);

	return s;
}



// vim: ft=c ts=4 noet sw=4:


