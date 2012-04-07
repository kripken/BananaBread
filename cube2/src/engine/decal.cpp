#include "engine.h"

struct decalvert
{
    vec pos;
    float u, v;
    bvec color;
    uchar alpha;
};

struct decalinfo
{
    int millis;
    bvec color;
    ushort startvert, endvert;
};

enum
{
    DF_RND4       = 1<<0,
    DF_ROTATE     = 1<<1,
    DF_INVMOD     = 1<<2,
    DF_OVERBRIGHT = 1<<3,
    DF_ADD        = 1<<4,
    DF_SATURATE   = 1<<5
};

VARFP(maxdecaltris, 1, 1024, 16384, initdecals());
VARP(decalfade, 1000, 10000, 60000);
VAR(dbgdec, 0, 0, 1);

struct decalrenderer
{
    const char *texname;
    int flags, fadeintime, fadeouttime, timetolive;
    Texture *tex;
    decalinfo *decals;
    int maxdecals, startdecal, enddecal;
    decalvert *verts;
    int maxverts, startvert, endvert, availverts;

    decalrenderer(const char *texname, int flags = 0, int fadeintime = 0, int fadeouttime = 1000, int timetolive = -1)
        : texname(texname), flags(flags),
          fadeintime(fadeintime), fadeouttime(fadeouttime), timetolive(timetolive),
          tex(NULL),
          decals(NULL), maxdecals(0), startdecal(0), enddecal(0),
          verts(NULL), maxverts(0), startvert(0), endvert(0), availverts(0),
          decalu(0), decalv(0)
    {
    }

    void init(int tris)
    {
        if(decals)
        {
            DELETEA(decals);
            maxdecals = startdecal = enddecal = 0;
        }
        if(verts)
        {
            DELETEA(verts);
            maxverts = startvert = endvert = availverts = 0;
        }
        decals = new decalinfo[tris];
        maxdecals = tris;
        tex = textureload(texname, 3);
        maxverts = tris*3 + 3;
        availverts = maxverts - 3; 
        verts = new decalvert[maxverts];
    }

    int hasdecals()
    {
        return enddecal < startdecal ? maxdecals - (startdecal - enddecal) : enddecal - startdecal;
    }

    void cleardecals()
    {
        startdecal = enddecal = 0;
        startvert = endvert = 0;
        availverts = maxverts - 3;
    }

    int freedecal()
    {
        if(startdecal==enddecal) return 0;

        decalinfo &d = decals[startdecal];
        startdecal++;
        if(startdecal >= maxdecals) startdecal = 0;
        
        int removed = d.endvert < d.startvert ? maxverts - (d.startvert - d.endvert) : d.endvert - d.startvert;
        startvert = d.endvert;
        if(startvert==endvert) startvert = endvert = 0;
        availverts += removed;

        return removed;
    }

    void fadedecal(decalinfo &d, uchar alpha)
    {
        bvec color;
        if(flags&DF_OVERBRIGHT)
        {
            if(renderpath!=R_FIXEDFUNCTION || hasTE) color = bvec(128, 128, 128);
            else color = bvec(alpha, alpha, alpha);
        }
        else
        {
            color = d.color;
            if(flags&(DF_ADD|DF_INVMOD)) loopk(3) color[k] = uchar((int(color[k])*int(alpha))>>8);
        }

        decalvert *vert = &verts[d.startvert],
                  *end = &verts[d.endvert < d.startvert ? maxverts : d.endvert]; 
        while(vert < end)
        {
            vert->color = color;
            vert->alpha = alpha;
            vert++;
        }
        if(d.endvert < d.startvert)
        {
            vert = verts;
            end = &verts[d.endvert];
            while(vert < end)
            {
                vert->color = color;
                vert->alpha = alpha;
                vert++;
            }
        }
    }

    void clearfadeddecals()
    {
        int threshold = lastmillis - (timetolive>=0 ? timetolive : decalfade) - fadeouttime;
        decalinfo *d = &decals[startdecal],
                  *end = &decals[enddecal < startdecal ? maxdecals : enddecal];
        while(d < end && d->millis <= threshold) d++;
        if(d >= end && enddecal < startdecal)
        {
            d = decals;
            end = &decals[enddecal];
            while(d < end && d->millis <= threshold) d++;
        }
        startdecal = d - decals;
        if(startdecal!=enddecal) startvert = decals[startdecal].startvert;
        else startvert = endvert = 0;
        availverts = endvert < startvert ? startvert - endvert - 3 : maxverts - 3 - (endvert - startvert);
    }
 
