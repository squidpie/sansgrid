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


TEST_CORE=test-core
DISPATCH_ADVANCED=dispatch-advanced
ROUTING_BASIC=routing-basic-test
SYNC_QUEUE=sync_queue_main
STUBS=stubs/radio-stub


all: $(TEST_CORE).o $(ROUTING_BASIC).o $(ROUTING).o $(SYNC_QUEUE).o 
$(ROUTING_BASIC).o: $(ROUTING_BASIC).c tests.h
	$(CC) $(CFLAGS) $(ROUTING_BASIC).c 
$(DISPATCH_ADVANCED).o: $(DISPATCH_ADVANCED).c $(STUBS).c  tests.h
	$(CC) $(CFLAGS) $(DISPATCH_ADVANCED).c $(STUBS).c 
$(SYNC_QUEUE).o: $(SYNC_QUEUE).c tests.h
	$(CC) $(CFLAGS) $(SYNC_QUEUE).c
$(TEST_CORE).o: $(TEST_CORE).c tests.h 
	$(CC) $(CFLAGS) $(TEST_CORE).c
clean-test:
	rm *.o
