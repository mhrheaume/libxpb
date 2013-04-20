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

#include <stdlib.h>
#include <stdint.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "xpb.h"

#define DEFAULT_NRECT    20
#define DEFAULT_PADDING   2
#define DEFAULT_RECT_XSZ 12
#define DEFAULT_RECT_YSZ 20

#define DEFAULT_FG "#3475aa"
#define DEFAULT_BG "#1a1a1a"

#define XPB_PRIV(b) (struct xpb_priv*)b->priv

struct xpb_priv {
	Window win;
	GC gc;

	uint8_t nrect;
	uint8_t padding;
	uint8_t rect_xsz;
	uint8_t rect_ysz;

	uint16_t xpos;
	uint16_t ypos;

	uint16_t xsz;
	uint16_t ysz;

	XColor fg;
	XColor bg;
};

static xpb_status_t set_dimensions(struct xpb *bar,
	uint16_t mask,
	struct xpb_attr *attr);

static xpb_status_t alloc_colors(struct xpb *bar,
	uint16_t mask,
	struct xpb_attr *attr);

static void create_window(struct xpb *bar);

__attribute__((always_inline))
static inline uint16_t calc_xsize(uint8_t rect_xsz, uint8_t padding, uint8_t nrect)
{
	return rect_xsz * nrect + padding * (nrect + 1) + 2;
}

__attribute__((always_inline))
static inline uint16_t calc_ysize(uint8_t rect_ysz, uint8_t padding)
{
	return rect_ysz + 2 * padding + 2;
}

static inline
float get_fill_percent(unsigned int percent, float lower, float upper)
{
	return
		percent >= upper ? 1.0 :
		percent <= lower ? 0 :
		(float)(percent - lower) / (float)(upper - lower);
}

xpb_status_t set_dimensions(struct xpb *bar, uint16_t mask, struct xpb_attr *attr)
{
	int screen = DefaultScreen(bar->dpy);
	int screen_xsz = DisplayWidth(bar->dpy, screen);
	int screen_ysz = DisplayHeight(bar->dpy, screen);

	struct xpb_priv *priv = XPB_PRIV(bar);

	priv->nrect = mask & XPB_MASK_NRECT ? attr->nrect : DEFAULT_NRECT;
	if (priv->nrect == 0) {
		return XPB_STATUS_BAD_NRECT;
	}

	priv->padding = mask & XPB_MASK_PADDING ? attr->padding : DEFAULT_PADDING;
	if (priv->padding == 0) {
		return XPB_STATUS_BAD_PADDING;
	}

	priv->rect_xsz = mask & XPB_MASK_RECT_XSZ ?
		attr->rect_xsz :
		DEFAULT_RECT_XSZ;

	// Minimum of 5: 2 for outer box, 2 for inner box, 1 for filling
	if (priv->rect_xsz < 5) {
		return XPB_STATUS_BAD_XSZ;
	}

	priv->rect_ysz = mask & XPB_MASK_RECT_YSZ ?
		attr->rect_ysz :
		DEFAULT_RECT_YSZ;

	// Same as above
	if (priv->rect_ysz < 5) {
		return XPB_STATUS_BAD_YSZ;
	}

	priv->xsz = calc_xsize(priv->rect_xsz, priv->padding, priv->nrect);
	priv->ysz = calc_ysize(priv->rect_ysz, priv->padding);

	priv->xpos = mask & XPB_MASK_XPOS ?
		attr->xpos :
		screen_xsz / 2 - (priv->xsz / 2);

	if (priv->xpos > screen_xsz) {
		return XPB_STATUS_BAD_XPOS;
	}

	priv->ypos = mask & XPB_MASK_YPOS ?
		attr->ypos :
		screen_ysz * 15 / 16 - (priv->ysz / 2);

	if (priv->ypos > screen_ysz) {
		return XPB_STATUS_BAD_YPOS;
	}

	// Throw an error if the bar goes offscreen
	if ((priv->xpos + priv->xsz) > screen_xsz ||
		(priv->ypos + priv->ysz) > screen_ysz) {
		return XPB_STATUS_TOO_LARGE;
	}

	return XPB_STATUS_SUCCESS;
}

xpb_status_t alloc_colors(struct xpb *bar, uint16_t mask, struct xpb_attr *attr)
{
	Colormap cmap = DefaultColormap(bar->dpy, 0);
	struct xpb_priv *priv = XPB_PRIV(bar);
	xpb_status_t status;

	status = XAllocNamedColor(bar->dpy,
		cmap,
		mask & XPB_MASK_FG ? attr->fg : DEFAULT_FG,
		&priv->fg,
		&priv->fg);

	if (!status) {
		return XPB_STATUS_BAD_FG;
	}

	status = XAllocNamedColor(bar->dpy,
		cmap,
		mask & XPB_MASK_BG ? attr->bg : DEFAULT_BG,
		&priv->bg,
		&priv->bg);

	if (!status) {
		return XPB_STATUS_BAD_BG;
	}

	return XPB_STATUS_SUCCESS;
}

