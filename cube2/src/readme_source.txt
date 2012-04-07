Sauerbraten source code license, usage, and documentation.

You may use the Sauerbraten source code if you abide by the ZLIB license
http://www.opensource.org/licenses/zlib-license.php
(very similar to the BSD license):


LICENSE
=======

Sauerbraten game engine source code, any release.

Copyright (C) 2001-2011 Wouter van Oortmerssen, Lee Salzman, Mike Dysart, Robert Pointon, and Quinton Reeves

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


LICENSE NOTES
=============
The license covers the source code found in the "src" directory of this
archive as well as the .cfg files under the "data" directory. The included 
ENet network library which Sauerbraten uses is covered by an MIT-style 
license, which is however compatible with the above license for all 
practical purposes.

Game media included in the game (maps, textures, sounds, models etc.)
are NOT covered by this license, and may have individual copyrights and
distribution restrictions (see individual readmes).


USAGE
=====
Compiling the sources should be straight forward.

Unix users need to make sure to have the development version of all libs
installed (OpenGL, SDL, SDL_mixer, SDL_image, zlib). The included
Makefile can be used to build.

Windows users can use the included Visual Studio project files in the vcpp 
directory,  which references the lib/include directories for the external 
libraries and should thus be self contained. Release mode builds will place 
executables in the bin dir ready for testing and distribution.

An alternative to Visual Studio for Windows is MinGW/MSYS, which can be compiled
using the provided Makefile. Another alternative for Windows is to compile under
Code::Blocks with the provided vcpp/sauerbraten.cbp project file.

The Sauerbraten sources are very small, compact, and non-redundant, so anyone
wishing to modify the source code should be able to gain an overview of
Sauerbraten's inner workings by simply reading through the source code in its
entirety. Small amounts of comments should guide you through the more
tricky sections.

When reading the source code and trying to understand Sauerbaten's internal design,
keep in mind the goal of Cube: minimalism. I wanted to create a very complete
game / game engine with absolutely minimal means, and made a sport out of it
keeping the implementation small and simple. Sauerbraten is not a commercial 
product, it is merely the author's idea of a fun little programming project.


AUTHORS
======
Wouter "Aardappel" van Oortmerssen
http://strlen.com

Lee "eihrul" Salzman 
http://lee.fov120.com

Mike "Gilt" Dysart

Robert "baby-rabbit" Pointon
http://www.fernlightning.com

Quinton "Quin" Reeves
http://www.redeclipse.net

For additional authors/contributors, see the Sauerbraten binary distribution readme.

