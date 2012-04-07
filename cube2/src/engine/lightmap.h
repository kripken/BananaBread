#define LM_MINW 2
#define LM_MINH 2
#define LM_MAXW 128
#define LM_MAXH 128
#define LM_PACKW 512
#define LM_PACKH 512

struct PackNode
{
    PackNode *child1, *child2;
    ushort x, y, w, h;
    int available;

    PackNode() : child1(0), child2(0), x(0), y(0), w(LM_PACKW), h(LM_PACKH), available(min(LM_PACKW, LM_PACKH)) {}
    PackNode(ushort x, ushort y, ushort w, ushort h) : child1(0), child2(0), x(x), y(y), w(w), h(h), available(min(w, h)) {}

    void clear()
    {
        DELETEP(child1);
        DELETEP(child2);
    }

    ~PackNode()
    {
        clear();
    }

    bool insert(ushort &tx, ushort &ty, ushort tw, ushort th);
};

enum 
{ 
    LM_DIFFUSE = 0, 
    LM_BUMPMAP0, 
    LM_BUMPMAP1, 
    LM_TYPE = 0x0F,

    LM_ALPHA = 1<<4,  
    LM_FLAGS = 0xF0 
};

struct LightMap
{
    int type, bpp, tex, offsetx, offsety;
    PackNode packroot;
    uint lightmaps, lumels;
    int unlitx, unlity; 
    uchar *data;

    LightMap()
     : type(LM_DIFFUSE), bpp(3), tex(-1), offsetx(-1), offsety(-1),
       lightmaps(0), lumels(0), unlitx(-1), unlity(-1),
       data(NULL)
    {
    }

    ~LightMap()
    {
        if(data) delete[] data;
    }

    void finalize()
    {
        packroot.clear();
        packroot.available = 0;
    }

    void copy(ushort tx, ushort ty, uchar *src, ushort tw, ushort th);
    bool insert(ushort &tx, ushort &ty, uchar *src, ushort tw, ushort th);
};

extern vector<LightMap> lightmaps;

struct LightMapTexture
{
    int w, h, type;
    GLuint id;
    int unlitx, unlity;

    LightMapTexture()
     : w(0), h(0), type(LM_DIFFUSE), id(0), unlitx(-1), unlity(-1)
    {}
};

extern vector<LightMapTexture> lightmaptexs;

extern bvec ambientcolor, skylightcolor;

extern void clearlights();
extern void initlights();
extern void clearlightcache(int e = -1);
extern void resetlightmaps(bool fullclean = true);
extern void brightencube(cube &c);
extern void setsurfaces(cube &c, const surfaceinfo *surfs, const vertinfo *verts, int numverts);
extern void setsurface(cube &c, int orient, const surfaceinfo &surf, const vertinfo *verts, int numverts);
extern void previewblends(const ivec &bo, const ivec &bs);

struct lerpvert
{
    vec normal;
    float u, v;

    bool operator==(const lerpvert &l) const { return u == l.u && v == l.v; }
    bool operator!=(const lerpvert &l) const { return u != l.u || v != l.v; }
};
    
struct lerpbounds
{
    const lerpvert *min;
    const lerpvert *max;
    float u, ustep;
    vec normal, nstep;
    int winding;
};

extern void calcnormals(bool lerptjoints = false);
extern void clearnormals();
extern void findnormal(const vec &key, const vec &surface, vec &v);
extern void calclerpverts(const vec2 *c, const vec *n, lerpvert *lv, int &numv);
extern void initlerpbounds(float u, float v, const lerpvert *lv, int numv, lerpbounds &start, lerpbounds &end);
extern void lerpnormal(float u, float v, const lerpvert *lv, int numv, lerpbounds &start, lerpbounds &end, vec &normal, vec &nstep);

#define CHECK_CALCLIGHT_PROGRESS_LOCKED(exit, show_calclight_progress, before, after) \
    if(check_calclight_progress) \
    { \
        if(!calclight_canceled) \
        { \
            before; \
            show_calclight_progress(); \
            check_calclight_canceled(); \
            after; \
        } \
        if(calclight_canceled) { exit; } \
    }
#define CHECK_CALCLIGHT_PROGRESS(exit, show_calclight_progress) CHECK_CALCLIGHT_PROGRESS_LOCKED(exit, show_calclight_progress, , )

extern bool calclight_canceled;
extern volatile bool check_calclight_progress;

extern void check_calclight_canceled();

extern int lightmapping;

