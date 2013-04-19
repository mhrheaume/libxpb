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

#define DEFAULT_NRECT 20
#define DEFAULT_PADDING 2
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

static uint8_t set_dimensions(struct xpb *bar,
	uint16_t mask,
	struct xpb_attr *attr);

static uint8_t alloc_colors(struct xpb *bar,
	uint16_t mask,
	struct xpb_attr *attr);

static void create_window(struct xpb *bar);

__attribute__((always_inline))
static inline uint16_t calc_xsize(rect_xsz, padding, nrect)
{
	return rect_xsz * nrect + padding * (nrect + 1) + 2;
}

__attribute__((always_inline))
static inline uint16_t calc_ysize(rect_ysz, padding)
{
	return rect_ysz + 2 * padding + 2;
}

static inline
float get_fill_percent(uint8_t brightness_percent, float lower, float upper)
{
	return
		brightness_percent >= upper ? 1.0 :
		brightness_percent <= lower ? 0 :
		(float)(brightness_percent - lower) / (float)(upper - lower);
}

uint8_t set_dimensions(struct xpb *bar, uint16_t mask, struct xpb_attr *attr)
{
	uint8_t screen = DefaultScreen(bar->dpy);
	uint16_t screen_xsz = DisplayWidth(bar->dpy, screen);
	uint16_t screen_ysz = DisplayHeight(bar->dpy, screen);

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

uint8_t alloc_colors(struct xpb *bar, uint16_t mask, struct xpb_attr *attr)
{
	Colormap cmap = DefaultColormap(bar->dpy, 0);
	struct xpb_priv *priv = XPB_PRIV(bar);
	uint8_t status;

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
	uint32_t vmask;
	uint8_t screen = DefaultScreen(bar->dpy);

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

uint8_t xpb_init(uint16_t mask, struct xpb_attr *attr, struct xpb **bar_out)
{
	uint8_t status;
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

uint8_t xpb_draw(struct xpb *bar, uint16_t current, uint16_t max)
{
	uint8_t i;
	uint16_t base_x_offset, base_y_offset;
	uint16_t brightness_percent = current * 100 / max;
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
		uint16_t x_offset = base_x_offset + i * (priv->rect_xsz + priv->padding);
		uint16_t y_offset = base_y_offset;

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

void xpb_cleanup(struct xpb *bar)
{
	struct xpb_priv *priv = XPB_PRIV(bar);

	XDestroyWindow(bar->dpy, priv->win);
	free(priv);

	XCloseDisplay(bar->dpy);
	free(bar);
}

const char *xpb_status_tostring(uint8_t status)
{
	const char *str;

	switch (status) {
	case XPB_STATUS_SUCCESS:
		str = "success";
		break;
	case XPB_STATUS_BAD_NRECT:
		str = "bad number of rectangles";
		break;
	case XPB_STATUS_BAD_PADDING:
		str = "bad padding number";
		break;
	case XPB_STATUS_BAD_XSZ:
		str = "bad rectangle x size";
		break;
	case XPB_STATUS_BAD_YSZ:
		str = "bad rectangle y size";
		break;
	case XPB_STATUS_BAD_XPOS:
		str = "bad x position";
		break;
	case XPB_STATUS_BAD_YPOS:
		str = "bad y position";
		break;
	case XPB_STATUS_BAD_FG:
		str = "bad primary foreground color";
		break;
	case XPB_STATUS_BAD_BG:
		str = "bad background color";
		break;
	case XPB_STATUS_NOMEM:
		str = "out of memory";
		break;
	case XPB_STATUS_TOO_LARGE:
		str = "bar too large";
		break;
	case XPB_STATUS_BAD_PTR:
		str = "bad pointer";
		break;
	default:
		str = "unknown status";
		break;
	}

	return str;
}

