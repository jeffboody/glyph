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

#include <stdio.h>
#include <stdlib.h>

#define LOG_TAG "glyph"
#include "libbfs/bfs_file.h"
#include "libbfs/bfs_util.h"
#include "libcc/cc_log.h"
#include "libcc/cc_memory.h"
#include "libcc/cc_timestamp.h"
#include "libvkk/vkk_platform.h"
#include "glyph_engine.h"
#include "glyph_object.h"

/***********************************************************
* private                                                  *
***********************************************************/

static int
glyph_engine_addGlyph(glyph_engine_t* self,
                      jsmn_val_t* val)
{
	ASSERT(self);
	ASSERT(val);

	if(val->type != JSMN_TYPE_OBJECT)
	{
		LOGE("invalid type=%i", val->type);
		return 0;
	}

	glyph_object_t* glyph = glyph_object_new(val->obj);
	if(glyph == NULL)
	{
		return 0;
	}

	if(cc_map_addf(self->map_glyph, glyph, "%i",
	               glyph->i) == NULL)
	{
		goto fail_add;
	}

	// success
	return 1;

	// failure
	fail_add:
		glyph_object_delete(&glyph);
	return 0;
}

static int
glyph_engine_addGlyphs(glyph_engine_t* self,
                       jsmn_val_t* root)
{
	ASSERT(self);
	ASSERT(root);

	if(root->type != JSMN_TYPE_ARRAY)
	{
		LOGE("invalid type=%i", root->type);
		return 0;
	}

	cc_listIter_t* iter = cc_list_head(root->array->list);
	while(iter)
	{
		jsmn_val_t* val;
		val = (jsmn_val_t*) cc_list_peekIter(iter);
		if(glyph_engine_addGlyph(self, val) == 0)
		{
			goto fail_add_glyph;
		}

		iter = cc_list_next(iter);
	}

	// success
	return 1;

	// failure
	fail_add_glyph:
	{
		cc_mapIter_t* miter = cc_map_head(self->map_glyph);
		while(miter)
		{
			glyph_object_t* glyph;
			glyph = (glyph_object_t*)
			        cc_map_remove(self->map_glyph, &miter);
			glyph_object_delete(&glyph);
		}
	}
	return 0;
}

static int
glyph_engine_loadGlyphs(glyph_engine_t* self)
{
	ASSERT(self);

	char resource[256];
	snprintf(resource, 256, "%s/resource.bfs",
	         vkk_engine_internalPath(self->engine));

	bfs_file_t* bfs;
	bfs = bfs_file_open(resource, 1, BFS_MODE_RDONLY);
	if(bfs == NULL)
	{
		return 0;
	}

	size_t size = 0;
	char*  str  = NULL;
	if(bfs_file_blobGet(bfs, 0,
	                    "BarlowSemiCondensed-Regular-1000.json",
	                    &size, (void**) &str) == 0)
	{
		goto fail_bfs;
	}

	jsmn_val_t* root = jsmn_val_new(str, size);
	if(root == NULL)
	{
		goto fail_jsmn;
	}

	if(glyph_engine_addGlyphs(self, root) == 0)
	{
		goto fail_add_glyphs;
	}

	// cleanup
	jsmn_val_delete(&root);
	FREE(str);
	bfs_file_close(&bfs);

	// success
	return 1;

	// failure
	fail_add_glyphs:
		jsmn_val_delete(&root);
	fail_jsmn:
		FREE(str);
	fail_bfs:
		bfs_file_close(&bfs);
	return 0;
}

