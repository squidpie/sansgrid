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
/// \file
#include "payload_tests.h"



/// Test a Sensor-->Server Chirp Payload
int testChirpPayloadSensorToServer(PayloadTestStruct *test_struct) {
	// Call Chirp tests from sensor to server with all valid options
	PayloadTestNode chirp = { SG_TEST_COMM_WRITE_TCP, SG_DEVSTATUS_LEASED, 0};
	test_struct->chirp = &chirp;

	test_struct->chirp_mode = SG_CHIRP_DATA_SENSOR_TO_SERVER;
	return testNestPayload(test_struct);

}


/// Test a Server-->Sensor Chirp Payload
int testChirpPayloadServerToSensor(PayloadTestStruct *test_struct) {
	// Call Chirp tests from server to sensor with all valid options
	PayloadTestNode chirp = { SG_TEST_COMM_WRITE_SPI, SG_DEVSTATUS_LEASED, 0};
	test_struct->chirp = &chirp;

	test_struct->chirp_mode = SG_CHIRP_COMMAND_SERVER_TO_SENSOR;
	return testNestPayload(test_struct);
}


/// Test Sensor-->Server (toplevel)
START_TEST (testChirpSensorToServer) {
#if TESTS_DEBUG_LEVEL > 0
	printf("\n\nTesting Chirping (Sensor to Server)\n");
#endif
	PayloadTestStruct test_struct;
	testStructInit(&test_struct);
	testChirpPayloadSensorToServer(&test_struct);
#if TESTS_DEBUG_LEVEL > 0
	printf("Successfully Chirped (Sensor to Server)\n");
#endif
}
END_TEST;

/// Test Server-->Sensor (toplevel)
START_TEST (testChirpServerToSensor) {
#if TESTS_DEBUG_LEVEL > 0
	printf("\n\nTesting Chirping (Server to Sensor)\n");
#endif
	PayloadTestStruct test_struct;
	testStructInit(&test_struct);
	testChirpPayloadServerToSensor(&test_struct);
#if TESTS_DEBUG_LEVEL > 0
	printf("Successfully Chirped (Server to Sensor)\n");
#endif
}
END_TEST;


/// Test Chirp Payloads
Suite *payloadTestChirp(void) {
	Suite *s = suite_create("Chirp Payload Tests");
	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, testChirpSensorToServer);
	tcase_add_test(tc_core, testChirpServerToSensor);

	suite_add_tcase(s, tc_core);

	return s;
}





// vim: ft=c ts=4 noet sw=4:


