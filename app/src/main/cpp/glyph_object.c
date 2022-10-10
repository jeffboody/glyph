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

#include <stdlib.h>
#include <string.h>

#define LOG_TAG "glyph"
#include "libcc/cc_log.h"
#include "libcc/cc_memory.h"
#include "glyph_object.h"

/***********************************************************
* private                                                  *
***********************************************************/

static cc_vec2f_t*
glyph_newPoints(int np, jsmn_array_t* array)
{
	ASSERT(array);

	int size = cc_list_size(array->list);
	if(size != (2*np))
	{
		LOGE("invalid size=%i, np=%i", size, np);
		return NULL;
	}

	cc_vec2f_t* p;
	p = (cc_vec2f_t*) CALLOC(np, sizeof(cc_vec2f_t));
	if(p == NULL)
	{
		LOGE("CALLOC failed");
		return NULL;
	}

	// parse points
	int idx = 0;
	cc_listIter_t* iter = cc_list_head(array->list);
	while(iter)
	{
		jsmn_val_t* val = (jsmn_val_t*) cc_list_peekIter(iter);
		if(val->type != JSMN_TYPE_PRIMITIVE)
		{
			LOGE("invalid type=%i", val->type);
			goto fail_val;
		}
		p[idx].x = (float) strtod(val->data, NULL);

		iter = cc_list_next(iter);

		val  = (jsmn_val_t*) cc_list_peekIter(iter);
		if(val->type != JSMN_TYPE_PRIMITIVE)
		{
			LOGE("invalid type=%i", val->type);
			goto fail_val;
		}
		p[idx].y = (float) strtod(val->data, NULL);

		iter = cc_list_next(iter);
		++idx;
	}

	// success
	return p;

	// failure
	fail_val:
		FREE(p);
	return NULL;
}

static int*
glyph_newTags(int np, jsmn_array_t* array)
{
	ASSERT(array);

	int size = cc_list_size(array->list);
	if(size != np)
	{
		LOGE("invalid size=%i, np=%i", size, np);
		return NULL;
	}

	int* t;
	t = (int*) CALLOC(np, sizeof(int));
	if(t == NULL)
	{
		LOGE("CALLOC failed");
		return NULL;
	}

	// parse tags
	int idx = 0;
	cc_listIter_t* iter = cc_list_head(array->list);
	while(iter)
	{
		jsmn_val_t* val = (jsmn_val_t*) cc_list_peekIter(iter);
		if(val->type != JSMN_TYPE_PRIMITIVE)
		{
			LOGE("invalid type=%i", val->type);
			goto fail_val;
		}
		t[idx] = (int) strtol(val->data, NULL, 0);

		iter = cc_list_next(iter);
		++idx;
	}

	// success
	return t;

	// failure
	fail_val:
		FREE(t);
	return NULL;
}

static int*
glyph_newContour(int nc, jsmn_array_t* array)
{
	ASSERT(array);

	int size = cc_list_size(array->list);
	if(size != nc)
	{
		LOGE("invalid size=%i, nc=%i", size, nc);
		return NULL;
	}

	int* c;
	c = (int*) CALLOC(nc, sizeof(int));
	if(c == NULL)
	{
		LOGE("CALLOC failed");
		return NULL;
	}

	// parse contours
	int idx = 0;
	cc_listIter_t* iter = cc_list_head(array->list);
	while(iter)
	{
		jsmn_val_t* val = (jsmn_val_t*) cc_list_peekIter(iter);
		if(val->type != JSMN_TYPE_PRIMITIVE)
		{
			LOGE("invalid type=%i", val->type);
			goto fail_val;
		}
		c[idx] = (int) strtol(val->data, NULL, 0);

		iter = cc_list_next(iter);
		++idx;
	}

	// success
	return c;

	// failure
	fail_val:
		FREE(c);
	return NULL;
}

/***********************************************************
* public                                                   *
***********************************************************/

