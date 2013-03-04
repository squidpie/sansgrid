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


TEST_CORE = test-core
TEST_HEADER = tests.h

ifndef CC
	export CC = gcc
endif

ifndef CFLAGS
	export CFLAGS = -std=c99 -Wall -c -g
endif

all: $(TEST_CORE).o 
$(TEST_CORE).o: $(TEST_CORE).c $(TEST_HEADER)
	$(CC) $(CFLAGS) $(TEST_CORE).c $(TEST_HEADER)
	-cd routing && make all
	-cd dispatch && make all
	-cd stubs && make all
clean-test:
	-rm *.o
	-cd routing && make clean-routing-test
	-cd dispatch && make clean-dispatch-test
	-cd stubs && make clean-stubs-test