static vkk_vgPolygon_t*
glyph_engine_defaultPoly(glyph_engine_t* self)
{
	ASSERT(self);

	vkk_vgPolygonBuilder_t* pb = self->vg_polygon_builder;

	vkk_vgPolygonBuilder_reset(pb);
	if(vkk_vgPolygonBuilder_point(pb, 1, 2.5f, 2.5f) &&
	   vkk_vgPolygonBuilder_point(pb, 0, 2.5f, 7.5f) &&
	   vkk_vgPolygonBuilder_point(pb, 0, 7.5f, 7.5f) &&
	   vkk_vgPolygonBuilder_point(pb, 0, 7.5f, 2.5f) &&
	   vkk_vgPolygonBuilder_point(pb, 0, 2.5f, 2.5f))
	{
		return vkk_vgPolygonBuilder_build(pb);
	}

	return NULL;
}

/***********************************************************
* public                                                   *
***********************************************************/

glyph_engine_t* glyph_engine_new(vkk_engine_t* engine)
{
	ASSERT(engine);

	glyph_engine_t* self;
	self = (glyph_engine_t*)
	       CALLOC(1, sizeof(glyph_engine_t));
	if(self == NULL)
	{
		LOGE("CALLOC failed");
		return NULL;
	}

	self->engine      = engine;
	self->glyph_i     = 'g';
	self->glyph_steps = 4;

	if(bfs_util_initialize() == 0)
	{
		goto fail_bfs;
	}

	vkk_renderer_t* rend;
	rend = vkk_engine_defaultRenderer(engine);

	self->vg_context = vkk_vgContext_new(rend);
	if(self->vg_context == NULL)
	{
		goto fail_vg_context;
	}

	self->vg_polygon_builder = vkk_vgPolygonBuilder_new(engine);
	if(self->vg_polygon_builder == NULL)
	{
		goto fail_vg_polygon_builder;
	}

	self->default_poly = glyph_engine_defaultPoly(self);
	if(self->default_poly == NULL)
	{
		goto fail_default_poly;
	}

	self->map_glyph = cc_map_new();
	if(self->map_glyph == NULL)
	{
		goto fail_map_glyph;
	}

	if(glyph_engine_loadGlyphs(self) == 0)
	{
		goto fail_load_glyphs;
	}

	// success
	return self;

	// failure
	fail_load_glyphs:
		cc_map_delete(&self->map_glyph);
	fail_map_glyph:
		vkk_vgPolygon_delete(&self->default_poly);
	fail_default_poly:
		vkk_vgPolygonBuilder_delete(&self->vg_polygon_builder);
	fail_vg_polygon_builder:
		vkk_vgContext_delete(&self->vg_context);
	fail_vg_context:
		bfs_util_shutdown();
	fail_bfs:
		FREE(self);
	return NULL;
}

void glyph_engine_delete(glyph_engine_t** _self)
{
	ASSERT(_self);

	glyph_engine_t* self = *_self;
	if(self)
	{
		cc_mapIter_t* miter = cc_map_head(self->map_glyph);
		while(miter)
		{
			glyph_object_t* glyph;
			glyph = (glyph_object_t*)
			        cc_map_remove(self->map_glyph, &miter);
			glyph_object_delete(&glyph);
		}

		cc_map_delete(&self->map_glyph);
		vkk_vgPolygon_delete(&self->default_poly);
		vkk_vgPolygonBuilder_delete(&self->vg_polygon_builder);
		vkk_vgContext_delete(&self->vg_context);
		FREE(self);
		*_self = NULL;
	}
}

void glyph_engine_pause(glyph_engine_t* self)
{
	ASSERT(self);
}

