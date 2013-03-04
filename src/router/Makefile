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
NAME=sgrouter

# Global Variables
export CC = gcc
export DEBUG = -g
export CFLAGS = -std=c99 -Wall -c $(DEBUG)
export LFLAGS = -Wall -lpthread $(DEBUG)
export TEST_LFLAGS = $(LFLAGS) -lcheck


# Directories
DAEMON_DIR = daemon
DISPATCH_DIR = dispatch
ROUTE_DIR = routing

# Test Directories
TEST_DIR = test
TEST_DISPATCH_DIR = $(TEST_DIR)/dispatch
TEST_ROUTING_DIR = $(TEST_DIR)/routing
TEST_STUBS_DIR = $(TEST_DIR)/stubs

# Files
MAIN = $(DAEMON_DIR)/pi-runtime
DISPATCH = $(DISPATCH_DIR)/dispatch
ROUTING = $(ROUTE_DIR)/routing

# Test Files
TEST_CORE = $(TEST_DIR)/test-core
TEST_HEADER = $(TEST_DIR)/tests.h
DISPATCH_ADVANCED_TEST = $(TEST_DISPATCH_DIR)/dispatch-advanced
DISPATCH_BASIC_TEST = $(TEST_DISPATCH_DIR)/dispatch-basic
ROUTE_BASIC_TEST = $(TEST_ROUTING_DIR)/routing-basic-test
STUBS = $(TEST_STUBS_DIR)/radio-stub


# Object File Compilations
OBS = $(ROUTING).o $(DISPATCH).o
TEST_OBS = $(DISPATCH_ADVANCED_TEST).o $(ROUTE_BASIC_TEST).o $(DISPATCH_BASIC_TEST).o $(STUBS).o

# Object File Builds
SANSGRID_OBS = $(OBS) $(MAIN).o
TESTSUITE_OBS = $(OBS) $(TEST_OBS) $(TEST_CORE).o


sgrouter: $(SANSGRID_OBS)
	$(CC) $(LFLAGS) $(SANSGRID_OBS) -o $(NAME)
tests: sgrouter-tests
sgrouter-tests: $(TESTSUITE_OBS) $(TEST_HEADER)
	cd $(TEST_DIR) && make all
	$(CC) $(TEST_LFLAGS) $(TESTSUITE_OBS) $(TEST_HEADER) -o tests
all: sgrouter sgrouter-tests
clean: 
	-rm ./sgrouter
	-rm ./tests
	-cd $(DAEMON_DIR) && make clean-daemon
	-cd $(DISPATCH_DIR) && make clean-dispatch
	-cd $(ROUTE_DIR) && make clean-routing
	-cd $(TEST_DIR) && make clean-test
