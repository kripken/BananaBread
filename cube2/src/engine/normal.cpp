#include "engine.h"

struct normalgroup
{
    vec pos;
    int flat, normals, tnormals;

    normalgroup() : flat(0), normals(-1), tnormals(-1) {}
    normalgroup(const vec &pos) : pos(pos), flat(0), normals(-1), tnormals(-1) {}
};

static inline bool htcmp(const vec &v, const normalgroup &n) { return v == n.pos; } 

struct normal
{
    int next;
    vec surface;
};

struct tnormal
{
    int next;
    float offset;
    int normals[2];
    normalgroup *groups[2];
};

hashset<normalgroup> normalgroups(1<<16);
vector<normal> normals;
vector<tnormal> tnormals;

VARR(lerpangle, 0, 44, 180);

static float lerpthreshold = 0;
static bool usetnormals = true;

static int addnormal(const vec &key, const vec &surface)
{
    normalgroup &g = normalgroups.access(key, key);
    normal &n = normals.add();
    n.next = g.normals;
    n.surface = surface;
    return g.normals = normals.length()-1;
}

static void addtnormal(const vec &key, float offset, int normal1, int normal2, normalgroup *group1, normalgroup *group2)
{
    normalgroup &g = normalgroups.access(key, key);
    tnormal &n = tnormals.add();
    n.next = g.tnormals;
    n.offset = offset;
    n.normals[0] = normal1;
    n.normals[1] = normal2;
    n.groups[0] = group1;
    n.groups[1] = group2;
    g.tnormals = tnormals.length()-1;
}

static int addnormal(const vec &key, int axis)
{
    normalgroup &g = normalgroups.access(key, key);
    g.flat += 1<<(4*axis);
    return axis - 6;
}

static inline void findnormal(const normalgroup &g, const vec &surface, vec &v)
{
    v = vec(0, 0, 0);
    int total = 0;
    if(surface.x >= lerpthreshold) { int n = (g.flat>>4)&0xF; v.x += n; total += n; }
    else if(surface.x <= -lerpthreshold) { int n = g.flat&0xF; v.x -= n; total += n; }
    if(surface.y >= lerpthreshold) { int n = (g.flat>>12)&0xF; v.y += n; total += n; }
    else if(surface.y <= -lerpthreshold) { int n = (g.flat>>8)&0xF; v.y -= n; total += n; }
    if(surface.z >= lerpthreshold) { int n = (g.flat>>20)&0xF; v.z += n; total += n; }
    else if(surface.z <= -lerpthreshold) { int n = (g.flat>>16)&0xF; v.z -= n; total += n; }
    for(int cur = g.normals; cur >= 0;)
    {
        normal &o = normals[cur];
        if(o.surface.dot(surface) >= lerpthreshold)
        {
            v.add(o.surface);
            total++;
        }
        cur = o.next;
    }
    if(total > 1) v.normalize();
    else if(!total) v = surface;
}

static inline bool findtnormal(const normalgroup &g, const vec &surface, vec &v)
{
    float bestangle = lerpthreshold;
    tnormal *bestnorm = NULL;
    for(int cur = g.tnormals; cur >= 0;)
    {
        tnormal &o = tnormals[cur];
        static const vec flats[6] = { vec(-1, 0, 0), vec(1, 0, 0), vec(0, -1, 0), vec(0, 1, 0), vec(0, 0, -1), vec(0, 0, 1) };
        vec n1 = o.normals[0] < 0 ? flats[o.normals[0]+6] : normals[o.normals[0]].surface,
            n2 = o.normals[1] < 0 ? flats[o.normals[1]+6] : normals[o.normals[1]].surface,
            nt;
        nt.lerp(n1, n2, o.offset).normalize();
        float tangle = nt.dot(surface);
        if(tangle >= bestangle)
        {
            bestangle = tangle;
            bestnorm = &o;
        }
        cur = o.next;
    }
    if(!bestnorm) return false;
    vec n1, n2;
    findnormal(*bestnorm->groups[0], surface, n1);
    findnormal(*bestnorm->groups[1], surface, n2);
    v.lerp(n1, n2, bestnorm->offset).normalize();
    return true;
}

void findnormal(const vec &key, const vec &surface, vec &v)
{
    const normalgroup *g = normalgroups.access(key);
    if(!g) v = surface;
    else if(g->tnormals < 0 || !findtnormal(*g, surface, v)) 
        findnormal(*g, surface, v);
}

VARR(lerpsubdiv, 0, 2, 4);
VARR(lerpsubdivsize, 4, 4, 128);

static uint progress = 0;

void show_addnormals_progress()
{
    float bar1 = float(progress) / float(allocnodes);
    renderprogress(bar1, "computing normals...");
}

