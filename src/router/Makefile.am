bin_PROGRAMS = sansgrid 
check_PROGRAMS = sgtest
AM_CFLAGS = -Wall -std=c99
SANSGRID_COMM = communication/sg_tcp.c \
		communication/sg_serial.c

SANSGRID_BASE = dispatch/dispatch.c \
		dt_handlers/handlers.c \
		routing/routing.c
SANSGRID_TEST = test/communication/sg-serial-test.c \
		test/communication/sg-tcp-test.c \
		test/dispatch/dispatch-basic.c \
		test/dispatch/dispatch-advanced.c \
		test/routing/routing-basic-test.c \
		test/payloads/payloads-tests.c \
		test/payloads/payload-stub-handlers.c
			
sansgrid_SOURCES = $(SANSGRID_BASE) \
		   $(SANSGRID_COMM) \
		   daemon/sansgrid_daemon.c \
		   sansgrid_router/sansgrid_router.c
sgtest_SOURCES = $(SANSGRID_BASE) \
		 $(SANSGRID_TEST) \
		 test/test-core.c

