#include "engine.h"

VARNP(blobs, showblobs, 0, 1, 1);
VARFP(blobintensity, 0, 60, 100, resetblobs());
VARFP(blobheight, 1, 32, 128, resetblobs());
VARFP(blobfadelow, 1, 8, 32, resetblobs());
VARFP(blobfadehigh, 1, 8, 32, resetblobs());
VARFP(blobmargin, 0, 1, 16, resetblobs());

VAR(dbgblob, 0, 0, 1);

struct blobinfo
{
    vec o;
    float radius;
    int millis;
    uint startindex, endindex;
    ushort startvert, endvert;
};

struct blobvert
{
    vec pos;
    float u, v;
    bvec color;
    uchar alpha;
};

struct blobrenderer
{
    const char *texname;
    Texture *tex;
    blobinfo **cache;
    int cachesize;
    blobinfo *blobs;
    int maxblobs, startblob, endblob;
    blobvert *verts;
    int maxverts, startvert, endvert, availverts;
    ushort *indexes;
    int maxindexes, startindex, endindex, availindexes;
    
    blobinfo *lastblob, *flushblob;

    vec blobmin, blobmax;
    ivec bborigin, bbsize;
    float blobalphalow, blobalphahigh;
    uchar blobalpha;

    blobrenderer(const char *texname)
      : texname(texname), tex(NULL),
        cache(NULL), cachesize(0),
        blobs(NULL), maxblobs(0), startblob(0), endblob(0),
        verts(NULL), maxverts(0), startvert(0), endvert(0), availverts(0),
        indexes(NULL), maxindexes(0), startindex(0), endindex(0), availindexes(0),
        lastblob(NULL)
    {}

    void init(int tris)
    {
        if(cache)
        {
            DELETEA(cache);
            cachesize = 0;
        }
        if(blobs)
        {
            DELETEA(blobs);
            maxblobs = startblob = endblob = 0;
        }
        if(verts)
        {
            DELETEA(verts);
            maxverts = startvert = endvert = availverts = 0;
        }
        if(indexes)
        {
            DELETEA(indexes);
            maxindexes = startindex = endindex = availindexes = 0;
        }
        if(!tris) return;
        tex = textureload(texname, 3);
        cachesize = tris/2;
        cache = new blobinfo *[cachesize];
        memset(cache, 0, cachesize * sizeof(blobinfo *));
        maxblobs = tris/2;
        blobs = new blobinfo[maxblobs];
        memset(blobs, 0, maxblobs * sizeof(blobinfo));
        maxindexes = tris*3 + 3;
        availindexes = maxindexes - 3;
        indexes = new ushort[maxindexes];
        maxverts = min(tris*3/2 + 1, (1<<16)-1);
        availverts = maxverts - 1;
        verts = new blobvert[maxverts];
    }

    bool freeblob()
    {
        blobinfo &b = blobs[startblob];
        if(&b == lastblob) return false;

        startblob++;
        if(startblob >= maxblobs) startblob = 0;

        startvert = b.endvert;
        if(startvert>=maxverts) startvert = 0;
        availverts += b.endvert - b.startvert;

        startindex = b.endindex;
        if(startindex>=maxindexes) startindex = 0;
        availindexes += b.endindex - b.startindex;

        b.millis = 0;

        return true;
    }

    blobinfo &newblob(const vec &o, float radius)
    {
        blobinfo &b = blobs[endblob];
        int next = endblob + 1;
        if(next>=maxblobs) next = 0;
        if(next==startblob) 
        {
            lastblob = &b;
            freeblob();
        }
        endblob = next;
        b.o = o;
        b.radius = radius;
        b.millis = totalmillis;
        b.startindex = b.endindex = endindex;
        b.startvert = b.endvert = endvert;
        lastblob = &b;
        return b;
    }

    void clearblobs()
    {
        startblob = endblob = 0;
        startvert = endvert = 0;
        availverts = maxverts - 1;
        startindex = endindex = 0;
        availindexes = maxindexes - 3;
    }
    
