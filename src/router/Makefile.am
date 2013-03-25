bin_PROGRAMS = sansgrid 
check_PROGRAMS = sgtest
AM_CFLAGS = -Wall -std=c99
SANSGRID_COMM = communication/sg_tcp.c \
		communication/sg_serial.c

SANSGRID_BASE = dispatch/dispatch.c \
		dt_handlers/handlers.c \
		routing/routing.c
SANSGRID_TEST = test/communication/sg-serial-stubs.c \
		test/communication/sg-tcp-stubs.c \
		test/dispatch/dispatch-basic.c \
		test/dispatch/dispatch-advanced.c \
		test/routing/routing-basic-test.c \
		test/payloads/payload-tests.c \
		test/payloads/payload-stub-handlers.c \
		test/payloads/payload-size-test.c \
		test/payloads/payload-dt-eyeball.c \
		test/payloads/payload-dt-peck.c
			
sansgrid_SOURCES = $(SANSGRID_BASE) \
		   $(SANSGRID_COMM) \
		   daemon/sansgrid_daemon.c \
		   sansgrid_router/sansgrid_router.c
sgtest_SOURCES = $(SANSGRID_BASE) \
		 $(SANSGRID_TEST) \
		 test/tests.c

