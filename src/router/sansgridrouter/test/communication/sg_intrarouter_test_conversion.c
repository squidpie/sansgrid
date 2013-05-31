/* Tests for router<-->server communication (conversion)
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

#include "sg_tcp_test.h"
#include "../payloads/payload_tests.h"
#include "../../communication/sg_tcp.h"
#include "../../routing_table/routing_table.h"
/// \file

/**
 * \brief Bitwise compare two serial structures for differences
 *
 * This checks to make sure a SansgridSerial structure is the same after being
 * converted to and from an intrarouter null-terminated string.
 * \param	testname	Name of the test
 * \param	sg_serial	Converted SansgridSerial structure
 * \param	sg_serial_orig	Original SansgridSerial structure
 * \param	rdid	new identifier
 * \param	rdid_orig	original identifier
 */
int checkSerialDiff(const char *testname, 
		SansgridSerial *sg_serial, 
		SansgridSerial *sg_serial_orig,
		uint32_t rdid, uint32_t rdid_orig) {
	// Check to see if sg_serial and sg_serial_orig are the same
	mark_point();
	fail_if((sg_serial->control != sg_serial_orig->control),
			"Control Byte mismatch: expected %u, got %u", 
			sg_serial_orig->control, sg_serial->control);
	fail_if(memcmp(sg_serial->payload, 
				sg_serial_orig->payload, 
				sizeof(SansgridNest)),
			"%s: Converted Payload doesn't match original!", testname);

	fail_if(memcmp(sg_serial->ip_addr, sg_serial_orig->ip_addr, IP_SIZE),
			"%s: IP address doesn't match original!", testname);
	fail_if((rdid != rdid_orig), 
			"%s: returned identifier doesn't match original!\
\n\tExpected: %i \
\n\tGot: %i", rdid_orig, rdid);
	fail_if(memcmp(sg_serial, sg_serial_orig, sizeof(SansgridSerial)),
			"%s: Converted serial packet doesn't match original!", testname);

	return 0;

}


/**
 * Test conversion using Eyeball Sansgrid Structure
 */
START_TEST(testEyeballConversion) {
	// Test the eyeball conversion for the intrarouter communication system
	int exit_code;
	SansgridSerial sg_serial,
				   sg_serial_orig;
	SansgridEyeball sg_eyeball;
	PayloadTestStruct test_struct;
	char payload[300] = "";
	uint32_t rdid = ~0,
			 rdid_orig = 0;
	uint8_t base[IP_SIZE];
	memset(base, 0x0, IP_SIZE);
	memset(&sg_serial, 0x0, sizeof(SansgridSerial));
	routing_table = routingTableInit(base, "Test ESSID");

	mark_point();
	testStructInit(&test_struct);
	PayloadTestNode eyeball = { SG_TEST_COMM_WRITE_TCP, SG_DEVSTATUS_PECKING, 0 };
	test_struct.eyeball = &eyeball;
	test_struct.eyeball_mode = SG_EYEBALL_MATE;
	mark_point();
	payloadMkSerial(&sg_serial);
	payloadMkEyeball(&sg_eyeball, &test_struct);

	mark_point();
	memcpy(sg_serial.payload, &sg_eyeball, sizeof(SansgridEyeball));

	mark_point();
	routingTableAssignIPStatic(routing_table, sg_serial.ip_addr);
	memcpy(&sg_serial_orig, &sg_serial, sizeof(SansgridSerial));
	rdid_orig = routingTableIPToRDID(routing_table, sg_serial.ip_addr);
	exit_code = sgRouterToServerConvert(&sg_serial, payload);
	mark_point();
	fail_if((exit_code != 0), "sgRouterToServer Failed! Expected: 0\tGot: %i", exit_code);
#if TESTS_DEBUG_LEVEL > 0
	printf("Eyeball: Size of payload is: %i\n", strlen(payload));
	printf("Eyeball: Converted to: %s\n", payload);
#endif
	mark_point();
	rdid = sgServerToRouterConvert(payload, &sg_serial);
	routingTableDestroy(routing_table);

	mark_point();
	checkSerialDiff("Eyeball", &sg_serial, &sg_serial_orig, rdid, rdid_orig);
	mark_point();
}
END_TEST;


/**
 * Test conversion using Peck Sansgrid Structure
 */