    void fadeindecals()
    {
        if(!fadeintime) return;
        decalinfo *d = &decals[enddecal],
                  *end = &decals[enddecal < startdecal ? 0 : startdecal];
        while(d > end)
        {
            d--;
            int fade = lastmillis - d->millis;
            if(fade >= fadeintime) return;
            fadedecal(*d, (fade<<8)/fadeintime);
        }
        if(enddecal < startdecal)
        {
            d = &decals[maxdecals];
            end = &decals[startdecal];
            while(d > end)
            {
                d--;
                int fade = lastmillis - d->millis;
                if(fade >= fadeintime) return;
                fadedecal(*d, (fade<<8)/fadeintime);
            }
        }
    }

    void fadeoutdecals()
    {
        decalinfo *d = &decals[startdecal],
                  *end = &decals[enddecal < startdecal ? maxdecals : enddecal];
        int offset = (timetolive>=0 ? timetolive : decalfade) + fadeouttime - lastmillis;
        while(d < end)
        {
            int fade = d->millis + offset;
            if(fade >= fadeouttime) return;
            fadedecal(*d, (fade<<8)/fadeouttime);
            d++;
        }
        if(enddecal < startdecal)
        {
            d = decals;
            end = &decals[enddecal];
            while(d < end)
            {
                int fade = d->millis + offset;
                if(fade >= fadeouttime) return;
                fadedecal(*d, (fade<<8)/fadeouttime);
                d++;
            }
        }
    }
         
    static void setuprenderstate()
    {
        enablepolygonoffset(GL_POLYGON_OFFSET_FILL);

        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);

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

    void render()
    {
        if(startvert==endvert) return;

        float oldfogc[4];
        if(flags&(DF_ADD|DF_INVMOD|DF_OVERBRIGHT))
        {
            glGetFloatv(GL_FOG_COLOR, oldfogc);
            static float zerofog[4] = { 0, 0, 0, 1 }, grayfog[4] = { 0.5f, 0.5f, 0.5f, 1 };
            glFogfv(GL_FOG_COLOR, flags&DF_OVERBRIGHT && (renderpath!=R_FIXEDFUNCTION || hasTE) ? grayfog : zerofog);
        }
        
        if(flags&DF_OVERBRIGHT) 
        {
            if(renderpath!=R_FIXEDFUNCTION)
            {
                glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR); 
                SETSHADER(overbrightdecal);
            }
            else if(hasTE)
            {
                glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
                setuptmu(0, "T , C @ Ca");
            }
            else glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
        }
        else 
        {
            if(flags&DF_INVMOD) glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
            else if(flags&DF_ADD) glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
            else glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            if(flags&DF_SATURATE)
            {
                if(renderpath!=R_FIXEDFUNCTION) SETSHADER(saturatedecal);
                else if(hasTE) setuptmu(0, "C * T x 2");
            }
            else foggedshader->set();
        }

        glBindTexture(GL_TEXTURE_2D, tex->id);

