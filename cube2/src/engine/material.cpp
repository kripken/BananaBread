#include "engine.h"

struct QuadNode
{
    int x, y, size;
    uint filled;
    QuadNode *child[4];

    QuadNode(int x, int y, int size) : x(x), y(y), size(size), filled(0) { loopi(4) child[i] = 0; }

    void clear() 
    {
        loopi(4) DELETEP(child[i]);
    }
    
    ~QuadNode()
    {
        clear();
    }

    void insert(int mx, int my, int msize)
    {
        if(size == msize)
        {
            filled = 0xF;
            return;
        }
        int csize = size>>1, i = 0;
        if(mx >= x+csize) i |= 1;
        if(my >= y+csize) i |= 2;
        if(csize == msize)
        {
            filled |= (1 << i);
            return;
        }
        if(!child[i]) child[i] = new QuadNode(i&1 ? x+csize : x, i&2 ? y+csize : y, csize);
        child[i]->insert(mx, my, msize);
        loopj(4) if(child[j])
        {
            if(child[j]->filled == 0xF)
            {
                DELETEP(child[j]);
                filled |= (1 << j);
            }
        }
    }

    void genmatsurf(uchar mat, uchar orient, uchar flags, int x, int y, int z, int size, materialsurface *&matbuf)
    {
        materialsurface &m = *matbuf++;
        m.material = mat;
        m.orient = orient;
        m.flags = flags;
        m.csize = size;
        m.rsize = size;
        int dim = dimension(orient);
        m.o[C[dim]] = x;
        m.o[R[dim]] = y;
        m.o[dim] = z;
    }

    void genmatsurfs(uchar mat, uchar orient, uchar flags, int z, materialsurface *&matbuf)
    {
        if(filled == 0xF) genmatsurf(mat, orient, flags, x, y, z, size, matbuf);
        else if(filled)
        {
            int csize = size>>1;
            loopi(4) if(filled & (1 << i))
                genmatsurf(mat, orient, flags, i&1 ? x+csize : x, i&2 ? y+csize : y, z, csize, matbuf);
        }
        loopi(4) if(child[i]) child[i]->genmatsurfs(mat, orient, flags, z, matbuf);
    }
};

static float wfwave, wfscroll, wfxscale, wfyscale;

static void renderwaterfall(const materialsurface &m, float offset, const vec *normal = NULL)
{
    if(varray::data.empty())
    {
        varray::defattrib(varray::ATTRIB_VERTEX, 3, GL_FLOAT);
        if(normal) varray::defattrib(varray::ATTRIB_NORMAL, 3, GL_FLOAT);
        varray::defattrib(varray::ATTRIB_TEXCOORD0, 2, GL_FLOAT);
        varray::begin(GL_QUADS);
    }
    float x = m.o.x, y = m.o.y, zmin = m.o.z, zmax = zmin;
    if(m.ends&1) zmin += -WATER_OFFSET-WATER_AMPLITUDE;
    if(m.ends&2) zmax += wfwave;
    int csize = m.csize, rsize = m.rsize;
#define GENFACEORIENT(orient, v0, v1, v2, v3) \
        case orient: v0 v1 v2 v3 break;
#undef GENFACEVERTX
#define GENFACEVERTX(orient, vert, mx,my,mz, sx,sy,sz) \
            { \
                vec v(mx sx, my sy, mz sz); \
                varray::attrib<float>(v.x, v.y, v.z); \
                GENFACENORMAL \
                varray::attrib<float>(wfxscale*v.y, wfyscale*(v.z+wfscroll)); \
            }
#undef GENFACEVERTY
#define GENFACEVERTY(orient, vert, mx,my,mz, sx,sy,sz) \
            { \
                vec v(mx sx, my sy, mz sz); \
                varray::attrib<float>(v.x, v.y, v.z); \
                GENFACENORMAL \
                varray::attrib<float>(wfxscale*v.x, wfyscale*(v.z+wfscroll)); \
            }
#define GENFACENORMAL varray::attrib<float>(n.x, n.y, n.z);
    if(normal) 
    { 
        vec n = *normal; 
        switch(m.orient) { GENFACEVERTSXY(x, x, y, y, zmin, zmax, /**/, + csize, /**/, + rsize, + offset, - offset) }
    }
#undef GENFACENORMAL
#define GENFACENORMAL
    else switch(m.orient) { GENFACEVERTSXY(x, x, y, y, zmin, zmax, /**/, + csize, /**/, + rsize, + offset, - offset) }
#undef GENFACENORMAL
#undef GENFACEORIENT
#undef GENFACEVERTX
#define GENFACEVERTX(o,n, x,y,z, xv,yv,zv) GENFACEVERT(o,n, x,y,z, xv,yv,zv)
#undef GENFACEVERTY
#define GENFACEVERTY(o,n, x,y,z, xv,yv,zv) GENFACEVERT(o,n, x,y,z, xv,yv,zv)
}

