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

#ifndef glyph_object_H
#define glyph_object_H

#include "jsmn/wrapper/jsmn_wrapper.h"
#include "libcc/math/cc_vec2f.h"
#include "libvkk/vkk_vg.h"

typedef struct glyph_object_s
{
	char* name;

	float w;
	float h;

	// points
	int         np;
	cc_vec2f_t* p;

	// tags
	int* t;

	// contours
	int  nc;
	int* c;

	// build glyph on demand
	vkk_vgPolygon_t* poly;

	int last_steps;
	int last_thresh;
} glyph_object_t;

glyph_object_t*  glyph_object_new(jsmn_object_t* obj);
void             glyph_object_delete(glyph_object_t** _self);
vkk_vgPolygon_t* glyph_object_build(glyph_object_t* self,
                                    vkk_vgPolygonBuilder_t* pb,
                                    int steps,
                                    int thresh);

#endif
