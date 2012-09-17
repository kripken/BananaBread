// octarender.cpp: fill vertex arrays with different cube surfaces.

#include "engine.h"

struct vboinfo
{
    int uses;
    uchar *data;
};

static inline uint hthash(GLuint key)
{
    return key;
}

static inline bool htcmp(GLuint x, GLuint y)
{
    return x==y;
}

hashtable<GLuint, vboinfo> vbos;

VAR(printvbo, 0, 0, 1);
VARFN(vbosize, maxvbosize, 0, 1<<14, 1<<16, allchanged());

enum
{
    VBO_VBUF = 0,
    VBO_EBUF,
    VBO_SKYBUF,
    NUMVBO
};

static vector<uchar> vbodata[NUMVBO];
static vector<vtxarray *> vbovas[NUMVBO];
static int vbosize[NUMVBO];

void destroyvbo(GLuint vbo)
{
    vboinfo *exists = vbos.access(vbo);
    if(!exists) return;
    vboinfo &vbi = *exists;
    if(vbi.uses <= 0) return;
    vbi.uses--;
    if(!vbi.uses) 
    {
        if(hasVBO) glDeleteBuffers_(1, &vbo);
        else if(vbi.data) delete[] vbi.data;
        vbos.remove(vbo);
    }
}

void genvbo(int type, void *buf, int len, vtxarray **vas, int numva)
{
    GLuint vbo;
    uchar *data = NULL;
    if(hasVBO)
    {
        glGenBuffers_(1, &vbo);
        GLenum target = type==VBO_VBUF ? GL_ARRAY_BUFFER_ARB : GL_ELEMENT_ARRAY_BUFFER_ARB;
        glBindBuffer_(target, vbo);
        glBufferData_(target, len, buf, GL_STATIC_DRAW_ARB);
        glBindBuffer_(target, 0);
    }
    else
    {
        static GLuint nextvbo = 0;
        if(!nextvbo) nextvbo++; // just in case it ever wraps around
        vbo = nextvbo++;
        data = new uchar[len];
        memcpy(data, buf, len);
    }
    vboinfo &vbi = vbos[vbo]; 
    vbi.uses = numva;
    vbi.data = data;
 
    if(printvbo) conoutf(CON_DEBUG, "vbo %d: type %d, size %d, %d uses", vbo, type, len, numva);

    loopi(numva)
    {
        vtxarray *va = vas[i];
        switch(type)
        {
            case VBO_VBUF: 
                va->vbuf = vbo; 
                if(!hasVBO) va->vdata = (vertex *)(data + (size_t)va->vdata);
                break;
            case VBO_EBUF: 
                va->ebuf = vbo; 
                if(!hasVBO) va->edata = (ushort *)(data + (size_t)va->edata);
                break;
            case VBO_SKYBUF: 
                va->skybuf = vbo; 
                if(!hasVBO) va->skydata = (ushort *)(data + (size_t)va->skydata);
                break;
        }
    }
}

bool readva(vtxarray *va, ushort *&edata, uchar *&vdata)
{
    if(!va->vbuf || !va->ebuf) return false;

    edata = new ushort[3*va->tris];
    vdata = new uchar[va->verts*VTXSIZE];

    if(hasVBO)
    {
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER_ARB, va->ebuf);
        glGetBufferSubData_(GL_ELEMENT_ARRAY_BUFFER_ARB, (size_t)va->edata, 3*va->tris*sizeof(ushort), edata);
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

        glBindBuffer_(GL_ARRAY_BUFFER_ARB, va->vbuf);
        glGetBufferSubData_(GL_ARRAY_BUFFER_ARB, va->voffset*VTXSIZE, va->verts*VTXSIZE, vdata);
        glBindBuffer_(GL_ARRAY_BUFFER_ARB, 0);
        return true;
    }
    else
    {
        memcpy(edata, va->edata, 3*va->tris*sizeof(ushort));
        memcpy(vdata, (uchar *)va->vdata + va->voffset*VTXSIZE, va->verts*VTXSIZE);
        return true;
    }
}

void flushvbo(int type = -1)
{
    if(type < 0)
    {
        loopi(NUMVBO) flushvbo(i);
        return;
    }

    vector<uchar> &data = vbodata[type];
    if(data.empty()) return;
    vector<vtxarray *> &vas = vbovas[type];
    genvbo(type, data.getbuf(), data.length(), vas.getbuf(), vas.length());
    data.setsize(0);
    vas.setsize(0);
    vbosize[type] = 0;
}

uchar *addvbo(vtxarray *va, int type, int numelems, int elemsize)
{
    vbosize[type] += numelems;

    vector<uchar> &data = vbodata[type];
    vector<vtxarray *> &vas = vbovas[type];

    vas.add(va);

    int len = numelems*elemsize;
    uchar *buf = data.reserve(len).buf;
    data.advance(len);
    return buf; 
}
 
struct verthash
{
    static const int SIZE = 1<<13;
    int table[SIZE];
    vector<vertex> verts;
    vector<int> chain;

    verthash() { clearverts(); }

    void clearverts() 
    { 
        memset(table, -1, sizeof(table));
        chain.setsize(0); 
        verts.setsize(0);
    }

    int addvert(const vertex &v)
    {
        uint h = hthash(v.pos)&(SIZE-1);
        for(int i = table[h]; i>=0; i = chain[i])
        {
            const vertex &c = verts[i];
            if(c.pos==v.pos && c.u==v.u && c.v==v.v && c.norm==v.norm && c.tangent==v.tangent && c.bitangent==v.bitangent)
            {
                 if(!v.lmu && !v.lmv) return i; 
                 if(c.lmu==v.lmu && c.lmv==v.lmv) return i;
            }
        }
        if(verts.length() >= USHRT_MAX) return -1;
        verts.add(v);
        chain.add(table[h]);
        return table[h] = verts.length()-1;
    }

    int addvert(const vec &pos, float u = 0, float v = 0, short lmu = 0, short lmv = 0, const bvec &norm = bvec(128, 128, 128), const bvec &tangent = bvec(128, 128, 128), uchar bitangent = 128)
    {
        vertex vtx;
        vtx.pos = pos;
        vtx.u = u;
        vtx.v = v;
        vtx.lmu = lmu;
        vtx.lmv = lmv;
        vtx.norm = norm;
        vtx.reserved = 0;
        vtx.tangent = tangent;
        vtx.bitangent = bitangent;
        return addvert(vtx);
    } 
};

enum
{
    NO_ALPHA = 0,
    ALPHA_BACK,
    ALPHA_FRONT
};

struct sortkey
{
     ushort tex, lmid, envmap;
     uchar dim, layer, alpha;

     sortkey() {}
     sortkey(ushort tex, ushort lmid, uchar dim, uchar layer = LAYER_TOP, ushort envmap = EMID_NONE, uchar alpha = NO_ALPHA)
      : tex(tex), lmid(lmid), envmap(envmap), dim(dim), layer(layer), alpha(alpha)
     {}

     bool operator==(const sortkey &o) const { return tex==o.tex && lmid==o.lmid && envmap==o.envmap && dim==o.dim && layer==o.layer && alpha==o.alpha; }
};

struct sortval
{
     int unlit;
     vector<ushort> tris[2];

     sortval() : unlit(0) {}
};

static inline bool htcmp(const sortkey &x, const sortkey &y)
{
    return x == y;
}

static inline uint hthash(const sortkey &k)
{
    return k.tex + k.lmid*9741;
}

struct vacollect : verthash
{
    ivec origin;
    int size;
    hashtable<sortkey, sortval> indices;
    vector<sortkey> texs;
    vector<grasstri> grasstris;
    vector<materialsurface> matsurfs;
    vector<octaentities *> mapmodels;
    vector<ushort> skyindices, explicitskyindices;
    int worldtris, skytris, skyfaces, skyclip, skyarea;

    void clear()
    {
        clearverts();
        worldtris = skytris = 0;
        skyfaces = 0;
        skyclip = INT_MAX;
        skyarea = 0;
        indices.clear();
        skyindices.setsize(0);
        explicitskyindices.setsize(0);
        matsurfs.setsize(0);
        mapmodels.setsize(0);
        grasstris.setsize(0);
        texs.setsize(0);
    }

