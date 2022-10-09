/*
 * Copyright (c) 2022 Jeff Boody
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef glyph_engine_H
#define glyph_engine_H

#include "libvkk/vkk.h"

/***********************************************************
* public                                                   *
***********************************************************/

typedef struct glyph_engine_s
{
	vkk_engine_t* engine;

	double escape_t0;

	uint32_t content_rect_top;
	uint32_t content_rect_left;
	uint32_t content_rect_width;
	uint32_t content_rect_height;
} glyph_engine_t;

glyph_engine_t* glyph_engine_new(vkk_engine_t* engine);
void            glyph_engine_delete(glyph_engine_t** _self);
void            glyph_engine_pause(glyph_engine_t* self);
void            glyph_engine_draw(glyph_engine_t* self);
void            glyph_engine_event(glyph_engine_t* self,
                                   vkk_platformEvent_t* event);

#endif
