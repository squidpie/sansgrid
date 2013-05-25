/* Payload Tests
 *
 * Copyright (C) 2013 SansGrid
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 *
 * This dispatch test uses a named pipe as a stub to read from.
 * The data from the stub is enqueued, and the dispatch thread dequeues the data.
 */
/// \file

#include "payload_tests.h"



/// Make sure all payloads are the same, correct size.
START_TEST (testPayloadSize) {
	// unit test code for making sure all payloads are the same size
	checkSize("SansgridHatching", sizeof(SansgridHatching));
	checkSize("SansgridFly", sizeof(SansgridFly));
	checkSize("SansgridEyeball", sizeof(SansgridEyeball));
	checkSize("SansgridPeck", sizeof(SansgridPeck));
	checkSize("SansgridSing", sizeof(SansgridSing));
	checkSize("SansgridMock", sizeof(SansgridMock));
	checkSize("SansgridPeacock", sizeof(SansgridPeacock));
	checkSize("SansgridNest", sizeof(SansgridNest));
	checkSize("SansgridSquawk", sizeof(SansgridSquawk));
	checkSize("SansgridHeartbeat", sizeof(SansgridHeartbeat));
	checkSize("SansgridChirp", sizeof(SansgridChirp));
}
END_TEST;




/// Payload size testing unit tests
Suite *payloadSizeTesting (void) {
	Suite *s = suite_create("Payload Size Test");
	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, testPayloadSize);

	suite_add_tcase(s, tc_core);

	return s;
}


// vim: ft=c ts=4 noet sw=4:
