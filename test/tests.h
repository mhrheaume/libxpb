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

#ifndef TESTS_H
#define TESTS_H

#include "macro.h"

#define NTESTS 10

DECLARE_TEST(defaults);
DECLARE_TEST(top_left);
DECLARE_TEST(green_fg);
DECLARE_TEST(green_bg);
DECLARE_TEST(big_pads);
DECLARE_TEST(squares);
DECLARE_TEST(many_squares);
DECLARE_TEST(bad_pointers);
DECLARE_TEST(bad_vals);
DECLARE_TEST(too_large);

#endif // TESTS_H
