
The BananaBread Engine
======================

A port of the Cube 2/Sauerbraten 3D game engine/first person shooter to the
web, compiling C++ and OpenGL to JavaScript and WebGL using Emscripten.

See the **[FAQ](https://github.com/kripken/BananaBread/wiki/FAQ)**.


Demos
-----

**[A live demo is available](https://kripken.github.io/BananaBread/cube2/bb.html)**. Note that it requires [WebAssembly](http://webassembly.org/).


Overview
--------

Cube 2 is a compact and efficient 3D game engine. By compiling it
to JavaScript and WebGL you can run a complete first person
shooter in your web browser, using only standard web APIs and
without any plugins.

Features:

 * A multitude of visual effects including water reflection/refraction,
   parallax mapping, glare, particle effects,
   lightmaps, skeletal animation (on gpu), etc. etc.
 * Streamlined and quick physics system
  * Ragdoll physics
 * Bot AI with adjustable skill level
 * Integrated in-game editor
 * Fast performance both running on the web or natively
 * zlib license

The original engine also has a lot of other features not yet
enabled (but will be):

 * Multiplayer
  * Multiplayer editing
 * Shadowmapping


Building
--------

Get emscripten and its dependencies,

  http://emscripten.org

It's recommended to go through the emscripten tutorial to see that it is set
up properly.

You will also need crunch if you want smaller downloads,

  https://github.com/richgel999/crunch

Build in crnlib using the Makefile. You can disable
crunch if you don't want it, remove all mentions of crunch in
`cube2/src/web/Makefile`.

Then do `emmake make` in `cube2/src/web` for a web build. (You can also do
`make` in `cube2/src/native` for a native linux build.)

You can then run bb.html to see the output.

If you have any problems building, feel free to file an issue here or to
find us on emscripten IRC (see [emscripten site](http://emscripten.org)).

 * See needed.txt for some possibly useful scripts to package the output.


Running
-------

0. Run
     python -m SimpleHTTPServer 8888
   in cube2/

1. Load localhost:8888 in your browser


Modding
-------

To use maps of your own or make other kinds of changes or additions to the
artwork, see the
**[Modding](https://github.com/kripken/BananaBread/wiki/Modding)**
page.


Debugging
---------

If you append `,debug` to the URL of one of the levels, it will use
`bb.debug.js` instead of `bb.js`. The debug build has not been run
through closure compiler and is much more readable for stack traces
and profiling and so forth.


License
-------

### Code

Code is zlib licensed (just like Sauerbraten):

Copyright (C) 2001-2012 Sauerbraten authors (see cube2/src/readme_source.txt)
and BananaBread authors (see below)

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

BananaBread authors:

 * Alon Zakai
 * Gregor Koch
 * Bobby Richter

### Art

All art content in this project (levels/maps, character model, etc.
etc.) is either CC-BY or CC-BY-SA, which means you can use it in
your projects, including commercial ones. If a directory does not
contain a specific license file, then it is new artwork created for
this project, which has the CC-BY license,

https://creativecommons.org/licenses/by/3.0/

and copyright is held by the Mozilla Foundation.

Previously-existing content can also be CC-BY-SA, see license files in
specific directories.