    template<int C>
    static int split(const vec *in, int numin, float below, float above, vec *out)
    {
        int numout = 0;
        const vec *p = &in[numin-1];
        float pc = (*p)[C];
        loopi(numin)
        {
            const vec &v = in[i];
            float c = v[C];
            if(c < below)
            {
                if(pc > above) out[numout++] = vec(*p).sub(v).mul((above - c)/(pc - c)).add(v);
                if(pc > below) out[numout++] = vec(*p).sub(v).mul((below - c)/(pc - c)).add(v);
            }
            else if(c > above)
            {
                if(pc < below) out[numout++] = vec(*p).sub(v).mul((below - c)/(pc - c)).add(v);
                if(pc < above) out[numout++] = vec(*p).sub(v).mul((above - c)/(pc - c)).add(v);
            }
            else if(pc < below)
            {
                if(c > below) out[numout++] = vec(*p).sub(v).mul((below - c)/(pc - c)).add(v);
            }
            else if(pc > above && c < above) out[numout++] = vec(*p).sub(v).mul((above - c)/(pc - c)).add(v);
            out[numout++] = v;
            p = &v;
            pc = c;
        }
        return numout;
    }

    template<int C>
    static int clip(const vec *in, int numin, float below, float above, vec *out)
    {
        int numout = 0;
        const vec *p = &in[numin-1];
        float pc = (*p)[C];
        loopi(numin)
        {
            const vec &v = in[i];
            float c = v[C];
            if(c < below)
            {
                if(pc > above) out[numout++] = vec(*p).sub(v).mul((above - c)/(pc - c)).add(v);
                if(pc > below) out[numout++] = vec(*p).sub(v).mul((below - c)/(pc - c)).add(v);
            }
            else if(c > above)
            {
                if(pc < below) out[numout++] = vec(*p).sub(v).mul((below - c)/(pc - c)).add(v);
                if(pc < above) out[numout++] = vec(*p).sub(v).mul((above - c)/(pc - c)).add(v);
            }
            else
            {
                if(pc < below)  
                {
                    if(c > below) out[numout++] = vec(*p).sub(v).mul((below - c)/(pc - c)).add(v);
                }
                else if(pc > above && c < above) out[numout++] = vec(*p).sub(v).mul((above - c)/(pc - c)).add(v);
                out[numout++] = v;
            }
            p = &v;
            pc = c;
        }
        return numout;
    }
        
    void dupblob()
    {
        if(lastblob->startvert >= lastblob->endvert) 
        {
            lastblob->startindex = lastblob->endindex = endindex;
            lastblob->startvert = lastblob->endvert = endvert;
            return; 
        }
        blobinfo &b = newblob(lastblob->o, lastblob->radius);
        b.millis = -1;
    }

    inline int addvert(const vec &pos)
    {
        blobvert &v = verts[endvert];
        v.pos = pos;
        v.u = (pos.x - blobmin.x) / (blobmax.x - blobmin.x);
        v.v = (pos.y - blobmin.y) / (blobmax.y - blobmin.y);
        v.color = bvec(255, 255, 255);
        if(pos.z < blobmin.z + blobfadelow) v.alpha = uchar(blobalphalow * (pos.z - blobmin.z));
        else if(pos.z > blobmax.z - blobfadehigh) v.alpha = uchar(blobalphahigh * (blobmax.z - pos.z));
        else v.alpha = blobalpha;
        return endvert++;
    }

    void addtris(const vec *v, int numv)
    {
        if(endvert != int(lastblob->endvert) || endindex != int(lastblob->endindex)) dupblob();
        for(const vec *cur = &v[2], *end = &v[numv];;)
        {
            int limit = maxverts - endvert - 2;
            if(limit <= 0)
            {
                while(availverts < limit+2) if(!freeblob()) return;
                availverts -= limit+2;
                lastblob->endvert = maxverts;
                endvert = 0;
                dupblob();
                limit = maxverts - 2;
            }
            limit = min(int(end - cur), min(limit, (maxindexes - endindex)/3));
            while(availverts < limit+2) if(!freeblob()) return;
            while(availindexes < limit*3) if(!freeblob()) return;

            int i1 = addvert(v[0]), i2 = addvert(cur[-1]);
            loopk(limit)
            {
                indexes[endindex++] = i1;
                indexes[endindex++] = i2;
                i2 = addvert(*cur++);
                indexes[endindex++] = i2; 
            }

            availverts -= endvert - lastblob->endvert;
            availindexes -= endindex - lastblob->endindex;
            lastblob->endvert = endvert;
            lastblob->endindex = endindex;
            if(endvert >= maxverts) endvert = 0;
            if(endindex >= maxindexes) endindex = 0;

            if(cur >= end) break;
            dupblob();
        }
    }

