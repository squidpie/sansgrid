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
	routing_table = routingTableInit(base, "Test ESSID");

	mark_point();
	testStructInit(&test_struct);
	PayloadTestNode eyeball = { SG_TEST_COMM_WRITE_TCP, SG_DEVSTATUS_PECKING, 0 };
	test_struct.eyeball = &eyeball;
	test_struct.eyeball_mode = SG_EYEBALL_MATE;
	payloadMkSerial(&sg_serial);
	routingTableGetBroadcast(routing_table, sg_serial.ip_addr);
	payloadMkEyeball(&sg_eyeball, &test_struct);

	memcpy(sg_serial.payload, &sg_eyeball, sizeof(SansgridEyeball));
	memcpy(&sg_serial_orig, &sg_serial, sizeof(SansgridSerial));

	mark_point();
	exit_code = sgRouterToServerConvert(&sg_serial, payload);
	fail_if((exit_code != 0), "sgRouterToServer Failed! Expected: 0\tGot: %i", exit_code);
#if TESTS_DEBUG_LEVEL > 0
	printf("Eyeball: Size of payload is: %i\n", strlen(payload));
	printf("Eyeball: Converted to: %s\n", payload);
#endif
	mark_point();
	rdid = sgServerToRouterConvert(payload, &sg_serial);
	routingTableDestroy(routing_table);

	mark_point();
	fail_if(memcmp(&sg_serial, &sg_serial_orig, sizeof(SansgridSerial)),
			"Eyeball: Converted serial packet doesn't match original!");
	fail_if((rdid != rdid_orig), 
			"Eyeball: returned identifier doesn't match original!\
\n\tExpected: %i \
\n\tGot: %i", rdid_orig, rdid);
	mark_point();
}
END_TEST


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
	payloadMkSerial(&sg_serial);
	routingTableGetBroadcast(routing_table, sg_serial.ip_addr);
	payloadMkPeck(&sg_peck, &test_struct);

	memcpy(sg_serial.payload, &sg_peck, sizeof(SansgridPeck));
	memcpy(&sg_serial_orig, &sg_serial, sizeof(SansgridSerial));

	mark_point();
	exit_code = sgRouterToServerConvert(&sg_serial, payload);
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
	fail_if(memcmp(&sg_serial, &sg_serial_orig, sizeof(SansgridSerial)),
			"Peck: Converted serial packet doesn't match original!");
	fail_if((rdid != rdid_orig), 
			"Peck: returned identifier doesn't match original! \
\n\tExpected: %i \
\n\tGot: %i", rdid_orig, rdid);
	mark_point();

}
END_TEST

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
	payloadMkSerial(&sg_serial);
	routingTableGetBroadcast(routing_table, sg_serial.ip_addr);
	payloadMkSing(&sg_sing, &test_struct);

	memcpy(sg_serial.payload, &sg_sing, sizeof(SansgridSing));
	memcpy(&sg_serial_orig, &sg_serial, sizeof(SansgridSerial));

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
	fail_if(memcmp(&sg_serial, &sg_serial_orig, sizeof(SansgridSerial)),
			"Sing: Converted serial packet doesn't match original!");
	fail_if((rdid != rdid_orig), 
			"Sing: returned identifier doesn't match original!\
\n\tExpected: %i \
\n\tGot: %i", rdid_orig, rdid);
	mark_point();
}
END_TEST


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
	routingTableGetBroadcast(routing_table, sg_serial.ip_addr);
	payloadMkMock(&sg_mock, &test_struct);

	memcpy(sg_serial.payload, &sg_mock, sizeof(SansgridMock));
	memcpy(&sg_serial_orig, &sg_serial, sizeof(SansgridSerial));

	mark_point();
	exit_code = sgRouterToServerConvert(&sg_serial, payload);
	fail_if((exit_code != 0), "sgRouterToServer Failed! Expected: 0\tGot: %i", exit_code);
#if TESTS_DEBUG_LEVEL > 0
	printf("Mock: Size of payload is: %i\n", strlen(payload));
	printf("Mock: Converted to: %s\n", payload);
#endif
	mark_point();
	rdid = sgServerToRouterConvert(payload, &sg_serial);
	routingTableDestroy(routing_table);

	mark_point();
	fail_if(memcmp(&sg_serial, &sg_serial_orig, sizeof(SansgridSerial)),
			"Mock: Converted serial packet doesn't match original!");
	fail_if((rdid != rdid_orig), 
			"Mock: returned identifier doesn't match original!\
\n\tExpected: %i \
\n\tGot: %i", rdid_orig, rdid);
	mark_point();
}
END_TEST



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
	payloadMkSerial(&sg_serial);
	routingTableGetBroadcast(routing_table, sg_serial.ip_addr);
	payloadMkPeacock(&sg_peacock, &test_struct);

	memcpy(sg_serial.payload, &sg_peacock, sizeof(SansgridPeacock));
	memcpy(&sg_serial_orig, &sg_serial, sizeof(SansgridSerial));

	mark_point();
	exit_code = sgRouterToServerConvert(&sg_serial, payload);
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
	fail_if(memcmp(&sg_serial, &sg_serial_orig, sizeof(SansgridSerial)),
			"Peacock: Converted serial packet doesn't match original!");
	fail_if((rdid != rdid_orig), 
			"Peacock: returned identifier doesn't match original!\
\n\tExpected: %i \
\n\tGot: %i", rdid_orig, rdid);
	mark_point();
}
END_TEST



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
	payloadMkSerial(&sg_serial);
	routingTableGetBroadcast(routing_table, sg_serial.ip_addr);
	payloadMkNest(&sg_nest, &test_struct);

	memcpy(sg_serial.payload, &sg_nest, sizeof(SansgridNest));
	memcpy(&sg_serial_orig, &sg_serial, sizeof(SansgridSerial));

	mark_point();
	exit_code = sgRouterToServerConvert(&sg_serial, payload);
	fail_if((exit_code != 0), "sgRouterToServer Failed! Expected: 0\tGot: %i", exit_code);
#if TESTS_DEBUG_LEVEL > 0
	printf("Nest: Size of payload is: %i\n", strlen(payload));
	printf("Nest: Converted to: %s\n", payload);
#endif
	mark_point();
	rdid = sgServerToRouterConvert(payload, &sg_serial);
	routingTableDestroy(routing_table);

	mark_point();
	fail_if(memcmp(&sg_serial, &sg_serial_orig, sizeof(SansgridSerial)),
			"Nest: Converted serial packet doesn't match original!");
	fail_if((rdid != rdid_orig), 
			"Nest: returned identifier doesn't match original!\
\n\tExpected: %i \
\n\tGot: %i", rdid_orig, rdid);
	mark_point();
}
END_TEST



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
	payloadMkSerial(&sg_serial);
	routingTableGetBroadcast(routing_table, sg_serial.ip_addr);
	payloadMkSquawkSensor(&sg_squawk, &test_struct);

	memcpy(sg_serial.payload, &sg_squawk, sizeof(SansgridSquawk));
	memcpy(&sg_serial_orig, &sg_serial, sizeof(SansgridSerial));

	mark_point();
	exit_code = sgRouterToServerConvert(&sg_serial, payload);
	fail_if((exit_code != 0), "sgRouterToServer Failed! Expected: 0\tGot: %i", exit_code);
#if TESTS_DEBUG_LEVEL > 0
	printf("Squawk: Size of payload is: %i\n", strlen(payload));
	printf("Squawk: Converted to: %s\n", payload);
#endif
	mark_point();
	rdid = sgServerToRouterConvert(payload, &sg_serial);
	routingTableDestroy(routing_table);

	mark_point();
	fail_if(memcmp(&sg_serial, &sg_serial_orig, sizeof(SansgridSerial)),
			"Squawk: Converted serial packet doesn't match original!");
	fail_if((rdid != rdid_orig), 
			"Squawk: returned identifier doesn't match original!\
\n\tExpected: %i \
\n\tGot: %i", rdid_orig, rdid);
	mark_point();
}
END_TEST



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