void addnormals(cube &c, const ivec &o, int size)
{
    CHECK_CALCLIGHT_PROGRESS(return, show_addnormals_progress);

    if(c.children)
    {
        progress++;
        size >>= 1;
        loopi(8) addnormals(c.children[i], ivec(i, o.x, o.y, o.z, size), size);
        return;
    }
    else if(isempty(c)) return;

    vec pos[MAXFACEVERTS];
    int norms[MAXFACEVERTS];
    int tj = usetnormals && c.ext ? c.ext->tjoints : -1, vis;
    loopi(6) if((vis = visibletris(c, i, o.x, o.y, o.z, size)))
    {
        CHECK_CALCLIGHT_PROGRESS(return, show_addnormals_progress);
        if(c.texture[i] == DEFAULT_SKY) continue;

        vec planes[2];
        int numverts = c.ext ? c.ext->surfaces[i].numverts&MAXFACEVERTS : 0, convex = 0, numplanes = 0;
        if(numverts)
        {
            vertinfo *verts = c.ext->verts() + c.ext->surfaces[i].verts;
            vec vo = ivec(o).mask(~0xFFF).tovec();
            loopj(numverts)
            {
                vertinfo &v = verts[j];
                pos[j] = vec(v.x, v.y, v.z).mul(1.0f/8).add(vo);
            }
            if(!(c.merged&(1<<i)) && !flataxisface(c, i)) convex = faceconvexity(verts, numverts);
        }
        else if(c.merged&(1<<i)) continue;
        else
        {
            ivec v[4];
            genfaceverts(c, i, v);
            if(!flataxisface(c, i)) convex = faceconvexity(v);
            int order = vis&4 || convex < 0 ? 1 : 0;
            vec vo = o.tovec();
            pos[numverts++] = v[order].tovec().mul(size/8.0f).add(vo);
            if(vis&1) pos[numverts++] = v[order+1].tovec().mul(size/8.0f).add(vo);
            pos[numverts++] = v[order+2].tovec().mul(size/8.0f).add(vo);
            if(vis&2) pos[numverts++] = v[(order+3)&3].tovec().mul(size/8.0f).add(vo);
        }

        if(!flataxisface(c, i))
        {
            planes[numplanes++].cross(pos[0], pos[1], pos[2]).normalize();
            if(convex) planes[numplanes++].cross(pos[0], pos[2], pos[3]).normalize();
        }

        if(!numplanes) loopk(numverts) norms[k] = addnormal(pos[k], i);
        else if(numplanes==1) loopk(numverts) norms[k] = addnormal(pos[k], planes[0]);
        else 
        { 
            vec avg = vec(planes[0]).add(planes[1]).normalize();
            norms[0] = addnormal(pos[0], avg);
            norms[1] = addnormal(pos[1], planes[0]);
            norms[2] = addnormal(pos[2], avg);
            for(int k = 3; k < numverts; k++) norms[k] = addnormal(pos[k], planes[1]);
        }

        while(tj >= 0 && tjoints[tj].edge < i*(MAXFACEVERTS+1)) tj = tjoints[tj].next;
        while(tj >= 0 && tjoints[tj].edge < (i+1)*(MAXFACEVERTS+1))
        {
            int edge = tjoints[tj].edge, e1 = edge%(MAXFACEVERTS+1), e2 = (e1+1)%numverts;
            const vec &v1 = pos[e1], &v2 = pos[e2];
            ivec d = vec(v2).sub(v1).mul(8);
            int axis = abs(d.x) > abs(d.y) ? (abs(d.x) > abs(d.z) ? 0 : 2) : (abs(d.y) > abs(d.z) ? 1 : 2);
            if(d[axis] < 0) d.neg();
            reduceslope(d);
            int origin = int(min(v1[axis], v2[axis])*8)&~0x7FFF,
                offset1 = (int(v1[axis]*8) - origin) / d[axis],
                offset2 = (int(v2[axis]*8) - origin) / d[axis];
            vec o = vec(v1).sub(d.tovec().mul(offset1/8.0f)), n1, n2;
            float doffset = 1.0f / (offset2 - offset1);

            while(tj >= 0)
            {
                tjoint &t = tjoints[tj];
                if(t.edge != edge) break;
                float offset = (t.offset - offset1) * doffset;
                vec tpos = d.tovec().mul(t.offset/8.0f).add(o); 
                addtnormal(tpos, offset, norms[e1], norms[e2], normalgroups.access(v1), normalgroups.access(v2));
                tj = t.next;
            }
        }
    }
}

void calcnormals(bool lerptjoints)
{
    if(!lerpangle) return;
    usetnormals = lerptjoints; 
    if(usetnormals) findtjoints();
    lerpthreshold = cos(lerpangle*RAD) - 1e-5f; 
    progress = 1;
    loopi(8) addnormals(worldroot[i], ivec(i, 0, 0, 0, worldsize/2), worldsize/2);
}