    void gentris(cube &cu, int orient, const ivec &o, int size, materialsurface *mat = NULL)
    {
        vec pos[MAXFACEVERTS+8];
        int dim = dimension(orient), numverts = 0, numplanes = 1, flat = -1;
        if(mat)
        {
            switch(orient)
            {
            #define GENFACEORIENT(orient, v0, v1, v2, v3) \
                case orient: v0 v1 v2 v3 break;
            #define GENFACEVERT(orient, vert, x,y,z, xv,yv,zv) \
                    pos[numverts++] = vec(x xv, y yv, z zv);
                GENFACEVERTS(o.x, o.x, o.y, o.y, o.z, o.z, , + mat->csize, , + mat->rsize, + 0.1f, - 0.1f);
            #undef GENFACEORIENT
            #undef GENFACEVERT 
            }
            flat = dim;
        }
        else if(cu.texture[orient] == DEFAULT_SKY) return;
        else if(cu.ext && (numverts = cu.ext->surfaces[orient].numverts&MAXFACEVERTS))
        {
            vertinfo *verts = cu.ext->verts() + cu.ext->surfaces[orient].verts;
            ivec vo = ivec(o).mask(~0xFFF).shl(3);
            loopj(numverts) pos[j] = verts[j].getxyz().add(vo).tovec().mul(1/8.0f);
            if(numverts >= 4 && !(cu.merged&(1<<orient)) && !flataxisface(cu, orient) && faceconvexity(verts, numverts)) numplanes++;
            else flat = dim;
        }
        else if(cu.merged&(1<<orient)) return;
        else
        {
            ivec v[4];
            genfaceverts(cu, orient, v);
            int vis = 3, convex = faceconvexity(v, vis), order = convex < 0 ? 1 : 0;
            vec vo = o.tovec();
            pos[numverts++] = v[order].tovec().mul(size/8.0f).add(vo);
            if(vis&1) pos[numverts++] = v[order+1].tovec().mul(size/8.0f).add(vo);
            pos[numverts++] = v[order+2].tovec().mul(size/8.0f).add(vo);
            if(vis&2) pos[numverts++] = v[(order+3)&3].tovec().mul(size/8.0f).add(vo);
            if(convex) numplanes++;
            else flat = dim;
        }

        if(flat >= 0)
        {
            float offset = pos[0][dim];
            if(offset < blobmin[dim] || offset > blobmax[dim]) return;
            flat = dim;
        }
    
        vec vmin = pos[0], vmax = pos[0];
        for(int i = 1; i < numverts; i++) { vmin.min(pos[i]); vmax.max(pos[i]); }
        if(vmax.x < blobmin.x || vmin.x > blobmax.x || vmax.y < blobmin.y || vmin.y > blobmax.y ||
           vmax.z < blobmin.z || vmin.z > blobmax.z)
            return;

        vec v1[MAXFACEVERTS+6+4], v2[MAXFACEVERTS+6+4];
        loopl(numplanes)
        {
            vec *v = pos;
            int numv = numverts;
            if(numplanes >= 2)
            {
                if(l) { pos[1] = pos[2]; pos[2] = pos[3]; }
                numv = 3;
            }
            #define CLIPSIDE(clip, below, above) \
                { \
                    vec *in = v; \
                    v = in==v1 ? v2 : v1; \
                    numv = clip(in, numv, below, above, v); \
                    if(numv < 3) continue; \
                }
            if(flat!=0) CLIPSIDE(clip<0>, blobmin.x, blobmax.x);
            if(flat!=1) CLIPSIDE(clip<1>, blobmin.y, blobmax.y);
            if(flat!=2) 
            {
                CLIPSIDE(clip<2>, blobmin.z, blobmax.z);
                CLIPSIDE(split<2>, blobmin.z + blobfadelow, blobmax.z - blobfadehigh);
            }
            addtris(v, numv);
        }
    }

