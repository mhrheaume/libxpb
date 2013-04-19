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

#ifndef XPB_H
#define XPB_H

#include <stdint.h>

#define XPB_STATUS_SUCCESS      0
#define XPB_STATUS_BAD_NRECT    1
#define XPB_STATUS_BAD_PADDING  2
#define XPB_STATUS_BAD_XSZ      3
#define XPB_STATUS_BAD_YSZ      4
#define XPB_STATUS_BAD_XPOS     5
#define XPB_STATUS_BAD_YPOS     6
#define XPB_STATUS_BAD_FG       7
#define XPB_STATUS_BAD_BG       8
#define XPB_STATUS_NOMEM        9
#define XPB_STATUS_TOO_LARGE   10
#define XPB_STATUS_BAD_PTR     11
#define XPB_STATUS_END         12 // Never returned, should always be last

#define XPB_MASK_NRECT     0x0001
#define XPB_MASK_PADDING   0x0002
#define XPB_MASK_RECT_XSZ  0x0004
#define XPB_MASK_RECT_YSZ  0x0008
#define XPB_MASK_XPOS      0x0010
#define XPB_MASK_YPOS      0x0020
#define XPB_MASK_FG        0x0040
#define XPB_MASK_BG        0x0080

struct xpb {
	Display *dpy;
	Window root;

	void *priv;
};

struct xpb_attr {
	uint8_t nrect;
	uint8_t padding;
	uint8_t rect_xsz;
	uint8_t rect_ysz;

	uint16_t xpos;
	uint16_t ypos;

	char *fg;
	char *bg;
};

uint8_t xpb_init(uint16_t mask,
	struct xpb_attr *attr,
	struct xpb **bar_out);

uint8_t xpb_draw(struct xpb *bar, uint16_t current, uint16_t max);
uint8_t xpb_cleanup(struct xpb *bar);

const char *xpb_status_tostring(uint8_t status);

#endif // XPB_H
