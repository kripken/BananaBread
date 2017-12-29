
# You must define the EMSCRIPTEN env var

CXX = $(EMSCRIPTEN)/emcc
EMCONFIGURE = $(EMSCRIPTEN)/emconfigure
EMMAKE = $(EMSCRIPTEN)/emmake

CXXFLAGS= -O3 -s USE_ZLIB=1
override CXXFLAGS+= -Wall -fsigned-char -fno-exceptions -fno-rtti

PLATFORM= emscripten
PLATFORM_PREFIX= native

INCLUDES= -Ishared -Iengine -Ifpsgame -Ienet/include -I.

STRIP=
ifeq (,$(findstring -g,$(CXXFLAGS)))
ifeq (,$(findstring -pg,$(CXXFLAGS)))
  STRIP=strip
endif
endif

MV=mv

ifneq (,$(findstring MINGW,$(PLATFORM)))
WINDRES= windres
CLIENT_INCLUDES= $(INCLUDES) -Iinclude
CLIENT_LIBS= -mwindows -Llib -lSDL -lSDL_image -lSDL_mixer -lzdll -lopengl32 -lenet -lws2_32 -lwinmm
else	
CLIENT_INCLUDES= $(INCLUDES)
CLIENT_LIBS= -Lenet/.libs -lenet -lX11 -lSDL_image -lSDL_mixer -lz -lGL
endif
ifeq ($(PLATFORM),Linux)
CLIENT_LIBS+= -lrt
endif
CLIENT_OBJS= \
	shared/crypto.o \
	shared/geom.o \
	shared/stream.o \
	shared/tools.o \
	shared/zip.o \
	engine/3dgui.o \
	engine/bih.o \
	engine/blend.o \
	engine/blob.o \
	engine/client.o	\
	engine/command.o \
	engine/console.o \
	engine/cubeloader.o \
	engine/decal.o \
	engine/dynlight.o \
	engine/glare.o \
	engine/grass.o \
	engine/lightmap.o \
	engine/main.o \
	engine/material.o \
	engine/menus.o \
	engine/movie.o \
	engine/normal.o	\
	engine/octa.o \
	engine/octaedit.o \
	engine/octarender.o \
	engine/physics.o \
	engine/pvs.o \
	engine/rendergl.o \
	engine/rendermodel.o \
	engine/renderparticles.o \
	engine/rendersky.o \
	engine/rendertext.o \
	engine/renderva.o \
	engine/server.o	\
	engine/serverbrowser.o \
	engine/shader.o \
	engine/shadowmap.o \
	engine/sound.o \
	engine/texture.o \
	engine/water.o \
	engine/world.o \
	engine/worldio.o \
	fpsgame/ai.o \
	fpsgame/client.o \
	fpsgame/entities.o \
	fpsgame/fps.o \
	fpsgame/monster.o \
	fpsgame/movable.o \
	fpsgame/render.o \
	fpsgame/scoreboard.o \
	fpsgame/server.o \
	fpsgame/waypoint.o \
	fpsgame/weapon.o

CLIENT_PCH= shared/cube.h.gch engine/engine.h.gch fpsgame/game.h.gch

ifneq (,$(findstring MINGW,$(PLATFORM)))
SERVER_INCLUDES= -DSTANDALONE $(INCLUDES) -Iinclude
SERVER_LIBS= -mwindows -Llib -lzdll -lenet -lws2_32 -lwinmm
MASTER_LIBS= -Llib -lzdll -lenet -lws2_32 -lwinmm
else
SERVER_INCLUDES= -DSTANDALONE $(INCLUDES)
SERVER_LIBS= -Lenet/.libs -lenet -lz
MASTER_LIBS= $(SERVER_LIBS)
endif
SERVER_OBJS= \
	shared/crypto-standalone.o \
	shared/stream-standalone.o \
	shared/tools-standalone.o \
	engine/command-standalone.o \
	engine/server-standalone.o \
	engine/worldio-standalone.o \
	fpsgame/entities-standalone.o \
	fpsgame/server-standalone.o

MASTER_OBJS= \
	shared/crypto-standalone.o \
	shared/stream-standalone.o \
	shared/tools-standalone.o \
	engine/command-standalone.o \
	engine/master-standalone.o

ifeq ($(PLATFORM),SunOS)
CLIENT_LIBS+= -lsocket -lnsl -lX11
SERVER_LIBS+= -lsocket -lnsl
endif

default: all

all: client server