// Creates a window and the associated graphics context
void create_window(struct xpb *bar)
{
	XSetWindowAttributes wa;
	unsigned long vmask;
	int screen = DefaultScreen(bar->dpy);

	struct xpb_priv *priv = XPB_PRIV(bar);

	wa.override_redirect = True;
	wa.background_pixmap = ParentRelative;
	wa.event_mask = ExposureMask | KeyPressMask | VisibilityChangeMask;

	vmask = CWOverrideRedirect | CWBackPixmap | CWEventMask;

	priv->win = XCreateWindow(bar->dpy,
		bar->root,
		priv->xpos,
		priv->ypos,
		priv->xsz,
		priv->ysz,
		0,
		DefaultDepth(bar->dpy, screen),
		CopyFromParent,
		DefaultVisual(bar->dpy, screen),
		vmask,
		&wa);

	priv->gc = XCreateGC(bar->dpy, priv->win, 0, NULL);
}

xpb_status_t xpb_init(uint16_t mask, struct xpb_attr *attr, struct xpb **bar_out)
{
	xpb_status_t status;
	struct xpb *bar;
	struct xpb_priv *priv;

	if (bar_out == NULL || (attr == NULL && mask != 0)) {
		return XPB_STATUS_BAD_PTR;
	}

	bar = malloc(sizeof(struct xpb));
	if (bar == NULL) {
		return XPB_STATUS_NOMEM;
	}

	bar->priv = malloc(sizeof(struct xpb_priv));
	if (bar->priv == NULL) {
		free(bar);
		return XPB_STATUS_NOMEM;
	}

	priv = XPB_PRIV(bar);

	bar->dpy = XOpenDisplay(NULL);
	bar->root = RootWindow(bar->dpy, 0);

	status = set_dimensions(bar, mask, attr);
	if (status != XPB_STATUS_SUCCESS) {
		goto error;
	}

	status = alloc_colors(bar, mask, attr);
	if (status != XPB_STATUS_SUCCESS) {
		goto error;
	}

	create_window(bar);

	XMapRaised(bar->dpy, priv->win);
	XFlush(bar->dpy);

	*bar_out = bar;
	return XPB_STATUS_SUCCESS;

error:
	XCloseDisplay(bar->dpy);
	free(priv);
	free(bar);

	return status;
}

xpb_status_t xpb_draw(struct xpb *bar, uint16_t current, uint16_t max)
{
	unsigned int i;
	unsigned int base_x_offset, base_y_offset;
	unsigned int brightness_percent = current * 100 / max;
	struct xpb_priv *priv;

	if (bar == NULL) {
		return XPB_STATUS_BAD_PTR;
	}

	priv = XPB_PRIV(bar);

	XSetForeground(bar->dpy, priv->gc, priv->fg.pixel);
	XDrawRectangle(bar->dpy,
		priv->win,
		priv->gc,
		0,
		0,
		priv->xsz - 1,
		priv->ysz - 1);

	XSetForeground(bar->dpy, priv->gc, priv->bg.pixel);
	XFillRectangle(bar->dpy,
		priv->win,
		priv->gc,
		1,
		1,
		priv->xsz - 2,
		priv->ysz - 2);

	XSetForeground(bar->dpy, priv->gc, priv->fg.pixel);

	base_x_offset = 1 + priv->padding;
	base_y_offset = 1 + priv->padding;

	for (i = 0; i < priv->nrect; i++) {
		unsigned int x_offset = base_x_offset + i * (priv->rect_xsz + priv->padding);
		unsigned int y_offset = base_y_offset;

		float fill_percent = get_fill_percent(brightness_percent,
			i * (100.0 / (float)priv->nrect),
			(i + 1) * (100.0 / (float)priv->nrect));

		XDrawRectangle(bar->dpy,
			priv->win,
			priv->gc,
			x_offset,
			y_offset,
			priv->rect_xsz - 1,
			priv->rect_ysz - 1);

		XFillRectangle(bar->dpy,
			priv->win,
			priv->gc,
			x_offset + 2,
			y_offset + 2,
			(int)((priv->rect_xsz - 4) * fill_percent),
			priv->rect_ysz - 4);
	}

	return XPB_STATUS_SUCCESS;
}

xpb_status_t xpb_cleanup(struct xpb *bar)
{
	struct xpb_priv *priv;

	if (bar == NULL) {
		return XPB_STATUS_BAD_PTR;
	}

	priv = XPB_PRIV(bar);

	XDestroyWindow(bar->dpy, priv->win);
	XCloseDisplay(bar->dpy);

	free(priv);
	free(bar);

	return XPB_STATUS_SUCCESS;
}

const char *xpb_status_tostring(xpb_status_t status)
{
	static const char *strmap[XPB_STATUS_END] = {
		"success",                  // SUCCESS
		"bad number of rectangles", // BAD_NRECT
		"bad padding number",       // BAD_PADDING
		"bad rectangle x-size",     // BAD_XSZ
		"bad rectangle y-size",     // BAD_YSZ
		"bad x-position",           // BAD_XPOS
		"bad y-position",           // BAD_YPOS
		"bad foreground color",     // BAD_FG
		"bad background color",     // BAD_BG
		"out of memory",            // NOMEM
		"bar too large",            // TOO_LARGE
		"bad pointer"               // BAD_PTR
	};

	if (status >= XPB_STATUS_END) {
		return NULL;
	}

	return strmap[status];
}

