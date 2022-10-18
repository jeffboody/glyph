About
=====

Glyph is an experimental project for testing glyph
and font rendering techniques. While researching this
topic I developed a rather simple technique to render
glyps with good quality. I'm sure others have proposed
similar algorithms but I did not encounter them during my
research. My proposed algorithm is called the Adaptive
Subdivision Algorithm (ASA) which I will be comparing
against a Fixed Subdivision Algorithm (FSA).

Fixed Subdivision Algorithm
===========================

The FSA is simply subdivides Bezier curve segments using N
subdivision steps to achieve the 'best' results. All
other aspects of FSA such as tesselation and anti-aliasing
are identical to ASA as described below.

Adaptive Subdivision Algorithm
==============================

The ASA also subdivides Bezier curve segments however it
uses a precise error function to minimize the number of
subdivision steps required to subdivide Bezier curve
segments for a given error threshold.

Subdivision
-----------

The subdivision is performed by interpolating the quadratic
Bezier curve function between two points on a curve (P0, P2)
and their corresponding control point (P1).

	B(t) = P1 + (1 - t)^2(P0 - P1) + t^2(P2 - P1), 0 <= t <= 1

The error resulting from interpolating the Bezier curve
is dependent on the number of subdivision steps, the
curvature and the length of the curve segment. The
following diagram shows various subdivisions of a Bezier
curve using 1 (A,E), 2 (A,C,E) and 4 (A,B,C,D,E)
subdivision steps.

![Subdivision](resource/adaptive-subdivision-interpolation.jpg?raw=true "Subdivision")

The absolute error can be approximated as the area of the
curve between the i-step solution and the best N-step
solution. This error can be computed exactly using Heron's
Formula.

	s = (a + b + c)/2 (semi-perimeter)
	area = sqrt(s*(s - a)*(s - b)*(s - c))
	where a, b, c are the lengths of each edge

Applying Heron's formula to the above diagram results in
the following absolute error functions.

	Eabs(A,E) = H(A,B,C) + H(A,C,D) + H(A,D,E)
	Eabs(A,C,E) = H(A,B,C) + H(C,D,E)
	Eabs(A,B,C,D,E) = 0

I initially found it surprising to discover that
subdividing curve segments with the highest absolute error
did not always result in the best improvements to perceived
visual quality. For long curves the absolute error is
spread out while short curves the error is isolated. In
addition, the error for longer curves is hard to detect
visually when anti-aliasing is enabled. To address this
deficiency, multiply the absolute error function by
1/length of the most subdivided curve to compute the
average error.

	Eavg(X) = Eabs(X)/Length(A,B,C,D,E)

I use an error threshold to determine the number of
subdivision steps to be performed for each Bezier curve
segment. The following plot shows how the number of curve
points and the error varies as the threshold changes for
the 'g' character. As you would expect, both the number of
points decreases and the error increases as the error
threshold increases.

![Points and Error vs Threshold](resource/adaptive-subdivision-plot.jpg?raw=true "Points and Error vs Threshold")

Contour Decomposition
---------------------

A glyph is defined by a set of points, tags and contours.
Points are simply the (x,y) coordinate, tags are a
per-point attribute which specifies if the point is ON/OFF
the curve and contours define the range of points for each
closed contour. The OFF points are the Bezier control
points which are also known as conic points. Multiple
contours may be required to describe points with multiple
parts (j) and those which include holes (g).

Process the points/tags defined by each contour separately.

For each point/tag in the contour, form a tag triplet which
is defined by the points/tags surrounding the current point
(p1).

	(t0, t1, t2)

The prev (p0, t0) and next (p2, t2) point/tag values wrap
around at the start/end of the contour so that the contour
forms a complete loop. In the event where two OFF points
occur in a row you should create a virtual point between.
The virtual point pi is the midpoint of p0 and p1 while the
virtual point pj is the midpoint of p1 and p2. The following
state machine describes how to process each tag triplet in
the contour. Notice that the subdivision/interpolation
ranges do not include the first point but do include the
last point.

	000 - interpolate (pi,pj]
	001 - interpolate (pi,p2]
	01X - skip
	100 - interpolate (p0,pj]
	101 - interpolate (p0,p2]
	11X - straight line [p2]

See the Glyph Description below for more details regarding
the glyph format and TTF decomposition rules.

Tesselation
-----------

The output of the contour decomposition and subdivision
stages is a sequence of points on the curves/contours which
can be fed into a tesselator library. My VKK library
includes a vector graphics module to generate polygons
(e.g. indexed triangles) using these sequences of points.
The underlying implementation uses the libtess2 library.

Here are some sample results which show the output of the
tesselator for various scenarios.

![Wireframe Tesselation](resource/adaptive-subdivision-wireframe.jpg?raw=true "Wireframe Tesselation")

1. Top-Left: FSA using 16 subdivision steps (517 points)
2. Top-Right: FSA using 3 subdivision steps (101 points)
3. Bottom-Left: ASA using thresh of 3 (152 points)
4. Bottom-Right: ASA using thresh of 13 (80 points)