static void drawmaterial(const materialsurface &m, float offset)
{
    if(varray::data.empty())
    {
        varray::defattrib(varray::ATTRIB_VERTEX, 3, GL_FLOAT);
        varray::begin(GL_QUADS);
    }
    float x = m.o.x, y = m.o.y, z = m.o.z, csize = m.csize, rsize = m.rsize;
    switch(m.orient)
    {
#define GENFACEORIENT(orient, v0, v1, v2, v3) \
        case orient: v0 v1 v2 v3 break;
#define GENFACEVERT(orient, vert, mx,my,mz, sx,sy,sz) \
            varray::attrib<float>(mx sx, my sy, mz sz); 
        GENFACEVERTS(x, x, y, y, z, z, /**/, + csize, /**/, + rsize, + offset, - offset)
#undef GENFACEORIENT
#undef GENFACEVERT
    }
}

struct material
{
    const char *name;
    uchar id;
} materials[] = 
{
    {"air", MAT_AIR},
    {"water", MAT_WATER},
    {"clip", MAT_CLIP},
    {"glass", MAT_GLASS},
    {"noclip", MAT_NOCLIP},
    {"lava", MAT_LAVA},
    {"gameclip", MAT_GAMECLIP},
    {"death", MAT_DEATH},
    {"alpha", MAT_ALPHA}
};

int findmaterial(const char *name)
{
    loopi(sizeof(materials)/sizeof(material))
    {
        if(!strcmp(materials[i].name, name)) return materials[i].id;
    } 
    return -1;
}  
    
int visiblematerial(cube &c, int orient, int x, int y, int z, int size, uchar matmask)
{   
    uchar mat = c.material&matmask;
    switch(mat)
    {
    case MAT_AIR:
         break;

    case MAT_LAVA:
    case MAT_WATER:
        if(visibleface(c, orient, x, y, z, size, mat, MAT_AIR, matmask))
            return (orient != O_BOTTOM ? MATSURF_VISIBLE : MATSURF_EDIT_ONLY);
        break;

    case MAT_GLASS:
        if(visibleface(c, orient, x, y, z, size, MAT_GLASS, MAT_AIR, matmask))
            return MATSURF_VISIBLE;
        break;

    default:
        if(visibleface(c, orient, x, y, z, size, mat, MAT_AIR, matmask))
            return MATSURF_EDIT_ONLY;
        break;
    }
    return MATSURF_NOT_VISIBLE;
}

void genmatsurfs(cube &c, int cx, int cy, int cz, int size, vector<materialsurface> &matsurfs)
{
    loopi(6)
    {
        static uchar matmasks[] = { MATF_VOLUME, MATF_CLIP, MAT_DEATH, MAT_ALPHA };
        loopj(sizeof(matmasks)/sizeof(matmasks[0]))
        {
            int matmask = matmasks[j];
            int vis = visiblematerial(c, i, cx, cy, cz, size, matmask);
            if(vis != MATSURF_NOT_VISIBLE) 
            {
                materialsurface m;
                m.material = c.material&matmask;
                m.orient = i;
                m.flags = vis == MATSURF_EDIT_ONLY ? materialsurface::F_EDIT : 0;
                m.o = ivec(cx, cy, cz);
                m.csize = m.rsize = size;
                if(dimcoord(i)) m.o[dimension(i)] += size;
                matsurfs.add(m);
                break;
            }
        }
    }
}

static inline bool mergematcmp(const materialsurface &x, const materialsurface &y)
{
    int dim = dimension(x.orient), c = C[dim], r = R[dim];
    if(x.o[r] + x.rsize < y.o[r] + y.rsize) return true;
    if(x.o[r] + x.rsize > y.o[r] + y.rsize) return false;
    return x.o[c] < y.o[c];
}

static int mergematr(materialsurface *m, int sz, materialsurface &n)
{
    int dim = dimension(n.orient), c = C[dim], r = R[dim];
    for(int i = sz-1; i >= 0; --i)
    {
        if(m[i].o[r] + m[i].rsize < n.o[r]) break;
        if(m[i].o[r] + m[i].rsize == n.o[r] && m[i].o[c] == n.o[c] && m[i].csize == n.csize)
        {
            n.o[r] = m[i].o[r];
            n.rsize += m[i].rsize;
            memmove(&m[i], &m[i+1], (sz - (i+1)) * sizeof(materialsurface));
            return 1;
        }
    }
    return 0;
}

