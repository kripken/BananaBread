
BananaBread
===========

A port of the Cube 2/Sauerbraten first person shooter to the web,
compiling C++ and OpenGL to JavaScript and WebGL using Emscripten.

Online demo: http://syntensity.com/static/bb/client.html


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
2. Press the "fullscreen" button
3. Click "GO!"
4. See instructions in disclaimer.html


License
-------

Code is zlib licensed (just like Sauerbraten). Content is CC-BY or
CC-BY-SA, see license files in specific directories. An exception
are the character models, their license is sadly more restrictive (we
hope to get new ones when possible), see details in

  packages/models/snoutx10k/

