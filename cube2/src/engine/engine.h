#ifndef __ENGINE_H__
#define __ENGINE_H__

#include "cube.h"
#include "world.h"

#ifndef STANDALONE

#include "octa.h"
#include "lightmap.h"
#include "bih.h"
#include "texture.h"
#include "model.h"

// GL_ARB_multitexture
extern PFNGLACTIVETEXTUREARBPROC       glActiveTexture_;
extern PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTexture_;
extern PFNGLMULTITEXCOORD2FARBPROC     glMultiTexCoord2f_;
extern PFNGLMULTITEXCOORD3FARBPROC     glMultiTexCoord3f_;
extern PFNGLMULTITEXCOORD4FARBPROC     glMultiTexCoord4f_;

// GL_ARB_vertex_buffer_object
extern PFNGLGENBUFFERSARBPROC       glGenBuffers_;
extern PFNGLBINDBUFFERARBPROC       glBindBuffer_;
extern PFNGLMAPBUFFERARBPROC        glMapBuffer_;
extern PFNGLUNMAPBUFFERARBPROC      glUnmapBuffer_;
extern PFNGLBUFFERDATAARBPROC       glBufferData_;
extern PFNGLBUFFERSUBDATAARBPROC    glBufferSubData_;
extern PFNGLDELETEBUFFERSARBPROC    glDeleteBuffers_;
extern PFNGLGETBUFFERSUBDATAARBPROC glGetBufferSubData_;

// GL_ARB_occlusion_query
extern PFNGLGENQUERIESARBPROC        glGenQueries_;
extern PFNGLDELETEQUERIESARBPROC     glDeleteQueries_;
extern PFNGLBEGINQUERYARBPROC        glBeginQuery_;
extern PFNGLENDQUERYARBPROC          glEndQuery_;
extern PFNGLGETQUERYIVARBPROC        glGetQueryiv_;
extern PFNGLGETQUERYOBJECTIVARBPROC  glGetQueryObjectiv_;
extern PFNGLGETQUERYOBJECTUIVARBPROC glGetQueryObjectuiv_;

// GL_EXT_framebuffer_object
extern PFNGLBINDRENDERBUFFEREXTPROC        glBindRenderbuffer_;
extern PFNGLDELETERENDERBUFFERSEXTPROC     glDeleteRenderbuffers_;
extern PFNGLGENFRAMEBUFFERSEXTPROC         glGenRenderbuffers_;
extern PFNGLRENDERBUFFERSTORAGEEXTPROC     glRenderbufferStorage_;
extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC  glCheckFramebufferStatus_;
extern PFNGLBINDFRAMEBUFFEREXTPROC         glBindFramebuffer_;
extern PFNGLDELETEFRAMEBUFFERSEXTPROC      glDeleteFramebuffers_;
extern PFNGLGENFRAMEBUFFERSEXTPROC         glGenFramebuffers_;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC    glFramebufferTexture2D_;
extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbuffer_;
extern PFNGLGENERATEMIPMAPEXTPROC          glGenerateMipmap_;

// GL_EXT_framebuffer_blit
#ifndef GL_EXT_framebuffer_blit
#define GL_READ_FRAMEBUFFER_EXT           0x8CA8
#define GL_DRAW_FRAMEBUFFER_EXT           0x8CA9
#define GL_DRAW_FRAMEBUFFER_BINDING_EXT   0x8CA6
#define GL_READ_FRAMEBUFFER_BINDING_EXT   0x8CAA
typedef void (APIENTRYP PFNGLBLITFRAMEBUFFEREXTPROC) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
#endif
extern PFNGLBLITFRAMEBUFFEREXTPROC         glBlitFramebuffer_;

// GL_EXT_draw_range_elements
extern PFNGLDRAWRANGEELEMENTSEXTPROC glDrawRangeElements_;

// GL_EXT_blend_minmax
extern PFNGLBLENDEQUATIONEXTPROC glBlendEquation_;

// GL_EXT_blend_color
extern PFNGLBLENDCOLOREXTPROC glBlendColor_;

// GL_EXT_multi_draw_arrays
extern PFNGLMULTIDRAWARRAYSEXTPROC   glMultiDrawArrays_;
extern PFNGLMULTIDRAWELEMENTSEXTPROC glMultiDrawElements_;

