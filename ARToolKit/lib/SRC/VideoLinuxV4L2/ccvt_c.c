/*  CCVT: ColourConVerT: simple library for converting colourspaces
    Copyright (C) 2002 Nemosoft Unv.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For questions, remarks, patches, etc. for this program, the author can be
    reached at nemosoft@smcc.demon.nl.
*/

#include "ccvt.h"

#define PUSH_RGB24	1
#define PUSH_BGR24	2
#define PUSH_RGB32	3
#define PUSH_BGR32	4


/* This is a simplistic approach. */
static void ccvt_420p(int width, int height, const unsigned char *src, unsigned char *dst, int push)
{
	int line, col, linewidth;
	int y, u, v, yy, vr, ug, vg, ub;
	int r, g, b;
	const unsigned char *py, *pu, *pv;

	linewidth = width >> 1;
	py = src;
	pu = py + (width * height);
	pv = pu + (width * height) / 4;

	y = *py++;
	yy = y << 8;
	u = *pu - 128;
	ug =   88 * u;
	ub =  454 * u;
	v = *pv - 128;
	vg =  183 * v;
	vr =  359 * v;

	for (line = 0; line < height; line++) {
		for (col = 0; col < width; col++) {
			r = (yy +      vr) >> 8;
			g = (yy - ug - vg) >> 8;
			b = (yy + ub     ) >> 8;

			if (r < 0)   r = 0;
			if (r > 255) r = 255;
			if (g < 0)   g = 0;
			if (g > 255) g = 255;
			if (b < 0)   b = 0;
			if (b > 255) b = 255;

			switch(push) {
			case PUSH_RGB24:
				*dst++ = r;
				*dst++ = g;
				*dst++ = b;
				break;

			case PUSH_BGR24:
				*dst++ = b;
				*dst++ = g;
				*dst++ = r;
				break;

			case PUSH_RGB32:
				*dst++ = r;
				*dst++ = g;
				*dst++ = b;
				*dst++ = 0;
				break;

			case PUSH_BGR32:
				*dst++ = b;
				*dst++ = g;
				*dst++ = r;
				*dst++ = 0;
				break;
			}

			y = *py++;
			yy = y << 8;
			if (col & 1) {
				pu++;
				pv++;

				u = *pu - 128;
				ug =   88 * u;
				ub =  454 * u;
				v = *pv - 128;
				vg =  183 * v;
				vr =  359 * v;
			}
		} /* ..for col */
		if ((line & 1) == 0) { // even line: rewind
			pu -= linewidth;
			pv -= linewidth;
		}
	} /* ..for line */
}

static void ccvt_yuyv(int width, int height, const unsigned char *src, unsigned char *dst, int push)
{
  int i;
  const unsigned char *py0, *py1, *pu, *pv;
  int y0, y1, u, ug, ub, v, vg, vr;
  int r0, g0, b0, r1, g1, b1;

  for( i =0; i <width * height / 2; i++){

    py0 =src++;
    pu  =src++;
    py1 =src++;
    pv  =src++;

    y0 =*py0;
    y1 =*py1;
    
    y0 <<=8;
    y1 <<=8;

    u  =*pu - 128;
    ug =88 * u;
    ub =454 * u;
    
    v  =*pv - 128;
    vg =183 * v;
    vr =359 * v;
    
    r0 = (y0 +      vr) >>8;
    g0 = (y0 - ug - vg) >>8;
    b0 = (y0 + ub     ) >>8;
    
    r1 = (y1 +      vr) >>8;
    g1 = (y1 - ug - vg) >>8;
    b1 = (y1 + ub     ) >>8;

    if (r0 < 0)   r0 =0;
    if (r0 > 255) r0 =255;
    if (g0 < 0)   g0 =0;
    if (g0 > 255) g0 =255;
    if (b0 < 0)   b0 =0;
    if (b0 > 255) b0 =255;
    
    if (r1 < 0)   r1 =0;
    if (r1 > 255) r1 =255;
    if (g1 < 0)   g1 =0;
    if (g1 > 255) g1 =255;
    if (b1 < 0)   b1 =0;
    if (b1 > 255) b1 =255;

    switch( push) {
      
    case PUSH_RGB24:
      *dst++ =r0;
      *dst++ =g0;
      *dst++ =b0;
      
      *dst++ =r1;
      *dst++ =g1;
      *dst++ =b1;

      break;

    case PUSH_RGB32:
      *dst++ =r0;
      *dst++ =g0;
      *dst++ =b0;
      *dst++ =0;

      *dst++ =r1;
      *dst++ =g1;
      *dst++ =b1;
      *dst++ =0;

      break;

    case PUSH_BGR32:
      *dst++ =b0;
      *dst++ =g0;
      *dst++ =r0;
      *dst++ =0;

      *dst++ =b1;
      *dst++ =g1;
      *dst++ =r1;
      *dst++ =0;
      
      break;
    }
  }
}

void ccvt_420p_rgb24(int width, int height, const void *src, void *dst)
{
	ccvt_420p(width, height, (const unsigned char *)src, (unsigned char *)dst, PUSH_RGB24);
}

void ccvt_420p_bgr24(int width, int height, const void *src, void *dst)
{
	ccvt_420p(width, height, (const unsigned char *)src, (unsigned char *)dst, PUSH_BGR24);
}

void ccvt_420p_rgb32(int width, int height, const void *src, void *dst)
{
	ccvt_420p(width, height, (const unsigned char *)src, (unsigned char *)dst, PUSH_RGB32);
}

void ccvt_420p_bgr32(int width, int height, const void *src, void *dst)
{
	ccvt_420p(width, height, (const unsigned char *)src, (unsigned char *)dst, PUSH_BGR32);
}

void ccvt_yuyv_rgb24(int width, int height, const void *src, void *dst)
{
  ccvt_yuyv(width, height, (const unsigned char *)src, (unsigned char *)dst, PUSH_RGB24);
}

void ccvt_yuyv_rgb32(int width, int height, const void *src, void *dst)
{
  ccvt_yuyv(width, height, (const unsigned char *)src, (unsigned char *)dst, PUSH_RGB32);
}

void ccvt_yuyv_bgr32(int width, int height, const void *src, void *dst)
{ 
  ccvt_yuyv(width, height, (const unsigned char *)src, (unsigned char *)dst, PUSH_BGR32);
}

