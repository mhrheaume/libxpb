/*
 *  Copyright (C) 2013 Matthew Rheaume
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdio.h>

#include "macro.h"
#include "tests.h"

int main(int argc, char **argv)
{
	int i, status;
	struct test_unit *current_test;

	struct test_unit test_list[NTESTS] = {
		ADD_TEST(defaults),
		ADD_TEST(top_left),
		ADD_TEST(green_fg),
		ADD_TEST(green_bg),
		ADD_TEST(bigpads),
		ADD_TEST(squares),
		ADD_TEST(manysquares),
		ADD_TEST(badpointers),
		ADD_TEST(badvals)
	};

	for (i = 0; i < NTESTS; i++) {
		current_test = &test_list[i];

		printf("Running %s.. ", current_test->name);
		status = (*(current_test->run))();
		printf(status == TEST_SUCCESS ? "Pass\n" : "Fail\n");
	}
	
	return 0;
}
