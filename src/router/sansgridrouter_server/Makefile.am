bin_PROGRAMS = sansgridrouterd 
check_PROGRAMS = sgtest
AM_CFLAGS = -Wall -std=c99
SANSGRID_COMM = device/raspberry_pi/communication/sg_tcp.c \
		device/raspberry_pi/communication/sg_serial.c

SANSGRID_BASE = dispatch/dispatch.c \
		payload_handlers/payload_handlers.c \
		routing_table/routing_table.c \
		routing_table/heartbeat.c
SANSGRID_TEST = test/communication/sg_communication_stubs.c \
		test/dispatch/dispatch_basic.c \
		test/dispatch/dispatch_advanced.c \
		test/routing/routing_basic_test.c \
		test/payloads/payload_tests.c \
		test/payloads/payload_stub_handlers.c \
		test/payloads/payload_size_test.c \
		test/payloads/payload_dt_tests.c \
		test/payloads/payload_dt_eyeball_tests.c \
		test/payloads/payload_dt_peck_tests.c \
		test/payloads/payload_dt_sing_tests.c \
		test/payloads/payload_dt_mock_tests.c \
		test/payloads/payload_dt_peacock_tests.c \
		test/payloads/payload_dt_squawk_tests.c \
		test/payloads/payload_dt_nest_tests.c \
		test/payloads/payload_dt_chirp_tests.c

			
sansgridrouterd_SOURCES = $(SANSGRID_BASE) \
			  $(SANSGRID_COMM) \
			  daemon/sansgrid_daemon.c \
			  main.c
sgtest_SOURCES = $(SANSGRID_BASE) \
		 $(SANSGRID_TEST) \
		 test/tests.c

