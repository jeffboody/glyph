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
#include "libvkk/vkk_platform.h"
#include "glyph_engine.h"

/***********************************************************
* callbacks                                                *
***********************************************************/

void* glyph_onCreate(vkk_engine_t* engine)
{
	ASSERT(engine);

	return glyph_engine_new(engine);
}

void glyph_onDestroy(void** _priv)
{
	ASSERT(_priv);

	glyph_engine_delete((glyph_engine_t**) _priv);
}

void glyph_onPause(void* priv)
{
	ASSERT(priv);

	glyph_engine_t* self = (glyph_engine_t*) priv;
	glyph_engine_pause(self);
}

void glyph_onDraw(void* priv)
{
	ASSERT(priv);

	glyph_engine_t* self = (glyph_engine_t*) priv;
	glyph_engine_draw(self);
}

void glyph_onEvent(void* priv, vkk_platformEvent_t* event)
{
	ASSERT(priv);
	ASSERT(event);

	glyph_engine_t* self = (glyph_engine_t*) priv;
	glyph_engine_event(self, event);
}

vkk_platformInfo_t VKK_PLATFORM_INFO =
{
	.app_name    = "Glyph",
	.app_version =
	{
		.major = 1,
		.minor = 0,
		.patch = 0,
	},
	.app_dir   = "Glyph",
	.onCreate  = glyph_onCreate,
	.onDestroy = glyph_onDestroy,
	.onPause   = glyph_onPause,
	.onDraw    = glyph_onDraw,
	.onEvent   = glyph_onEvent,
};