void glyph_engine_draw(glyph_engine_t* self)
{
	ASSERT(self);

	vkk_renderer_t* rend;
	rend = vkk_engine_defaultRenderer(self->engine);

	float clear_color[4] =
	{
		0.0f, 0.0f, 0.0f, 1.0f
	};
	if(vkk_renderer_beginDefault(rend,
	                             VKK_RENDERER_MODE_DRAW,
	                             clear_color) == 0)
	{
		return;
	}

	// query screen size
	uint32_t screen_wu;
	uint32_t screen_hu;
	vkk_renderer_surfaceSize(rend,
	                         &screen_wu,
	                         &screen_hu);

	// update screen, viewport and scissor
	float screen_x = 0.0f;
	float screen_y = 0.0f;
	float screen_w = (float) screen_wu;
	float screen_h = (float) screen_hu;
	if((self->content_rect_width  > 0) &&
	   (self->content_rect_height > 0) &&
	   (self->content_rect_width  <= screen_wu) &&
	   (self->content_rect_height <= screen_hu))
	{
		screen_x = (float) self->content_rect_left;
		screen_y = (float) self->content_rect_top;
		screen_w = (float) self->content_rect_width;
		screen_h = (float) self->content_rect_height;

		vkk_renderer_viewport(rend, screen_x, screen_y,
		                      screen_w, screen_h);
		vkk_renderer_scissor(rend,
		                     self->content_rect_left,
		                     self->content_rect_top,
		                     self->content_rect_width,
		                     self->content_rect_height);
	}

	float l = 0.0f;
	float r = 10.0f;
	float b = 10.0f;
	float t = 0.0f;

	vkk_vgPolygon_t* poly = self->default_poly;

	cc_mapIter_t* miter;
	miter = cc_map_findf(self->map_glyph, "%i",
	                     self->glyph_i);
	if(miter)
	{
		glyph_object_t* glyph;
		glyph = (glyph_object_t*) cc_map_val(miter);

		vkk_vgPolygon_t* tmp;
		tmp = glyph_object_build(glyph,
		                         self->vg_polygon_builder,
		                         self->glyph_steps);
		if(tmp)
		{
			poly = tmp;
			b    = glyph->h;
			l    = -(glyph->h - glyph->w)/2.0f;
			r    = l + glyph->h;
		}
	}

	vkk_vgPolygonStyle_t vg_polygon_style =
	{
		.color =
		{
			.r = 1.0f,
			.g = 0.0f,
			.b = 1.0f,
			.a = 1.0f,
		}
	};

	cc_mat4f_t mvp;
	cc_mat4f_orthoVK(&mvp, 1, l, r,
	                 b, t, 0.0f, 2.0f);
	vkk_vgContext_reset(self->vg_context, &mvp);
	vkk_vgContext_bindPolygons(self->vg_context);
	vkk_vgPolygon_draw(poly, self->vg_context,
	                   &vg_polygon_style);
	vkk_renderer_end(rend);
}

void glyph_engine_event(glyph_engine_t* self,
                        vkk_platformEvent_t* event)
{
	ASSERT(self);
	ASSERT(event);

	if((event->type == VKK_PLATFORM_EVENTTYPE_KEY_UP) ||
	   ((event->type == VKK_PLATFORM_EVENTTYPE_KEY_DOWN) &&
	    (event->key.repeat)))
	{
		if(event->key.keycode == VKK_PLATFORM_KEYCODE_ESCAPE)
		{
			// double tap back to exit
			double t1 = cc_timestamp();
			if((t1 - self->escape_t0) < 0.5)
			{
				vkk_engine_platformCmd(self->engine,
				                       VKK_PLATFORM_CMD_EXIT);
			}

			self->escape_t0 = t1;
		}
		else if((event->key.keycode >= '0') &&
		        (event->key.keycode <= '9'))
		{
			self->glyph_steps = event->key.keycode - '0';
			if(self->glyph_steps == 9)
			{
				self->glyph_steps = 16;
			}
		}
		else if((event->key.keycode >= 32) &&
		        (event->key.keycode <= 126))
		{
			self->glyph_i = event->key.keycode;
		}
	}
	else if(event->type == VKK_PLATFORM_EVENTTYPE_CONTENT_RECT)
	{
		self->content_rect_top    = event->content_rect.t;
		self->content_rect_left   = event->content_rect.l;
		self->content_rect_width  = event->content_rect.b - event->content_rect.t;
		self->content_rect_height = event->content_rect.r - event->content_rect.l;
	}
}