static int mergematc(materialsurface &m, materialsurface &n)
{
    int dim = dimension(n.orient), c = C[dim], r = R[dim];
    if(m.o[r] == n.o[r] && m.rsize == n.rsize && m.o[c] + m.csize == n.o[c])
    {
        n.o[c] = m.o[c];
        n.csize += m.csize;
        return 1;
    }
    return 0;
}

static int mergemat(materialsurface *m, int sz, materialsurface &n)
{
    for(bool merged = false; sz; merged = true)
    {
        int rmerged = mergematr(m, sz, n);
        sz -= rmerged;
        if(!rmerged && merged) break;
        if(!sz) break;
        int cmerged = mergematc(m[sz-1], n);
        sz -= cmerged;
        if(!cmerged) break;
    }
    m[sz++] = n;
    return sz;
}

static int mergemats(materialsurface *m, int sz)
{
    quicksort(m, sz, mergematcmp);

    int nsz = 0;
    loopi(sz) nsz = mergemat(m, nsz, m[i]);
    return nsz;
}

static inline bool optmatcmp(const materialsurface &x, const materialsurface &y)
{
    if(x.material < y.material) return true;
    if(x.material > y.material) return false;
    if(x.orient > y.orient) return true;
    if(x.orient < y.orient) return false;
    int dim = dimension(x.orient);
    return x.o[dim] < y.o[dim];
}

VARF(optmats, 0, 1, 1, allchanged());

int optimizematsurfs(materialsurface *matbuf, int matsurfs)
{
    quicksort(matbuf, matsurfs, optmatcmp);
    if(!optmats) return matsurfs;
    materialsurface *cur = matbuf, *end = matbuf+matsurfs;
    while(cur < end)
    {
         materialsurface *start = cur++;
         int dim = dimension(start->orient);
         while(cur < end &&
               cur->material == start->material &&
               cur->orient == start->orient &&
               cur->flags == start->flags && 
               cur->o[dim] == start->o[dim])
            ++cur;
         if(!isliquid(start->material) || start->orient != O_TOP || !vertwater)
         {
            if(start!=matbuf) memmove(matbuf, start, (cur-start)*sizeof(materialsurface));
            matbuf += mergemats(matbuf, cur-start);
         }
         else if(cur-start>=4)
         {
            QuadNode vmats(0, 0, worldsize);
            loopi(cur-start) vmats.insert(start[i].o[C[dim]], start[i].o[R[dim]], start[i].csize);
            vmats.genmatsurfs(start->material, start->orient, start->flags, start->o[dim], matbuf);
         }
         else
         {
            if(start!=matbuf) memmove(matbuf, start, (cur-start)*sizeof(materialsurface));
            matbuf += cur-start;
         }
    }
    return matsurfs - (end-matbuf);
}

extern vector<vtxarray *> valist;

struct waterinfo
{
    materialsurface *m;
    double depth, area;
};

