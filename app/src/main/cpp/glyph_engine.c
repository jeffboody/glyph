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

#define LOG_TAG "glyph"
#include "libcc/cc_log.h"
#include "libcc/cc_memory.h"
#include "libcc/cc_timestamp.h"
#include "libvkk/vkk_platform.h"
#include "glyph_engine.h"

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

	self->engine = engine;

	return self;
}

void glyph_engine_delete(glyph_engine_t** _self)
{
	ASSERT(_self);

	glyph_engine_t* self = *_self;
	if(self)
	{
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
		1.0f, 0.0f, 1.0f, 1.0f
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
	}
	else if(event->type == VKK_PLATFORM_EVENTTYPE_CONTENT_RECT)
	{
		self->content_rect_top    = event->content_rect.t;
		self->content_rect_left   = event->content_rect.l;
		self->content_rect_width  = event->content_rect.b - event->content_rect.t;
		self->content_rect_height = event->content_rect.r - event->content_rect.l;
	}
}