glyph_object_t*
glyph_object_new(jsmn_object_t* obj)
{
	ASSERT(obj);

	glyph_object_t* self;
	self = (glyph_object_t*)
	       CALLOC(1, sizeof(glyph_object_t));
	if(self == NULL)
	{
		LOGE("CALLOC failed");
		return NULL;
	}

	// initialize state
	self->i  = -1;
	self->w  = -1.0f;
	self->h  = -1.0f;
	self->np = -1;
	self->nc = -1;

	cc_listIter_t* iter = cc_list_head(obj->list);
	while(iter)
	{
		jsmn_keyval_t* kv;
		kv = (jsmn_keyval_t*) cc_list_peekIter(iter);

		if((strcmp(kv->key, "i") == 0) &&
		   (kv->val->type == JSMN_TYPE_PRIMITIVE))
		{
			if(self->i != -1)
			{
				LOGE("invalid %s", kv->val->data);
				goto fail_glyph;
			}

			self->i = (int) strtol(kv->val->data, NULL, 0);
		}
		else if((strcmp(kv->key, "w") == 0) &&
		        (kv->val->type == JSMN_TYPE_PRIMITIVE))
		{
			if(self->w != -1.0f)
			{
				LOGE("invalid %s", kv->val->data);
				goto fail_glyph;
			}

			self->w = strtof(kv->val->data, NULL);
		}
		else if((strcmp(kv->key, "h") == 0) &&
		        (kv->val->type == JSMN_TYPE_PRIMITIVE))
		{
			if(self->h != -1.0f)
			{
				LOGE("invalid %s", kv->val->data);
				goto fail_glyph;
			}

			self->h = strtof(kv->val->data, NULL);
		}
		else if((strcmp(kv->key, "np") == 0) &&
		        (kv->val->type == JSMN_TYPE_PRIMITIVE))
		{
			if(self->np != -1)
			{
				LOGE("invalid %s", kv->val->data);
				goto fail_glyph;
			}

			self->np = (int) strtol(kv->val->data, NULL, 0);
		}
		else if((strcmp(kv->key, "p") == 0) &&
		        (kv->val->type == JSMN_TYPE_ARRAY))
		{
			if((self->np < 0) || (self->p != NULL))
			{
				LOGE("invalid np=%i, p=%p",
				     self->np, self->p);
				goto fail_glyph;
			}

			self->p = glyph_newPoints(self->np, kv->val->array);
			if(self->p == NULL)
			{
				goto fail_glyph;
			}
		}
		else if((strcmp(kv->key, "t") == 0) &&
		        (kv->val->type == JSMN_TYPE_ARRAY))
		{
			if((self->np < 0) || (self->t != NULL))
			{
				LOGE("invalid np=%i, t=%p",
				     self->np, self->t);
				goto fail_glyph;
			}

			self->t = glyph_newTags(self->np, kv->val->array);
			if(self->t == NULL)
			{
				goto fail_glyph;
			}
		}
		else if((strcmp(kv->key, "nc") == 0) &&
		        (kv->val->type == JSMN_TYPE_PRIMITIVE))
		{
			if(self->nc != -1)
			{
				LOGE("invalid %s", kv->val->data);
				goto fail_glyph;
			}

			self->nc = (int) strtol(kv->val->data, NULL, 0);
		}
		else if((strcmp(kv->key, "c") == 0) &&
		        (kv->val->type == JSMN_TYPE_ARRAY))
		{
			if((self->nc < 0) || (self->c != NULL))
			{
				LOGE("invalid nc=%i, c=%p",
				     self->nc, self->c);
				goto fail_glyph;
			}

			self->c = glyph_newContour(self->nc, kv->val->array);
			if(self->c == NULL)
			{
				goto fail_glyph;
			}
		}

		iter = cc_list_next(iter);
	}

	// validate data
	if((self->i  < 0)     ||
	   (self->w  < 0.0f)  ||
	   (self->h  < 0.0f)  ||
	   (self->np < 0)     ||
	   (self->nc < 0)     ||
	   (self->p  == NULL) ||
	   (self->t  == NULL) ||
	   (self->c  == NULL))
	{
		LOGE("invalid i=%i, w=%f, h=%f, np=%i, nc=%i, p=%p, t=%p, c=%p",
		     self->i, self->w, self->h,
		     self->np, self->nc,
		     self->p, self->t, self->c);
		goto fail_glyph;
	}

	// success
	return self;

	// failure
	fail_glyph:
		FREE(self->c);
		FREE(self->t);
		FREE(self->p);
		FREE(self);
	return NULL;
}

void glyph_object_delete(glyph_object_t** _self)
{
	ASSERT(_self);

	glyph_object_t* self = *_self;
	if(self)
	{
		vkk_vgPolygon_delete(&self->poly);
		FREE(self->c);
		FREE(self->t);
		FREE(self->p);
		FREE(self);
		*_self = NULL;
	}
}

static int
glyph_object_interpolate(glyph_object_t* self,
                         vkk_vgPolygonBuilder_t* pb,
                         int* _first, int steps,
                         cc_vec2f_t* p0,
                         cc_vec2f_t* p1,
                         cc_vec2f_t* p2)
{
	ASSERT(self);
	ASSERT(pb);
	ASSERT(_first);
	ASSERT(p0);
	ASSERT(p2);
	ASSERT(p2);

	int i;
	float t;
	cc_vec2f_t p;
	for(i = 1; i <= steps; ++i)
	{
		t = ((float) i)/((float) steps);
		cc_vec2f_quadraticBezier(p0, p1, p2, t, &p);
		if(vkk_vgPolygonBuilder_point(pb, *_first,
		                              p.x,
		                              p.y) == 0)
		{
			return 0;
		}

		*_first = 0;
	}

	return 1;
}

