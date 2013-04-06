/* Payload Tests
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
 */
#include "payload_tests.h"


int testSquawkPayloadAuthBoth(PayloadTestStruct *test_struct) {
	// Call squawk tests with all valid options
	//int exit_code;
	PayloadTestNode squawk_server = {SG_TEST_COMM_WRITE_SPI, SG_DEVSTATUS_SQUAWKING, 0},
					squawk_sensor = {SG_TEST_COMM_WRITE_TCP, SG_DEVSTATUS_SQUAWKING, 0};
	PayloadTestNode peck = { SG_TEST_COMM_WRITE_SPI, SG_DEVSTATUS_SQUAWKING, 0};
	test_struct->squawk_server = &squawk_server;
	test_struct->squawk_sensor = &squawk_sensor;
	test_struct->peck = &peck;
	test_struct->peck_mode = SG_PECK_RECOGNIZED;

	// Server Challenges sensor
	test_struct->squawk_server_mode = SG_SQUAWK_SERVER_CHALLENGE_SENSOR;

	// Sensor Challenges sever
	test_struct->squawk_sensor_mode = SG_SQUAWK_SENSOR_CHALLENGE_SERVER;

	return testEyeballPayload(test_struct);
}


int testSquawkPayloadAuthSensor(PayloadTestStruct *test_struct) {
	// Call squawk: sensor requires authentication
	// with all valid options
	//int exit_code;
	PayloadTestNode squawk_server = {SG_TEST_COMM_WRITE_SPI, SG_DEVSTATUS_SQUAWKING, 0},
					squawk_sensor = {SG_TEST_COMM_WRITE_TCP, SG_DEVSTATUS_SQUAWKING, 0};
	PayloadTestNode peck = { SG_TEST_COMM_WRITE_SPI, SG_DEVSTATUS_SQUAWKING, 0};
	test_struct->squawk_server = &squawk_server;
	test_struct->squawk_sensor = &squawk_sensor;
	test_struct->peck = &peck;
	test_struct->peck_mode = SG_PECK_RECOGNIZED;

	// Server Doesn't challenge sensor
	test_struct->squawk_server_mode = SG_SQUAWK_SERVER_NOCHALLENGE_SENSOR;

	// Sensor Challenges server
	test_struct->squawk_sensor_mode = SG_SQUAWK_SENSOR_CHALLENGE_SERVER;

	return testEyeballPayload(test_struct);
}
	

int testSquawkPayloadAuthServer(PayloadTestStruct *test_struct) {
	// Call squawk: server requires authentication
	// with all valid options
	//int exit_code;
	PayloadTestNode squawk_server = {SG_TEST_COMM_WRITE_SPI, SG_DEVSTATUS_SQUAWKING, 0},
					squawk_sensor = {SG_TEST_COMM_WRITE_TCP, SG_DEVSTATUS_SQUAWKING, 0};
	PayloadTestNode peck = { SG_TEST_COMM_WRITE_SPI, SG_DEVSTATUS_SQUAWKING };
	test_struct->squawk_server = &squawk_server;
	test_struct->squawk_sensor = &squawk_sensor;
	test_struct->peck = &peck;
	test_struct->peck_mode = SG_PECK_RECOGNIZED;

	// Server Doesn't challenge sensor
	test_struct->squawk_server_mode = SG_SQUAWK_SERVER_CHALLENGE_SENSOR;

	// Sensor Challenges server
	test_struct->squawk_sensor_mode = SG_SQUAWK_SENSOR_RESPOND_NO_REQUIRE_CHALLENGE;

	return testEyeballPayload(test_struct);
}


int testSquawkPayloadNoAuth(PayloadTestStruct *test_struct) {
	// Call squawk: No authentication
	// with all valid options
	//int exit_code;
	PayloadTestNode squawk_server = {SG_TEST_COMM_WRITE_SPI, SG_DEVSTATUS_SQUAWKING, 0},
					squawk_sensor = {SG_TEST_COMM_WRITE_TCP, SG_DEVSTATUS_SQUAWKING, 0};
	PayloadTestNode peck = { SG_TEST_COMM_WRITE_SPI, SG_DEVSTATUS_SQUAWKING };
	test_struct->squawk_server = &squawk_server;
	test_struct->squawk_sensor = &squawk_sensor;
	test_struct->peck = &peck;
	test_struct->peck_mode = SG_PECK_RECOGNIZED;

	// Server Doesn't challenge sensor
	test_struct->squawk_server_mode = SG_SQUAWK_SERVER_NOCHALLENGE_SENSOR;

	// Sensor Challenges server
	test_struct->squawk_sensor_mode = SG_SQUAWK_SENSOR_RESPOND_NO_REQUIRE_CHALLENGE;

	return testEyeballPayload(test_struct);
}



START_TEST (testSquawkAuthBoth) {
#if TESTS_DEBUG_LEVEL > 0
	printf("\n\nTesting Squawking (Both Authenticate)\n");
#endif
	PayloadTestStruct test_struct;
	// Test both authenticate
	testStructInit(&test_struct);
	testSquawkPayloadAuthBoth(&test_struct);
#if TESTS_DEBUG_LEVEL > 0
	printf("Successfully Squawked\n");
#endif
}
END_TEST


START_TEST (testSquawkAuthSensor) {
#if TESTS_DEBUG_LEVEL > 0
	printf("\n\nTesting Squawking (Sensor Authenticates)\n");
#endif
	PayloadTestStruct test_struct;
	// Test sensor authenticates
	testStructInit(&test_struct);
	testSquawkPayloadAuthSensor(&test_struct);
#if TESTS_DEBUG_LEVEL > 0
	printf("Successfully Squawked\n");
#endif
}
END_TEST


START_TEST (testSquawkAuthServer) {
#if TESTS_DEBUG_LEVEL > 0
	printf("\n\nTesting Squawking (Server Authenticates)\n");
#endif
	PayloadTestStruct test_struct;
	// Test server authenticates
	testStructInit(&test_struct);
	testSquawkPayloadAuthServer(&test_struct);
#if TESTS_DEBUG_LEVEL > 0
	printf("Successfully Squawked\n");
#endif
}
END_TEST


START_TEST (testSquawkNoAuth) {
#if TESTS_DEBUG_LEVEL > 0
	printf("\n\nTesting Squawking (Neither Authenticate)\n");
#endif
	PayloadTestStruct test_struct;
	// Test no authentication
	testStructInit(&test_struct);
	testSquawkPayloadNoAuth(&test_struct);
#if TESTS_DEBUG_LEVEL > 0
	printf("Successfully Squawked\n");
#endif
}
END_TEST



Suite *payloadTestSquawk(void) {
	Suite *s = suite_create("Squawk Payload Tests");
	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, testSquawkAuthBoth);
	tcase_add_test(tc_core, testSquawkAuthSensor);
	tcase_add_test(tc_core, testSquawkAuthServer);
	tcase_add_test(tc_core, testSquawkNoAuth);

	suite_add_tcase(s, tc_core);

	return s;
}



// vim: ft=c ts=4 noet sw=4:


