bin_PROGRAMS = sansgrid 
check_PROGRAMS = sgtest
SANSGRID_COMM = communication/sg_router_server_communication.c \
		communication/sg_serial.c

SANSGRID_BASE = dispatch/dispatch.c \
		dt_handlers/handlers.c \
		routing/routing.c
SANSGRID_TEST = test/communication/sg-serial-test.c \
		test/dispatch/dispatch-basic.c \
		test/dispatch/dispatch-advanced.c \
		test/routing/routing-basic-test.c
			
sansgrid_SOURCES = $(SANSGRID_BASE) \
		   $(SANSGRID_COMM) \
		   daemon/pi-runtime.c 
sgtest_SOURCES = $(SANSGRID_BASE) \
		 $(SANSGRID_TEST) \
		 test/test-core.c