vkk_vgPolygon_t*
glyph_object_build(glyph_object_t* self,
                   vkk_vgPolygonBuilder_t* pb,
                   int steps)
{
	ASSERT(self);
	ASSERT(pb);

	// check for cached polygon
	if(self->poly)
	{
		if(self->last_steps == steps)
		{
			return self->poly;
		}
		else
		{
			vkk_vgPolygon_delete(&self->poly);
		}
	}

	// minimal check to eliminate incomplete polygons
	// e.g. space character
	if(self->np < 3)
	{
		return NULL;
	}

	vkk_vgPolygonBuilder_reset(pb);

	// check algorithm
	if(steps == 0)
	{
		// naive algorithm
		int p;
		int c     = 0;
		int first = 1;
		for(p = 0; p < self->np; ++p)
		{
			// skip control points at start of contour
			if(first && (self->t[p] == 0))
			{
				continue;
			}

			// add non-control points
			if(self->t[p])
			{
				if(vkk_vgPolygonBuilder_point(pb, first,
				                              self->p[p].x,
				                              self->p[p].y) == 0)
				{
					return NULL;
				}

				first = 0;
			}

			// detect end of contour
			if(self->c[c] == p)
			{
				first = 1;
				++c;
			}
		}
	}
	else
	{
		// bezier interpolation algorithm
		// The following rules are applied to decompose the
		// contour's points into segments and arcs:
		// 1) Two successive ON points indicate a line
		//    segment joining them.
		// 2) One conic OFF point between two ON points
		//    indicates a conic Bezier arc, the OFF point
		//    being the control point, and the ON ones the
		//    start and end points.
		// 3) Two successive conic OFF points force the rasterizer
		//    to create a virtual ON point inbetween, at their
		//    exact middle.
		// 4) The last point in a contour uses the first as an end
		//    point to create a closed contour.
		// 5) The first point in a contour can be a conic OFF
		//    point itself. In that case, use the last point of
		//    the contour as the contour's starting point. If the
		//    last point is a conic OFF point itself, start the
		//    contour with the virtual ON point between the last
		//    and first point of the contour.
		// Note: Cubic curves are not supported.
		// https://freetype.org/freetype2/docs/glyphs/glyphs-6.html

		int c;
		int p;
		int p0;
		int p1;
		int p2;
		int t0;
		int t1;
		int t2;
		int start = 0;
		int end   = 0;
		int first = 1;
		cc_vec2f_t* pp0;
		cc_vec2f_t* pp1;
		cc_vec2f_t* pp2;
		cc_vec2f_t  ppi;
		cc_vec2f_t  ppj;
		for(c = 0; c < self->nc; ++c)
		{
			first = 1;
			end   = self->c[c];

			for(p = start; p <= end; ++p)
			{
				p0  = ((p - 1) < start) ? end : p - 1;
				p1  = p;
				p2  = ((p + 1) > end) ? start : p + 1;
				pp0 = &self->p[p0];
				pp1 = &self->p[p1];
				pp2 = &self->p[p2];
				t0  = self->t[p0];
				t1  = self->t[p1];
				t2  = self->t[p2];

				// apply contour rules
				// 000 - interpolate (pi,pj]
				// 001 - interpolate (pi,p2]
				// 01X - skip
				// 100 - interpolate (p0,pi]
				// 101 - interpolate (p0,p2]
				// 11X - straight line [p2]
				if((t0 == 0) && (t1 == 0) && (t2 == 0))
				{
					// add virtual point between p0 and p1
					ppi.x = pp0->x + (pp1->x - pp0->x)/2.0f;
					ppi.y = pp0->y + (pp1->y - pp0->y)/2.0f;

					// add virtual point between p1 and p2
					ppj.x = pp1->x + (pp2->x - pp1->x)/2.0f;
					ppj.y = pp1->y + (pp2->y - pp1->y)/2.0f;

					// interpolate contour between pi and pj
					if(glyph_object_interpolate(self, pb, &first, steps,
					                            &ppi, pp1, &ppj) == 0)
					{
						return NULL;
					}
				}
				else if((t0 == 0) && (t1 == 0) && t2)
				{
					// add virtual point between p0 and p1
					ppi.x = pp0->x + (pp1->x - pp0->x)/2.0f;
					ppi.y = pp0->y + (pp1->y - pp0->y)/2.0f;

					// interpolate contour between pi and p2
					if(glyph_object_interpolate(self, pb, &first, steps,
					                            &ppi, pp1, pp2) == 0)
					{
						return NULL;
					}
				}
				else if((t0 == 0) && t1)
				{
					// skip
				}
				else if(t0 && (t1 == 0) && (t2 == 0))
				{
					// add virtual point between p1 and p2
					ppj.x = pp1->x + (pp2->x - pp1->x)/2.0f;
					ppj.y = pp1->y + (pp2->y - pp1->y)/2.0f;

					// interpolate contour between p0 and pj
					if(glyph_object_interpolate(self, pb, &first, steps,
					                            pp0, pp1, &ppj) == 0)
					{
						return NULL;
					}
				}
				else if(t0 && (t1 == 0) && t2)
				{
					// interpolate contour between p0 and p2
					if(glyph_object_interpolate(self, pb, &first, steps,
					                            pp0, pp1, pp2) == 0)
					{
						return NULL;
					}
				}
				else if(t0 && t1)
				{
					// straight line
					if(vkk_vgPolygonBuilder_point(pb, first, pp1->x,
					                              pp1->y) == 0)
					{
						return NULL;
					}

					first = 0;
				}
			}

			start = end + 1;
		}
	}

	self->poly = vkk_vgPolygonBuilder_build(pb);

	self->last_steps = steps;

	return self->poly;
}
