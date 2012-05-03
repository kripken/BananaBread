
BananaBread
===========

A port of the Cube 2/Sauerbraten first person shooter to the web,
compiling C++ and OpenGL to JavaScript and WebGL using Emscripten.

Code is zlib licensed, content is CC-BY or CC-BY-SA.


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
4. Move with WASD, jump with space, look around with the mouse. You can shoot
   a little by clicking the mouse. Some editing stuff works (press 'e' and fly
   around)

Please note that

 * The C++ game code has not been optimized at all in any way yet
 * The generated JavaScript is itself not fully optimized yet, nor even minified
 * The level you see when you press "GO!" was made by azakai, a person with 0
   artistic talent
 * The game assets (textures) have not been optimized for faster downloads at all 

So this is a very *very* early demo - ignore performance and content quality!