enet/Makefile:
	cp -r ../enet/* enet/  
	cd enet; $(EMCONFIGURE) ./configure --enable-shared=no --enable-static=yes
	
libenet: enet/Makefile
	$(EMMAKE) $(MAKE)	-C enet/ all

clean-enet: enet/Makefile
	$(MAKE) -C enet/ clean

clean:
	-$(RM) $(CLIENT_PCH) $(CLIENT_OBJS) $(SERVER_OBJS) $(MASTER_OBJS) sauer_client sauer_server sauer_master

%.h.gch: %.h
	$(CXX) $(CXXFLAGS) -o $(subst .h.gch,.tmp.h.gch,$@) $(subst .h.gch,.h,$@)
	$(MV) $(subst .h.gch,.tmp.h.gch,$@) $@

%-standalone.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $(subst -standalone.o,.cpp,$@)

$(CLIENT_OBJS): CXXFLAGS += $(CLIENT_INCLUDES)
$(filter shared/%,$(CLIENT_OBJS)): $(filter shared/%,$(CLIENT_PCH))
$(filter engine/%,$(CLIENT_OBJS)): $(filter engine/%,$(CLIENT_PCH))
$(filter fpsgame/%,$(CLIENT_OBJS)): $(filter fpsgame/%,$(CLIENT_PCH))

$(SERVER_OBJS): CXXFLAGS += $(SERVER_INCLUDES)
$(filter-out $(SERVER_OBJS),$(MASTER_OBJS)): CXXFLAGS += $(SERVER_INCLUDES)

ifneq (,$(findstring MINGW,$(PLATFORM)))
client: $(CLIENT_OBJS)
	$(WINDRES) -I vcpp -i vcpp/mingw.rc -J rc -o vcpp/mingw.res -O coff 
	$(CXX) $(CXXFLAGS) -o ../bin/sauerbraten.exe vcpp/mingw.res $(CLIENT_OBJS) $(CLIENT_LIBS)

server: $(SERVER_OBJS)
	$(WINDRES) -I vcpp -i vcpp/mingw.rc -J rc -o vcpp/mingw.res -O coff
	$(CXX) $(CXXFLAGS) -o ../bin/sauer_server.exe vcpp/mingw.res $(SERVER_OBJS) $(SERVER_LIBS)

master: $(MASTER_OBJS)
	$(CXX) $(CXXFLAGS) -o ../bin/sauer_master.exe $(MASTER_OBJS) $(MASTER_LIBS)

install: all
else
client.bc:	libenet $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) -o client.bc $(CLIENT_OBJS) $(CLIENT_LIBS)

BASE_FILES = data/ \
             packages/textures/ \
             packages/fonts/ \
             packages/icons/ \
             packages/particles/ \
             packages/sounds/aard/ \
             packages/sounds/q009/ \
             packages/sounds/yo_frankie/ \
             packages/gk/lava/lava_cc.dds \
             packages/gk/lava/lava_nm.dds \
             packages/caustics/ \
             packages/models/debris/ \
             packages/models/projectiles/ \
             packages/brushes/ \
             packages/hud/
             #packages/music/ \

SILLY_FILES = packages/base/silly.ogz \
              packages/base/silly.cfg \
              packages/base/silly_msg.png \
              packages/models/nut/ \
              packages/yo_frankie \
	            packages/skyboxes/philo/

MD_FILES = packages/base/md.ogz \
           packages/base/md.cfg \
           $(FANTASY_TEXTURES)

# Tip: use 'listtex' in the engine to check which textures are actually used! That is how these lists were generated.
TWO_TOWERS_FILES =  packages/base/two_towers.ogz \
                    packages/base/two_towers.cfg \
                    packages/base/two_towers.wpt \
                    packages/gk/fantasy/skyfantasyJPG/ \
                    `find packages/gk/fantasy -name "*.cfg"` \
                    packages/gk/fantasy/rock_formation_gk_v01/rock_formation_gk_v01_cc.dds \
                    packages/gk/fantasy/rock_formation_gk_v01/rock_formation_gk_v01_nm.dds \
                    packages/gk/fantasy/rock_formation_gk_v02/rock_formation_gk_v02_cc.dds \
                    packages/gk/fantasy/rock_formation_gk_v02/rock_formation_gk_v02_nm.dds \
                    packages/gk/fantasy/castell_wall_gk_v01/castell_wall_gk_v01_cc.dds \
                    packages/gk/fantasy/castell_wall_gk_v01/castell_wall_gk_v01_nm.dds \
                    packages/gk/fantasy/castell_wall_gk_v01/castell_wall_gk_v01_cc.dds \
                    packages/gk/fantasy/castell_wall_gk_v01/castell_wall_gk_v01_nm.dds \
                    packages/gk/fantasy/castell_wall_gk_v02/castell_wall_gk_v02_cc.dds \
                    packages/gk/fantasy/castell_wall_gk_v02/castell_wall_gk_v02_nm.dds \
                    packages/gk/fantasy/castell_wall_trim_gk_v01/castell_wall_trim_gk_v01_cc.dds \
                    packages/gk/fantasy/castell_wall_trim_gk_v01/castell_wall_trim_gk_v01_nm.dds \
                    packages/gk/fantasy/castell_wall_trim_gk_v01/castell_wall_trim_gk_v01_cc.dds \
                    packages/gk/fantasy/castell_wall_trim_gk_v01/castell_wall_trim_gk_v01_nm.dds \
                    packages/gk/fantasy/stone_ground_gk_v01/stone_ground_gk_v01_cc.dds \
                    packages/gk/fantasy/stone_ground_gk_v01/stone_ground_gk_v01_nm.dds \
                    packages/gk/fantasy/stone_ground_tiles_gk_v01/stone_ground_tiles_gk_v01_cc.dds \
                    packages/gk/fantasy/stone_ground_tiles_gk_v01/stone_ground_tiles_gk_v01_nm.dds \
                    packages/gk/fantasy/wooden_roof_tiles_gk_v01/wooden_roof_tiles_gk_v01_cc.dds \
                    packages/gk/fantasy/wooden_roof_tiles_gk_v01/wooden_roof_tiles_gk_v01_nm.dds \
                    packages/gk/fantasy/wooden_roof_tiles_gk_v01/wooden_roof_tiles_gk_v01_cc.dds \
                    packages/gk/fantasy/wooden_roof_tiles_gk_v01/wooden_roof_tiles_gk_v01_nm.dds \
                    packages/gk/fantasy/wooden_planks_gk_v01/wooden_planks_gk_v01_cc.dds \
                    packages/gk/fantasy/wooden_planks_gk_v01/wooden_planks_gk_v01_nm.dds \
                    packages/gk/fantasy/wooden_planks_gk_v01/wooden_planks_gk_v01_cc.dds \
                    packages/gk/fantasy/wooden_planks_gk_v01/wooden_planks_gk_v01_nm.dds \
                    packages/gk/fantasy/castell_plaster_gk_v01/castell_plaster_gk_v01_cc.dds \
                    packages/gk/fantasy/castell_plaster_gk_v01/castell_plaster_gk_v01_nm.dds \
                    packages/gk/fantasy/iron_plates_gk_v01/iron_plates_gk_v01_cc.dds \
                    packages/gk/fantasy/iron_plates_gk_v01/iron_plates_gk_v01_nm.dds \
                    packages/gk/fantasy/iron_plates_gk_v01/iron_plates_gk_v01_cc.dds \
                    packages/gk/fantasy/iron_plates_gk_v01/iron_plates_gk_v01_nm.dds \
                    packages/gk/fantasy/iron_trim_gk_v01/iron_trim_gk_v01_cc.dds \
                    packages/gk/fantasy/iron_trim_gk_v01/iron_trim_gk_v01_nm.dds \
                    packages/gk/fantasy/iron_trim_gk_v01/iron_trim_gk_v01_cc.dds \
                    packages/gk/fantasy/iron_trim_gk_v01/iron_trim_gk_v01_nm.dds

ZOOM_TEXTURES = `find packages/gk/future/skysfJPG/ -name "*.jpg"` \
                `find packages/gk/future/ -name "*.cfg"` \
                packages/gk/future/panel_gk_000/panel_gk_000_cc.dds \
                packages/gk/future/panel_gk_000/panel_gk_000_nm.dds \
                packages/gk/future/panel_gk_001/panel_gk_001_cc.dds \
                packages/gk/future/panel_gk_001/panel_gk_001_nm.dds \
                packages/gk/future/panel_gk_001/panel_gk_001_cc.dds \
                packages/gk/future/panel_gk_001/panel_gk_001_nm.dds \
                packages/gk/future/panel_gk_002/panel_gk_002_cc.dds \
                packages/gk/future/panel_gk_002/panel_gk_002_nm.dds \
                packages/gk/future/panel_gk_003/panel_gk_003_cc.dds \
                packages/gk/future/panel_gk_003/panel_gk_003_nm.dds \
                packages/gk/future/panel_gk_004/panel_gk_004_cc.dds \
                packages/gk/future/panel_gk_004/panel_gk_004_nm.dds \
                packages/gk/future/panel_gk_005/panel_gk_005_cc.dds \
                packages/gk/future/panel_gk_005/panel_gk_005_nm.dds \
                packages/gk/future/panel_gk_005/panel_gk_005_cc.dds \
                packages/gk/future/panel_gk_005/panel_gk_005_nm.dds \
                packages/gk/future/panel_gk_006/panel_gk_006_cc.dds \
                packages/gk/future/panel_gk_006/panel_gk_006_nm.dds \
                packages/gk/future/panel_gk_006/panel_gk_006_cc.dds \
                packages/gk/future/panel_gk_006/panel_gk_006_nm.dds \
                packages/gk/future/panel_gk_007/panel_gk_007_cc.dds \
                packages/gk/future/panel_gk_007/panel_gk_007_nm.dds \
                packages/gk/future/panel_gk_007/panel_gk_007_cc.dds \
                packages/gk/future/panel_gk_007/panel_gk_007_nm.dds \
                packages/gk/future/panel_gk_008/panel_gk_008_cc.dds \
                packages/gk/future/panel_gk_008/panel_gk_008_nm.dds \
                packages/gk/future/panel_gk_008/panel_gk_008_cc.dds \
                packages/gk/future/panel_gk_008/panel_gk_008_nm.dds \
                packages/gk/future/panel_gk_009/panel_gk_009_cc.dds \
                packages/gk/future/panel_gk_009/panel_gk_009_nm.dds \
                packages/gk/future/panel_gk_010/panel_gk_010_cc.dds \
                packages/gk/future/panel_gk_010/panel_gk_010_nm.dds \
                packages/gk/future/panel_gk_010/panel_gk_010_cc.dds \
                packages/gk/future/panel_gk_010/panel_gk_010_nm.dds \
                packages/gk/future/panel_gk_011/panel_gk_011_cc.dds \
                packages/gk/future/panel_gk_011/panel_gk_011_nm.dds \
                packages/gk/future/panel_gk_011/panel_gk_011_cc.dds \
                packages/gk/future/panel_gk_011/panel_gk_011_nm.dds \
                packages/gk/future/panel_gk_012/panel_gk_012_cc.dds \
                packages/gk/future/panel_gk_012/panel_gk_012_nm.dds \
                packages/gk/future/panel_gk_014/panel_gk_014_cc.dds \
                packages/gk/future/panel_gk_014/panel_gk_014_nm.dds \
                packages/gk/future/panel_gk_015/panel_gk_015_cc.dds \
                packages/gk/future/panel_gk_015/panel_gk_015_nm.dds \
                packages/gk/future/panel_gk_016/panel_gk_016_cc.dds \
                packages/gk/future/panel_gk_016/panel_gk_016_nm.dds \
                packages/gk/future/panel_gk_016/panel_gk_016_cc.dds \
                packages/gk/future/panel_gk_016/panel_gk_016_nm.dds \
                packages/gk/future/panel_gk_017/panel_gk_017_cc.dds \
                packages/gk/future/panel_gk_017/panel_gk_017_nm.dds \
                packages/gk/future/panel_gk_018/panel_gk_018_cc.dds \
                packages/gk/future/panel_gk_018/panel_gk_018_nm.dds \
                packages/gk/future/lamps_01_gk/lamps_01_gk_cc.dds \
                packages/gk/future/lamps_01_gk/lamps_01_gk_nm.dds \
                packages/gk/future/lamps_01_gk/lamps_01_gk_si.png

ZOOM_FILES = packages/base/zoom.ogz \
             packages/base/zoom.cfg \
             packages/base/zoom.wpt \
             $(ZOOM_TEXTURES)

ZOOMIN_FILES = packages/base/zoomin.ogz \
               packages/base/zoomin.cfg \
               packages/base/zoomin.wpt \
               $(ZOOM_TEXTURES)

ZOOMOUT_FILES = packages/base/zoomout.ogz \
                packages/base/zoomout.cfg \
                packages/base/zoomout.wpt \
                $(ZOOM_TEXTURES)

MD2_FILES = packages/base/md2.ogz \
            packages/base/md2.cfg \
            packages/base/md2.wpt \
            $(FANTASY_TEXTURES)

FFFLAG_FILES = packages/models/ffflag/md5.cfg \
               `find packages/models/ffflag/ -name "*.dds"` \
               `find packages/models/ffflag/ -name "*.md5mesh"` \
               `find packages/models/ffflag/ -name "*.md5anim"`

FFPIT_FILES = packages/models/ffpit/md5.cfg \
              `find packages/models/ffpit/ -name "*.dds"` \
              `find packages/models/ffpit/ -name "*.md5mesh"`

COLOS_FILES = packages/base/colos.ogz \
              packages/base/colos.cfg \
              packages/base/colos.wpt \
              $(FFFLAG_FILES) \
              $(FFPIT_FILES) \
              packages/gk/fantasy/skyfantasyJPG/ \
              `find packages/gk/fantasy -name "*.cfg"` \
              packages/gk/fantasy/rock_formation_gk_v01/rock_formation_gk_v01_cc.dds \
              packages/gk/fantasy/rock_formation_gk_v01/rock_formation_gk_v01_nm.dds \
              packages/gk/fantasy/castell_wall_gk_v01/castell_wall_gk_v01_cc.dds \
              packages/gk/fantasy/castell_wall_gk_v01/castell_wall_gk_v01_nm.dds \
              packages/gk/fantasy/castell_wall_gk_v02/castell_wall_gk_v02_cc.dds \
              packages/gk/fantasy/castell_wall_gk_v02/castell_wall_gk_v02_nm.dds \
              packages/gk/fantasy/castell_wall_trim_gk_v01/castell_wall_trim_gk_v01_cc.dds \
              packages/gk/fantasy/castell_wall_trim_gk_v01/castell_wall_trim_gk_v01_nm.dds \
              packages/gk/fantasy/castell_wall_trim_gk_v01/castell_wall_trim_gk_v01_cc.dds \
              packages/gk/fantasy/castell_wall_trim_gk_v01/castell_wall_trim_gk_v01_nm.dds \
              packages/gk/fantasy/stone_ground_tiles_gk_v01/stone_ground_tiles_gk_v01_cc.dds \
              packages/gk/fantasy/stone_ground_tiles_gk_v01/stone_ground_tiles_gk_v01_nm.dds \
              packages/gk/fantasy/wooden_planks_gk_v01/wooden_planks_gk_v01_cc.dds \
              packages/gk/fantasy/wooden_planks_gk_v01/wooden_planks_gk_v01_nm.dds \
              packages/gk/fantasy/castell_plaster_gk_v01/castell_plaster_gk_v01_cc.dds \
              packages/gk/fantasy/castell_plaster_gk_v01/castell_plaster_gk_v01_nm.dds \
              packages/gk/fantasy/iron_plates_gk_v01/iron_plates_gk_v01_cc.dds \
              packages/gk/fantasy/iron_plates_gk_v01/iron_plates_gk_v01_nm.dds \
              packages/gk/fantasy/iron_plates_gk_v01/iron_plates_gk_v01_cc.dds \
              packages/gk/fantasy/iron_plates_gk_v01/iron_plates_gk_v01_nm.dds \
              packages/gk/fantasy/iron_trim_gk_v01/iron_trim_gk_v01_cc.dds \
              packages/gk/fantasy/iron_trim_gk_v01/iron_trim_gk_v01_nm.dds \
              packages/gk/fantasy/iron_trim_gk_v01/iron_trim_gk_v01_cc.dds \
              packages/gk/fantasy/iron_trim_gk_v01/iron_trim_gk_v01_nm.dds \
              packages/gk/fantasy/iron_intersection_gk_v01/iron_intersection_gk_v01_cc.dds \
              packages/gk/fantasy/iron_intersection_gk_v01/iron_intersection_gk_v01_nm.dds

CAVE_FILES = packages/base/cave.ogz \
             packages/base/cave.cfg \
             $(FANTASY_TEXTURES) \
             $(EXTRA_FANTASY_TEXTURES)

MD3_FILES = packages/base/md3.ogz \
            packages/base/md3.cfg \
            $(FANTASY_TEXTURES)

TT_FILES = packages/base/tt.ogz \
           packages/base/tt.cfg \
           `find packages/gk/fantasy/skyfantasyJPG/ -name "*.jpg"`

CB_TEXTURES = packages/gk/cyber/ \
              packages/gk/cyber/cybersky/

CB_FILES = $(CB_TEXTURES) \
           packages/base/cb.ogz \
           packages/base/cb.cfg

CB2_FILES = $(CB_TEXTURES) \
            packages/base/cb2.ogz \
            packages/base/cb2.cfg

CW_FILES = $(CB_TEXTURES) \
            packages/base/gk_cyber_world.ogz \
            packages/base/gk_cyber_world.cfg

CYBER1_FILES = $(CB_TEXTURES) \
               packages/base/cyber1.ogz \
               packages/base/cyber1.cfg \
               packages/base/cyber1.wpt

FRANKIE_MODEL_FILES = packages/models/frankie/

HUDGUNS_FILES = packages/models/hudguns/ \
                packages/models/snoutx10k/hudguns

SNOUTX10K_MODEL_FILES = packages/models/vwep/ \
	                      packages/models/snoutx10k/

CHARACTER_FILES = $(SNOUTX10K_MODEL_FILES) \
                  $(HUDGUNS_FILES)

FIREWORKS_FILES = $(BASE_FILES) \
                  packages/base/fireworks.ogz \
                  packages/sounds/olpc/MichaelBierylo/sfx_DoorSlam.wav \
                  packages/yo_frankie \
	                packages/skyboxes/philo/

CRUNCH_QUALITY = 64

TOTAL_MEMORY = 67108864 # May need to adjust this for very large levels

client: client.bc
	cd .. ; cd .. ; python $(EMSCRIPTEN)/tools/file_packager.py base.data --use-preload-plugins  --preload $(BASE_FILES) > game/preload_base.js
	cd .. ; cd .. ; python $(EMSCRIPTEN)/tools/file_packager.py character.data --use-preload-plugins  --preload $(CHARACTER_FILES) > game/preload_character.js
	cd .. ; cd .. ; python $(EMSCRIPTEN)/tools/file_packager.py low.data --use-preload-plugins  --preload $(COLOS_FILES) > game/preload_low.js
	#cd .. ; cd .. ; python $(EMSCRIPTEN)/tools/file_packager.py medium.data --use-preload-plugins  --preload $(TWO_TOWERS_FILES) > game/preload_medium.js
	#cd .. ; cd .. ; python $(EMSCRIPTEN)/tools/file_packager.py high.data --use-preload-plugins  --preload $(ZOOM_FILES) > game/preload_high.js
	#cd .. ; cd .. ; python $(EMSCRIPTEN)/tools/file_packager.py four.data --use-preload-plugins  --preload $(CYBER1_FILES) > game/preload_four.js
	#cd .. ; cd .. ; python $(EMSCRIPTEN)/tools/file_packager.py five.data --use-preload-plugins  --preload $(ZOOMOUT_FILES) > game/preload_five.js
	cd .. ; cd .. ; $(CXX) $(CXXFLAGS) src/web/client.bc -o bb.html --shell-file mini.html -s DISABLE_EXCEPTION_CATCHING=1 -s EXPORTED_FUNCTIONS="['_main', '__ZN4game18setplayermodelinfoEPKcS1_S1_S1_S1_S1_S1_S1_S1_S1_S1_S1_b', '__Z7executePKc', '__Z10executestrPKc', '_setforcecamera', '_bb_splash', '_bb_playsoundname']" src/web/enet/.libs/libenet.a -s TOTAL_MEMORY=$(TOTAL_MEMORY) -s LEGACY_GL_EMULATION=1 -Oz -s USE_ZLIB=1 -s NO_EXIT_RUNTIME=1 -s WASM=1 -s 'BINARYEN_TRAP_MODE="clamp"' -s FORCE_FILESYSTEM=1 -s 'EXTRA_EXPORTED_RUNTIME_METHODS=["cwrap"]' # --closure 1 -s IGNORE_CLOSURE_COMPILER_ERRORS=1

server.bc:	libenet $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) -o server.bc $(SERVER_OBJS) $(SERVER_LIBS)  

server:	server.bc
	cd .. ; cd .. ; $(CXX) $(CXXFLAGS) src/web/server.bc -o server.debug.html -s EXPORTED_FUNCTIONS="['_main']" -O3 --closure 0 src/web/enet/.libs/libenet.a
	cd .. ; cd .. ; $(CXX) $(CXXFLAGS) src/web/server.bc -o server.html -s EXPORTED_FUNCTIONS="['_main']" -O3 src/web/enet/.libs/libenet.a

master: libenet $(MASTER_OBJS)
	$(CXX) $(CXXFLAGS) -o sauer_master $(MASTER_OBJS) $(MASTER_LIBS)  

shared/cube2font.o: shared/cube2font.c
	$(CXX) $(CXXFLAGS) -c -o $@ $< `freetype-config --cflags`

cube2font: shared/cube2font.o
	$(CXX) $(CXXFLAGS) -o cube2font shared/cube2font.o `freetype-config --libs` -lz

install: all
	cp sauer_client	../bin_unix/$(PLATFORM_PREFIX)_client
	cp sauer_server	../bin_unix/$(PLATFORM_PREFIX)_server
ifneq (,$(STRIP))
	$(STRIP) ../bin_unix/$(PLATFORM_PREFIX)_client
	$(STRIP) ../bin_unix/$(PLATFORM_PREFIX)_server
endif
endif

depend:
	makedepend -Y -Ishared -Iengine -Ifpsgame $(subst .o,.cpp,$(CLIENT_OBJS))
	makedepend -a -o.h.gch -Y -Ishared -Iengine -Ifpsgame $(subst .h.gch,.h,$(CLIENT_PCH))
	makedepend -a -o-standalone.o -Y -DSTANDALONE -Ishared -Iengine -Ifpsgame $(subst -standalone.o,.cpp,$(SERVER_OBJS))
	makedepend -a -o-standalone.o -Y -DSTANDALONE -Ishared -Iengine -Ifpsgame $(subst -standalone.o,.cpp,$(filter-out $(SERVER_OBJS), $(MASTER_OBJS)))

engine/engine.h.gch: shared/cube.h.gch
fpsgame/game.h.gch: shared/cube.h.gch

shared/%.h: ../shared/%.h
	cp $< $@
engine/%.h: ../engine/%.h
	cp $< $@
fpsgame/%.h: ../fpsgame/%.h
	cp $< $@

shared/%.cpp: ../shared/%.cpp
	cp $< $@
engine/%.cpp: ../engine/%.cpp
	cp $< $@
fpsgame/%.cpp: ../fpsgame/%.cpp
	cp $< $@

# DO NOT DELETE

shared/crypto.o: shared/cube.h shared/tools.h shared/geom.h shared/ents.h
shared/crypto.o: shared/command.h shared/iengine.h shared/igame.h
shared/geom.o: shared/cube.h shared/tools.h shared/geom.h shared/ents.h
shared/geom.o: shared/command.h shared/iengine.h shared/igame.h
shared/stream.o: shared/cube.h shared/tools.h shared/geom.h shared/ents.h
shared/stream.o: shared/command.h shared/iengine.h shared/igame.h
shared/tools.o: shared/cube.h shared/tools.h shared/geom.h shared/ents.h
shared/tools.o: shared/command.h shared/iengine.h shared/igame.h
shared/zip.o: shared/cube.h shared/tools.h shared/geom.h shared/ents.h
shared/zip.o: shared/command.h shared/iengine.h shared/igame.h
engine/3dgui.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/3dgui.o: shared/ents.h shared/command.h shared/iengine.h
engine/3dgui.o: shared/igame.h engine/world.h engine/octa.h engine/lightmap.h
engine/3dgui.o: engine/bih.h engine/texture.h engine/model.h engine/varray.h
engine/3dgui.o: engine/textedit.h
engine/bih.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/bih.o: shared/ents.h shared/command.h shared/iengine.h shared/igame.h
engine/bih.o: engine/world.h engine/octa.h engine/lightmap.h engine/bih.h
engine/bih.o: engine/texture.h engine/model.h engine/varray.h
engine/blend.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/blend.o: shared/ents.h shared/command.h shared/iengine.h
engine/blend.o: shared/igame.h engine/world.h engine/octa.h engine/lightmap.h
engine/blend.o: engine/bih.h engine/texture.h engine/model.h engine/varray.h
engine/blob.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/blob.o: shared/ents.h shared/command.h shared/iengine.h shared/igame.h
engine/blob.o: engine/world.h engine/octa.h engine/lightmap.h engine/bih.h
engine/blob.o: engine/texture.h engine/model.h engine/varray.h
engine/client.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/client.o: shared/ents.h shared/command.h shared/iengine.h
engine/client.o: shared/igame.h engine/world.h engine/octa.h
engine/client.o: engine/lightmap.h engine/bih.h engine/texture.h
engine/client.o: engine/model.h engine/varray.h
engine/command.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/command.o: shared/ents.h shared/command.h shared/iengine.h
engine/command.o: shared/igame.h engine/world.h engine/octa.h
engine/command.o: engine/lightmap.h engine/bih.h engine/texture.h
engine/command.o: engine/model.h engine/varray.h
engine/console.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/console.o: shared/ents.h shared/command.h shared/iengine.h
engine/console.o: shared/igame.h engine/world.h engine/octa.h
engine/console.o: engine/lightmap.h engine/bih.h engine/texture.h
engine/console.o: engine/model.h engine/varray.h
engine/cubeloader.o: engine/engine.h shared/cube.h shared/tools.h
engine/cubeloader.o: shared/geom.h shared/ents.h shared/command.h
engine/cubeloader.o: shared/iengine.h shared/igame.h engine/world.h
engine/cubeloader.o: engine/octa.h engine/lightmap.h engine/bih.h
engine/cubeloader.o: engine/texture.h engine/model.h engine/varray.h
engine/decal.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/decal.o: shared/ents.h shared/command.h shared/iengine.h
engine/decal.o: shared/igame.h engine/world.h engine/octa.h engine/lightmap.h
engine/decal.o: engine/bih.h engine/texture.h engine/model.h engine/varray.h
engine/dynlight.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/dynlight.o: shared/ents.h shared/command.h shared/iengine.h
engine/dynlight.o: shared/igame.h engine/world.h engine/octa.h
engine/dynlight.o: engine/lightmap.h engine/bih.h engine/texture.h
engine/dynlight.o: engine/model.h engine/varray.h
engine/glare.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/glare.o: shared/ents.h shared/command.h shared/iengine.h
engine/glare.o: shared/igame.h engine/world.h engine/octa.h engine/lightmap.h
engine/glare.o: engine/bih.h engine/texture.h engine/model.h engine/varray.h
engine/glare.o: engine/rendertarget.h
engine/grass.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/grass.o: shared/ents.h shared/command.h shared/iengine.h
engine/grass.o: shared/igame.h engine/world.h engine/octa.h engine/lightmap.h
engine/grass.o: engine/bih.h engine/texture.h engine/model.h engine/varray.h
engine/lightmap.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/lightmap.o: shared/ents.h shared/command.h shared/iengine.h
engine/lightmap.o: shared/igame.h engine/world.h engine/octa.h
engine/lightmap.o: engine/lightmap.h engine/bih.h engine/texture.h
engine/lightmap.o: engine/model.h engine/varray.h
engine/main.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/main.o: shared/ents.h shared/command.h shared/iengine.h shared/igame.h
engine/main.o: engine/world.h engine/octa.h engine/lightmap.h engine/bih.h
engine/main.o: engine/texture.h engine/model.h engine/varray.h
engine/material.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/material.o: shared/ents.h shared/command.h shared/iengine.h
engine/material.o: shared/igame.h engine/world.h engine/octa.h
engine/material.o: engine/lightmap.h engine/bih.h engine/texture.h
engine/material.o: engine/model.h engine/varray.h
engine/menus.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/menus.o: shared/ents.h shared/command.h shared/iengine.h
engine/menus.o: shared/igame.h engine/world.h engine/octa.h engine/lightmap.h
engine/menus.o: engine/bih.h engine/texture.h engine/model.h engine/varray.h
engine/movie.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/movie.o: shared/ents.h shared/command.h shared/iengine.h
engine/movie.o: shared/igame.h engine/world.h engine/octa.h engine/lightmap.h
engine/movie.o: engine/bih.h engine/texture.h engine/model.h engine/varray.h
engine/normal.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/normal.o: shared/ents.h shared/command.h shared/iengine.h
engine/normal.o: shared/igame.h engine/world.h engine/octa.h
engine/normal.o: engine/lightmap.h engine/bih.h engine/texture.h
engine/normal.o: engine/model.h engine/varray.h
engine/octa.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/octa.o: shared/ents.h shared/command.h shared/iengine.h shared/igame.h
engine/octa.o: engine/world.h engine/octa.h engine/lightmap.h engine/bih.h
engine/octa.o: engine/texture.h engine/model.h engine/varray.h
engine/octaedit.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/octaedit.o: shared/ents.h shared/command.h shared/iengine.h
engine/octaedit.o: shared/igame.h engine/world.h engine/octa.h
engine/octaedit.o: engine/lightmap.h engine/bih.h engine/texture.h
engine/octaedit.o: engine/model.h engine/varray.h
engine/octarender.o: engine/engine.h shared/cube.h shared/tools.h
engine/octarender.o: shared/geom.h shared/ents.h shared/command.h
engine/octarender.o: shared/iengine.h shared/igame.h engine/world.h
engine/octarender.o: engine/octa.h engine/lightmap.h engine/bih.h
engine/octarender.o: engine/texture.h engine/model.h engine/varray.h
engine/physics.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/physics.o: shared/ents.h shared/command.h shared/iengine.h
engine/physics.o: shared/igame.h engine/world.h engine/octa.h
engine/physics.o: engine/lightmap.h engine/bih.h engine/texture.h
engine/physics.o: engine/model.h engine/varray.h engine/mpr.h
engine/pvs.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/pvs.o: shared/ents.h shared/command.h shared/iengine.h shared/igame.h
engine/pvs.o: engine/world.h engine/octa.h engine/lightmap.h engine/bih.h
engine/pvs.o: engine/texture.h engine/model.h engine/varray.h
engine/rendergl.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/rendergl.o: shared/ents.h shared/command.h shared/iengine.h
engine/rendergl.o: shared/igame.h engine/world.h engine/octa.h
engine/rendergl.o: engine/lightmap.h engine/bih.h engine/texture.h
engine/rendergl.o: engine/model.h engine/varray.h
engine/rendermodel.o: engine/engine.h shared/cube.h shared/tools.h
engine/rendermodel.o: shared/geom.h shared/ents.h shared/command.h
engine/rendermodel.o: shared/iengine.h shared/igame.h engine/world.h
engine/rendermodel.o: engine/octa.h engine/lightmap.h engine/bih.h
engine/rendermodel.o: engine/texture.h engine/model.h engine/varray.h
engine/rendermodel.o: engine/ragdoll.h engine/animmodel.h engine/vertmodel.h
engine/rendermodel.o: engine/skelmodel.h engine/md2.h engine/md3.h
engine/rendermodel.o: engine/md5.h engine/obj.h engine/smd.h engine/iqm.h
engine/renderparticles.o: engine/engine.h shared/cube.h shared/tools.h
engine/renderparticles.o: shared/geom.h shared/ents.h shared/command.h
engine/renderparticles.o: shared/iengine.h shared/igame.h engine/world.h
engine/renderparticles.o: engine/octa.h engine/lightmap.h engine/bih.h
engine/renderparticles.o: engine/texture.h engine/model.h engine/varray.h
engine/renderparticles.o: engine/rendertarget.h engine/depthfx.h
engine/renderparticles.o: engine/explosion.h engine/lensflare.h
engine/renderparticles.o: engine/lightning.h
engine/rendersky.o: engine/engine.h shared/cube.h shared/tools.h
engine/rendersky.o: shared/geom.h shared/ents.h shared/command.h
engine/rendersky.o: shared/iengine.h shared/igame.h engine/world.h
engine/rendersky.o: engine/octa.h engine/lightmap.h engine/bih.h
engine/rendersky.o: engine/texture.h engine/model.h engine/varray.h
engine/rendertext.o: engine/engine.h shared/cube.h shared/tools.h
engine/rendertext.o: shared/geom.h shared/ents.h shared/command.h
engine/rendertext.o: shared/iengine.h shared/igame.h engine/world.h
engine/rendertext.o: engine/octa.h engine/lightmap.h engine/bih.h
engine/rendertext.o: engine/texture.h engine/model.h engine/varray.h
engine/renderva.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/renderva.o: shared/ents.h shared/command.h shared/iengine.h
engine/renderva.o: shared/igame.h engine/world.h engine/octa.h
engine/renderva.o: engine/lightmap.h engine/bih.h engine/texture.h
engine/renderva.o: engine/model.h engine/varray.h
engine/server.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/server.o: shared/ents.h shared/command.h shared/iengine.h
engine/server.o: shared/igame.h engine/world.h engine/octa.h
engine/server.o: engine/lightmap.h engine/bih.h engine/texture.h
engine/server.o: engine/model.h engine/varray.h
engine/serverbrowser.o: engine/engine.h shared/cube.h shared/tools.h
engine/serverbrowser.o: shared/geom.h shared/ents.h shared/command.h
engine/serverbrowser.o: shared/iengine.h shared/igame.h engine/world.h
engine/serverbrowser.o: engine/octa.h engine/lightmap.h engine/bih.h
engine/serverbrowser.o: engine/texture.h engine/model.h engine/varray.h
engine/shader.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/shader.o: shared/ents.h shared/command.h shared/iengine.h
engine/shader.o: shared/igame.h engine/world.h engine/octa.h
engine/shader.o: engine/lightmap.h engine/bih.h engine/texture.h
engine/shader.o: engine/model.h engine/varray.h
engine/shadowmap.o: engine/engine.h shared/cube.h shared/tools.h
engine/shadowmap.o: shared/geom.h shared/ents.h shared/command.h
engine/shadowmap.o: shared/iengine.h shared/igame.h engine/world.h
engine/shadowmap.o: engine/octa.h engine/lightmap.h engine/bih.h
engine/shadowmap.o: engine/texture.h engine/model.h engine/varray.h
engine/shadowmap.o: engine/rendertarget.h
engine/sound.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/sound.o: shared/ents.h shared/command.h shared/iengine.h
engine/sound.o: shared/igame.h engine/world.h engine/octa.h engine/lightmap.h
engine/sound.o: engine/bih.h engine/texture.h engine/model.h engine/varray.h
engine/texture.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/texture.o: shared/ents.h shared/command.h shared/iengine.h
engine/texture.o: shared/igame.h engine/world.h engine/octa.h
engine/texture.o: engine/lightmap.h engine/bih.h engine/texture.h
engine/texture.o: engine/model.h engine/varray.h engine/scale.h
engine/water.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/water.o: shared/ents.h shared/command.h shared/iengine.h
engine/water.o: shared/igame.h engine/world.h engine/octa.h engine/lightmap.h
engine/water.o: engine/bih.h engine/texture.h engine/model.h engine/varray.h
engine/world.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/world.o: shared/ents.h shared/command.h shared/iengine.h
engine/world.o: shared/igame.h engine/world.h engine/octa.h engine/lightmap.h
engine/world.o: engine/bih.h engine/texture.h engine/model.h engine/varray.h
engine/worldio.o: engine/engine.h shared/cube.h shared/tools.h shared/geom.h
engine/worldio.o: shared/ents.h shared/command.h shared/iengine.h
engine/worldio.o: shared/igame.h engine/world.h engine/octa.h
engine/worldio.o: engine/lightmap.h engine/bih.h engine/texture.h
engine/worldio.o: engine/model.h engine/varray.h
fpsgame/ai.o: fpsgame/game.h shared/cube.h shared/tools.h shared/geom.h
fpsgame/ai.o: shared/ents.h shared/command.h shared/iengine.h shared/igame.h
fpsgame/ai.o: fpsgame/ai.h
fpsgame/client.o: fpsgame/game.h shared/cube.h shared/tools.h shared/geom.h
fpsgame/client.o: shared/ents.h shared/command.h shared/iengine.h
fpsgame/client.o: shared/igame.h fpsgame/ai.h fpsgame/capture.h fpsgame/ctf.h
fpsgame/client.o: fpsgame/collect.h
fpsgame/entities.o: fpsgame/game.h shared/cube.h shared/tools.h shared/geom.h
fpsgame/entities.o: shared/ents.h shared/command.h shared/iengine.h
fpsgame/entities.o: shared/igame.h fpsgame/ai.h
fpsgame/fps.o: fpsgame/game.h shared/cube.h shared/tools.h shared/geom.h
fpsgame/fps.o: shared/ents.h shared/command.h shared/iengine.h shared/igame.h
fpsgame/fps.o: fpsgame/ai.h
fpsgame/monster.o: fpsgame/game.h shared/cube.h shared/tools.h shared/geom.h
fpsgame/monster.o: shared/ents.h shared/command.h shared/iengine.h
fpsgame/monster.o: shared/igame.h fpsgame/ai.h
fpsgame/movable.o: fpsgame/game.h shared/cube.h shared/tools.h shared/geom.h
fpsgame/movable.o: shared/ents.h shared/command.h shared/iengine.h
fpsgame/movable.o: shared/igame.h fpsgame/ai.h
fpsgame/render.o: fpsgame/game.h shared/cube.h shared/tools.h shared/geom.h
fpsgame/render.o: shared/ents.h shared/command.h shared/iengine.h
fpsgame/render.o: shared/igame.h fpsgame/ai.h
fpsgame/scoreboard.o: fpsgame/game.h shared/cube.h shared/tools.h
fpsgame/scoreboard.o: shared/geom.h shared/ents.h shared/command.h
fpsgame/scoreboard.o: shared/iengine.h shared/igame.h fpsgame/ai.h
fpsgame/server.o: fpsgame/game.h shared/cube.h shared/tools.h shared/geom.h
fpsgame/server.o: shared/ents.h shared/command.h shared/iengine.h
fpsgame/server.o: shared/igame.h fpsgame/ai.h fpsgame/capture.h fpsgame/ctf.h
fpsgame/server.o: fpsgame/collect.h fpsgame/extinfo.h fpsgame/aiman.h
fpsgame/waypoint.o: fpsgame/game.h shared/cube.h shared/tools.h shared/geom.h
fpsgame/waypoint.o: shared/ents.h shared/command.h shared/iengine.h
fpsgame/waypoint.o: shared/igame.h fpsgame/ai.h
fpsgame/weapon.o: fpsgame/game.h shared/cube.h shared/tools.h shared/geom.h
fpsgame/weapon.o: shared/ents.h shared/command.h shared/iengine.h
fpsgame/weapon.o: shared/igame.h fpsgame/ai.h

shared/cube.h.gch: shared/tools.h shared/geom.h shared/ents.h
shared/cube.h.gch: shared/command.h shared/iengine.h shared/igame.h
engine/engine.h.gch: shared/cube.h shared/tools.h shared/geom.h shared/ents.h
engine/engine.h.gch: shared/command.h shared/iengine.h shared/igame.h
engine/engine.h.gch: engine/world.h engine/octa.h engine/lightmap.h
engine/engine.h.gch: engine/bih.h engine/texture.h engine/model.h
engine/engine.h.gch: engine/varray.h
fpsgame/game.h.gch: shared/cube.h shared/tools.h shared/geom.h shared/ents.h
fpsgame/game.h.gch: shared/command.h shared/iengine.h shared/igame.h
fpsgame/game.h.gch: fpsgame/ai.h

shared/crypto-standalone.o: shared/cube.h shared/tools.h shared/geom.h
shared/crypto-standalone.o: shared/ents.h shared/command.h shared/iengine.h
shared/crypto-standalone.o: shared/igame.h
shared/stream-standalone.o: shared/cube.h shared/tools.h shared/geom.h
shared/stream-standalone.o: shared/ents.h shared/command.h shared/iengine.h
shared/stream-standalone.o: shared/igame.h
shared/tools-standalone.o: shared/cube.h shared/tools.h shared/geom.h
shared/tools-standalone.o: shared/ents.h shared/command.h shared/iengine.h
shared/tools-standalone.o: shared/igame.h
engine/command-standalone.o: engine/engine.h shared/cube.h shared/tools.h
engine/command-standalone.o: shared/geom.h shared/ents.h shared/command.h
engine/command-standalone.o: shared/iengine.h shared/igame.h engine/world.h
engine/server-standalone.o: engine/engine.h shared/cube.h shared/tools.h
engine/server-standalone.o: shared/geom.h shared/ents.h shared/command.h
engine/server-standalone.o: shared/iengine.h shared/igame.h engine/world.h
engine/worldio-standalone.o: engine/engine.h shared/cube.h shared/tools.h
engine/worldio-standalone.o: shared/geom.h shared/ents.h shared/command.h
engine/worldio-standalone.o: shared/iengine.h shared/igame.h engine/world.h
fpsgame/entities-standalone.o: fpsgame/game.h shared/cube.h shared/tools.h
fpsgame/entities-standalone.o: shared/geom.h shared/ents.h shared/command.h
fpsgame/entities-standalone.o: shared/iengine.h shared/igame.h fpsgame/ai.h
fpsgame/server-standalone.o: fpsgame/game.h shared/cube.h shared/tools.h
fpsgame/server-standalone.o: shared/geom.h shared/ents.h shared/command.h
fpsgame/server-standalone.o: shared/iengine.h shared/igame.h fpsgame/ai.h
fpsgame/server-standalone.o: fpsgame/capture.h fpsgame/ctf.h
fpsgame/server-standalone.o: fpsgame/collect.h fpsgame/extinfo.h
fpsgame/server-standalone.o: fpsgame/aiman.h

engine/master-standalone.o: shared/cube.h shared/tools.h shared/geom.h
engine/master-standalone.o: shared/ents.h shared/command.h shared/iengine.h
engine/master-standalone.o: shared/igame.h