        glVertexPointer(3, GL_FLOAT, sizeof(decalvert), &verts->pos);
        glTexCoordPointer(2, GL_FLOAT, sizeof(decalvert), &verts->u);
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(decalvert), &verts->color);

        int count = endvert < startvert ? maxverts - startvert : endvert - startvert;
        glDrawArrays(GL_TRIANGLES, startvert, count);
        if(endvert < startvert) 
        {
            count += endvert;
            glDrawArrays(GL_TRIANGLES, 0, endvert);
        }
        xtravertsva += count;

        if(flags&(DF_ADD|DF_INVMOD|DF_OVERBRIGHT)) glFogfv(GL_FOG_COLOR, oldfogc);
        if(flags&(DF_OVERBRIGHT|DF_SATURATE) && hasTE) resettmu(0);
    }

    decalinfo &newdecal()
    {
        decalinfo &d = decals[enddecal];
        int next = enddecal + 1;
        if(next>=maxdecals) next = 0;
        if(next==startdecal) freedecal();
        enddecal = next;
        return d;
    }

    ivec bborigin, bbsize;
    vec decalcenter, decalnormal, decaltangent, decalbitangent;
    float decalradius, decalu, decalv;
    bvec decalcolor;

    void adddecal(const vec &center, const vec &dir, float radius, const bvec &color, int info)
    {
        int isz = int(ceil(radius));
        bborigin = ivec(center).sub(isz);
        bbsize = ivec(isz*2, isz*2, isz*2);

        decalcolor = color;
        decalcenter = center;
        decalradius = radius;
        decalnormal = dir;
#if 0
        decaltangent.orthogonal(dir);
#else
        decaltangent = vec(dir.z, -dir.x, dir.y);
        decaltangent.sub(vec(dir).mul(decaltangent.dot(dir)));
#endif
        if(flags&DF_ROTATE) decaltangent.rotate(rnd(360)*RAD, dir);
        decaltangent.normalize();
        decalbitangent.cross(decaltangent, dir);
        if(flags&DF_RND4)
        {
            decalu = 0.5f*(info&1);
            decalv = 0.5f*((info>>1)&1);
        }

        ushort dstart = endvert;
        gentris(worldroot, ivec(0, 0, 0), worldsize>>1);
        if(dbgdec)
        {
            int nverts = endvert < dstart ? endvert + maxverts - dstart : endvert - dstart;
            conoutf(CON_DEBUG, "tris = %d, verts = %d, total tris = %d", nverts/3, nverts, (maxverts - 3 - availverts)/3);
        }
        if(endvert==dstart) return;

        decalinfo &d = newdecal();
        d.color = color;
        d.millis = lastmillis;
        d.startvert = dstart;
        d.endvert = endvert;
    }

    static int clip(const vec *in, int numin, const vec &dir, float below, float above, vec *out)
    {
        int numout = 0;
        const vec *p = &in[numin-1];
        float pc = dir.dot(*p);
        loopi(numin)
        {
            const vec &v = in[i];
            float c = dir.dot(v);
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

    void gentris(cube &cu, int orient, const ivec &o, int size, materialsurface *mat = NULL)
    {
        vec pos[MAXFACEVERTS+4];
        int numverts = 0, numplanes = 1;
        vec planes[2];
        if(mat)
        {
            planes[0] = vec(0, 0, 0);
            switch(orient)
            {
            #define GENFACEORIENT(orient, v0, v1, v2, v3) \
                case orient: \
                    planes[0][dimension(orient)] = dimcoord(orient) ? 1 : -1; \
                    v0 v1 v2 v3 \
                    break;
            #define GENFACEVERT(orient, vert, x,y,z, xv,yv,zv) \
                    pos[numverts++] = vec(x xv, y yv, z zv);
                GENFACEVERTS(o.x, o.x, o.y, o.y, o.z, o.z, , + mat->csize, , + mat->rsize, + 0.1f, - 0.1f);
            #undef GENFACEORIENT
            #undef GENFACEVERT 
            }
        }
        else if(cu.texture[orient] == DEFAULT_SKY) return;
        else if(cu.ext && (numverts = cu.ext->surfaces[orient].numverts&MAXFACEVERTS))
        {
            vertinfo *verts = cu.ext->verts() + cu.ext->surfaces[orient].verts;
            ivec vo = ivec(o).mask(~0xFFF).shl(3);
            loopj(numverts) pos[j] = verts[j].getxyz().add(vo).tovec().mul(1/8.0f);
            planes[0].cross(pos[0], pos[1], pos[2]).normalize();
            if(numverts >= 4 && !(cu.merged&(1<<orient)) && !flataxisface(cu, orient) && faceconvexity(verts, numverts))
            {
                planes[1].cross(pos[0], pos[2], pos[3]).normalize();
                numplanes++;
            }
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
            planes[0].cross(pos[0], pos[1], pos[2]).normalize();
            if(convex) { planes[1].cross(pos[0], pos[2], pos[3]).normalize(); numplanes++; }
        } 

        loopl(numplanes)
        {
            const vec &n = planes[l];
            float facing = n.dot(decalnormal);
            if(facing <= 0) continue;
            vec p = vec(pos[0]).sub(decalcenter);
#if 0
            // intersect ray along decal normal with plane
            float dist = n.dot(p) / facing;
            if(fabs(dist) > decalradius) continue;
            vec pcenter = vec(decalnormal).mul(dist).add(decalcenter);
#else
            // travel back along plane normal from the decal center
            float dist = n.dot(p);
            if(fabs(dist) > decalradius) continue;
            vec pcenter = vec(n).mul(dist).add(decalcenter);
#endif
            vec ft, fb;
            ft.orthogonal(n);
            ft.normalize();
            fb.cross(ft, n);
            vec pt = vec(ft).mul(ft.dot(decaltangent)).add(vec(fb).mul(fb.dot(decaltangent))).normalize(),
                pb = vec(ft).mul(ft.dot(decalbitangent)).add(vec(fb).mul(fb.dot(decalbitangent))).normalize();
            // orthonormalize projected bitangent to prevent streaking
            pb.sub(vec(pt).mul(pt.dot(pb))).normalize();
            vec v1[MAXFACEVERTS+4], v2[MAXFACEVERTS+4];
            float ptc = pt.dot(pcenter), pbc = pb.dot(pcenter);
            int numv;
            if(numplanes >= 2)
            {
                if(l) { pos[1] = pos[2]; pos[2] = pos[3]; } 
                numv = clip(pos, 3, pt, ptc - decalradius, ptc + decalradius, v1);
                if(numv<3) continue;
            }
            else
            {
                numv = clip(pos, numverts, pt, ptc - decalradius, ptc + decalradius, v1);
                if(numv<3) continue;
            }
            numv = clip(v1, numv, pb, pbc - decalradius, pbc + decalradius, v2);
            if(numv<3) continue;
            float tsz = flags&DF_RND4 ? 0.5f : 1.0f, scale = tsz*0.5f/decalradius,
                  tu = decalu + tsz*0.5f - ptc*scale, tv = decalv + tsz*0.5f - pbc*scale;
            pt.mul(scale); pb.mul(scale);
            decalvert dv1 = { v2[0], pt.dot(v2[0]) + tu, pb.dot(v2[0]) + tv, decalcolor, 255 },
                      dv2 = { v2[1], pt.dot(v2[1]) + tu, pb.dot(v2[1]) + tv, decalcolor, 255 };
            int totalverts = 3*(numv-2);
            if(totalverts > maxverts-3) return;
            while(availverts < totalverts)
            {
                if(!freedecal()) return;
            }
            availverts -= totalverts;
            loopk(numv-2)
            {
                verts[endvert++] = dv1;
                verts[endvert++] = dv2;
                dv2.pos = v2[k+2];
                dv2.u = pt.dot(v2[k+2]) + tu;
                dv2.v = pb.dot(v2[k+2]) + tv;
                verts[endvert++] = dv2;
                if(endvert>=maxverts) endvert = 0;
            }
        }
    }

    void findmaterials(vtxarray *va)
    {
        materialsurface *matbuf = va->matbuf;
        int matsurfs = va->matsurfs;
        loopi(matsurfs)
        {
            materialsurface &m = matbuf[i];
            if(!isclipped(m.material&MATF_VOLUME)) { i += m.skip; continue; }
            int dim = dimension(m.orient), dc = dimcoord(m.orient);
            if(dc ? decalnormal[dim] <= 0 : decalnormal[dim] >= 0) { i += m.skip; continue; }
            int c = C[dim], r = R[dim];
            for(;;)
            {
                materialsurface &m = matbuf[i];
                if(m.o[dim] >= bborigin[dim] && m.o[dim] <= bborigin[dim] + bbsize[dim] &&
                   m.o[c] + m.csize >= bborigin[c] && m.o[c] <= bborigin[c] + bbsize[c] &&
                   m.o[r] + m.rsize >= bborigin[r] && m.o[r] <= bborigin[r] + bbsize[r])
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
};

decalrenderer decals[] =
{
    decalrenderer("<grey>packages/particles/scorch.png", DF_ROTATE, 500),
    decalrenderer("<grey>packages/particles/blood.png", DF_RND4|DF_ROTATE|DF_INVMOD),
    decalrenderer("<grey><decal>packages/particles/bullet.png", DF_OVERBRIGHT)
};

void initdecals()
{
    loopi(sizeof(decals)/sizeof(decals[0])) decals[i].init(maxdecaltris);
}

void cleardecals()
{
    loopi(sizeof(decals)/sizeof(decals[0])) decals[i].cleardecals();
}

VARNP(decals, showdecals, 0, 1, 1);

void renderdecals(bool mainpass)
{
    bool rendered = false;
    loopi(sizeof(decals)/sizeof(decals[0]))
    {
        decalrenderer &d = decals[i];
        if(mainpass)
        {
            d.clearfadeddecals();
            d.fadeindecals();
            d.fadeoutdecals();
        }
        if(!showdecals || !d.hasdecals()) continue;
        if(!rendered)
        {
            rendered = true;
            decalrenderer::setuprenderstate();
        }
        d.render();
    }
    if(!rendered) return;
    decalrenderer::cleanuprenderstate();
}

VARP(maxdecaldistance, 1, 512, 10000);

void adddecal(int type, const vec &center, const vec &surface, float radius, const bvec &color, int info)
{
    if(!showdecals || type<0 || (size_t)type>=sizeof(decals)/sizeof(decals[0]) || center.dist(camera1->o) - radius > maxdecaldistance) return;
    decalrenderer &d = decals[type];
    d.adddecal(center, surface, radius, color, info);
}
 