    void remapunlit(vector<sortkey> &remap)
    {
        uint lastlmid[8] = { LMID_AMBIENT, LMID_AMBIENT, LMID_AMBIENT, LMID_AMBIENT, LMID_AMBIENT, LMID_AMBIENT, LMID_AMBIENT, LMID_AMBIENT }, 
             firstlmid[8] = { LMID_AMBIENT, LMID_AMBIENT, LMID_AMBIENT, LMID_AMBIENT, LMID_AMBIENT, LMID_AMBIENT, LMID_AMBIENT, LMID_AMBIENT };
        int firstlit[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
        loopv(texs)
        {
            sortkey &k = texs[i];
            if(k.lmid>=LMID_RESERVED) 
            {
                LightMapTexture &lmtex = lightmaptexs[k.lmid];
                int type = lmtex.type&LM_TYPE;
                if(k.layer==LAYER_BLEND) type += 2;
                else if(k.alpha) type = 4 + 2*(k.alpha-1);
                lastlmid[type] = lmtex.unlitx>=0 ? k.lmid : LMID_AMBIENT;
                if(firstlmid[type]==LMID_AMBIENT && lastlmid[type]!=LMID_AMBIENT)
                {
                    firstlit[type] = i;
                    firstlmid[type] = lastlmid[type];
                }
            }
            else if(k.lmid==LMID_AMBIENT)
            {
                Shader *s = lookupvslot(k.tex, false).slot->shader;
                int type = s->type&SHADER_NORMALSLMS ? LM_BUMPMAP0 : LM_DIFFUSE;
                if(k.layer==LAYER_BLEND) type += 2;
                else if(k.alpha) type = 4 + 2*(k.alpha-1);
                if(lastlmid[type]!=LMID_AMBIENT)
                {
                    sortval &t = indices[k];
                    if(t.unlit<=0) t.unlit = lastlmid[type];
                }
            }
        }
        loopj(2)
        {
            int offset = 2*j;
            if(firstlmid[offset]==LMID_AMBIENT && firstlmid[offset+1]==LMID_AMBIENT) continue;
            loopi(max(firstlit[offset], firstlit[offset+1]))
            {
                sortkey &k = texs[i];
                if((j ? k.layer!=LAYER_BLEND : k.layer==LAYER_BLEND) || k.alpha) continue;
                if(k.lmid!=LMID_AMBIENT) continue;
                Shader *s = lookupvslot(k.tex, false).slot->shader;
                int type = offset + (s->type&SHADER_NORMALSLMS ? LM_BUMPMAP0 : LM_DIFFUSE);
                if(firstlmid[type]==LMID_AMBIENT) continue;
                indices[k].unlit = firstlmid[type];
            }
        }  
        loopj(2)
        {
            int offset = 4 + 2*j;
            if(firstlmid[offset]==LMID_AMBIENT && firstlmid[offset+1]==LMID_AMBIENT) continue;
            loopi(max(firstlit[offset], firstlit[offset+1]))
            {
                sortkey &k = texs[i];
                if(k.alpha != j+1) continue;
                if(k.lmid!=LMID_AMBIENT) continue;
                Shader *s = lookupvslot(k.tex, false).slot->shader;
                int type = offset + (s->type&SHADER_NORMALSLMS ? LM_BUMPMAP0 : LM_DIFFUSE);
                if(firstlmid[type]==LMID_AMBIENT) continue;
                indices[k].unlit = firstlmid[type];
            }
        } 
        loopv(remap)
        {
            sortkey &k = remap[i];
            sortval &t = indices[k];
            if(t.unlit<=0) continue; 
            LightMapTexture &lm = lightmaptexs[t.unlit];
            short u = short(ceil((lm.unlitx + 0.5f) * SHRT_MAX/lm.w)), 
                  v = short(ceil((lm.unlity + 0.5f) * SHRT_MAX/lm.h));
            loopl(2) loopvj(t.tris[l])
            {
                vertex &vtx = verts[t.tris[l][j]];
                if(!vtx.lmu && !vtx.lmv)
                {
                    vtx.lmu = u;
                    vtx.lmv = v;
                }
                else if(vtx.lmu != u || vtx.lmv != v) 
                {
                    vertex vtx2 = vtx;
                    vtx2.lmu = u;
                    vtx2.lmv = v;
                    t.tris[l][j] = addvert(vtx2);
                }
            }
            sortval *dst = indices.access(sortkey(k.tex, t.unlit, k.dim, k.layer, k.envmap, k.alpha));
            if(dst) loopl(2) loopvj(t.tris[l]) dst->tris[l].add(t.tris[l][j]);
        }
    }
                    
    void optimize()
    {
        vector<sortkey> remap;
        enumeratekt(indices, sortkey, k, sortval, t,
            loopl(2) if(t.tris[l].length() && t.unlit<=0)
            {
                if(k.lmid>=LMID_RESERVED && lightmaptexs[k.lmid].unlitx>=0)
                {
                    sortkey ukey(k.tex, LMID_AMBIENT, k.dim, k.layer, k.envmap, k.alpha);
                    sortval *uval = indices.access(ukey);
                    if(uval && uval->unlit<=0)
                    {
                        if(uval->unlit<0) texs.removeobj(ukey);
                        else remap.add(ukey);
                        uval->unlit = k.lmid;
                    }
                }
                else if(k.lmid==LMID_AMBIENT)
                {
                    remap.add(k);
                    t.unlit = -1;
                }
                texs.add(k);
                break;
            }
        );
        texs.sort(texsort);

        remapunlit(remap);

        matsurfs.shrink(optimizematsurfs(matsurfs.getbuf(), matsurfs.length()));
    }

    static inline bool texsort(const sortkey &x, const sortkey &y)
    {
        if(x.alpha < y.alpha) return true;
        if(x.alpha > y.alpha) return false;
        if(x.layer < y.layer) return true;
        if(x.layer > y.layer) return false;
        if(x.tex == y.tex) 
        {
            if(x.lmid < y.lmid) return true;
            if(x.lmid > y.lmid) return false;
            if(x.envmap < y.envmap) return true;
            if(x.envmap > y.envmap) return false;
            if(x.dim < y.dim) return true;
            if(x.dim > y.dim) return false;
            return false;
        }
        if(renderpath!=R_FIXEDFUNCTION)
        {
            VSlot &xs = lookupvslot(x.tex, false), &ys = lookupvslot(y.tex, false);
            if(xs.slot->shader < ys.slot->shader) return true;
            if(xs.slot->shader > ys.slot->shader) return false;
            if(xs.slot->params.length() < ys.slot->params.length()) return true;
            if(xs.slot->params.length() > ys.slot->params.length()) return false;
        }
        if(x.tex < y.tex) return true;
        else return false;
    }

#define GENVERTS(type, ptr, body) do \
    { \
        type *f = (type *)ptr; \
        loopv(verts) \
        { \
            const vertex &v = verts[i]; \
            body; \
            f++; \
        } \
    } while(0)
#define GENVERTSPOSNORMUV(type, ptr, body) GENVERTS(type, ptr, { f->pos = v.pos; f->norm = v.norm; f->norm.flip(); f->reserved = 0; f->u = v.u; f->v = v.v; body; })

    void genverts(void *buf)
    {
        if(renderpath==R_FIXEDFUNCTION)
            GENVERTSPOSNORMUV(vertexff, buf, { f->lmu = v.lmu/float(SHRT_MAX); f->lmv = v.lmv/float(SHRT_MAX); });
        else 
            GENVERTS(vertex, buf, { *f = v; f->norm.flip(); });
    }

    void setupdata(vtxarray *va)
    {
        va->verts = verts.length();
        va->tris = worldtris/3;
        va->vbuf = 0;
        va->vdata = 0;
        va->minvert = 0;
        va->maxvert = va->verts-1;
        va->voffset = 0;
        if(va->verts)
        {
            if(vbosize[VBO_VBUF] + verts.length() > maxvbosize || 
               vbosize[VBO_EBUF] + worldtris > USHRT_MAX ||
               vbosize[VBO_SKYBUF] + skytris > USHRT_MAX) 
                flushvbo();

            va->voffset = vbosize[VBO_VBUF];
            uchar *vdata = addvbo(va, VBO_VBUF, va->verts, VTXSIZE);
            genverts(vdata);
            va->minvert += va->voffset;
            va->maxvert += va->voffset;
        }

        va->matbuf = NULL;
        va->matsurfs = matsurfs.length();
        if(va->matsurfs) 
        {
            va->matbuf = new materialsurface[matsurfs.length()];
            memcpy(va->matbuf, matsurfs.getbuf(), matsurfs.length()*sizeof(materialsurface));
        }

        va->skybuf = 0;
        va->skydata = 0;
        va->sky = skyindices.length();
        va->explicitsky = explicitskyindices.length();
        if(va->sky + va->explicitsky)
        {
            va->skydata += vbosize[VBO_SKYBUF];
            ushort *skydata = (ushort *)addvbo(va, VBO_SKYBUF, va->sky+va->explicitsky, sizeof(ushort));
            memcpy(skydata, skyindices.getbuf(), va->sky*sizeof(ushort));
            memcpy(skydata+va->sky, explicitskyindices.getbuf(), va->explicitsky*sizeof(ushort));
            if(va->voffset) loopi(va->sky+va->explicitsky) skydata[i] += va->voffset; 
        }

        va->eslist = NULL;
        va->texs = texs.length();
        va->blendtris = 0;
        va->blends = 0;
        va->alphabacktris = 0;
        va->alphaback = 0;
        va->alphafronttris = 0;
        va->alphafront = 0;
        va->ebuf = 0;
        va->edata = 0;
        if(va->texs)
        {
            va->eslist = new elementset[va->texs];
            va->edata += vbosize[VBO_EBUF];
            ushort *edata = (ushort *)addvbo(va, VBO_EBUF, worldtris, sizeof(ushort)), *curbuf = edata;
            loopv(texs)
            {
                const sortkey &k = texs[i];
                const sortval &t = indices[k];
                elementset &e = va->eslist[i];
                e.texture = k.tex;
                e.lmid = t.unlit>0 ? t.unlit : k.lmid;
                e.dim = k.dim;
                e.layer = k.layer;
                e.envmap = k.envmap;
                ushort *startbuf = curbuf;
                loopl(2) 
                {
                    e.minvert[l] = USHRT_MAX;
                    e.maxvert[l] = 0;

                    if(t.tris[l].length())
                    {
                        memcpy(curbuf, t.tris[l].getbuf(), t.tris[l].length() * sizeof(ushort));

                        loopvj(t.tris[l])
                        {
                            curbuf[j] += va->voffset;
                            e.minvert[l] = min(e.minvert[l], curbuf[j]);
                            e.maxvert[l] = max(e.maxvert[l], curbuf[j]);
                        }

                        curbuf += t.tris[l].length();
                    }
                    e.length[l] = curbuf-startbuf;
                }
                if(k.layer==LAYER_BLEND) { va->texs--; va->tris -= e.length[1]/3; va->blends++; va->blendtris += e.length[1]/3; }
                else if(k.alpha==ALPHA_BACK) { va->texs--; va->tris -= e.length[1]/3; va->alphaback++; va->alphabacktris += e.length[1]/3; }
                else if(k.alpha==ALPHA_FRONT) { va->texs--; va->tris -= e.length[1]/3; va->alphafront++; va->alphafronttris += e.length[1]/3; } 
            }
        }

        va->texmask = 0;
        loopi(va->texs+va->blends+va->alphaback+va->alphafront)
        {
            Slot &slot = *lookupvslot(va->eslist[i].texture, false).slot;
            loopvj(slot.sts) va->texmask |= 1<<slot.sts[j].type;
            if(slot.shader->type&SHADER_ENVMAP && (renderpath!=R_FIXEDFUNCTION || (slot.ffenv && hasCM && maxtmus >= 2))) va->texmask |= 1<<TEX_ENVMAP;
        }

        if(grasstris.length())
        {
            va->grasstris.move(grasstris);
            useshaderbyname("grass");
        }

        if(mapmodels.length()) va->mapmodels.put(mapmodels.getbuf(), mapmodels.length());
    }

    bool emptyva()
    {
        return verts.empty() && matsurfs.empty() && skyindices.empty() && explicitskyindices.empty() && grasstris.empty() && mapmodels.empty();
    }            
} vc;

int recalcprogress = 0;
#define progress(s)     if((recalcprogress++&0xFFF)==0) renderprogress(recalcprogress/(float)allocnodes, s);

vector<tjoint> tjoints;

vec shadowmapmin, shadowmapmax;

int calcshadowmask(vec *pos, int numpos)
{
    extern vec shadowdir;
    int mask = 0, used = 1;
    vec pe = vec(pos[1]).sub(pos[0]);
    loopk(numpos-2)
    {
        vec e = vec(pos[k+2]).sub(pos[0]);
        if(vec().cross(pe, e).dot(shadowdir)>0)
        {
            mask |= 1<<k;
            used |= 6<<k;
        }
        pe = e;
    }
    if(!mask) return 0;
    loopk(numpos) if(used&(1<<k))
    {
        const vec &v = pos[k];
        shadowmapmin.min(v);
        shadowmapmax.max(v);
    }
    return mask;
}

VARFP(filltjoints, 0, 1, 1, allchanged());

void reduceslope(ivec &n)
{
    int mindim = -1, minval = 64;
    loopi(3) if(n[i])
    {
        int val = abs(n[i]);
        if(mindim < 0 || val < minval)
        {
            mindim = i;
            minval = val;
        }
    }
    if(!(n[R[mindim]]%minval) && !(n[C[mindim]]%minval)) n.div(minval);
    while(!((n.x|n.y|n.z)&1)) n.shr(1);
}

// [rotation][dimension]
vec orientation_tangent [6][3] =
{
    { vec(0,  1,  0), vec( 1, 0,  0), vec( 1,  0, 0) },
    { vec(0,  0, -1), vec( 0, 0, -1), vec( 0,  1, 0) },
    { vec(0, -1,  0), vec(-1, 0,  0), vec(-1,  0, 0) },
    { vec(0,  0,  1), vec( 0, 0,  1), vec( 0, -1, 0) },
    { vec(0, -1,  0), vec(-1, 0,  0), vec(-1,  0, 0) },
    { vec(0,  1,  0), vec( 1, 0,  0), vec( 1,  0, 0) },
};
vec orientation_binormal[6][3] =
{
    { vec(0,  0, -1), vec( 0, 0, -1), vec( 0,  1, 0) },
    { vec(0, -1,  0), vec(-1, 0,  0), vec(-1,  0, 0) },
    { vec(0,  0,  1), vec( 0, 0,  1), vec( 0, -1, 0) },
    { vec(0,  1,  0), vec( 1, 0,  0), vec( 1,  0, 0) },
    { vec(0,  0, -1), vec( 0, 0, -1), vec( 0,  1, 0) },
    { vec(0,  0,  1), vec( 0, 0,  1), vec( 0, -1, 0) },
};

void addtris(const sortkey &key, int orient, vertex *verts, int *index, int numverts, int convex, int shadowmask, int tj)
{
    int &total = key.tex==DEFAULT_SKY ? vc.skytris : vc.worldtris;
    int edge = orient*(MAXFACEVERTS+1);
    loopi(numverts-2) if(index[0]!=index[i+1] && index[i+1]!=index[i+2] && index[i+2]!=index[0])
    {
        vector<ushort> &idxs = key.tex==DEFAULT_SKY ? vc.explicitskyindices : vc.indices[key].tris[(shadowmask>>i)&1];
        int left = index[0], mid = index[i+1], right = index[i+2], start = left, i0 = left, i1 = -1;
        loopk(4)
        {
            int i2 = -1, ctj = -1, cedge = -1;
            switch(k)
            {
            case 1: i1 = i2 = mid; cedge = edge+i+1; break;
            case 2: if(i1 != mid || i0 == left) { i0 = i1; i1 = right; } i2 = right; if(i+1 == numverts-2) cedge = edge+i+2; break;
            case 3: if(i0 == start) { i0 = i1; i1 = left; } i2 = left; // fall-through 
            default: if(!i) cedge = edge; break;
            }
            if(i1 != i2)
            {
                if(total + 3 > USHRT_MAX) return;
                total += 3;
                idxs.add(i0);
                idxs.add(i1);
                idxs.add(i2);
                i1 = i2;
            }
            if(cedge >= 0)
            {
                for(ctj = tj;;)
                {
                    if(ctj < 0) break;
                    if(tjoints[ctj].edge < cedge) { ctj = tjoints[ctj].next; continue; }
                    if(tjoints[ctj].edge != cedge) ctj = -1;
                    break;
                }
            }
            if(ctj >= 0)
            {
                int e1 = cedge%(MAXFACEVERTS+1), e2 = (e1+1)%numverts;
                vertex &v1 = verts[e1], &v2 = verts[e2];
                ivec d(vec(v2.pos).sub(v1.pos).mul(8));
                int axis = abs(d.x) > abs(d.y) ? (abs(d.x) > abs(d.z) ? 0 : 2) : (abs(d.y) > abs(d.z) ? 1 : 2);
                if(d[axis] < 0) d.neg();
                reduceslope(d);
                int origin = int(min(v1.pos[axis], v2.pos[axis])*8)&~0x7FFF,
                    offset1 = (int(v1.pos[axis]*8) - origin) / d[axis],
                    offset2 = (int(v2.pos[axis]*8) - origin) / d[axis];
                vec o = vec(v1.pos).sub(d.tovec().mul(offset1/8.0f));
                float doffset = 1.0f / (offset2 - offset1);
    
                if(i1 < 0) for(;;)
                {
                    tjoint &t = tjoints[ctj];
                    if(t.next < 0 || tjoints[t.next].edge != cedge) break;
                    ctj = t.next;
                } 
                while(ctj >= 0)
                {
                    tjoint &t = tjoints[ctj];
                    if(t.edge != cedge) break;
                    float offset = (t.offset - offset1) * doffset;
                    vertex vt;
                    vt.pos = d.tovec().mul(t.offset/8.0f).add(o);
                    vt.reserved = 0;
                    vt.u = v1.u + (v2.u-v1.u)*offset;
                    vt.v = v1.v + (v2.v-v1.v)*offset;
                    vt.lmu = short(v1.lmu + (v2.lmu-v1.lmu)*offset),
                    vt.lmv = short(v1.lmv + (v2.lmv-v1.lmv)*offset);
                    vt.norm.lerp(v1.norm, v2.norm, offset);
                    vt.tangent.lerp(v1.tangent, v2.tangent, offset);
                    vt.bitangent = v1.bitangent;
                    int i2 = vc.addvert(vt);
                    if(i2 < 0) return;
                    if(i1 >= 0)
                    {
                        if(total + 3 > USHRT_MAX) return;
                        total += 3;
                        idxs.add(i0);
                        idxs.add(i1);
                        idxs.add(i2);
                        i1 = i2;
                    }
                    else start = i0 = i2;
                    ctj = t.next;
                }
            }
        }
    }
}

void addgrasstri(int face, vertex *verts, int numv, ushort texture, ushort lmid)
{
    grasstri &g = vc.grasstris.add();
    int i1, i2, i3, i4;
    if(numv <= 3 && face%2) { i1 = face+1; i2 = face+2; i3 = i4 = 0; }
    else { i1 = 0; i2 = face+1; i3 = face+2; i4 = numv > 3 ? face+3 : i3; } 
    g.v[0] = verts[i1].pos;
    g.v[1] = verts[i2].pos;
    g.v[2] = verts[i3].pos;
    g.v[3] = verts[i4].pos;
    g.numv = numv;

    g.surface.toplane(g.v[0], g.v[1], g.v[2]);
    if(g.surface.z <= 0) { vc.grasstris.pop(); return; }

    g.minz = min(min(g.v[0].z, g.v[1].z), min(g.v[2].z, g.v[3].z));
    g.maxz = max(max(g.v[0].z, g.v[1].z), max(g.v[2].z, g.v[3].z));

    g.center = vec(0, 0, 0);
    loopk(numv) g.center.add(g.v[k]);
    g.center.div(numv);
    g.radius = 0;
    loopk(numv) g.radius = max(g.radius, g.v[k].dist(g.center));

    vec area, bx, by;
    area.cross(vec(g.v[1]).sub(g.v[0]), vec(g.v[2]).sub(g.v[0]));
    float scale;
    int px, py;

    if(fabs(area.x) >= fabs(area.y) && fabs(area.x) >= fabs(area.z))
        scale = 1/area.x, px = 1, py = 2;
    else if(fabs(area.y) >= fabs(area.x) && fabs(area.y) >= fabs(area.z))
        scale = -1/area.y, px = 0, py = 2;
    else
        scale = 1/area.z, px = 0, py = 1;

    bx.x = (g.v[2][py] - g.v[0][py])*scale;
    bx.y = (g.v[2][px] - g.v[0][px])*scale;
    bx.z = bx.x*g.v[2][px] - bx.y*g.v[2][py];

    by.x = (g.v[2][py] - g.v[1][py])*scale;
    by.y = (g.v[2][px] - g.v[1][px])*scale;
    by.z = by.x*g.v[1][px] - by.y*g.v[1][py] - 1;
    by.sub(bx);

    float tc1u = verts[i1].lmu/float(SHRT_MAX),
          tc1v = verts[i1].lmv/float(SHRT_MAX),
          tc2u = (verts[i2].lmu - verts[i1].lmu)/float(SHRT_MAX),
          tc2v = (verts[i2].lmv - verts[i1].lmv)/float(SHRT_MAX),
          tc3u = (verts[i3].lmu - verts[i1].lmu)/float(SHRT_MAX),
          tc3v = (verts[i3].lmv - verts[i1].lmv)/float(SHRT_MAX);
        
    g.tcu = vec4(0, 0, 0, tc1u - (bx.z*tc2u + by.z*tc3u));
    g.tcu[px] = bx.x*tc2u + by.x*tc3u;
    g.tcu[py] = -(bx.y*tc2u + by.y*tc3u);

    g.tcv = vec4(0, 0, 0, tc1v - (bx.z*tc2v + by.z*tc3v));
    g.tcv[px] = bx.x*tc2v + by.x*tc3v;
    g.tcv[py] = -(bx.y*tc2v + by.y*tc3v);

    g.texture = texture;
    g.lmid = lmid;
}

static inline void calctexgen(VSlot &vslot, int dim, vec4 &sgen, vec4 &tgen)
{
    Texture *tex = vslot.slot->sts.empty() ? notexture : vslot.slot->sts[0].t;
    float k = TEX_SCALE/vslot.scale,
          xs = vslot.rotation>=2 && vslot.rotation<=4 ? -tex->xs : tex->xs,
          ys = (vslot.rotation>=1 && vslot.rotation<=2) || vslot.rotation==5 ? -tex->ys : tex->ys,
          sk = k/xs, tk = k/ys,
          soff = -((vslot.rotation&5)==1 ? vslot.yoffset : vslot.xoffset)/xs,
          toff = -((vslot.rotation&5)==1 ? vslot.xoffset : vslot.yoffset)/ys;
    static const int si[] = { 1, 0, 0 }, ti[] = { 2, 2, 1 };
    int sdim = si[dim], tdim = ti[dim];
    sgen = vec4(0, 0, 0, soff); 
    tgen = vec4(0, 0, 0, toff);
    if((vslot.rotation&5)==1)
    {
        sgen[tdim] = (dim <= 1 ? -sk : sk);
        tgen[sdim] = tk;
    }
    else
    {
        sgen[sdim] = sk;
        tgen[tdim] = (dim <= 1 ? -tk : tk);
    }
}

ushort encodenormal(const vec &n)
{               
    if(n.iszero()) return 0;
    int yaw = int(-atan2(n.x, n.y)/RAD), pitch = int(asin(n.z)/RAD);
    return ushort(clamp(pitch + 90, 0, 180)*360 + (yaw < 0 ? yaw%360 + 360 : yaw%360) + 1);
}

vec decodenormal(ushort norm)
{
    if(!norm) return vec(0, 0, 0);
    norm--;
    const vec2 &yaw = sincos360[norm%360], &pitch = sincos360[norm/360+270];
    return vec(-yaw.y*pitch.x, yaw.x*pitch.x, pitch.y);
}

void addcubeverts(VSlot &vslot, int orient, int size, vec *pos, int convex, ushort texture, ushort lmid, vertinfo *vinfo, int numverts, int tj = -1, ushort envmap = EMID_NONE, int grassy = 0, bool alpha = false, int layer = LAYER_TOP)
{
    int dim = dimension(orient);
    int shadowmask = texture==DEFAULT_SKY || alpha ? 0 : calcshadowmask(pos, numverts);

    LightMap *lm = NULL;
    LightMapTexture *lmtex = NULL;
    if(!nolights && lightmaps.inrange(lmid-LMID_RESERVED))
    {
        lm = &lightmaps[lmid-LMID_RESERVED];
        if((lm->type&LM_TYPE)==LM_DIFFUSE ||
            ((lm->type&LM_TYPE)==LM_BUMPMAP0 &&
                lightmaps.inrange(lmid+1-LMID_RESERVED) &&
                (lightmaps[lmid+1-LMID_RESERVED].type&LM_TYPE)==LM_BUMPMAP1))
            lmtex = &lightmaptexs[lm->tex];
        else lm = NULL;
    }

    vec4 sgen, tgen;
    calctexgen(vslot, dim, sgen, tgen);
    vertex verts[MAXFACEVERTS];
    int index[MAXFACEVERTS];
    loopk(numverts)
    {
        vertex &v = verts[k];
        v.pos = pos[k];
        v.reserved = 0;
        v.u = sgen.dot(v.pos);
        v.v = tgen.dot(v.pos);
        if(lmtex) 
        { 
            v.lmu = short(ceil((lm->offsetx + vinfo[k].u*(float(LM_PACKW)/float(USHRT_MAX+1)) + 0.5f) * float(SHRT_MAX)/lmtex->w)); 
            v.lmv = short(ceil((lm->offsety + vinfo[k].v*(float(LM_PACKH)/float(USHRT_MAX+1)) + 0.5f) * float(SHRT_MAX)/lmtex->h));
        }
        else v.lmu = v.lmv = 0;
        if(renderpath!=R_FIXEDFUNCTION && vinfo && vinfo[k].norm)
        {
            vec n = decodenormal(vinfo[k].norm), t = orientation_tangent[vslot.rotation][dim];
            t.sub(vec(n).mul(n.dot(t))).normalize();
            v.norm = bvec(n);
            v.tangent = bvec(t);
            v.bitangent = vec().cross(n, t).dot(orientation_binormal[vslot.rotation][dim]) < 0 ? 0 : 255;
        }
        else
        {
            v.norm = vinfo && vinfo[k].norm && envmap != EMID_NONE ? bvec(decodenormal(vinfo[k].norm)) : bvec(128, 128, 128);
            v.tangent = bvec(128, 128, 128);
            v.bitangent = 128;
        }
        index[k] = vc.addvert(v);
        if(index[k] < 0) return;
    }

    if(texture == DEFAULT_SKY)
    {
        loopk(numverts) vc.skyclip = min(vc.skyclip, int(pos[k].z*8)>>3);
        vc.skyfaces |= 0x3F&~(1<<orient);
    }

    if(lmid >= LMID_RESERVED) lmid = lm ? lm->tex : LMID_AMBIENT;

    sortkey key(texture, lmid, vslot.scrollS || vslot.scrollT ? dim : 3, layer == LAYER_BLEND ? LAYER_BLEND : LAYER_TOP, envmap, alpha ? (vslot.alphaback ? ALPHA_BACK : (vslot.alphafront ? ALPHA_FRONT : NO_ALPHA)) : NO_ALPHA);
    addtris(key, orient, verts, index, numverts, convex, shadowmask, tj);

    if(grassy) 
    {
        for(int i = 0; i < numverts-2; i += 2)
        {
            int faces = 0;
            if(index[0]!=index[i+1] && index[i+1]!=index[i+2] && index[i+2]!=index[0]) faces |= 1;
            if(i+3 < numverts && index[0]!=index[i+2] && index[i+2]!=index[i+3] && index[i+3]!=index[0]) faces |= 2;
            if(grassy > 1 && faces==3) addgrasstri(i, verts, 4, texture, lmid);
            else 
            {
                if(faces&1) addgrasstri(i, verts, 3, texture, lmid);
                if(faces&2) addgrasstri(i+1, verts, 3, texture, lmid);
            }
        }
    }
}

struct edgegroup
{
    ivec slope, origin;
    int axis;
};

static uint hthash(const edgegroup &g)
{
    return g.slope.x^g.slope.y^g.slope.z^g.origin.x^g.origin.y^g.origin.z;
}

static bool htcmp(const edgegroup &x, const edgegroup &y) 
{ 
    return x.slope==y.slope && x.origin==y.origin;
}

enum
{
    CE_START = 1<<0,
    CE_END   = 1<<1,
    CE_FLIP  = 1<<2,
    CE_DUP   = 1<<3
};

struct cubeedge
{
    cube *c;
    int next, offset;
    ushort size;
    uchar index, flags;
};

vector<cubeedge> cubeedges;
hashtable<edgegroup, int> edgegroups(1<<13);

void gencubeedges(cube &c, int x, int y, int z, int size)
{
    ivec pos[MAXFACEVERTS];
    int vis;
    loopi(6) if((vis = visibletris(c, i, x, y, z, size)))
    {
        int numverts = c.ext ? c.ext->surfaces[i].numverts&MAXFACEVERTS : 0;
        if(numverts)
        {
            vertinfo *verts = c.ext->verts() + c.ext->surfaces[i].verts;
            ivec vo = ivec(x, y, z).mask(~0xFFF).shl(3);
            loopj(numverts)
            {
                vertinfo &v = verts[j];
                pos[j] = ivec(v.x, v.y, v.z).add(vo);
            }
        }
        else if(c.merged&(1<<i)) continue;
        else
        {
            ivec v[4];
            genfaceverts(c, i, v);
            int order = vis&4 || (!flataxisface(c, i) && faceconvexity(v) < 0) ? 1 : 0;
            ivec vo = ivec(x, y, z).shl(3);
            pos[numverts++] = v[order].mul(size).add(vo);
            if(vis&1) pos[numverts++] = v[order+1].mul(size).add(vo);
            pos[numverts++] = v[order+2].mul(size).add(vo);
            if(vis&2) pos[numverts++] = v[(order+3)&3].mul(size).add(vo);
        }
        loopj(numverts)
        {
            int e1 = j, e2 = j+1 < numverts ? j+1 : 0;
            ivec d = pos[e2];
            d.sub(pos[e1]);
            if(d.iszero()) continue;
            int axis = abs(d.x) > abs(d.y) ? (abs(d.x) > abs(d.z) ? 0 : 2) : (abs(d.y) > abs(d.z) ? 1 : 2);
            if(d[axis] < 0)
            {
                d.neg();
                swap(e1, e2);
            }
            reduceslope(d);

            int t1 = pos[e1][axis]/d[axis],
                t2 = pos[e2][axis]/d[axis];
            edgegroup g;
            g.origin = ivec(pos[e1]).sub(ivec(d).mul(t1));
            g.slope = d;
            g.axis = axis;
            cubeedge ce;
            ce.c = &c;
            ce.offset = t1;
            ce.size = t2 - t1;
            ce.index = i*(MAXFACEVERTS+1)+j;
            ce.flags = CE_START | CE_END | (e1!=j ? CE_FLIP : 0);
            ce.next = -1;

            bool insert = true;
            int *exists = edgegroups.access(g);
            if(exists)
            {
                int prev = -1, cur = *exists;
                while(cur >= 0)
                {
                    cubeedge &p = cubeedges[cur];
                    if(p.flags&CE_DUP ? 
                        ce.offset>=p.offset && ce.offset+ce.size<=p.offset+p.size : 
                        ce.offset==p.offset && ce.size==p.size)
                    {
                        p.flags |= CE_DUP;
                        insert = false;
                        break;
                    }
                    else if(ce.offset >= p.offset)
                    {
                        if(ce.offset == p.offset+p.size) ce.flags &= ~CE_START;
                        prev = cur;
                        cur = p.next;
                    }
                    else break;
                }
                if(insert)
                {
                    ce.next = cur;
                    while(cur >= 0)
                    {
                        cubeedge &p = cubeedges[cur];
                        if(ce.offset+ce.size==p.offset) { ce.flags &= ~CE_END; break; }
                        cur = p.next;
                    }
                    if(prev>=0) cubeedges[prev].next = cubeedges.length();
                    else *exists = cubeedges.length();
                }
            }
            else edgegroups[g] = cubeedges.length();

            if(insert) cubeedges.add(ce);
        }
    }
}

void gencubeedges(cube *c = worldroot, int x = 0, int y = 0, int z = 0, int size = worldsize>>1)
{
    progress("fixing t-joints...");
    neighbourstack[++neighbourdepth] = c;
    loopi(8)
    {
        ivec o(i, x, y, z, size);
        if(c[i].ext) c[i].ext->tjoints = -1;
        if(c[i].children) gencubeedges(c[i].children, o.x, o.y, o.z, size>>1);
        else if(!isempty(c[i])) gencubeedges(c[i], o.x, o.y, o.z, size);
    }
    --neighbourdepth;
}

void gencubeverts(cube &c, int x, int y, int z, int size, int csi)
{
    c.visible = 0;
    c.collide = 0;
    int tj = filltjoints && c.ext ? c.ext->tjoints : -1, vis;
    loopi(6) if((vis = visibletris(c, i, x, y, z, size)))
    {
        // this is necessary for physics to work, even if the face is merged
        if(collideface(c, i)) c.collide |= 1<<i;

        if(c.merged&(1<<i)) continue;

        c.visible |= 1<<i;

        vec pos[MAXFACEVERTS];
        vertinfo *verts = NULL;
        int numverts = c.ext ? c.ext->surfaces[i].numverts&MAXFACEVERTS : 0, convex = 0;
        if(numverts)
        {
            verts = c.ext->verts() + c.ext->surfaces[i].verts;
            vec vo = ivec(x, y, z).mask(~0xFFF).tovec();
            loopj(numverts) pos[j] = verts[j].getxyz().tovec().mul(1.0f/8).add(vo);
            if(!(c.merged&(1<<i)) && !flataxisface(c, i)) convex = faceconvexity(verts, numverts);
        }
        else
        {
            ivec v[4];
            genfaceverts(c, i, v);
            if(!flataxisface(c, i)) convex = faceconvexity(v);
            int order = vis&4 || convex < 0 ? 1 : 0;
            vec vo(x, y, z);
            pos[numverts++] = v[order].tovec().mul(size/8.0f).add(vo);
            if(vis&1) pos[numverts++] = v[order+1].tovec().mul(size/8.0f).add(vo);
            pos[numverts++] = v[order+2].tovec().mul(size/8.0f).add(vo);
            if(vis&2) pos[numverts++] = v[(order+3)&3].tovec().mul(size/8.0f).add(vo);
        }

        VSlot &vslot = lookupvslot(c.texture[i], true),
              *layer = vslot.layer && !(c.material&MAT_ALPHA) ? &lookupvslot(vslot.layer, true) : NULL;
        ushort envmap = vslot.slot->shader->type&SHADER_ENVMAP ? (vslot.slot->texmask&(1<<TEX_ENVMAP) ? EMID_CUSTOM : closestenvmap(i, x, y, z, size)) : EMID_NONE,
               envmap2 = layer && layer->slot->shader->type&SHADER_ENVMAP ? (layer->slot->texmask&(1<<TEX_ENVMAP) ? EMID_CUSTOM : closestenvmap(i, x, y, z, size)) : EMID_NONE;
        while(tj >= 0 && tjoints[tj].edge < i*(MAXFACEVERTS+1)) tj = tjoints[tj].next;
        int hastj = tj >= 0 && tjoints[tj].edge < (i+1)*(MAXFACEVERTS+1) ? tj : -1;
        int grassy = vslot.slot->autograss && i!=O_BOTTOM ? (vis!=3 || convex ? 1 : 2) : 0;
        if(!c.ext)
            addcubeverts(vslot, i, size, pos, convex, c.texture[i], LMID_AMBIENT, NULL, numverts, hastj, envmap, grassy, (c.material&MAT_ALPHA)!=0);
        else
        { 
            const surfaceinfo &surf = c.ext->surfaces[i];
            if(!surf.numverts || surf.numverts&LAYER_TOP)
                addcubeverts(vslot, i, size, pos, convex, c.texture[i], surf.lmid[0], verts, numverts, hastj, envmap, grassy, (c.material&MAT_ALPHA)!=0, LAYER_TOP|(surf.numverts&LAYER_BLEND));
            if(surf.numverts&LAYER_BOTTOM)
                addcubeverts(layer ? *layer : vslot, i, size, pos, convex, vslot.layer, surf.lmid[1], surf.numverts&LAYER_DUP ? verts + numverts : verts, numverts, hastj, envmap2);
        }
    }
    else
    {
        if(visibleface(c, i, x, y, z, size, MAT_AIR, MAT_NOCLIP, MATF_CLIP) && collideface(c, i)) c.collide |= 1<<i;
    }
}

bool skyoccluded(cube &c, int orient)
{
    if(isempty(c)) return false;
//    if(c.texture[orient] == DEFAULT_SKY) return true;
    if(touchingface(c, orient) && faceedges(c, orient) == F_SOLID && !(c.material&MAT_ALPHA)) return true;
    return false;
}

int hasskyfaces(cube &c, int x, int y, int z, int size, int faces[6])
{
    int numfaces = 0;
    if(x == 0 && !skyoccluded(c, O_LEFT)) faces[numfaces++] = O_LEFT;
    if(x + size == worldsize && !skyoccluded(c, O_RIGHT)) faces[numfaces++] = O_RIGHT;
    if(y == 0 && !skyoccluded(c, O_BACK)) faces[numfaces++] = O_BACK;
    if(y + size == worldsize && !skyoccluded(c, O_FRONT)) faces[numfaces++] = O_FRONT;
    if(z == 0 && !skyoccluded(c, O_BOTTOM)) faces[numfaces++] = O_BOTTOM;
    if(z + size == worldsize && !skyoccluded(c, O_TOP)) faces[numfaces++] = O_TOP;
    return numfaces;
}

vector<facebounds> skyfaces[6];
 
void minskyface(cube &cu, int orient, const ivec &co, int size, facebounds &orig)
{   
    facebounds mincf;
    mincf.u1 = orig.u2;
    mincf.u2 = orig.u1;
    mincf.v1 = orig.v2;
    mincf.v2 = orig.v1;
    mincubeface(cu, orient, co, size, orig, mincf, MAT_ALPHA, MAT_ALPHA);
    orig.u1 = max(mincf.u1, orig.u1);
    orig.u2 = min(mincf.u2, orig.u2);
    orig.v1 = max(mincf.v1, orig.v1);
    orig.v2 = min(mincf.v2, orig.v2);
}  

void genskyfaces(cube &c, const ivec &o, int size)
{
    if(isentirelysolid(c) && !(c.material&MAT_ALPHA)) return;

    int faces[6],
        numfaces = hasskyfaces(c, o.x, o.y, o.z, size, faces);
    if(!numfaces) return;

    loopi(numfaces)
    {
        int orient = faces[i], dim = dimension(orient);
        facebounds m;
        m.u1 = (o[C[dim]]&0xFFF)<<3; 
        m.u2 = m.u1 + (size<<3);
        m.v1 = (o[R[dim]]&0xFFF)<<3;
        m.v2 = m.v1 + (size<<3);
        minskyface(c, orient, o, size, m);
        if(m.u1 >= m.u2 || m.v1 >= m.v2) continue;
        vc.skyarea += (int(m.u2-m.u1)*int(m.v2-m.v1) + (1<<(2*3))-1)>>(2*3);
        skyfaces[orient].add(m);
    }
}

void addskyverts(const ivec &o, int size)
{
    loopi(6)
    {
        int dim = dimension(i), c = C[dim], r = R[dim];
        vector<facebounds> &sf = skyfaces[i]; 
        if(sf.empty()) continue;
        vc.skyfaces |= 0x3F&~(1<<opposite(i));
        sf.setsize(mergefaces(i, sf.getbuf(), sf.length()));
        loopvj(sf)
        {
            facebounds &m = sf[j];
            int index[4];
            loopk(4)
            {
                const ivec &coords = facecoords[opposite(i)][k];
                vec v;
                v[dim] = o[dim];
                if(coords[dim]) v[dim] += size;
                v[c] = (o[c]&~0xFFF) + (coords[c] ? m.u2 : m.u1)/8.0f;
                v[r] = (o[r]&~0xFFF) + (coords[r] ? m.v2 : m.v1)/8.0f;
                index[k] = vc.addvert(v);
                if(index[k] < 0) goto nextskyface;
                vc.skyclip = min(vc.skyclip, int(v.z*8)>>3);
            }
            if(vc.skytris + 6 > USHRT_MAX) break;
            vc.skytris += 6;
            vc.skyindices.add(index[0]);
            vc.skyindices.add(index[1]);
            vc.skyindices.add(index[2]);

            vc.skyindices.add(index[0]);
            vc.skyindices.add(index[2]);
            vc.skyindices.add(index[3]);
        nextskyface:;
        }
        sf.setsize(0);
    }
}
                    
////////// Vertex Arrays //////////////

int allocva = 0;
int wtris = 0, wverts = 0, vtris = 0, vverts = 0, glde = 0, gbatches = 0;
vector<vtxarray *> valist, varoot;

vtxarray *newva(int x, int y, int z, int size)
{
    vc.optimize();

    vtxarray *va = new vtxarray;
    va->parent = NULL;
    va->o = ivec(x, y, z);
    va->size = size;
    va->skyarea = vc.skyarea;
    va->skyfaces = vc.skyfaces;
    va->skyclip = vc.skyclip < INT_MAX ? vc.skyclip : INT_MAX;
    va->curvfc = VFC_NOT_VISIBLE;
    va->occluded = OCCLUDE_NOTHING;
    va->query = NULL;
    va->bbmin = ivec(-1, -1, -1);
    va->bbmax = ivec(-1, -1, -1);
    va->hasmerges = 0;
    va->mergelevel = -1;

    vc.setupdata(va);

    wverts += va->verts;
    wtris  += va->tris + va->blends + va->alphabacktris + va->alphafronttris;
    allocva++;
    valist.add(va);

    return va;
}

void destroyva(vtxarray *va, bool reparent)
{
    wverts -= va->verts;
    wtris -= va->tris + va->blends + va->alphabacktris + va->alphafronttris;
    allocva--;
    valist.removeobj(va);
    if(!va->parent) varoot.removeobj(va);
    if(reparent)
    {
        if(va->parent) va->parent->children.removeobj(va);
        loopv(va->children)
        {
            vtxarray *child = va->children[i];
            child->parent = va->parent;
            if(child->parent) child->parent->children.add(child);
        }
    }
    if(va->vbuf) destroyvbo(va->vbuf);
    if(va->ebuf) destroyvbo(va->ebuf);
    if(va->skybuf) destroyvbo(va->skybuf);
    if(va->eslist) delete[] va->eslist;
    if(va->matbuf) delete[] va->matbuf;
    delete va;
}

void clearvas(cube *c)
{
    loopi(8)
    {
        if(c[i].ext)
        {
            if(c[i].ext->va) destroyva(c[i].ext->va, false);
            c[i].ext->va = NULL;
            c[i].ext->tjoints = -1;
        }
        if(c[i].children) clearvas(c[i].children);
    }
}

void updatevabb(vtxarray *va, bool force)
{
    if(!force && va->bbmin.x >= 0) return;

    va->bbmin = va->geommin;
    va->bbmax = va->geommax;
    va->bbmin.min(va->matmin);
    va->bbmax.max(va->matmax);
    loopv(va->children)
    {
        vtxarray *child = va->children[i];
        updatevabb(child, force);
        va->bbmin.min(child->bbmin);
        va->bbmax.max(child->bbmax);
    }
    loopv(va->mapmodels)
    {
        octaentities *oe = va->mapmodels[i];
        va->bbmin.min(oe->bbmin);
        va->bbmax.max(oe->bbmax);
    }

    if(va->skyfaces)
    {
        va->skyfaces |= 0x80;
        if(va->sky) loop(dim, 3) if(va->skyfaces&(3<<(2*dim)))
        {
            int r = R[dim], c = C[dim];
            if((va->skyfaces&(1<<(2*dim)) && va->o[dim] < va->bbmin[dim]) ||
               (va->skyfaces&(2<<(2*dim)) && va->o[dim]+va->size > va->bbmax[dim]) ||
               va->o[r] < va->bbmin[r] || va->o[r]+va->size > va->bbmax[r] ||
               va->o[c] < va->bbmin[c] || va->o[c]+va->size > va->bbmax[c])
            {
                va->skyfaces &= ~0x80;
                break;
            }
        }
    }
}

void updatevabbs(bool force)
{
    loopv(varoot) updatevabb(varoot[i], force);
}

struct mergedface
{   
    uchar orient, mat, lmid, numverts;
    ushort tex, envmap;
    vertinfo *verts;
    int tjoints;
};  

#define MAXMERGELEVEL 12
static int vahasmerges = 0, vamergemax = 0;
static vector<mergedface> vamerges[MAXMERGELEVEL+1];

int genmergedfaces(cube &c, const ivec &co, int size, int minlevel = -1)
{
    if(!c.ext || isempty(c)) return -1;
    int tj = c.ext->tjoints, maxlevel = -1;
    if(minlevel < 0) c.escaped &= ~c.merged;
    loopi(6) 
    {
        if(!(c.merged&(1<<i))) continue;
        surfaceinfo &surf = c.ext->surfaces[i];
        int numverts = surf.numverts&MAXFACEVERTS;
        if(!numverts) 
        {
            if(minlevel < 0) vahasmerges |= MERGE_PART;
            continue;
        }
        mergedface mf;
        mf.orient = i;
        mf.mat = c.material;
        mf.tex = c.texture[i];
        mf.envmap = EMID_NONE;
        mf.lmid = surf.lmid[0];
        mf.numverts = surf.numverts;
        mf.verts = c.ext->verts() + surf.verts; 
        mf.tjoints = -1;
        int level = calcmergedsize(i, co, size, mf.verts, mf.numverts&MAXFACEVERTS);
        if(minlevel < 0 && 1<<level > size) 
            c.escaped |= 1<<i;
        if(level > minlevel)
        {
            maxlevel = max(maxlevel, level);

            while(tj >= 0 && tjoints[tj].edge < i*(MAXFACEVERTS+1)) tj = tjoints[tj].next;
            if(tj >= 0 && tjoints[tj].edge < (i+1)*(MAXFACEVERTS+1)) mf.tjoints = tj;

            VSlot &vslot = lookupvslot(mf.tex, true),
                  *layer = vslot.layer && !(c.material&MAT_ALPHA) ? &lookupvslot(vslot.layer, true) : NULL;
            if(vslot.slot->shader->type&SHADER_ENVMAP)
                mf.envmap = vslot.slot->texmask&(1<<TEX_ENVMAP) ? EMID_CUSTOM : closestenvmap(i, co.x, co.y, co.z, size);
            ushort envmap2 = layer && layer->slot->shader->type&SHADER_ENVMAP ? (layer->slot->texmask&(1<<TEX_ENVMAP) ? EMID_CUSTOM : closestenvmap(i, co.x, co.y, co.z, size)) : EMID_NONE;

            if(surf.numverts&LAYER_TOP) vamerges[level].add(mf); 
            if(surf.numverts&LAYER_BOTTOM)
            {
                mf.tex = vslot.layer;
                mf.envmap = envmap2;
                mf.lmid = surf.lmid[1];
                mf.numverts &= ~LAYER_TOP;
                if(surf.numverts&LAYER_DUP) mf.verts += numverts;
                vamerges[level].add(mf);
            }
        }
    }
    if(maxlevel >= 0)
    {
        vamergemax = max(vamergemax, maxlevel);
        vahasmerges |= MERGE_ORIGIN;
    }
    return maxlevel;
}

int findmergedfaces(cube &c, const ivec &co, int size, int csi, int minlevel)
{
    if(c.ext && c.ext->va && !(c.ext->va->hasmerges&MERGE_ORIGIN)) return c.ext->va->mergelevel;
    else if(c.children)
    {
        int maxlevel = -1;
        loopi(8)
        {
            ivec o(i, co.x, co.y, co.z, size/2); 
            int level = findmergedfaces(c.children[i], o, size/2, csi-1, minlevel);
            maxlevel = max(maxlevel, level);
        }
        return maxlevel;
    }
    else if(c.ext && c.merged) return genmergedfaces(c, co, size, minlevel);
    else return -1;
}

void addmergedverts(int level, const ivec &o)
{
    vector<mergedface> &mfl = vamerges[level];
    if(mfl.empty()) return;
    vec vo = ivec(o).mask(~0xFFF).tovec();
    vec pos[MAXFACEVERTS];
    loopv(mfl)
    {
        mergedface &mf = mfl[i];
        int numverts = mf.numverts&MAXFACEVERTS;
        loopi(numverts)
        {
            vertinfo &v = mf.verts[i];
            pos[i] = vec(v.x, v.y, v.z).mul(1.0f/8).add(vo);
        }
        VSlot &vslot = lookupvslot(mf.tex, true);
        int grassy = vslot.slot->autograss && mf.orient!=O_BOTTOM && mf.numverts&LAYER_TOP ? 2 : 0;
        addcubeverts(vslot, mf.orient, 1<<level, pos, 0, mf.tex, mf.lmid, mf.verts, numverts, mf.tjoints, mf.envmap, grassy, (mf.mat&MAT_ALPHA)!=0, mf.numverts&LAYER_BLEND);
        vahasmerges |= MERGE_USE;
    }
    mfl.setsize(0);
}

void rendercube(cube &c, int cx, int cy, int cz, int size, int csi, int &maxlevel)  // creates vertices and indices ready to be put into a va
{
    //if(size<=16) return;
    if(c.ext && c.ext->va) 
    {
        maxlevel = max(maxlevel, c.ext->va->mergelevel);
        return;                            // don't re-render
    }

    if(c.children)
    {
        neighbourstack[++neighbourdepth] = c.children;
        c.escaped = 0;
        loopi(8)
        {
            ivec o(i, cx, cy, cz, size/2);
            int level = -1;
            rendercube(c.children[i], o.x, o.y, o.z, size/2, csi-1, level);
            if(level >= csi) 
                c.escaped |= 1<<i;
            maxlevel = max(maxlevel, level);   
        }
        --neighbourdepth;

        if(csi <= MAXMERGELEVEL && vamerges[csi].length()) addmergedverts(csi, ivec(cx, cy, cz));

        if(c.ext)
        {
            if(c.ext->ents && c.ext->ents->mapmodels.length()) vc.mapmodels.add(c.ext->ents);
        }
        return;
    }
    
    genskyfaces(c, ivec(cx, cy, cz), size);

    if(!isempty(c)) 
    {
        gencubeverts(c, cx, cy, cz, size, csi);
        if(c.merged) maxlevel = max(maxlevel, genmergedfaces(c, ivec(cx, cy, cz), size));
    }
    if(c.material != MAT_AIR) genmatsurfs(c, cx, cy, cz, size, vc.matsurfs);

    if(c.ext)
    {
        if(c.ext->ents && c.ext->ents->mapmodels.length()) vc.mapmodels.add(c.ext->ents);
    }

    if(csi <= MAXMERGELEVEL && vamerges[csi].length()) addmergedverts(csi, ivec(cx, cy, cz));
}

void calcgeombb(int cx, int cy, int cz, int size, ivec &bbmin, ivec &bbmax)
{
    vec vmin(cx, cy, cz), vmax = vmin;
    vmin.add(size);

    loopv(vc.verts)
    {
        const vec &v = vc.verts[i].pos;
        vmin.min(v);
        vmax.max(v);
    }

    bbmin = ivec(vmin.mul(8)).shr(3);
    bbmax = ivec(vmax.mul(8)).add(7).shr(3);
}

void calcmatbb(int cx, int cy, int cz, int size, ivec &bbmin, ivec &bbmax)
{
    bbmax = ivec(cx, cy, cz);
    (bbmin = bbmax).add(size);
    loopv(vc.matsurfs)
    {
        materialsurface &m = vc.matsurfs[i];
        switch(m.material)
        {
            case MAT_WATER:
            case MAT_GLASS:
            case MAT_LAVA:
                break;

            default:
                continue;
        }

        int dim = dimension(m.orient),
            r = R[dim],
            c = C[dim];
        bbmin[dim] = min(bbmin[dim], m.o[dim]);
        bbmax[dim] = max(bbmax[dim], m.o[dim]);

        bbmin[r] = min(bbmin[r], m.o[r]);
        bbmax[r] = max(bbmax[r], m.o[r] + m.rsize);

        bbmin[c] = min(bbmin[c], m.o[c]);
        bbmax[c] = max(bbmax[c], m.o[c] + m.csize);
    }
}

void setva(cube &c, int cx, int cy, int cz, int size, int csi)
{
    ASSERT(size <= 0x1000);

    int vamergeoffset[MAXMERGELEVEL+1];
    loopi(MAXMERGELEVEL+1) vamergeoffset[i] = vamerges[i].length();

    vc.origin = ivec(cx, cy, cz);
    vc.size = size;

    shadowmapmin = vec(cx+size, cy+size, cz+size);
    shadowmapmax = vec(cx, cy, cz);

    int maxlevel = -1;
    rendercube(c, cx, cy, cz, size, csi, maxlevel);

    ivec bbmin, bbmax;

    calcgeombb(cx, cy, cz, size, bbmin, bbmax);

    addskyverts(ivec(cx, cy, cz), size);

    if(!vc.emptyva())
    {
        vtxarray *va = newva(cx, cy, cz, size);
        ext(c).va = va;
        va->geommin = bbmin;
        va->geommax = bbmax;
        calcmatbb(cx, cy, cz, size, va->matmin, va->matmax);
        va->shadowmapmin = ivec(shadowmapmin.mul(8)).shr(3);
        va->shadowmapmax = ivec(shadowmapmax.mul(8)).add(7).shr(3);
        va->hasmerges = vahasmerges;
        va->mergelevel = vamergemax;
    }
    else
    {
        loopi(MAXMERGELEVEL+1) vamerges[i].setsize(vamergeoffset[i]);
    }

    vc.clear();
}

VARF(vacubemax, 64, 512, 256*256, allchanged());
VARF(vacubesize, 32, 128, 0x1000, allchanged());
VARF(vacubemin, 0, 128, 256*256, allchanged());

int updateva(cube *c, int cx, int cy, int cz, int size, int csi)
{
    progress("recalculating geometry...");
    static int faces[6];
    int ccount = 0, cmergemax = vamergemax, chasmerges = vahasmerges;
    neighbourstack[++neighbourdepth] = c;
    loopi(8)                                    // counting number of semi-solid/solid children cubes
    {
        int count = 0, childpos = varoot.length();
        ivec o(i, cx, cy, cz, size);
        vamergemax = 0;
        vahasmerges = 0;
        if(c[i].ext && c[i].ext->va) 
        {
            //count += vacubemax+1;       // since must already have more then max cubes
            varoot.add(c[i].ext->va);
            if(c[i].ext->va->hasmerges&MERGE_ORIGIN) findmergedfaces(c[i], o, size, csi, csi);
        }
        else
        {
            if(c[i].children) count += updateva(c[i].children, o.x, o.y, o.z, size/2, csi-1);
            else if(!isempty(c[i]) || hasskyfaces(c[i], o.x, o.y, o.z, size, faces)) count++;
            int tcount = count + (csi <= MAXMERGELEVEL ? vamerges[csi].length() : 0);
            if(tcount > vacubemax || (tcount >= vacubemin && size >= vacubesize) || size == min(0x1000, worldsize/2)) 
            {
                loadprogress = clamp(recalcprogress/float(allocnodes), 0.0f, 1.0f);
                setva(c[i], o.x, o.y, o.z, size, csi);
                if(c[i].ext && c[i].ext->va)
                {
                    while(varoot.length() > childpos)
                    {
                        vtxarray *child = varoot.pop();
                        c[i].ext->va->children.add(child);
                        child->parent = c[i].ext->va;
                    }
                    varoot.add(c[i].ext->va);
                    if(vamergemax > size)
                    {
                        cmergemax = max(cmergemax, vamergemax);
                        chasmerges |= vahasmerges&~MERGE_USE;
                    }
                    continue;
                }
                else count = 0;
            }
        }
        if(csi+1 <= MAXMERGELEVEL && vamerges[csi].length()) vamerges[csi+1].move(vamerges[csi]);
        cmergemax = max(cmergemax, vamergemax);
        chasmerges |= vahasmerges;
        ccount += count;
    }
    --neighbourdepth;
    vamergemax = cmergemax;
    vahasmerges = chasmerges;

    return ccount;
}

void addtjoint(const edgegroup &g, const cubeedge &e, int offset)
{
    int vcoord = (g.slope[g.axis]*offset + g.origin[g.axis]) & 0x7FFF;
    tjoint &tj = tjoints.add();
    tj.offset = vcoord / g.slope[g.axis];
    tj.edge = e.index;

    int prev = -1, cur = ext(*e.c).tjoints;
    while(cur >= 0)
    {
        tjoint &o = tjoints[cur];
        if(tj.edge < o.edge || (tj.edge==o.edge && (e.flags&CE_FLIP ? tj.offset > o.offset : tj.offset < o.offset))) break;
        prev = cur;
        cur = o.next;
    }

    tj.next = cur;
    if(prev < 0) e.c->ext->tjoints = tjoints.length()-1;
    else tjoints[prev].next = tjoints.length()-1; 
}

void findtjoints(int cur, const edgegroup &g)
{
    int active = -1;
    while(cur >= 0)
    {
        cubeedge &e = cubeedges[cur];
        int prevactive = -1, curactive = active;
        while(curactive >= 0)
        {
            cubeedge &a = cubeedges[curactive];
            if(a.offset+a.size <= e.offset)
            {
                if(prevactive >= 0) cubeedges[prevactive].next = a.next;
                else active = a.next;
            }
            else
            {
                prevactive = curactive;
                if(!(a.flags&CE_DUP))
                {
                    if(e.flags&CE_START && e.offset > a.offset && e.offset < a.offset+a.size)
                        addtjoint(g, a, e.offset);
                    if(e.flags&CE_END && e.offset+e.size > a.offset && e.offset+e.size < a.offset+a.size)
                        addtjoint(g, a, e.offset+e.size);
                }
                if(!(e.flags&CE_DUP))
                {
                    if(a.flags&CE_START && a.offset > e.offset && a.offset < e.offset+e.size)
                        addtjoint(g, e, a.offset);
                    if(a.flags&CE_END && a.offset+a.size > e.offset && a.offset+a.size < e.offset+e.size)
                        addtjoint(g, e, a.offset+a.size);
                }
            }
            curactive = a.next;
        }
        int next = e.next;
        e.next = active;
        active = cur;
        cur = next;
    }
}

void findtjoints()
{
    recalcprogress = 0;
    gencubeedges();
    tjoints.setsize(0);
    enumeratekt(edgegroups, edgegroup, g, int, e, findtjoints(e, g));
    cubeedges.setsize(0);
    edgegroups.clear();
}

void octarender()                               // creates va s for all leaf cubes that don't already have them
{
    int csi = 0;
    while(1<<csi < worldsize) csi++;

    recalcprogress = 0;
    varoot.setsize(0);
    updateva(worldroot, 0, 0, 0, worldsize/2, csi-1);
    loadprogress = 0;
    flushvbo();

    explicitsky = 0;
    skyarea = 0;
    loopv(valist)
    {
        vtxarray *va = valist[i];
        explicitsky += va->explicitsky;
        skyarea += va->skyarea;
    }

    extern vtxarray *visibleva;
    visibleva = NULL;
}

void precachetextures()
{
    vector<int> texs;
    loopv(valist)
    {
        vtxarray *va = valist[i];
        loopj(va->texs + va->blends) if(texs.find(va->eslist[j].texture) < 0) texs.add(va->eslist[j].texture);
    }
    loopv(texs)
    {
        loadprogress = float(i+1)/texs.length();
        lookupvslot(texs[i]);
    }
    loadprogress = 0;
}

// XXX EMSCRIPTEN: split up during load
static void allchanged2(void *);
static void allchanged3(void *);
static void allchanged4(void *);
static void allchanged5(void *);
static bool allchanged_load;
static void (*allchanged_next)(void *);

void allchanged(bool load, void (*next)(void *))
{
    allchanged_load = load;
    allchanged_next = next;

    renderprogress(0, "clearing vertex arrays...");
    clearvas(worldroot);
    resetqueries();
    resetclipplanes();

    if (allchanged_next) emscripten_push_main_loop_blocker(allchanged2, NULL);
    else allchanged2(NULL);
}

void allchanged2(void*)
{
    if(allchanged_load) initenvmaps();
    guessshadowdir();
    entitiesinoctanodes();
    tjoints.setsize(0);

    if (allchanged_next) emscripten_push_main_loop_blocker(allchanged3, NULL);
    else allchanged3(NULL);
}

void allchanged3(void*)
{
    if(filltjoints) findtjoints();

    if (allchanged_next) emscripten_push_main_loop_blocker(allchanged4, NULL);
    else allchanged4(NULL);
}

void allchanged4(void*)
{
    octarender();

    if (allchanged_next) emscripten_push_main_loop_blocker(allchanged5, NULL);
    else allchanged5(NULL);
}

void allchanged5(void*)
{
    if(allchanged_load) precachetextures();
    setupmaterials();
    invalidatepostfx();
    updatevabbs(true);
    resetblobs();
    if(allchanged_load) 
    {
        seedparticles();
        genenvmaps();
        drawminimap();
    }

    if (allchanged_next) emscripten_push_main_loop_blocker(allchanged_next, NULL);
}

void recalc()
{
    allchanged(true);
}

COMMAND(recalc, "");

