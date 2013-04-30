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

#include "tests.h"

#define ADD_TEST(t) { .name = #t, .fn = test_##t }
#define RUN_TEST(t) do { \
	printf("Running %s.. ", t.name); \
	printf((*(t.fn))() == TEST_SUCCESS ? "Pass\n" : "Fail\n"); \
} while (0)

struct test_unit {
	char *name;
	int (*fn)();
};

int main(int argc, char **argv)
{
	int i;

	struct test_unit test_list[NTESTS] = {
		ADD_TEST(defaults),
		ADD_TEST(top_right)
	};

	for (i = 0; i < NTESTS; i++) {
		RUN_TEST(test_list[i]);
	}
	
	return 0;
}
