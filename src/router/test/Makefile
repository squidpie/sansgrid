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



ROUTING=routing-test
STUBS=stubs/radio-stub
SYNC_QUEUE=sync_queue_main

all: $(ROUTING).o $(SYNC_QUEUE).o
$(ROUTING).o: $(ROUTING).c $(STUBS).c
	$(CC) $(CFLAGS) $(ROUTING).c $(STUBS).c
$(SYNC_QUEUE).o: $(SYNC_QUEUE).c
	$(CC) $(CFLAGS) $(SYNC_QUEUE).c
clean-test:
	rm *.o