START_TEST(testPeckConversion) {
	// Test the peck conversion for the intrarouter communication system
	int exit_code;
	SansgridSerial sg_serial,
				   sg_serial_orig;
	SansgridPeck sg_peck;
	PayloadTestStruct test_struct;
	char payload[300] = "";
	uint32_t rdid = ~0,
			 rdid_orig = 0;
	uint8_t base[IP_SIZE];
	memset(base, 0x0, IP_SIZE);
	routing_table = routingTableInit(base, "Test ESSID");

	mark_point();
	testStructInit(&test_struct);
	test_struct.peck_mode = SG_PECK_MATE;
	mark_point();
	payloadMkSerial(&sg_serial);
	payloadMkPeck(&sg_peck, &test_struct);
	memcpy(sg_peck.assigned_ip, sg_serial.ip_addr, IP_SIZE);
	memset(sg_serial.ip_addr, 0xff, IP_SIZE);

	mark_point();
	memcpy(sg_serial.payload, &sg_peck, sizeof(SansgridPeck));
	memcpy(&sg_serial_orig, &sg_serial, sizeof(SansgridSerial));

	mark_point();
	routingTableAssignIPStatic(routing_table, sg_peck.assigned_ip);
	mark_point();
	rdid_orig = routingTableIPToRDID(routing_table, sg_peck.assigned_ip);

	mark_point();
	exit_code = sgRouterToServerConvert(&sg_serial, payload);
	mark_point();
	fail_if((exit_code != 0), "sgRouterToServer Failed! Expected: 0\tGot: %i", exit_code);
	mark_point();
#if TESTS_DEBUG_LEVEL > 0
	printf("Peck: Size of payload is: %i\n", strlen(payload));
	printf("Peck: Converted to: %s\n", payload);
#endif
	mark_point();
	rdid = sgServerToRouterConvert(payload, &sg_serial);
	routingTableDestroy(routing_table);

	mark_point();
	checkSerialDiff("Peck", &sg_serial, &sg_serial_orig, rdid, rdid_orig);
	mark_point();

}
END_TEST;


/**
 * Test conversion using Sing Sansgrid Structure
 */
START_TEST(testSingConversion) {
	// Test the sing conversion for the intrarouter communication system
	int exit_code;
	SansgridSerial sg_serial,
				   sg_serial_orig;
	SansgridSing sg_sing;
	PayloadTestStruct test_struct;
	char payload[300] = "";
	uint32_t rdid = ~0,
			 rdid_orig = 0;
	uint8_t base[IP_SIZE];
	memset(base, 0x0, IP_SIZE);
	routing_table = routingTableInit(base, "Test ESSID");

	mark_point();
	testStructInit(&test_struct);
	PayloadTestNode sing = { SG_TEST_COMM_WRITE_SPI, SG_DEVSTATUS_MOCKING, 0 };
	test_struct.sing = &sing;
	test_struct.sing_mode = SG_SING_WITH_KEY;
	mark_point();
	payloadMkSerial(&sg_serial);
	payloadMkSing(&sg_sing, &test_struct);

	mark_point();
	memcpy(sg_serial.payload, &sg_sing, sizeof(SansgridSing));
	memcpy(&sg_serial_orig, &sg_serial, sizeof(SansgridSerial));

	mark_point();
	routingTableAssignIPStatic(routing_table, sg_serial.ip_addr);
	mark_point();
	rdid_orig = routingTableIPToRDID(routing_table, sg_serial.ip_addr);

	mark_point();
	exit_code = sgRouterToServerConvert(&sg_serial, payload);
	fail_if((exit_code != 0), "sgRouterToServer Failed! Expected: 0\tGot: %i", exit_code);
#if TESTS_DEBUG_LEVEL > 0
	printf("Sing: Size of payload is: %i\n", strlen(payload));
	printf("Sing: Converted to: %s\n", payload);
#endif
	mark_point();
	rdid = sgServerToRouterConvert(payload, &sg_serial);
	routingTableDestroy(routing_table);

	mark_point();
	checkSerialDiff("Sing", &sg_serial, &sg_serial_orig, rdid, rdid_orig);
	mark_point();
}
END_TEST;


/**
 * Test conversion using Mock Sansgrid Structure
 */
