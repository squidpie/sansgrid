bin_PROGRAMS = sansgridrouter
check_PROGRAMS = sgtest
AM_CFLAGS = -Wall -std=c99

SANSGRID_BASE = dispatch/dispatch.c \
		payload_handlers/payload_handlers.c \
		communication/sg_tcp_handler.c \
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
		test/payloads/payload_dt_chirp_tests.c \
		test/communication/sg_tcp_test.c

#noinst_LIBRARIES = libSgPayloads.a libSgSerial.a

#libSgSerial_a_SOURCES = ../../sg_serial.h
#libSgPayloads_a_SOURCES = ../../lib/payloads.h

sansgridrouter_SOURCES = $(SANSGRID_BASE) \
			  $(SANSGRID_COMM) \
			  daemon/sansgrid_daemon.c \
			  communication/sg_tcp.c \
			  main.c
EXTRA_sansgridrouter_SOURCES = 	communication/sg_serial_rpi.c \
				communication/sg_serial_generic.c
#sansgridrouter_LDADD = $(SANSGRIDROUTER_SYSTEM) libSgPayloads.a libSgSerial.a
#sansgridrouter_DEPENDENCIES = $(SANSGRIDROUTER_SYSTEM) libSgPayloads.a libSgSerial.a
sansgridrouter_LDADD = $(SANSGRIDROUTER_SYSTEM) 
sansgridrouter_DEPENDENCIES = $(SANSGRIDROUTER_SYSTEM) 
#sgtest_LDADD = libSgPayloads.a libSgSerial.a
sgtest_SOURCES = $(SANSGRID_BASE) \
		 $(SANSGRID_TEST) \
		 test/tests.c

AM_CFLAGS += -DDATADIR=\"$(pkgdatadir)\"
