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

vkk_vgPolygon_t*
glyph_object_build(glyph_object_t* self,
                   vkk_vgPolygonBuilder_t* pb)
{
	ASSERT(self);
	ASSERT(pb);

	if(self->poly)
	{
		return self->poly;
	}

	vkk_vgPolygonBuilder_reset(pb);

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

	self->poly = vkk_vgPolygonBuilder_build(pb);

	return self->poly;
}