    void findmaterials(vtxarray *va)
    {
        materialsurface *matbuf = va->matbuf;
        int matsurfs = va->matsurfs;
        loopi(matsurfs)
        {
            materialsurface &m = matbuf[i];
            if(!isclipped(m.material&MATF_VOLUME) || m.orient == O_BOTTOM) { i += m.skip; continue; }
            int dim = dimension(m.orient), c = C[dim], r = R[dim];
            for(;;)
            {
                materialsurface &m = matbuf[i];
                if(m.o[dim] >= blobmin[dim] && m.o[dim] <= blobmax[dim] &&
                   m.o[c] + m.csize >= blobmin[c] && m.o[c] <= blobmax[c] &&
                   m.o[r] + m.rsize >= blobmin[r] && m.o[r] <= blobmax[r])
                {
                    static cube dummy;
                    gentris(dummy, m.orient, m.o, max(m.csize, m.rsize), &m);
                }
                if(i+1 >= matsurfs) break;
                materialsurface &n = matbuf[i+1];
                if(n.material != m.material || n.orient != m.orient) break;
                i++;
            }
        }
    }
            
    void findescaped(cube *cu, const ivec &o, int size, int escaped)
    {
        loopi(8)
        {
            if(escaped&(1<<i))
            {
                ivec co(i, o.x, o.y, o.z, size);
                if(cu[i].children) findescaped(cu[i].children, co, size>>1, cu[i].escaped);
                else
                {
                    int vismask = cu[i].escaped&cu[i].merged;
                    if(vismask) loopj(6) if(vismask&(1<<j)) gentris(cu[i], j, co, size);
                }
            }
        }
    }

    void gentris(cube *cu, const ivec &o, int size, int escaped = 0)
    {
        int overlap = octantrectangleoverlap(o, size, bborigin, bbsize);
        loopi(8)
        {
            if(overlap&(1<<i))
            {
                ivec co(i, o.x, o.y, o.z, size);
                if(cu[i].ext && cu[i].ext->va && cu[i].ext->va->matsurfs)
                    findmaterials(cu[i].ext->va);
                if(cu[i].children) gentris(cu[i].children, co, size>>1, cu[i].escaped);
                else
                {
                    int vismask = cu[i].visible|cu[i].merged;
                    if(vismask) loopj(6) if(vismask&(1<<j)) gentris(cu[i], j, co, size);
                }
            }
            else if(escaped&(1<<i))
            {
                ivec co(i, o.x, o.y, o.z, size);
                if(cu[i].children) findescaped(cu[i].children, co, size>>1, cu[i].escaped);
                else
                {
                    int vismask = cu[i].escaped&cu[i].merged;
                    if(vismask) loopj(6) if(vismask&(1<<j)) gentris(cu[i], j, co, size);
                }
            }
        }
    }

    blobinfo *addblob(const vec &o, float radius, float fade)
    {
        lastblob = &blobs[endblob];
        blobinfo &b = newblob(o, radius);
        blobmin = blobmax = o;
        blobmin.x -= radius;
        blobmin.y -= radius;
        blobmin.z -= blobheight + blobfadelow;
        blobmax.x += radius;
        blobmax.y += radius;
        blobmax.z += blobfadehigh;
        (bborigin = blobmin).sub(2);
        (bbsize = blobmax).sub(blobmin).add(4);
        float scale =  fade*blobintensity*255/100.0f;
        blobalphalow = scale / blobfadelow;
        blobalphahigh = scale / blobfadehigh;
        blobalpha = uchar(scale);
        gentris(worldroot, ivec(0, 0, 0), worldsize>>1);
        return b.millis >= 0 ? &b : NULL;
    } 

