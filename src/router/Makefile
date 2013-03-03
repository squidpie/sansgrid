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
export TEST_LFLAGS = $(LFLAGS) -lcheck

# Directories
ROUTE_DIR = routing
DISPATCH_DIR = synchronous_queue
TEST_DIR = test
STUBS_DIR = $(TEST_DIR)/stubs
DAEMON_DIR = daemon

# Files
TEST_CORE = $(TEST_DIR)/test-core
DISPATCH_ADVANCED_TEST = $(TEST_DIR)/dispatch-advanced
ROUTE_BASIC_TEST = $(TEST_DIR)/routing-basic-test
DISPATCH_TEST = $(TEST_DIR)/sync_queue_main
STUBS = $(TEST_DIR)/stubs/radio-stub
ROUTING = $(ROUTE_DIR)/routing
DISPATCH = $(DISPATCH_DIR)/sync_queue
MAIN = $(DAEMON_DIR)/pi-runtime


# Files that are compiled
OBS = $(ROUTING).o $(DISPATCH).o
TEST_OBS = $(TEST_CORE).o $(DISPATCH_ADVANCED_TEST).o $(ROUTE_BASIC_TEST).o $(DISPATCH_TEST).o $(STUBS).o


sansgrid: $(OBS) $(MAIN).o
	$(CC) $(LFLAGS) $(OBS) $(MAIN).o -o $(NAME)
tests: $(TEST_OBS) $(OBS)
	$(CC) $(TEST_LFLAGS) $(TEST_OBS) $(OBS) -o tests
all: sansgrid tests
clean: 
	-rm ./sansgrid
	-rm ./tests
	-cd $(DAEMON_DIR) && make clean-daemon
	-cd $(DISPATCH_DIR) && make clean-sync-queue
	-cd $(ROUTE_DIR) && make clean-routing
	-cd $(TEST_DIR) && make clean-test
	-cd $(STUBS_DIR) && make clean-stubs