// GL_EXT_packed_depth_stencil
#ifndef GL_DEPTH_STENCIL_EXT
#define GL_DEPTH_STENCIL_EXT 0x84F9
#endif
#ifndef GL_DEPTH24_STENCIL8_EXT
#define GL_DEPTH24_STENCIL8_EXT 0x88F0
#endif

// GL_ARB_texture_compression
extern PFNGLCOMPRESSEDTEXIMAGE3DARBPROC    glCompressedTexImage3D_;
extern PFNGLCOMPRESSEDTEXIMAGE2DARBPROC    glCompressedTexImage2D_;
extern PFNGLCOMPRESSEDTEXIMAGE1DARBPROC    glCompressedTexImage1D_;
extern PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC glCompressedTexSubImage3D_;
extern PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC glCompressedTexSubImage2D_;
extern PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC glCompressedTexSubImage1D_;
extern PFNGLGETCOMPRESSEDTEXIMAGEARBPROC   glGetCompressedTexImage_;

// GL_EXT_fog_coord
extern PFNGLFOGCOORDPOINTEREXTPROC glFogCoordPointer_;

#include "varray.h"

extern dynent *player;
extern physent *camera1;                // special ent that acts as camera, same object as player1 in FPS mode

extern int worldscale, worldsize;
extern int mapversion;
extern char *maptitle;
extern vector<ushort> texmru;
extern int xtraverts, xtravertsva;
extern const ivec cubecoords[8];
extern const ivec facecoords[6][4];
extern const uchar fv[6][4];
extern const uchar fvmasks[64];
extern const uchar faceedgesidx[6][4];
extern bool inbetweenframes, renderedframe;

extern SDL_Surface *screen;
extern int zpass, glowpass;

extern vector<int> entgroup;

// rendertext
struct font
{
    struct charinfo
    {
        short x, y, w, h, offsetx, offsety, advance, tex;
    };

    char *name;
    vector<Texture *> texs;
    vector<charinfo> chars;
    int charoffset, defaultw, defaulth, scale;

    font() : name(NULL) {}
    ~font() { DELETEA(name); }
};

#define FONTH (curfont->scale)
#define FONTW (FONTH/2)
#define MINRESW 640
#define MINRESH 480

extern font *curfont;

// texture
extern int hwtexsize, hwcubetexsize, hwmaxaniso, maxtexsize;

extern Texture *textureload(const char *name, int clamp = 0, bool mipit = true, bool msg = true);
extern int texalign(void *data, int w, int bpp);
extern void cleanuptexture(Texture *t);
extern void loadalphamask(Texture *t);
extern void loadlayermasks();
extern Texture *cubemapload(const char *name, bool mipit = true, bool msg = true, bool transient = false);
extern void drawcubemap(int size, const vec &o, float yaw, float pitch, const cubemapside &side);
extern void loadshaders(bool firstload = false); // XXX EMSCRIPTEN: during first load we do this asynchronously
extern void setuptexparameters(int tnum, void *pixels, int clamp, int filter, GLenum format = GL_RGB, GLenum target = GL_TEXTURE_2D);
extern void createtexture(int tnum, int w, int h, void *pixels, int clamp, int filter, GLenum component = GL_RGB, GLenum target = GL_TEXTURE_2D, int pw = 0, int ph = 0, int pitch = 0, bool resize = true, GLenum format = GL_FALSE);
extern void blurtexture(int n, int bpp, int w, int h, uchar *dst, const uchar *src, int margin = 0);
extern void blurnormals(int n, int w, int h, bvec *dst, const bvec *src, int margin = 0);
extern void renderpostfx();
extern void initenvmaps();
extern void genenvmaps();
extern ushort closestenvmap(const vec &o);
extern ushort closestenvmap(int orient, int x, int y, int z, int size);
extern GLuint lookupenvmap(ushort emid);
extern GLuint lookupenvmap(Slot &slot);
extern bool reloadtexture(Texture &tex);
extern bool reloadtexture(const char *name);
extern void setuptexcompress();
extern void clearslots();
extern void compacteditvslots();
extern void compactmruvslots();
extern void compactvslots(cube *c, int n = 8);
extern void compactvslot(int &index);
extern int compactvslots();

// shadowmap

