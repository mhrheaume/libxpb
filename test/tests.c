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

#include <unistd.h>
#include <xpb.h>

#include <X11/Xlib.h>

#include "tests.h"

DECLARE_TEST(defaults)
{
	int i, status;
	struct xpb *bar = NULL;

	status = xpb_init(0, NULL, &bar);
	if (!XPB_SUCCESS(status)) {
		return TEST_FAIL;
	}

	for (i = 0; i <= 4; i++) {
		status = xpb_draw(bar, i, 4);
		if (!XPB_SUCCESS(status)) {
			return TEST_FAIL;
		}

		XFlush(bar->dpy);
		sleep(1);
	}

	status = xpb_cleanup(bar);
	if (!XPB_SUCCESS(status)) {
		return TEST_FAIL;
	}

	return TEST_SUCCESS;
}

DECLARE_TEST(top_left)
{
	int i, status;
	struct xpb *bar = NULL;
	struct xpb_attr attr;
	unsigned long mask = 0;

	attr.xpos = 30;
	attr.ypos = 30;

	mask |= XPB_MASK_XPOS;
	mask |= XPB_MASK_YPOS;

	status = xpb_init(mask, &attr, &bar);
	if (!XPB_SUCCESS(status)) {
		return TEST_FAIL;
	}

	for (i = 0; i <= 4; i++) {
		status = xpb_draw(bar, i, 4);
		if (!XPB_SUCCESS(status)) {
			return TEST_FAIL;
		}

		XFlush(bar->dpy);
		sleep(1);
	}

	status = xpb_cleanup(bar);
	if (!XPB_SUCCESS(status)) {
		return TEST_FAIL;
	}

	return TEST_SUCCESS;
}