Prior to contour decompositon the 'g' glyph contained 59
points (including contour points). The number of points
required varies depending on the character but seems like
's' and 'g' were the most complex requiring roughtly twice
as many points as the simple characters.

Minor visual artifacts for ASA could be detected with a
threshold between (3,13) and more severe artifacts were
observed as the threshold increased.

Anti-aliasing
-------------

The algorithms that I researched reduce the number of points
required by using shaders to evaluate the Bezier curve
functions directly. This however, causes a problem in that
the anti-aliasing stage becomes much more difficult and
requires advanced anti-aliasing techniques such as:

1. Shader based anti-aliasing with extended geometry
2. Smoothing by rendering multiple times then accumulating
   results using blending or color buffer accumulation

Unlike the shader based aproaches, ASA uses fully defined
geometry (up to a user defined error threshold) so it can
take advantage of built-in hardware MSAA to perform
anti-aliasing.

Another consideration for anti-aliasing is that modern
devices have very high screen density and I'm not convinced
that anti-aliasing will provide a significant benefit.

Results
-------

Here are some sample results which show a comparison of
rendering with various settings.

![ASA Comparision](resource/adaptive-subdivision-comparision.jpg?raw=true "ASA Comparision")

1. Top-Left: Non-MSAA FSA using 16 subdivision steps (517 points)
1. Top-Right: 4x MSAA FSA using 16 subdivision steps (517 points)
3. Bottom-Left: 4x MSAA ASA using thresh of 3 (152 points)
4. Bottom-Right: 4x MSAA ASA using thresh of 13 (80 points)

I believe that ASA can achieve nearly equivalent rendering
quality for typical use cases where infinite scaling is not
required. The ASA can achieve these results using 1.5x to
3.0x as many points while greatly reducing implementation
complexity.

Glyph Description
=================

This project uses a very minimalistic glyph description
based on TTF fonts where glyphs are represented by a set of
points, contours and tags. Here is a very simple example
JSON file which includes a single square glyph although
multiple glyphs are allowed. The glyph data fields are the
index (i), width (w), height (h), number of points (np),
points (p), tags (t), number of contours (nc) and contours
(c). The point origin is in the top-left corner. Tags are
used to describe the type of curve point which can be an ON
(1) point, a conic Bezier OFF (0) point or a cubic Bezier
OFF (2) point. Cubic Bezier points are not typically used
by TTF fonts and are not supported by Glyph.

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
program to convert
[TTF](https://freetype.org/freetype2/docs/glyphs/glyphs-6.html)
fonts to the glyph description.

Hotkeys
=======

As Glyph is an experimental app the following hotkeys are
used to select between various rendering options.

* 0: Naive Algorithm
* 1-9: Adjust subdivision steps of FSA
* -,=: Adjust error threshold of ASA
* a-z: Select glyph to display

Dependencies
============

* [https://github.com/jeffboody/a3d-fonts](https://github.com/jeffboody/a3d-fonts)
* [https://github.com/jpt/barlow/](https://github.com/jpt/barlow/)
* [https://github.com/jeffboody/jsmn](https://github.com/jeffboody/jsmn)
* [https://github.com/jeffboody/libbfs](https://github.com/jeffboody/libbfs)
* [https://github.com/jeffboody/libcc](https://github.com/jeffboody/libcc)
* [https://github.com/jeffboody/libexpat](https://github.com/jeffboody/libexpat)
* [https://github.com/jeffboody/libsqlite3](https://github.com/jeffboody/libsqlite3)
* [https://github.com/jeffboody/libtess2](https://github.com/jeffboody/libtess2)
* [https://github.com/jeffboody/libvkk](https://github.com/jeffboody/libvkk)
* [https://github.com/jeffboody/libxmlstream](https://github.com/jeffboody/libxmlstream)
* [https://github.com/jeffboody/myjpeg](https://github.com/jeffboody/myjpeg)
* [https://github.com/jeffboody/texgz](https://github.com/jeffboody/texgz)
* [https://github.com/lvandeve/lodepng](https://github.com/lvandeve/lodepng)

References
==========

* [Easy Scalable Text Rendering on the GPU](https://medium.com/@evanwallace/easy-scalable-text-rendering-on-the-gpu-c3f4d782c5ac)
* [Resolution Independent Curve Rendering using Programmable Graphics Hardware](https://www.microsoft.com/en-us/research/wp-content/uploads/2005/01/p1000-loop.pdf)
* [Multi-channel signed distance field generator](https://github.com/Chlumsky/msdfgen)
* [Signed Distance Estimation](https://github.com/kocsis1david/font-demo/blob/master/howitworks.md)
* [Bezier Curves](https://en.wikipedia.org/wiki/B%C3%A9zier_curve#Quadratic_B%C3%A9zier_curves)
* [Heron’s Formula](https://www.cuemath.com/measurement/area-of-triangle/)
* [FreeType Outlines](https://freetype.org/freetype2/docs/glyphs/glyphs-6.html)

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