void clearnormals()
{
    normalgroups.clear();
    normals.setsize(0);
    tnormals.setsize(0);
}

void calclerpverts(const vec2 *c, const vec *n, lerpvert *lv, int &numv)
{
    int i = 0;
    loopj(numv)
    {
        if(j)
        {
            if(c[j] == c[j-1] && n[j] == n[j-1]) continue;
            if(j == numv-1 && c[j] == c[0] && n[j] == n[0]) continue;
        }
        lv[i].normal = n[j];
        lv[i].u = c[j].x;
        lv[i].v = c[j].y;
        i++;
    }
    numv = i;
}

void setlerpstep(float v, lerpbounds &bounds)
{
    if(bounds.min->v + 1 > bounds.max->v)
    {
        bounds.nstep = vec(0, 0, 0);
        bounds.normal = bounds.min->normal;
        if(bounds.min->normal != bounds.max->normal)
        {
            bounds.normal.add(bounds.max->normal);
            bounds.normal.normalize();
        }
        bounds.ustep = 0;
        bounds.u = bounds.min->u;
        return;
    }

    bounds.nstep = bounds.max->normal;
    bounds.nstep.sub(bounds.min->normal);
    bounds.nstep.div(bounds.max->v-bounds.min->v);

    bounds.normal = bounds.nstep;
    bounds.normal.mul(v - bounds.min->v);
    bounds.normal.add(bounds.min->normal);

    bounds.ustep = (bounds.max->u-bounds.min->u) / (bounds.max->v-bounds.min->v);
    bounds.u = bounds.ustep * (v-bounds.min->v) + bounds.min->u;
}

void initlerpbounds(float u, float v, const lerpvert *lv, int numv, lerpbounds &start, lerpbounds &end)
{
    const lerpvert *first = &lv[0], *second = NULL;
    loopi(numv-1)
    {
        if(lv[i+1].v < first->v) { second = first; first = &lv[i+1]; }
        else if(!second || lv[i+1].v < second->v) second = &lv[i+1];
    }

    if(int(first->v) < int(second->v)) { start.min = end.min = first; }
    else if(first->u > second->u) { start.min = second; end.min = first; }
    else { start.min = first; end.min = second; }

    if((lv[1].u - lv->u)*(lv[2].v - lv->v) > (lv[1].v - lv->v)*(lv[2].u - lv->u))
    { 
        start.winding = end.winding = 1;
        start.max = (start.min == lv ? &lv[numv-1] : start.min-1);
        end.max = (end.min == &lv[numv-1] ? lv : end.min+1);
    }
    else
    {
        start.winding = end.winding = -1;
        start.max = (start.min == &lv[numv-1] ? lv : start.min+1);
        end.max = (end.min == lv ? &lv[numv-1] : end.min-1);
    }

    setlerpstep(v, start);
    setlerpstep(v, end);
}

void updatelerpbounds(float v, const lerpvert *lv, int numv, lerpbounds &start, lerpbounds &end)
{
    if(v >= start.max->v)
    {
        const lerpvert *next = start.winding > 0 ?
                (start.max == lv ? &lv[numv-1] : start.max-1) :
                (start.max == &lv[numv-1] ? lv : start.max+1);
        if(next->v > start.max->v)
        {
            start.min = start.max;
            start.max = next;
            setlerpstep(v, start);
        }
    }
    if(v >= end.max->v)
    {
        const lerpvert *next = end.winding > 0 ?
                (end.max == &lv[numv-1] ? lv : end.max+1) :
                (end.max == lv ? &lv[numv-1] : end.max-1);
        if(next->v > end.max->v)
        {
            end.min = end.max;
            end.max = next;
            setlerpstep(v, end);
        }
    }
}

void lerpnormal(float u, float v, const lerpvert *lv, int numv, lerpbounds &start, lerpbounds &end, vec &normal, vec &nstep)
{   
    updatelerpbounds(v, lv, numv, start, end);

    if(start.u + 1 > end.u)
    {
        nstep = vec(0, 0, 0);
        normal = start.normal;
        normal.add(end.normal);
        normal.normalize();
    }
    else
    {
        vec nstart(start.normal), nend(end.normal);
        nstart.normalize();
        nend.normalize();
       
        nstep = nend;
        nstep.sub(nstart);
        nstep.div(end.u-start.u);

        normal = nstep;
        normal.mul(u-start.u);
        normal.add(nstart);
        normal.normalize();
    }
     
    start.normal.add(start.nstep);
    start.u += start.ustep;

    end.normal.add(end.nstep); 
    end.u += end.ustep;
}