void setupmaterials(int start, int len)
{
    int hasmat = 0;
    vector<waterinfo> water;
    unionfind uf;
    if(!len) len = valist.length();
    for(int i = start; i < len; i++) 
    {
        vtxarray *va = valist[i];
        materialsurface *skip = NULL;
        loopj(va->matsurfs)
        {
            materialsurface &m = va->matbuf[j];
            if(m.material==MAT_WATER && m.orient==O_TOP)
            {
                m.index = water.length();
                loopvk(water)
                {
                    materialsurface &n = *water[k].m;
                    if(m.o.z!=n.o.z) continue;
                    if(n.o.x+n.rsize==m.o.x || m.o.x+m.rsize==n.o.x)
                    {
                        if(n.o.y+n.csize>m.o.y && n.o.y<m.o.y+m.csize) uf.unite(m.index, n.index);
                    }
                    else if(n.o.y+n.csize==m.o.y || m.o.y+m.csize==n.o.y)
                    {
                        if(n.o.x+n.rsize>m.o.x && n.o.x<m.o.x+m.rsize) uf.unite(m.index, n.index);
                    }
                }
                waterinfo &wi = water.add();
                wi.m = &m;
                vec center(m.o.x+m.rsize/2, m.o.y+m.csize/2, m.o.z-WATER_OFFSET);
                m.light = brightestlight(center, vec(0, 0, 1));
                float depth = raycube(center, vec(0, 0, -1), 10000);
                wi.depth = double(depth)*m.rsize*m.csize;
                wi.area = m.rsize*m.csize; 
            }
            else if(isliquid(m.material) && m.orient!=O_BOTTOM && m.orient!=O_TOP)
            {
                m.ends = 0;
                int dim = dimension(m.orient), coord = dimcoord(m.orient);
                ivec o(m.o);
                o.z -= 1;
                o[dim] += coord ? 1 : -1;
                int minc = o[dim^1], maxc = minc + (C[dim]==2 ? m.rsize : m.csize);
                ivec co;
                int csize;
                while(o[dim^1] < maxc)
                {
                    cube &c = lookupcube(o.x, o.y, o.z, 0, co, csize);
                    if(isliquid(c.material&MATF_VOLUME)) { m.ends |= 1; break; }
                    o[dim^1] += csize;
                }
                o[dim^1] = minc;
                o.z += R[dim]==2 ? m.rsize : m.csize;
                o[dim] -= coord ? 2 : -2;
                while(o[dim^1] < maxc)
                {
                    cube &c = lookupcube(o.x, o.y, o.z, 0, co, csize);
                    if(visiblematerial(c, O_TOP, co.x, co.y, co.z, csize)) { m.ends |= 2; break; }
                    o[dim^1] += csize;
                }
            }
            else if(m.material==MAT_GLASS)
            {
                if(!hasCM) m.envmap = EMID_NONE;
                else
                {
                    int dim = dimension(m.orient);
                    vec center(m.o.tovec());
                    center[R[dim]] += m.rsize/2;
                    center[C[dim]] += m.csize/2;
                    m.envmap = closestenvmap(center);
                }
            }
            if(m.material&MATF_VOLUME) hasmat |= 1<<m.material;
            m.skip = 0;
            if(skip && m.material == skip->material && m.orient == skip->orient && skip->skip < 0xFF)
                skip->skip++;
            else 
                skip = &m;
        }
    }
    loopv(water)
    {
        int root = uf.find(i);
        if(i==root) continue;
        materialsurface &m = *water[i].m, &n = *water[root].m;
        if(m.light && (!m.light->attr1 || !n.light || (n.light->attr1 && m.light->attr1 > n.light->attr1))) n.light = m.light;
        water[root].depth += water[i].depth;
        water[root].area += water[i].area;
    }
    loopv(water)
    {
        int root = uf.find(i);
        water[i].m->light = water[root].m->light;
        water[i].m->depth = (short)(water[root].depth/water[root].area);
    }
    if(hasmat&(1<<MAT_WATER))
    {
        loadcaustics(true);
        preloadwatershaders(true);
        lookupmaterialslot(MAT_WATER);
    }
    if(hasmat&(1<<MAT_LAVA)) 
    {
        useshaderbyname("lava");
        useshaderbyname("lavaglare");
        lookupmaterialslot(MAT_LAVA);
    }
    if(hasmat&(1<<MAT_GLASS)) useshaderbyname("glass");
}

VARP(showmat, 0, 1, 1);

static int sortdim[3];
static ivec sortorigin;
static bool sortedit;

static inline bool vismatcmp(const materialsurface *xm, const materialsurface *ym)
{
    const materialsurface &x = *xm, &y = *ym;
    int xdim = dimension(x.orient), ydim = dimension(y.orient);
    loopi(3)
    {
        int dim = sortdim[i], xmin, xmax, ymin, ymax;
        xmin = xmax = x.o[dim];
        if(dim==C[xdim]) xmax += x.csize;
        else if(dim==R[xdim]) xmax += x.rsize;
        ymin = ymax = y.o[dim];
        if(dim==C[ydim]) ymax += y.csize;
        else if(dim==R[ydim]) ymax += y.rsize;
        if(xmax > ymin && ymax > xmin) continue;
        int c = sortorigin[dim];
        if(c > xmin && c < xmax) return sortedit;
        if(c > ymin && c < ymax) return !sortedit;
        xmin = abs(xmin - c);
        xmax = abs(xmax - c);
        ymin = abs(ymin - c);
        ymax = abs(ymax - c);
        if(max(xmin, xmax) <= min(ymin, ymax)) return sortedit;
        else if(max(ymin, ymax) <= min(xmin, xmax)) return !sortedit;
    }
    if(x.material < y.material) return sortedit;
    if(x.material > y.material) return !sortedit;
    return false;
}

extern vtxarray *visibleva, *reflectedva;

