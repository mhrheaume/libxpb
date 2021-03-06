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

static int run_fill_loop(unsigned long mask, struct xpb_attr *attr)
{
	int i;
	struct xpb *bar = NULL;

	if (!XPB_SUCCESS(xpb_init(mask, attr, &bar))) {
		return TEST_FAIL;
	}

	for (i = 0; i <= 4; i++) {
		if (!XPB_SUCCESS(xpb_draw(bar, i, 4))) {
			xpb_cleanup(bar);
			return TEST_FAIL;
		}

		XFlush(bar->dpy);
		sleep(1);
	}

	if (!XPB_SUCCESS(xpb_cleanup(bar))) {
		return TEST_FAIL;
	}

	return TEST_SUCCESS;
}

DECLARE_TEST(defaults)
{
	return run_fill_loop(0, NULL);
}

DECLARE_TEST(top_left)
{
	struct xpb_attr attr;
	unsigned long mask = 0;

	attr.xpos = 30;
	attr.ypos = 30;

	mask |= XPB_MASK_XPOS;
	mask |= XPB_MASK_YPOS;

	return run_fill_loop(mask, &attr);
}

DECLARE_TEST(green_fg)
{
	struct xpb_attr attr;
	unsigned long mask = 0;

	attr.fg = "#00ff00";
	attr.bg = "#000000";

	mask |= XPB_MASK_FG;
	mask |= XPB_MASK_BG;

	return run_fill_loop(mask, &attr);
}

DECLARE_TEST(green_bg)
{
	struct xpb_attr attr;
	unsigned long mask = 0;

	attr.fg = "#000000";
	attr.bg = "#00ff00";

	mask |= XPB_MASK_FG;
	mask |= XPB_MASK_BG;

	return run_fill_loop(mask, &attr);
}

DECLARE_TEST(big_pads)
{
	struct xpb_attr attr;
	unsigned long mask = 0;

	attr.padding = 6;
	mask |= XPB_MASK_PADDING;

	return run_fill_loop(mask, &attr);
}

DECLARE_TEST(squares)
{
	struct xpb_attr attr;
	unsigned long mask = 0;

	attr.rect_xsz = 20;
	attr.rect_ysz = 20;

	mask |= XPB_MASK_RECT_XSZ;
	mask |= XPB_MASK_RECT_YSZ;

	return run_fill_loop(mask, &attr);
}

DECLARE_TEST(many_squares)
{
	struct xpb_attr attr;
	unsigned long mask = 0;

	attr.rect_xsz = 10;
	attr.rect_ysz = 10;
	attr.nrect = 40;

	mask |= XPB_MASK_RECT_XSZ;
	mask |= XPB_MASK_RECT_YSZ;
	mask |= XPB_MASK_NRECT;

	return run_fill_loop(mask, &attr);
}

DECLARE_TEST(bad_pointers)
{
	int status;

	status = xpb_init(0, NULL, NULL);
	if (status != XPB_STATUS_BAD_PTR) {
		return TEST_FAIL;
	}

	status = xpb_draw(NULL, 0, 0);
	if (status != XPB_STATUS_BAD_PTR) {
		return TEST_FAIL;
	}

	status = xpb_cleanup(NULL);
	if (status != XPB_STATUS_BAD_PTR) {
		return TEST_FAIL;
	}

	return TEST_SUCCESS;
}

DECLARE_TEST(bad_vals)
{
	struct xpb_attr attr;
	struct xpb *bar;
	unsigned long mask;

	// Bad number of rectangles
	attr.nrect = -1;
	mask = XPB_MASK_NRECT;

	if (xpb_init(mask, &attr, &bar) != XPB_STATUS_BAD_NRECT) {
		return TEST_FAIL;
	}

	// Bad padding value
	attr.padding = -1;
	mask = XPB_MASK_PADDING;

	if (xpb_init(mask, &attr, &bar) != XPB_STATUS_BAD_PADDING) {
		return TEST_FAIL;
	}

	attr.rect_xsz = -1;
	mask = XPB_MASK_RECT_XSZ;

	if (xpb_init(mask, &attr, &bar) != XPB_STATUS_BAD_XSZ) {
		return TEST_FAIL;
	}

	attr.rect_ysz = -1;
	mask = XPB_MASK_RECT_YSZ;

	if (xpb_init(mask, &attr, &bar) != XPB_STATUS_BAD_YSZ) {
		return TEST_FAIL;
	}

	attr.xpos = -1;
	mask = XPB_MASK_XPOS;

	if (xpb_init(mask, &attr, &bar) != XPB_STATUS_BAD_XPOS) {
		return TEST_FAIL;
	}

	attr.ypos = -1;
	mask = XPB_MASK_YPOS;

	if (xpb_init(mask, &attr, &bar) != XPB_STATUS_BAD_YPOS) {
		return TEST_FAIL;
	}

	return TEST_SUCCESS;
}

DECLARE_TEST(too_large)
{
	struct xpb_attr attr;
	struct xpb *bar;
	unsigned long mask;

	attr.nrect = 1000;
	attr.rect_xsz = 2000;
	attr.xpos = 0;

	mask = XPB_MASK_NRECT;
	mask |= XPB_MASK_RECT_XSZ;
	mask |= XPB_MASK_XPOS;

	if (xpb_init(mask, &attr, &bar) != XPB_STATUS_TOO_LARGE) {
		return TEST_FAIL;
	}

	attr.rect_ysz = 20000;
	attr.ypos = 0;

	mask = XPB_MASK_RECT_YSZ;
	mask |= XPB_MASK_YPOS;

	if (xpb_init(mask, &attr, &bar) != XPB_STATUS_TOO_LARGE) {
		return TEST_FAIL;
	}

	return TEST_SUCCESS;
}

