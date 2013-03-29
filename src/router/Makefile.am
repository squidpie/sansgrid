bin_PROGRAMS = sansgrid 
check_PROGRAMS = sgtest
AM_CFLAGS = -Wall -std=c99
SANSGRID_COMM = communication/sg_tcp.c \
		communication/sg_serial.c

SANSGRID_BASE = dispatch/dispatch.c \
		payload_handlers/payload_handlers.c \
		routing_table/routing_table.c
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

			
sansgrid_SOURCES = $(SANSGRID_BASE) \
		   $(SANSGRID_COMM) \
		   daemon/sansgrid_daemon.c \
		   sansgrid_router/sansgrid_router.c
sgtest_SOURCES = $(SANSGRID_BASE) \
		 $(SANSGRID_TEST) \
		 test/tests.c