void sortmaterials(vector<materialsurface *> &vismats)
{
    sortorigin = ivec(camera1->o);
    if(reflecting) sortorigin.z = int(reflectz - (camera1->o.z - reflectz));
    vec dir;
    vecfromyawpitch(camera1->yaw, reflecting ? -camera1->pitch : camera1->pitch, 1, 0, dir);
    loopi(3) { dir[i] = fabs(dir[i]); sortdim[i] = i; }
    if(dir[sortdim[2]] > dir[sortdim[1]]) swap(sortdim[2], sortdim[1]);
    if(dir[sortdim[1]] > dir[sortdim[0]]) swap(sortdim[1], sortdim[0]);
    if(dir[sortdim[2]] > dir[sortdim[1]]) swap(sortdim[2], sortdim[1]);

    for(vtxarray *va = reflecting ? reflectedva : visibleva; va; va = reflecting ? va->rnext : va->next)
    {
        if(!va->matsurfs || va->occluded >= OCCLUDE_BB) continue;
        if(reflecting || refracting>0 ? va->o.z+va->size <= reflectz : va->o.z >= reflectz) continue;
        loopi(va->matsurfs)
        {
            materialsurface &m = va->matbuf[i];
            if(!editmode || !showmat || envmapping)
            {
                if(m.material==MAT_WATER && (m.orient==O_TOP || (refracting<0 && reflectz>worldsize))) { i += m.skip; continue; }
                if(m.flags&materialsurface::F_EDIT) { i += m.skip; continue; }
                if(glaring && m.material!=MAT_LAVA) { i += m.skip; continue; }
            }
            else if(glaring) continue;
            vismats.add(&m);
        }
    }
    sortedit = editmode && showmat && !envmapping;
    vismats.sort(vismatcmp);
}