START_TEST(testMockConversion) {
	// Test the mock conversion for the intrarouter communication system
	int exit_code;
	SansgridSerial sg_serial,
				   sg_serial_orig;
	SansgridMock sg_mock;
	PayloadTestStruct test_struct;
	char payload[300] = "";
	uint32_t rdid = ~0,
			 rdid_orig = 0;
	uint8_t base[IP_SIZE];
	memset(base, 0x0, IP_SIZE);
	routing_table = routingTableInit(base, "Test ESSID");

	mark_point();
	testStructInit(&test_struct);
	PayloadTestNode mock = { SG_TEST_COMM_WRITE_SPI, SG_DEVSTATUS_MOCKING, 0 };
	test_struct.mock = &mock;
	test_struct.mock_mode = SG_MOCK_WITH_KEY;
	payloadMkSerial(&sg_serial);
	payloadMkMock(&sg_mock, &test_struct);

	mark_point();
	memcpy(sg_serial.payload, &sg_mock, sizeof(SansgridMock));
	memcpy(&sg_serial_orig, &sg_serial, sizeof(SansgridSerial));

	mark_point();
	routingTableAssignIPStatic(routing_table, sg_serial.ip_addr);
	mark_point();
	rdid_orig = routingTableIPToRDID(routing_table, sg_serial.ip_addr);

	mark_point();
	exit_code = sgRouterToServerConvert(&sg_serial, payload);
	mark_point();
	fail_if((exit_code != 0), "sgRouterToServer Failed! Expected: 0\tGot: %i", exit_code);
#if TESTS_DEBUG_LEVEL > 0
	printf("Mock: Size of payload is: %i\n", strlen(payload));
	printf("Mock: Converted to: %s\n", payload);
#endif
	mark_point();
	rdid = sgServerToRouterConvert(payload, &sg_serial);
	mark_point();
	routingTableDestroy(routing_table);

	mark_point();
	checkSerialDiff("Mock", &sg_serial, &sg_serial_orig, rdid, rdid_orig);
	mark_point();
}
END_TEST;



/**
 * Test conversion using Peacock Sansgrid Structure
 */
START_TEST(testPeacockConversion) {
	// Test the peacock conversion for the intrarouter communication system
	int exit_code;
	SansgridSerial sg_serial,
				   sg_serial_orig;
	SansgridPeacock sg_peacock;
	PayloadTestStruct test_struct;
	char payload[300] = "";
	uint32_t rdid = ~0,
			 rdid_orig = 0;
	uint8_t base[IP_SIZE];
	memset(base, 0x0, IP_SIZE);
	routing_table = routingTableInit(base, "Test ESSID");


	mark_point();
	testStructInit(&test_struct);
	PayloadTestNode peacock = { SG_TEST_COMM_WRITE_TCP, SG_DEVSTATUS_NESTING, 0 };
	test_struct.peacock = &peacock;
	test_struct.peacock_mode = SG_PEACOCK;
	mark_point();
	payloadMkSerial(&sg_serial);
	payloadMkPeacock(&sg_peacock, &test_struct);

	mark_point();
	memcpy(sg_serial.payload, &sg_peacock, sizeof(SansgridPeacock));
	memcpy(&sg_serial_orig, &sg_serial, sizeof(SansgridSerial));

	mark_point();
	routingTableAssignIPStatic(routing_table, sg_serial.ip_addr);
	mark_point();
	rdid_orig = routingTableIPToRDID(routing_table, sg_serial.ip_addr);

	mark_point();
	exit_code = sgRouterToServerConvert(&sg_serial, payload);
	mark_point();
	memset(&sg_serial, 0x0, sizeof(SansgridSerial));
	fail_if((exit_code != 0), "sgRouterToServer Failed! Expected: 0\tGot: %i", exit_code);
#if TESTS_DEBUG_LEVEL > 0
	printf("Peacock: Size of payload is: %i\n", strlen(payload));
	printf("Peacock: Converted to: %s\n", payload);
#endif
	mark_point();
	rdid = sgServerToRouterConvert(payload, &sg_serial);
	routingTableDestroy(routing_table);

	mark_point();
	checkSerialDiff("Peacock", &sg_serial, &sg_serial_orig, rdid, rdid_orig);
	mark_point();
}
END_TEST;



/**
 * Test conversion using Nest Sansgrid Structure
 */
