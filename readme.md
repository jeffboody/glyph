About
=====

Glyph is an experimental project for testing glyph
and font rendering techniques such as the following
candidates.

1. [Easy Scalable Text Rendering on the GPU](https://medium.com/@evanwallace/easy-scalable-text-rendering-on-the-gpu-c3f4d782c5ac)
2. [Resolution Independent Curve Rendering using Programmable Graphics Hardware](https://www.microsoft.com/en-us/research/wp-content/uploads/2005/01/p1000-loop.pdf)
3. [Signed Distance Estimation](https://github.com/kocsis1david/font-demo/blob/master/howitworks.md)

Glyph Description
=================

This project uses a very minimalistic glyph description
based on TTF fonts where glyphs are represented by a set of
points, contours and tags. Here is a very simple example
JSON file which includes a single square glyph although
multiple glyphs are allowed. The glyph data fields are the
index (i), width (w), height (h), number of points (np),
points (p), tags (t), number of contours (nc) and contours
(c).

	[
	  {
	    "i"=0,
	    "w"=10.0,
	    "h"=10.0,
	    "np"=4,
	    "p"=[2.5,2.5,
	         2.5,7.5,
	         7.5,7.5,
	         7.5,2.5],
	    "t"=[1,1,1,1],
	    "nc"=1,
	    "c"=[3]
	  }
	]

Use the
[font-outline](https://github.com/jeffboody/a3d-fonts/tree/master/font-outline)
program to convert TTF fonts to the glyph description.

License
=======

The Glyph software was implemented by
[Jeff Boody](mailto:jeffboody@gmail.com)
under The MIT License.

	Copyright (c) 2022 Jeff Boody

	Permission is hereby granted, free of charge, to any person obtaining a
	copy of this software and associated documentation files (the "Software"),
	to deal in the Software without restriction, including without limitation
	the rights to use, copy, modify, merge, publish, distribute, sublicense,
	and/or sell copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included
	in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