extern int shadowmap, shadowmapcasters;
extern bool shadowmapping;

extern bool isshadowmapcaster(const vec &o, float rad);
extern bool addshadowmapcaster(const vec &o, float xyrad, float zrad);
extern bool isshadowmapreceiver(vtxarray *va);
extern void rendershadowmap();
extern void pushshadowmap();
extern void popshadowmap();
extern void rendershadowmapreceivers();
extern void guessshadowdir();

// pvs
extern void clearpvs();
extern bool pvsoccluded(const ivec &bborigin, const ivec &bbsize);
extern bool waterpvsoccluded(int height);
extern void setviewcell(const vec &p);
extern void savepvs(stream *f);
extern void loadpvs(stream *f, int numpvs);
extern int getnumviewcells();

static inline bool pvsoccluded(const ivec &bborigin, int size)
{
    return pvsoccluded(bborigin, ivec(size, size, size));
}

// rendergl
extern bool hasVBO, hasDRE, hasOQ, hasTR, hasFBO, hasDS, hasTF, hasBE, hasBC, hasCM, hasNP2, hasTC, hasTE, hasMT, hasD3, hasAF, hasVP2, hasVP3, hasPP, hasMDA, hasTE3, hasTE4, hasVP, hasFP, hasGLSL, hasGM, hasNVFB, hasSGIDT, hasSGISH, hasDT, hasSH, hasNVPCF, hasRN, hasPBO, hasFBB, hasUBO, hasBUE, hasFC, hasTEX;
extern int hasstencil;

extern bool envmapping, minimapping, renderedgame;
extern const glmatrixf viewmatrix;
extern glmatrixf mvmatrix, projmatrix, mvpmatrix, invmvmatrix, invmvpmatrix, fogmatrix, invfogmatrix, envmatrix;
extern bvec fogcolor;

extern void gl_checkextensions();
extern void gl_init(int w, int h, int bpp, int depth, int fsaa);
extern void cleangl();
extern void rendergame(bool mainpass = false);
extern void invalidatepostfx();
extern void gl_drawframe(int w, int h);
extern void gl_drawmainmenu(int w, int h);
extern void drawminimap();
extern void enablepolygonoffset(GLenum type);
extern void disablepolygonoffset(GLenum type);
extern void calcspherescissor(const vec &center, float size, float &sx1, float &sy1, float &sx2, float &sy2);
extern int pushscissor(float sx1, float sy1, float sx2, float sy2);
extern void popscissor();
extern void recomputecamera();
extern void findorientation();
extern void writecrosshairs(stream *f);

// renderextras
extern void render3dbox(vec &o, float tofloor, float toceil, float xradius, float yradius = 0);

// octa
extern cube *newcubes(uint face = F_EMPTY, int mat = MAT_AIR);
extern cubeext *growcubeext(cubeext *ext, int maxverts);
extern void setcubeext(cube &c, cubeext *ext);
extern cubeext *newcubeext(cube &c, int maxverts = 0, bool init = true);
extern void getcubevector(cube &c, int d, int x, int y, int z, ivec &p);
extern void setcubevector(cube &c, int d, int x, int y, int z, const ivec &p);
extern int familysize(cube &c);
extern void freeocta(cube *c);
extern void discardchildren(cube &c, bool fixtex = false, int depth = 0);
extern void optiface(uchar *p, cube &c);
extern void validatec(cube *c, int size = 0);
extern bool isvalidcube(cube &c);
extern ivec lu;
extern int lusize;
extern cube &lookupcube(int tx, int ty, int tz, int tsize = 0, ivec &ro = lu, int &rsize = lusize);
extern cube *neighbourstack[32];
extern int neighbourdepth;
extern cube &neighbourcube(cube &c, int orient, int x, int y, int z, int size, ivec &ro = lu, int &rsize = lusize);
extern void resetclipplanes();
extern int getmippedtexture(cube &p, int orient);
extern void forcemip(cube &c, bool fixtex = true);
extern bool subdividecube(cube &c, bool fullcheck=true, bool brighten=true);
extern void edgespan2vectorcube(cube &c);
extern int faceconvexity(ivec v[4]);
extern int faceconvexity(ivec v[4], int &vis);
extern int faceconvexity(vertinfo *verts, int numverts);
extern int faceconvexity(cube &c, int orient);
extern void calcvert(cube &c, int x, int y, int z, int size, ivec &vert, int i, bool solid = false);
extern void calcvert(cube &c, int x, int y, int z, int size, vec &vert, int i, bool solid = false);
extern uint faceedges(cube &c, int orient);
extern bool collapsedface(cube &c, int orient);
extern bool touchingface(cube &c, int orient);
extern bool flataxisface(cube &c, int orient);
extern bool collideface(cube &c, int orient);
extern int genclipplane(cube &c, int i, vec *v, plane *clip);
extern void genclipplanes(cube &c, int x, int y, int z, int size, clipplanes &p);
extern bool visibleface(cube &c, int orient, int x, int y, int z, int size, uchar mat = MAT_AIR, uchar nmat = MAT_AIR, uchar matmask = MATF_VOLUME);
extern int visibletris(cube &c, int orient, int x, int y, int z, int size, uchar nmat = MAT_AIR, uchar matmask = MAT_AIR);
extern int visibleorient(cube &c, int orient);
extern void genfaceverts(cube &c, int orient, ivec v[4]);
extern int calcmergedsize(int orient, const ivec &co, int size, const vertinfo *verts, int numverts);
extern void invalidatemerges(cube &c, const ivec &co, int size, bool msg);
extern void calcmerges();