START_TEST(testNestConversion) {
	// Test the nest conversion for the intrarouter communication system
	int exit_code;
	SansgridSerial sg_serial,
				   sg_serial_orig;
	SansgridNest sg_nest;
	PayloadTestStruct test_struct;
	char payload[300] = "";
	uint32_t rdid = ~0,
			 rdid_orig = 0;
	uint8_t base[IP_SIZE];
	memset(base, 0x0, IP_SIZE);
	routing_table = routingTableInit(base, "Test ESSID");

	mark_point();
	testStructInit(&test_struct);
	PayloadTestNode nest = { SG_TEST_COMM_WRITE_SPI, SG_DEVSTATUS_MOCKING, 0 };
	test_struct.nest = &nest;
	test_struct.nest_mode = SG_NEST;
	mark_point();
	payloadMkSerial(&sg_serial);
	payloadMkNest(&sg_nest, &test_struct);

	mark_point();
	memcpy(sg_serial.payload, &sg_nest, sizeof(SansgridNest));
	memcpy(&sg_serial_orig, &sg_serial, sizeof(SansgridSerial));

	mark_point();
	routingTableAssignIPStatic(routing_table, sg_serial.ip_addr);
	mark_point();
	rdid_orig = routingTableIPToRDID(routing_table, sg_serial.ip_addr);

	mark_point();
	exit_code = sgRouterToServerConvert(&sg_serial, payload);
	mark_point();
	fail_if((exit_code != 0), "sgRouterToServer Failed! Expected: 0\tGot: %i", exit_code);
#if TESTS_DEBUG_LEVEL > 0
	printf("Nest: Size of payload is: %i\n", strlen(payload));
	printf("Nest: Converted to: %s\n", payload);
#endif
	mark_point();
	rdid = sgServerToRouterConvert(payload, &sg_serial);
	routingTableDestroy(routing_table);

	mark_point();
	checkSerialDiff("Nest", &sg_serial, &sg_serial_orig, rdid, rdid_orig);
	mark_point();
}
END_TEST;



/**
 * Test conversion using Squawk Sansgrid Structure
 */
START_TEST(testSquawkConversion) {
	// Test the squawk conversion for the intrarouter communication system
	int exit_code;
	SansgridSerial sg_serial,
				   sg_serial_orig;
	SansgridSquawk sg_squawk;
	PayloadTestStruct test_struct;
	char payload[300] = "";
	uint32_t rdid = ~0,
			 rdid_orig = 0;
	uint8_t base[IP_SIZE];
	memset(base, 0x0, IP_SIZE);
	routing_table = routingTableInit(base, "Test ESSID");

	mark_point();
	testStructInit(&test_struct);
	PayloadTestNode squawk = { SG_TEST_COMM_WRITE_SPI, SG_DEVSTATUS_MOCKING, 0 };
	test_struct.squawk_sensor = &squawk;
	test_struct.squawk_sensor_mode = SG_SQUAWK_SERVER_RESPOND;
	mark_point();
	payloadMkSerial(&sg_serial);
	payloadMkSquawkSensor(&sg_squawk, &test_struct);

	mark_point();
	memcpy(sg_serial.payload, &sg_squawk, sizeof(SansgridSquawk));
	memcpy(&sg_serial_orig, &sg_serial, sizeof(SansgridSerial));

	mark_point();
	routingTableAssignIPStatic(routing_table, sg_serial.ip_addr);
	mark_point();
	rdid_orig = routingTableIPToRDID(routing_table, sg_serial.ip_addr);

	mark_point();
	exit_code = sgRouterToServerConvert(&sg_serial, payload);
	mark_point();
	fail_if((exit_code != 0), "sgRouterToServer Failed! Expected: 0\tGot: %i", exit_code);
#if TESTS_DEBUG_LEVEL > 0
	printf("Squawk: Size of payload is: %i\n", strlen(payload));
	printf("Squawk: Converted to: %s\n", payload);
#endif
	mark_point();
	rdid = sgServerToRouterConvert(payload, &sg_serial);
	routingTableDestroy(routing_table);

	mark_point();
	checkSerialDiff("Squawk", &sg_serial, &sg_serial_orig, rdid, rdid_orig);
	mark_point();
}
END_TEST;



/**
 * \brief Intrarouter Conversion Unit tests
 *
 * Tests to check the integrity of conversion to/from a null-terminated
 * Sansgrid intrarouter string
 */
Suite *intraRouterTestConversion(void) {
	Suite *s = suite_create("Intrarouter Conversion Tests");
	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, testEyeballConversion);
	tcase_add_test(tc_core, testPeckConversion);
	tcase_add_test(tc_core, testSingConversion);
	tcase_add_test(tc_core, testMockConversion);
	tcase_add_test(tc_core, testPeacockConversion);
	tcase_add_test(tc_core, testNestConversion);
	tcase_add_test(tc_core, testSquawkConversion);

	suite_add_tcase(s, tc_core);

	return s;
}


// vim: ft=c ts=4 noet sw=4:

