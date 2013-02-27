# Copyright (C) 2013 SansGrid
 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


# Executable Name
NAME=sansgrid

# Global Variables
export CC = gcc
export DEBUG = -g
export CFLAGS = -std=c99 -Wall -c $(DEBUG)
export LFLAGS = -Wall -lpthread $(DEBUG)

# Directories
ROUTE_DIR = routing
DISPATCH_DIR = synchronous_queue
TEST_DIR = test
STUBS_DIR = $(TEST_DIR)/stubs
DAEMON_DIR = daemon

# Files
ROUTE_TEST = $(TEST_DIR)/routing-test
DISPATCH_TEST = $(TEST_DIR)/sync_queue_main
STUBS = $(TEST_DIR)/stubs/radio-stub
ROUTING = $(ROUTE_DIR)/routing
DISPATCH = $(DISPATCH_DIR)/sync_queue
MAIN = $(DAEMON_DIR)/pi-runtime


# Makefiles
MAKE_DAEMON = $(DAEMON_DIR)/Makefile
MAKE_ROUTING = $(ROUTE_DIR)/Makefile
MAKE_DISPATCH = $(DISPATCH_DIR)/Makefile
MAKE_TEST = $(TEST_DIR)/Makefile


# Files that are compiled
TEST_ROUTE_OBS = $(ROUTE_TEST).o $(ROUTING).o $(DISPATCH).o $(STUBS).o
TEST_DISPATCH_OBS = $(DISPATCH_TEST).o $(DISPATCH).o
OBS = $(MAIN).o $(ROUTING).o $(DISPATCH).o



all: sansgrid route_test dispatch_test
test: route_test dispatch_test
sansgrid: $(OBS)
	$(CC) $(LFLAGS) $(OBS) -o $(NAME)
route_test: $(TEST_ROUTE_OBS)
	$(CC) $(LFLAGS) $(TEST_ROUTE_OBS) -o route_test
dispatch_test: $(TEST_DISPATCH_OBS)
	$(CC) $(LFLAGS) $(TEST_DISPATCH_OBS) -o dispatch_test
clean: 
	-rm ./sansgrid
	-rm ./route_test
	-rm ./dispatch_test
	-cd $(DAEMON_DIR) && make clean-daemon
	-cd $(DISPATCH_DIR) && make clean-sync-queue
	-cd $(ROUTE_DIR) && make clean-routing
	-cd $(TEST_DIR) && make clean-test
	-cd $(STUBS_DIR) && make clean-stubs