extern int mergefaces(int orient, facebounds *m, int sz);
extern void mincubeface(cube &cu, int orient, const ivec &o, int size, const facebounds &orig, facebounds &cf, uchar nmat = MAT_AIR, uchar matmask = MATF_VOLUME);

static inline uchar octantrectangleoverlap(const ivec &c, int size, const ivec &o, const ivec &s)
{
    uchar p = 0xFF; // bitmask of possible collisions with octants. 0 bit = 0 octant, etc
    ivec v(c);
    v.add(size);
    if(v.z <= o.z)     p &= 0xF0; // not in a -ve Z octant
    else if(v.z >= o.z+s.z) p &= 0x0F; // not in a +ve Z octant
    if(v.y <= o.y)     p &= 0xCC; // not in a -ve Y octant
    else if(v.y >= o.y+s.y) p &= 0x33; // etc..
    if(v.x <= o.x)     p &= 0xAA;
    else if(v.x >= o.x+s.x) p &= 0x55;
    return p;
}

static inline cubeext &ext(cube &c)
{
    return *(c.ext ? c.ext : newcubeext(c));
}

// ents
extern char *entname(entity &e);
extern bool haveselent();
extern undoblock *copyundoents(undoblock *u);
extern void pasteundoents(undoblock *u);

// octaedit
extern void cancelsel();
extern void rendertexturepanel(int w, int h);
extern void addundo(undoblock *u);
extern void commitchanges(bool force = false);
extern void rendereditcursor();
extern void tryedit();

// octarender
extern vector<tjoint> tjoints;

extern ushort encodenormal(const vec &n);
extern vec decodenormal(ushort norm);
extern void reduceslope(ivec &n);
extern void findtjoints();
extern void octarender();
extern void allchanged(bool load = false, void (*next)(void*) = NULL); // XXX EMSCRIPTEN next is a continuation, if we want to split this up
extern void clearvas(cube *c);
extern vtxarray *newva(int x, int y, int z, int size);
extern void destroyva(vtxarray *va, bool reparent = true);
extern bool readva(vtxarray *va, ushort *&edata, uchar *&vdata);
extern void updatevabb(vtxarray *va, bool force = false);
extern void updatevabbs(bool force = false);

// renderva
extern void visiblecubes(bool cull = true);
extern void setvfcP(float z = -1, const vec &bbmin = vec(-1, -1, -1), const vec &bbmax = vec(1, 1, 1));
extern void savevfcP();
extern void restorevfcP();
extern void rendergeom(float causticspass = 0, bool fogpass = false);
extern void renderalphageom(bool fogpass = false);
extern void rendermapmodels();
extern void renderreflectedgeom(bool causticspass = false, bool fogpass = false);
extern void renderreflectedmapmodels();
extern void renderoutline();
extern bool rendersky(bool explicitonly = false);