void rendermatgrid(vector<materialsurface *> &vismats)
{
    enablepolygonoffset(GL_POLYGON_OFFSET_LINE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    int lastmat = -1;
    loopvrev(vismats)
    {
        materialsurface &m = *vismats[i];
        if(m.material != lastmat)
        {
            xtraverts += varray::end();
            lastmat = m.material;
            switch(m.material)
            {
                case MAT_WATER:    glColor3ub( 0,  0, 85); break; // blue
                case MAT_CLIP:     glColor3ub(85,  0,  0); break; // red
                case MAT_GLASS:    glColor3ub( 0, 85, 85); break; // cyan
                case MAT_NOCLIP:   glColor3ub( 0, 85,  0); break; // green
                case MAT_LAVA:     glColor3ub(85, 40,  0); break; // orange
                case MAT_GAMECLIP: glColor3ub(85, 85,  0); break; // yellow
                case MAT_DEATH:    glColor3ub(40, 40, 40); break; // black
                case MAT_ALPHA:    glColor3ub(85,  0, 85); break; // pink
            }
        }
        drawmaterial(m, -0.1f);
    }
    xtraverts += varray::end();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    disablepolygonoffset(GL_POLYGON_OFFSET_LINE);
}

VARP(glassenv, 0, 1, 1);

static void drawglass(const materialsurface &m, float offset, const vec *normal = NULL)
{
    if(varray::data.empty())
    {
        varray::defattrib(varray::ATTRIB_VERTEX, 3, GL_FLOAT);
        if(normal) varray::defattrib(varray::ATTRIB_NORMAL, 3, GL_FLOAT);
        varray::defattrib(varray::ATTRIB_TEXCOORD0, 3, GL_FLOAT);
        varray::begin(GL_QUADS);
    }
    #define GENFACEORIENT(orient, v0, v1, v2, v3) \
        case orient: v0 v1 v2 v3 break;
    #define GENFACEVERT(orient, vert, mx,my,mz, sx,sy,sz) \
        { \
            vec v(mx sx, my sy, mz sz); \
            vec reflect = vec(v).sub(camera1->o); \
            reflect[dimension(orient)] = -reflect[dimension(orient)]; \
            varray::attrib<float>(v.x, v.y, v.z); \
            GENFACENORMAL \
            varray::attrib<float>(reflect.x, reflect.y, reflect.z); \
        }
    #define GENFACENORMAL varray::attrib<float>(n.x, n.y, n.z);
    float x = m.o.x, y = m.o.y, z = m.o.z, csize = m.csize, rsize = m.rsize;
    if(normal)
    {
        vec n = *normal;
        switch(m.orient) { GENFACEVERTS(x, x, y, y, z, z, /**/, + csize, /**/, + rsize, + offset, - offset) }     
    }
    #undef GENFACENORMAL
    #define GENFACENORMAL
    else switch(m.orient) { GENFACEVERTS(x, x, y, y, z, z, /**/, + csize, /**/, + rsize, + offset, - offset) }
    #undef GENFACENORMAL
    #undef GENFACEORIENT
    #undef GENFACEVERT
}

VARFP(waterfallenv, 0, 1, 1, preloadwatershaders());

void rendermaterials()
{
    vector<materialsurface *> vismats;
    sortmaterials(vismats);
    if(vismats.empty()) return;

    glDisable(GL_CULL_FACE);

    varray::enable();

    MSlot &wslot = lookupmaterialslot(MAT_WATER, false), &lslot = lookupmaterialslot(MAT_LAVA, false);
    uchar wcol[4] = { watercolor[0], watercolor[1], watercolor[2], 192 }, 
          wfcol[4] = { waterfallcolor[0], waterfallcolor[1], waterfallcolor[2], 192 };
    if(!wfcol[0] && !wfcol[1] && !wfcol[2]) memcpy(wfcol, wcol, 3);
    int lastorient = -1, lastmat = -1;
    GLenum textured = GL_TEXTURE_2D;
    bool depth = true, blended = false, overbright = false, usedcamera = false, usedwaterfall = false;
    ushort envmapped = EMID_NONE;
    static const vec normals[6] =
    {
        vec(-1, 0, 0),
        vec( 1, 0, 0),
        vec(0, -1, 0),
        vec(0,  1, 0),
        vec(0, 0, -1),
        vec(0, 0,  1)
    };

    static const float zerofog[4] = { 0, 0, 0, 1 };
    float oldfogc[4];
    glGetFloatv(GL_FOG_COLOR, oldfogc);
    int lastfogtype = 1;
    if(editmode && showmat && !envmapping)
    {
        glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
        glEnable(GL_BLEND); blended = true;
        glDisable(GL_TEXTURE_2D); textured = 0;
        foggednotextureshader->set();
        glFogfv(GL_FOG_COLOR, zerofog); lastfogtype = 0;
        loopv(vismats)
        {
            const materialsurface &m = *vismats[i];
            if(lastmat!=m.material)
            {
                xtraverts += varray::end();
                switch(m.material)
                {
                    case MAT_WATER:    glColor3ub(255, 128,   0); break; // blue
                    case MAT_CLIP:     glColor3ub(  0, 255, 255); break; // red
                    case MAT_GLASS:    glColor3ub(255,   0,   0); break; // cyan
                    case MAT_NOCLIP:   glColor3ub(255,   0, 255); break; // green
                    case MAT_LAVA:     glColor3ub(  0, 128, 255); break; // orange
                    case MAT_GAMECLIP: glColor3ub(  0,   0, 255); break; // yellow
                    case MAT_DEATH:    glColor3ub(192, 192, 192); break; // black
                    case MAT_ALPHA:    glColor3ub(  0, 255,   0); break; // pink
                    default: continue;
                }
                lastmat = m.material;
            }
            drawmaterial(m, -0.1f);
        }
    }
    else loopv(vismats)
    {
        const materialsurface &m = *vismats[i];
        if(lastmat!=m.material || lastorient!=m.orient || (m.material==MAT_GLASS && envmapped && m.envmap != envmapped))
        {
            int fogtype = lastfogtype;
            switch(m.material)
            {
                case MAT_WATER:
                    if(!wslot.loaded || m.orient == O_TOP) continue;
                    else if(lastmat==MAT_WATER) break;
                    else
                    {
                        if(!wslot.sts.inrange(1)) continue;
                        xtraverts += varray::end();
                        glBindTexture(GL_TEXTURE_2D, wslot.sts[1].t->id);
                        float angle = fmod(float(lastmillis/(renderpath!=R_FIXEDFUNCTION ? 600.0f : 300.0f)/(2*M_PI)), 1.0f), 
                              s = angle - int(angle) - 0.5f;
                        s *= 8 - fabs(s)*16;
                        wfwave = vertwater ? WATER_AMPLITUDE*s-WATER_OFFSET : -WATER_OFFSET;
                        wfscroll = 16.0f*lastmillis/1000.0f;
                        wfxscale = TEX_SCALE/(wslot.sts[1].t->xs*wslot.scale);
                        wfyscale = TEX_SCALE/(wslot.sts[1].t->ys*wslot.scale);
                    }
                    if(lastmat!=MAT_WATER || (lastorient==O_TOP)!=(m.orient==O_TOP))
                    {
                        if(overbright) { resettmu(0); overbright = false; }
                        if(m.orient==O_TOP)
                        {
                            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                            glColor4ubv(wcol);
                            foggedshader->set();
                            fogtype = 1;
                            if(!blended) { glEnable(GL_BLEND); blended = true; }
                            if(depth) { glDepthMask(GL_FALSE); depth = false; }
                        }
                        else if(renderpath==R_FIXEDFUNCTION || ((!waterfallrefract || reflecting || refracting) && (!hasCM || !waterfallenv)))
                        {
                            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
                            glColor3ubv(wfcol);
                            foggedshader->set();
                            fogtype = 0;
                            if(!blended) { glEnable(GL_BLEND); blended = true; }
                            if(depth) { glDepthMask(GL_FALSE); depth = false; }
                        }
                        else
                        {
                            glColor3ubv(wfcol);
                            fogtype = 1;

                            if(!usedcamera)
                            {
                                setenvparamf("camera", SHPARAM_VERTEX, 0, camera1->o.x, camera1->o.y, camera1->o.z);
                                usedcamera = true;
                            }

                            #define SETWATERFALLSHADER(name) \
                            do { \
                                static Shader *name##shader = NULL; \
                                if(!name##shader) name##shader = lookupshaderbyname(#name); \
                                name##shader->set(); \
                            } while(0)

                            if(waterfallrefract && !reflecting && !refracting)
                            {
                                if(hasCM && waterfallenv) SETWATERFALLSHADER(waterfallenvrefract);    
                                else SETWATERFALLSHADER(waterfallrefract);
                                if(blended) { glDisable(GL_BLEND); blended = false; }
                                if(!depth) { glDepthMask(GL_TRUE); depth = true; }
                            }
                            else 
                            {
                                SETWATERFALLSHADER(waterfallenv);
                                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                                if(!blended) { glEnable(GL_BLEND); blended = true; }
                                if(depth) { glDepthMask(GL_FALSE); depth = false; }
                            }

                            if(!usedwaterfall)
                            {
                                Texture *dudv = wslot.sts.inrange(5) ? wslot.sts[5].t : notexture;
                                float scale = 8.0f/(dudv->ys*wslot.scale);
                                setlocalparamf("dudvoffset", SHPARAM_PIXEL, 1, 0, scale*16*lastmillis/1000.0f);

                                glActiveTexture_(GL_TEXTURE1_ARB);
                                glBindTexture(GL_TEXTURE_2D, wslot.sts.inrange(4) ? wslot.sts[4].t->id : notexture->id);
                                glActiveTexture_(GL_TEXTURE2_ARB);
                                glBindTexture(GL_TEXTURE_2D, wslot.sts.inrange(5) ? wslot.sts[5].t->id : notexture->id);
                                if(hasCM && waterfallenv)
                                {
                                    glActiveTexture_(GL_TEXTURE3_ARB);
                                    glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, lookupenvmap(wslot));
                                }
                                if(waterfallrefract && (!reflecting || !refracting))
                                {
                                    extern void setupwaterfallrefract(GLenum tmu1, GLenum tmu2);
                                    setupwaterfallrefract(GL_TEXTURE4_ARB, GL_TEXTURE0_ARB);
                                }
                                else glActiveTexture_(GL_TEXTURE0_ARB);

                                usedwaterfall = true;
                            }
                        }
                    }
                    if(textured!=GL_TEXTURE_2D) 
                    { 
                        if(textured) glDisable(textured);
                        glEnable(GL_TEXTURE_2D);
                        textured = GL_TEXTURE_2D; 
                    }
                    break;

                case MAT_LAVA:
                    if(!lslot.loaded) continue;
                    if(lastmat==MAT_LAVA && lastorient!=O_TOP && m.orient!=O_TOP) break;
                    else
                    {
                        int subslot = m.orient==O_TOP ? 0 : 1;
                        if(!lslot.sts.inrange(subslot)) continue;
                        xtraverts += varray::end();
                        glBindTexture(GL_TEXTURE_2D, lslot.sts[subslot].t->id);
                    }
                    if(m.orient!=O_TOP)
                    {
                        float angle = fmod(float(lastmillis/2000.0f/(2*M_PI)), 1.0f),
                              s = angle - int(angle) - 0.5f;
                        s *= 8 - fabs(s)*16;
                        wfwave = vertwater ? WATER_AMPLITUDE*s-WATER_OFFSET : -WATER_OFFSET;
                        wfscroll = 16.0f*lastmillis/3000.0f;
                        wfxscale = TEX_SCALE/(lslot.sts[1].t->xs*lslot.scale);
                        wfyscale = TEX_SCALE/(lslot.sts[1].t->ys*lslot.scale);
                    }
                    if(lastmat!=MAT_LAVA)
                    {
                        if(!depth) { glDepthMask(GL_TRUE); depth = true; }
                        if(blended) { glDisable(GL_BLEND); blended = false; }
                        if(renderpath==R_FIXEDFUNCTION && !overbright) { setuptmu(0, "C * T x 2"); overbright = true; }
                        float t = lastmillis/2000.0f;
                        t -= floor(t);
                        t = 1.0f - 2*fabs(t-0.5f);
                        extern int glare;
                        if(renderpath!=R_FIXEDFUNCTION && glare) t = 0.625f + 0.075f*t;
                        else t = 0.5f + 0.5f*t;
                        glColor3f(t, t, t);
                        static Shader *lavashader = NULL, *lavaglareshader = NULL;
                        if(!lavashader) lavashader = lookupshaderbyname("lava");
                        if(!lavaglareshader) lavaglareshader = lookupshaderbyname("lavaglare");
                        (glaring ? lavaglareshader : lavashader)->set();
                        fogtype = 1;
                    }
                    if(textured!=GL_TEXTURE_2D)
                    {
                        if(textured) glDisable(textured);
                        glEnable(GL_TEXTURE_2D);
                        textured = GL_TEXTURE_2D;
                    }
                    break;

                case MAT_GLASS:
                    if((m.envmap==EMID_NONE || !glassenv || (envmapped==m.envmap && textured==GL_TEXTURE_CUBE_MAP_ARB)) && lastmat==MAT_GLASS) break;
                    xtraverts += varray::end();
                    if(m.envmap!=EMID_NONE && glassenv)
                    {
                        if(textured!=GL_TEXTURE_CUBE_MAP_ARB) 
                        { 
                            if(textured) glDisable(textured); 
                            glEnable(GL_TEXTURE_CUBE_MAP_ARB); 
                            textured = GL_TEXTURE_CUBE_MAP_ARB; 
                        }
                        if(envmapped!=m.envmap)
                        {
                            glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, lookupenvmap(m.envmap));
                            if(renderpath!=R_FIXEDFUNCTION && !usedcamera) 
                            {
                                setenvparamf("camera", SHPARAM_VERTEX, 0, camera1->o.x, camera1->o.y, camera1->o.z);
                                usedcamera = true;
                            }
                            envmapped = m.envmap;
                        }
                    }
                    if(lastmat!=MAT_GLASS)
                    {
                        if(!blended) { glEnable(GL_BLEND); blended = true; }
                        if(overbright) { resettmu(0); overbright = false; }
                        if(depth) { glDepthMask(GL_FALSE); depth = false; }
                        if(m.envmap!=EMID_NONE && glassenv)
                        {
                            if(renderpath==R_FIXEDFUNCTION)
                            {
                                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                                glColor4f(0.8f, 0.9f, 1.0f, 0.25f);
                                fogtype = 1;
                            }
                            else
                            {
                                glBlendFunc(GL_ONE, GL_SRC_ALPHA);
                                glColor3f(0, 0.5f, 1.0f);
                            }
                            static Shader *glassshader = NULL;
                            if(!glassshader) glassshader = lookupshaderbyname("glass");
                            glassshader->set();
                        }
                        else
                        {
                            if(textured) 
                            { 
                                glDisable(textured);
                                textured = 0;
                            }
                            glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
                            glColor3f(0.3f, 0.15f, 0.0f);
                            foggednotextureshader->set();
                            fogtype = 0;
                        }
                    }
                    break;
                
                default: continue;
            }
            lastmat = m.material;
            lastorient = m.orient;
            if(fogtype!=lastfogtype)
            {
                glFogfv(GL_FOG_COLOR, fogtype ? oldfogc : zerofog);
                lastfogtype = fogtype;
            }
        }
        switch(m.material)
        {
            case MAT_WATER:
                renderwaterfall(m, 0.1f, renderpath!=R_FIXEDFUNCTION && hasCM && waterfallenv ? &normals[m.orient] : NULL);
                break;

            case MAT_LAVA:
                if(m.orient==O_TOP) renderlava(m, lslot.sts[0].t, lslot.scale);
                else renderwaterfall(m, 0.1f);
                break;

            case MAT_GLASS:
                if(m.envmap!=EMID_NONE && glassenv) drawglass(m, 0.1f, renderpath!=R_FIXEDFUNCTION ? &normals[m.orient] : NULL);
                else drawmaterial(m, 0.1f);
                break;
        }
    }

    xtraverts += varray::end();

    if(!depth) glDepthMask(GL_TRUE);
    if(blended) glDisable(GL_BLEND);
    if(overbright) resettmu(0);
    if(!lastfogtype) glFogfv(GL_FOG_COLOR, oldfogc);
    if(editmode && showmat && !envmapping)
    {
        foggedlineshader->set();
        rendermatgrid(vismats);
    }

    varray::disable();

    glEnable(GL_CULL_FACE);
    if(textured!=GL_TEXTURE_2D)
    {
        if(textured) glDisable(textured);
        glEnable(GL_TEXTURE_2D);
    }
}

