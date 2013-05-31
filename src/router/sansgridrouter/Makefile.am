bin_PROGRAMS = sansgridrouter
check_PROGRAMS = sgtest
AM_CFLAGS = -Wall -g -fstack-protector-all -std=c99 -I../../../include/

SANSGRID_BASE = dispatch/dispatch.c \
		payload_handlers/payload_handlers.c \
		communication/sg_tcp_handler.c \
		routing_table/routing_table.c \
		routing_table/heartbeat.c \
		routing_table/auth_status.c
SANSGRID_TEST = \
		test/communication/sg_communication_stubs.c \
		test/communication/sg_communication_stubs_eeprom.c \
		test/communication/sg_communication_talkstub.c \
		test/dispatch/dispatch_basic.c \
		test/dispatch/dispatch_advanced.c \
		test/routing/routing_basic_test.c \
		test/routing/auth_basic_test.c \
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
		test/communication/sg_intrarouter_test_atox.c \
		test/communication/sg_intrarouter_test_conversion.c


#SANSGRID_COMM = communication/sg_serial_rpi.c
SANSGRID_COMM = communication/sg_serial_generic.c

sansgridrouter_SOURCES = $(SANSGRID_BASE) \
			  daemon/sansgrid_daemon.c \
			  communication/sg_tcp.c \
			  main.c
EXTRA_sansgridrouter_SOURCES = 	communication/sg_serial_rpi.c \
								communication/sg_serial_generic.c
sansgridrouter_LDADD = $(SANSGRIDROUTER_SYSTEM) 
sansgridrouter_DEPENDENCIES = $(SANSGRIDROUTER_SYSTEM) 

sgtest_SOURCES = $(SANSGRID_BASE) \
		 		$(SANSGRID_TEST) \
				 test/tests.c

AM_CFLAGS += -DDATADIR=\"$(pkgdatadir)\" 