extern bool isfoggedsphere(float rad, const vec &cv);
extern int isvisiblesphere(float rad, const vec &cv);
extern bool bboccluded(const ivec &bo, const ivec &br);
extern occludequery *newquery(void *owner);
extern bool checkquery(occludequery *query, bool nowait = false);
extern void resetqueries();
extern int getnumqueries();
extern void drawbb(const ivec &bo, const ivec &br, const vec &camera = camera1->o);

#define startquery(query) { glBeginQuery_(GL_SAMPLES_PASSED_ARB, ((occludequery *)(query))->id); }
#define endquery(query) \
    { \
        glEndQuery_(GL_SAMPLES_PASSED_ARB); \
        extern int ati_oq_bug; \
        if(ati_oq_bug) glFlush(); \
    }

// dynlight

extern void updatedynlights();
extern int finddynlights();
extern void calcdynlightmask(vtxarray *va);
extern int setdynlights(vtxarray *va);
extern bool getdynlight(int n, vec &o, float &radius, vec &color);

// material

extern int showmat;

extern int findmaterial(const char *name);
extern void genmatsurfs(cube &c, int cx, int cy, int cz, int size, vector<materialsurface> &matsurfs);
extern void rendermatsurfs(materialsurface *matbuf, int matsurfs);
extern void rendermatgrid(materialsurface *matbuf, int matsurfs);
extern int optimizematsurfs(materialsurface *matbuf, int matsurfs);
extern void setupmaterials(int start = 0, int len = 0);
extern void rendermaterials();
extern int visiblematerial(cube &c, int orient, int x, int y, int z, int size, uchar matmask = MATF_VOLUME);

// water
extern int refracting;
extern bool reflecting, fading, fogging;
extern float reflectz;
extern int reflectdist, vertwater, waterrefract, waterreflect, waterfade, caustics, waterfallrefract, waterfog, lavafog;
extern bvec watercolor, waterfallcolor, lavacolor;

extern void cleanreflections();
extern void queryreflections();
extern void drawreflections();
extern void renderwater();
extern void renderlava(const materialsurface &m, Texture *tex, float scale);
extern void loadcaustics(bool force = false);
extern void preloadwatershaders(bool force = false);

// glare
extern bool glaring;

extern void drawglaretex();
extern void addglare();

// depthfx
extern bool depthfxing;

extern void drawdepthfxtex();

// server
extern vector<const char *> gameargs;

extern void initserver(bool listen, bool dedicated);
extern void cleanupserver();
extern void serverslice(bool dedicated, uint timeout);

extern ENetSocket connectmaster();
extern void localclienttoserver(int chan, ENetPacket *);
extern void localconnect();
extern bool serveroption(char *opt);

// serverbrowser
extern bool resolverwait(const char *name, ENetAddress *address);
extern int connectwithtimeout(ENetSocket sock, const char *hostname, const ENetAddress &address);
extern void addserver(const char *name, int port = 0, const char *password = NULL, bool keep = false);
extern void writeservercfg();

// client
extern void localdisconnect(bool cleanup = true);
extern void localservertoclient(int chan, ENetPacket *packet);
extern void connectserv(const char *servername, int port, const char *serverpassword);
extern void abortconnect();
extern void clientkeepalive();

// command
extern hashset<ident> idents;
extern int identflags;

extern void explodelist(const char *s, vector<char *> &elems, int limit = -1);
extern char *indexlist(const char *s, int pos);

extern void clearoverrides();
extern void writecfg(const char *name = NULL);

extern void checksleep(int millis);
extern void clearsleep(bool clearoverrides = true);

// console
extern void keypress(int code, bool isdown, int cooked);
extern int rendercommand(int x, int y, int w);
extern int renderconsole(int w, int h, int abovehud);
extern void conoutf(const char *s, ...);
extern void conoutf(int type, const char *s, ...);
extern void resetcomplete();
extern void complete(char *s, const char *cmdprefix);
const char *getkeyname(int code);
extern const char *addreleaseaction(char *s);
extern void writebinds(stream *f);
extern void writecompletions(stream *f);

// main
enum
{
    NOT_INITING = 0,
    INIT_LOAD,
    INIT_RESET
};
extern int initing;

enum
{
    CHANGE_GFX   = 1<<0,
    CHANGE_SOUND = 1<<1
};
extern bool initwarning(const char *desc, int level = INIT_RESET, int type = CHANGE_GFX);

