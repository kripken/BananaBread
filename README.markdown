
The BananaBread Engine
======================

A port of the Cube 2/Sauerbraten 3D game engine/first person shooter to the
web, compiling C++ and OpenGL to JavaScript and WebGL using Emscripten.

**Online demo**: http://www.syntensity.com/static/night8/index.html

**Screenshots**: http://mozakai.blogspot.com/2012/06/bananabread-02-levels.html


Overview
--------

Cube 2 is a compact and efficient 3D game engine. By compiling it
to JavaScript and WebGL you can run a complete first person
shooter in your web browser, using only standard web APIs and
without any plugins.

Features:

 * A multitude of visual effects including water reflection/
   refraction, parallax mapping, glare, particle effects,
   lightmaps, skeletal animation (on gpu), etc. etc.
 * Compile a single codebase both to a native application and to
   the web.
 * Streamlined and quick physics system
  * Ragdoll physics
 * Bot AI with adjustable skill level.
 * Integrated in-game editor.
 * Fast performance both when compiled natively and to the web.
 * zlib license, see below.

The original engine also has a lot of other features not yet
enabled (but will be):

 * Multiplayer
  * Multiplayer editing
 * Shadowmapping


Building
--------

Do 'make' in cube2/src/web for a web build. You can also do 'make' in
cube2/src/native for a native linux build.


Running
-------

0. Run
     python -m SimpleHTTPServer 8888
   in cube2/
1. Load localhost:8888/client.html


License
-------

Code is zlib licensed (just like Sauerbraten).

New content created for this project specifically (for example, by
Gregor Koch or Alon Zakai) is all CC-BY. If there isn't a specific
license file, this is the license, so you can use that content
freely.

Existing content is mostly CC-BY or CC-BY-SA, see license files in
specific directories. An exception are the character models, their license
is sadly more restrictive (we hope to get new ones when possible), see
details in

  packages/models/snoutx10k/
  packages/models/hudguns/