    static void setuprenderstate()
    {
        foggedshader->set();

        enablepolygonoffset(GL_POLYGON_OFFSET_FILL);

        glDepthMask(GL_FALSE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        if(!dbgblob) glEnable(GL_BLEND);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
    }

    static void cleanuprenderstate()
    {
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        disablepolygonoffset(GL_POLYGON_OFFSET_FILL);
    }

    static int lastreset;

    static void reset()
    {
        lastreset = totalmillis;
    }

    static blobrenderer *lastrender;

    void fadeblob(blobinfo *b, float fade)
    {
        float minz = b->o.z - (blobheight + blobfadelow), maxz = b->o.z + blobfadehigh,
              scale = fade*blobintensity*255/100.0f, scalelow = scale / blobfadelow, scalehigh = scale / blobfadehigh;
        uchar alpha = uchar(scale); 
        b->millis = totalmillis;
        do
        {
            if(b->endvert - b->startvert >= 3) for(blobvert *v = &verts[b->startvert], *end = &verts[b->endvert]; v < end; v++)
            {
                float z = v->pos.z;
                if(z < minz + blobfadelow) v->alpha = uchar(scalelow * (z - minz));
                else if(z > maxz - blobfadehigh) v->alpha = uchar(scalehigh * (maxz - z));
                else v->alpha = alpha;
            }
            int offset = b - &blobs[0] + 1;
            if(offset >= maxblobs) offset = 0;
            if(offset < endblob ? offset > startblob || startblob > endblob : offset > startblob) b = &blobs[offset];
            else break;
        } while(b->millis < 0);
    }

    void renderblob(const vec &o, float radius, float fade)
    {
        if(lastrender != this)
        {
            if(!lastrender) 
            {
                if(!blobs) initblobs();

                setuprenderstate();
            }
            glVertexPointer(3, GL_FLOAT, sizeof(blobvert), &verts->pos);
            glTexCoordPointer(2, GL_FLOAT, sizeof(blobvert), &verts->u);
            glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(blobvert), &verts->color);
            if(!lastrender || lastrender->tex != tex) glBindTexture(GL_TEXTURE_2D, tex->id);
            lastrender = this;
        }
    
        union { int i; float f; } ox, oy;
        ox.f = o.x; oy.f = o.y;
        uint hash = uint(ox.i^~oy.i^(INT_MAX-oy.i)^uint(radius));
        hash %= cachesize;
        blobinfo *b = cache[hash];
        if(!b || b->millis <= lastreset || b->o!=o || b->radius!=radius)
        {
            b = addblob(o, radius, fade);
            cache[hash] = b;
            if(!b) return;
        }
        else if(fade < 1 && b->millis < totalmillis) fadeblob(b, fade); 
        do
        {
            if(b->endvert - b->startvert >= 3)
            {
                if(hasDRE) glDrawRangeElements_(GL_TRIANGLES, b->startvert, b->endvert-1, b->endindex - b->startindex, GL_UNSIGNED_SHORT, &indexes[b->startindex]);
                else glDrawElements(GL_TRIANGLES, b->endindex - b->startindex, GL_UNSIGNED_SHORT, &indexes[b->startindex]);
                xtravertsva += b->endvert - b->startvert;
            }
            int offset = b - &blobs[0] + 1;
            if(offset >= maxblobs) offset = 0; 
            if(offset < endblob ? offset > startblob || startblob > endblob : offset > startblob) b = &blobs[offset];
            else break; 
        } while(b->millis < 0);
    }
};

int blobrenderer::lastreset = 0;
blobrenderer *blobrenderer::lastrender = NULL;

VARFP(blobstattris, 128, 4096, 1<<16, initblobs(BLOB_STATIC));
VARFP(blobdyntris, 128, 4096, 1<<16, initblobs(BLOB_DYNAMIC));

static blobrenderer blobs[] = 
{
    blobrenderer("<grey>packages/particles/blob.png"),
    blobrenderer("<grey>packages/particles/blob.png")
};

void initblobs(int type)
{
    if(type < 0 || (type==BLOB_STATIC && blobs[BLOB_STATIC].blobs)) blobs[BLOB_STATIC].init(showblobs ? blobstattris : 0);
    if(type < 0 || (type==BLOB_DYNAMIC && blobs[BLOB_DYNAMIC].blobs)) blobs[BLOB_DYNAMIC].init(showblobs ? blobdyntris : 0);
}

void resetblobs()
{
    blobrenderer::lastreset = totalmillis;
}

void renderblob(int type, const vec &o, float radius, float fade)
{
    if(!showblobs) return;
    if(refracting < 0 && o.z - blobheight - blobfadelow >= reflectz) return;
    blobs[type].renderblob(o, radius + blobmargin, fade);
}

void flushblobs()
{
    if(blobrenderer::lastrender) blobrenderer::cleanuprenderstate();
    blobrenderer::lastrender = NULL;
}