extern void pushevent(const SDL_Event &e);
extern bool interceptkey(int sym);

extern float loadprogress;
extern void renderbackground(const char *caption = NULL, Texture *mapshot = NULL, const char *mapname = NULL, const char *mapinfo = NULL, bool restore = false, bool force = false);
extern void renderprogress(float bar, const char *text, GLuint tex = 0, bool background = false);

extern void getfps(int &fps, int &bestdiff, int &worstdiff);
extern void swapbuffers();
extern int getclockmillis();

// menu
extern void menuprocess();
extern void addchange(const char *desc, int type);
extern void clearchanges(int type);

// physics
extern void mousemove(int dx, int dy);
extern bool pointincube(const clipplanes &p, const vec &v);
extern bool overlapsdynent(const vec &o, float radius);
extern void rotatebb(vec &center, vec &radius, int yaw);
extern float shadowray(const vec &o, const vec &ray, float radius, int mode, extentity *t = NULL);
struct ShadowRayCache;
extern ShadowRayCache *newshadowraycache();
extern void freeshadowraycache(ShadowRayCache *&cache);
extern void resetshadowraycache(ShadowRayCache *cache);
extern float shadowray(ShadowRayCache *cache, const vec &o, const vec &ray, float radius, int mode, extentity *t = NULL);

// world

extern vector<int> outsideents;

extern void entcancel();
extern void entitiesinoctanodes();
extern void attachentities();
extern void freeoctaentities(cube &c);
extern bool pointinsel(selinfo &sel, vec &o);

extern void resetmap();
extern void startmap(const char *name);

// rendermodel
struct mapmodelinfo { string name; model *m; };

extern void findanims(const char *pattern, vector<int> &anims);
extern void loadskin(const char *dir, const char *altdir, Texture *&skin, Texture *&masks);
extern mapmodelinfo *getmminfo(int i);
extern void startmodelquery(occludequery *query);
extern void endmodelquery();
extern void preloadmodelshaders();
extern void preloadusedmapmodels(bool msg = false, bool bih = false);

// renderparticles
extern void particleinit();
extern void clearparticles();
extern void clearparticleemitters();
extern void seedparticles();
extern void updateparticles();
extern void renderparticles(bool mainpass = false);
extern bool printparticles(extentity &e, char *buf);

// decal
extern void initdecals();
extern void cleardecals();
extern void renderdecals(bool mainpass = false);

// blob

enum
{
    BLOB_STATIC = 0,
    BLOB_DYNAMIC
};

extern int showblobs;

extern void initblobs(int type = -1);
extern void resetblobs();
extern void renderblob(int type, const vec &o, float radius, float fade = 1);
extern void flushblobs();

// rendersky
extern int explicitsky;
extern double skyarea;

extern void drawskybox(int farplane, bool limited);
extern bool limitsky();

// 3dgui
extern void g3d_render();
extern bool g3d_windowhit(bool on, bool act);

// menus
extern int mainmenu;

extern void clearmainmenu();
extern void g3d_mainmenu();

// sound
extern void clearmapsounds();
extern void checkmapsounds();
extern void updatesounds();

extern void initmumble();
extern void closemumble();
extern void updatemumble();

// grass
extern void generategrass();
extern void rendergrass();

// blendmap
extern int blendpaintmode;

struct BlendMapCache;
extern BlendMapCache *newblendmapcache();
extern void freeblendmapcache(BlendMapCache *&cache);
extern bool setblendmaporigin(BlendMapCache *cache, const ivec &o, int size);
extern bool hasblendmap(BlendMapCache *cache);
extern uchar lookupblendmap(BlendMapCache *cache, const vec &pos);
extern void resetblendmap();
extern void enlargeblendmap();
extern void shrinkblendmap(int octant);
extern void optimizeblendmap();
extern void stoppaintblendmap();
extern void trypaintblendmap();
extern void renderblendbrush(GLuint tex, float x, float y, float w, float h);
extern void renderblendbrush();
extern bool loadblendmap(stream *f, int info);
extern void saveblendmap(stream *f);
extern uchar shouldsaveblendmap();

// recorder

namespace recorder
{
    extern void stop();
    extern void capture();
    extern void cleanup();
}

#endif

#endif

