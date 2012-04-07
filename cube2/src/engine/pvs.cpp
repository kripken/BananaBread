#include "engine.h"
#include "SDL_thread.h"

enum
{
    PVS_HIDE_GEOM = 1<<0,
    PVS_HIDE_BB   = 1<<1
};

struct pvsnode
{
    bvec edges;
    uchar flags;
    uint children;
};

static vector<pvsnode> origpvsnodes;

static bool mergepvsnodes(pvsnode &p, pvsnode *children)
{
    loopi(7) if(children[i].flags!=children[7].flags) return false;
    bvec bbs[4];
    loop(x, 2) loop(y, 2)
    {
        const bvec &lo = children[octaindex(2, x, y, 0)].edges,
                   &hi = children[octaindex(2, x, y, 1)].edges;
        if(lo.x!=0xFF && (lo.x&0x11 || lo.y&0x11 || lo.z&0x11)) return false;
        if(hi.x!=0xFF && (hi.x&0x11 || hi.y&0x11 || hi.z&0x11)) return false;

#define MERGEBBS(res, coord, row, col) \
        if(lo.coord==0xFF) \
        { \
            if(hi.coord!=0xFF) \
            { \
                res.coord = ((hi.coord&~0x11)>>1) + 0x44; \
                res.row = hi.row; \
                res.col = hi.col; \
            } \
        } \
        else if(hi.coord==0xFF) \
        { \
            res.coord = (lo.coord&0xEE)>>1; \
            res.row = lo.row; \
            res.col = lo.col; \
        } \
        else if(lo.row!=hi.row || lo.col!=hi.col || (lo.coord&0xF0)!=0x80 || (hi.coord&0xF)!=0) return false; \
        else \
        { \
            res.coord = ((lo.coord&~0xF1)>>1) | (((hi.coord&~0x1F)>>1) + 0x40); \
            res.row = lo.row; \
            res.col = lo.col; \
        }

        bvec &res = bbs[x + 2*y];
        MERGEBBS(res, z, x, y);
        res.x = lo.x;
        res.y = lo.y;
    }
    loop(x, 2)
    {
        bvec &lo = bbs[x], &hi = bbs[x+2];
        MERGEBBS(lo, y, x, z);
    }
    bvec &lo = bbs[0], &hi = bbs[1];
    MERGEBBS(p.edges, x, y, z);

    return true;
}

static void genpvsnodes(cube *c, int parent = 0, const ivec &co = ivec(0, 0, 0), int size = worldsize/2)
{
    int index = origpvsnodes.length();
    loopi(8)
    {
        ivec o(i, co.x, co.y, co.z, size);
        pvsnode &n = origpvsnodes.add();
        n.flags = 0;
        n.children = 0;
        if(c[i].children || isempty(c[i]) || c[i].material&MAT_ALPHA) memset(n.edges.v, 0xFF, 3);
        else loopk(3)
        {
            uint face = c[i].faces[k];
            if(face==F_SOLID) n.edges[k] = 0x80;
            else
            {
                uchar low = max(max(face&0xF, (face>>8)&0xF), max((face>>16)&0xF, (face>>24)&0xF)),
                      high = min(min((face>>4)&0xF, (face>>12)&0xF), min((face>>20)&0xF, (face>>28)&0xF));
                if(size<8)
                {
                    if(low&((8/size)-1)) { low += 8/size - (low&((8/size)-1)); }
                    if(high&((8/size)-1)) high &= ~(8/size-1);
                }
                if(low >= high) { memset(n.edges.v, 0xFF, 3); break; }
                n.edges[k] = low | (high<<4);
            }
        }
    }
    int branches = 0;
    loopi(8) if(c[i].children)
    {
        ivec o(i, co.x, co.y, co.z, size);
        genpvsnodes(c[i].children, index+i, o, size>>1);
        if(origpvsnodes[index+i].children) branches++;
    }
    if(!branches && mergepvsnodes(origpvsnodes[parent], &origpvsnodes[index])) origpvsnodes.setsize(index);
    else origpvsnodes[parent].children = index;
}

struct shaftplane
{
    float r, c, offset;
    uchar rnear, cnear, rfar, cfar;
};

struct usvec
{
    union
    {
        struct { ushort x, y, z; };
        ushort v[3];
    };

    ushort &operator[](int i) { return v[i]; }
    ushort operator[](int i) const { return v[i]; }

    ivec toivec() const { return ivec(x, y, z); }
};

struct shaftbb
{
    union
    {
        ushort v[6];
        struct { usvec min, max; };
    };

    shaftbb() {}
    shaftbb(const ivec &o, int size)
    {
        min.x = o.x;
        min.y = o.y;
        min.z = o.z;
        max.x = o.x + size;
        max.y = o.y + size;
        max.z = o.z + size;
    }
    shaftbb(const ivec &o, int size, const bvec &edges)
    {
        min.x = o.x + (size*(edges.x&0xF))/8;
        min.y = o.y + (size*(edges.y&0xF))/8;
        min.z = o.z + (size*(edges.z&0xF))/8;
        max.x = o.x + (size*(edges.x>>4))/8;
        max.y = o.y + (size*(edges.y>>4))/8;
        max.z = o.z + (size*(edges.z>>4))/8;
    }

    ushort &operator[](int i) { return v[i]; }
    ushort operator[](int i) const { return v[i]; }

    bool contains(const shaftbb &o) const
    {
        return min.x<=o.min.x && min.y<=o.min.y && min.z<=o.min.z &&
               max.x>=o.max.x && max.y>=o.max.y && max.z>=o.max.z;
    }

    bool outside(const ivec &o, int size) const
    {
        return o.x>=max.x || o.y>=max.y || o.z>=max.z ||
               o.x+size<=min.x || o.y+size<=min.y || o.z+size<=min.z;
    }

    bool outside(const shaftbb &o) const
    {
        return o.min.x>max.x || o.min.y>max.y || o.min.z>max.z ||
               o.max.x<min.x || o.max.y<min.y || o.max.z<min.z;
    }

    bool notinside(const shaftbb &o) const
    {
        return o.min.x<min.x || o.min.y<min.y || o.min.z<min.z ||
               o.max.x>max.x || o.max.y>max.y || o.max.z>max.z;
    }
};

struct shaft
{
    shaftbb bounds;
    shaftplane planes[8];
    int numplanes;

    shaft(const shaftbb &from, const shaftbb &to)
    {
        calcshaft(from, to);
    }

    void calcshaft(const shaftbb &from, const shaftbb &to)
    {
        uchar match = 0, color = 0;
        loopi(3)
        {
            if(to.min[i] < from.min[i]) { color |= 1<<i; bounds.min[i] = 0; }
            else if(to.min[i] > from.min[i]) bounds.min[i] = to.min[i]+1;
            else { match |= 1<<i; bounds.min[i] = to.min[i]; }

            if(to.max[i] > from.max[i]) { color |= 8<<i; bounds.max[i] = USHRT_MAX; }
            else if(to.max[i] < from.max[i]) bounds.max[i] = to.max[i]-1;
            else { match |= 8<<i; bounds.max[i] = to.max[i]; }
        }
        numplanes = 0;
        loopi(5) if(!(match&(1<<i))) for(int j = i+1; j<6; j++) if(!(match&(1<<j)) && i+3!=j && ((color>>i)^(color>>j))&1)
        {
            int r = i%3, c = j%3, d = (r+1)%3;
            if(d==c) d = (c+1)%3;
            shaftplane &p = planes[numplanes++];
            p.r = from[j] - to[j];
            if(i<3 ? p.r >= 0 : p.r < 0)
            {
                p.r = -p.r;
                p.c = from[i] - to[i];
            }
            else p.c = to[i] - from[i];
            p.offset = -(from[i]*p.r + from[j]*p.c);
            p.rnear = p.r >= 0 ? r : 3+r;
            p.cnear = p.c >= 0 ? c : 3+c;
            p.rfar = p.r < 0 ? r : 3+r;
            p.cfar = p.c < 0 ? c : 3+c;
        }
    }

    bool outside(const shaftbb &o) const
    {
        if(bounds.outside(o)) return true;

        for(const shaftplane *p = planes; p < &planes[numplanes]; p++)
        {
            if(o[p->rnear]*p->r + o[p->cnear]*p->c + p->offset > 0) return true;
        }
        return false;
    }

    bool inside(const shaftbb &o) const
    {
        if(bounds.notinside(o)) return false;

        for(const shaftplane *p = planes; p < &planes[numplanes]; p++)
        {
            if(o[p->rfar]*p->r + o[p->cfar]*p->c + p->offset > 0) return false;
        }
        return true;
    }
};

struct pvsdata
{
    int offset, len;

    pvsdata() {}
    pvsdata(int offset, int len) : offset(offset), len(len) {}
};

static vector<uchar> pvsbuf;

static inline uint hthash(const pvsdata &k)
{
    uint h = 5381;
    loopi(k.len) h = ((h<<5)+h)^pvsbuf[k.offset+i];
    return h;
}

static inline bool htcmp(const pvsdata &x, const pvsdata &y)
{
    return x.len==y.len && !memcmp(&pvsbuf[x.offset], &pvsbuf[y.offset], x.len);
}

static SDL_mutex *pvsmutex = NULL;
static hashtable<pvsdata, int> pvscompress;
static vector<pvsdata> pvs;

static SDL_mutex *viewcellmutex = NULL;
struct viewcellrequest
{
    int *result;
    ivec o;
    int size;
};
static vector<viewcellrequest> viewcellrequests;

static bool genpvs_canceled = false;
static int numviewcells = 0;

VAR(maxpvsblocker, 1, 512, 1<<16);
VAR(pvsleafsize, 1, 64, 1024);

#define MAXWATERPVS 32

static struct
{
    int height;
    vector<materialsurface *> matsurfs;
} waterplanes[MAXWATERPVS];
static vector<materialsurface *> waterfalls;
uint numwaterplanes = 0;

struct pvsworker
{
    pvsworker() : thread(NULL), pvsnodes(new pvsnode[origpvsnodes.length()])
    {
    }
    ~pvsworker()
    {
        delete[] pvsnodes;
    }

    SDL_Thread *thread;
    pvsnode *pvsnodes;

    shaftbb viewcellbb;

    pvsnode *levels[32];
    int curlevel;
    ivec origin;

    void resetlevels()
    {
        curlevel = worldscale;
        levels[curlevel] = &pvsnodes[0];
        origin = ivec(0, 0, 0);
    }

    int hasvoxel(const ivec &p, int coord, int dir, int ocoord = 0, int odir = 0, int *omin = NULL)
    {
        uint diff = (origin.x^p.x)|(origin.y^p.y)|(origin.z^p.z);
        if(diff >= uint(worldsize)) return 0;
        diff >>= curlevel;
        while(diff)
        {
            curlevel++;
            diff >>= 1;
        }

        pvsnode *cur = levels[curlevel];
        while(cur->children && !(cur->flags&PVS_HIDE_BB))
        {
            cur = &pvsnodes[cur->children];
            curlevel--;
            cur += ((p.z>>(curlevel-2))&4) | ((p.y>>(curlevel-1))&2) | ((p.x>>curlevel)&1);
            levels[curlevel] = cur;
        }

        origin = ivec(p.x&(~0<<curlevel), p.y&(~0<<curlevel), p.z&(~0<<curlevel));

        if(cur->flags&PVS_HIDE_BB || cur->edges==bvec(0x80, 0x80, 0x80))
        {
            if(omin)
            {
                int step = origin[ocoord] + (odir<<curlevel) - p[ocoord] + odir - 1;
                if(odir ? step < *omin : step > *omin) *omin = step;
            }
            return origin[coord] + (dir<<curlevel) - p[coord] + dir - 1;
        }

        if(cur->edges.x==0xFF) return 0;
        ivec bbp(p);
        bbp.sub(origin);
        ivec bbmin, bbmax;
        bbmin.x = ((cur->edges.x&0xF)<<curlevel)/8;
        if(bbp.x < bbmin.x) return 0;
        bbmax.x = ((cur->edges.x>>4)<<curlevel)/8;
        if(bbp.x >= bbmax.x) return 0;
        bbmin.y = ((cur->edges.y&0xF)<<curlevel)/8;
        if(bbp.y < bbmin.y) return 0;
        bbmax.y = ((cur->edges.y>>4)<<curlevel)/8;
        if(bbp.y >= bbmax.y) return 0;
        bbmin.z = ((cur->edges.z&0xF)<<curlevel)/8;
        if(bbp.z < bbmin.z) return 0;
        bbmax.z = ((cur->edges.z>>4)<<curlevel)/8;
        if(bbp.z >= bbmax.z) return 0;

        if(omin)
        {
            int step = (odir ? bbmax[ocoord] : bbmin[ocoord]) - bbp[ocoord] + (odir - 1);
            if(odir ? step < *omin : step > *omin) *omin = step;
        }
        return (dir ? bbmax[coord] : bbmin[coord]) - bbp[coord] + (dir - 1);
    }

    void hidepvs(pvsnode &p)
    {
        if(p.children)
        {
            pvsnode *children = &pvsnodes[p.children];
            loopi(8) hidepvs(children[i]);
            p.flags |= PVS_HIDE_BB;
            return;
        }
        p.flags |= PVS_HIDE_BB;
        if(p.edges.x!=0xFF) p.flags |= PVS_HIDE_GEOM;
    }

    void shaftcullpvs(shaft &s, pvsnode &p, const ivec &co = ivec(0, 0, 0), int size = worldsize)
    {
        if(p.flags&PVS_HIDE_BB) return;
        shaftbb bb(co, size);
        if(s.outside(bb)) return;
        if(s.inside(bb)) { hidepvs(p); return; }
        if(p.children)
        {
            pvsnode *children = &pvsnodes[p.children];
            uchar flags = 0xFF;
            loopi(8)
            {
                ivec o(i, co.x, co.y, co.z, size>>1);
                shaftcullpvs(s, children[i], o, size>>1);
                flags &= children[i].flags;
            }
            if(flags & PVS_HIDE_BB) p.flags |= PVS_HIDE_BB;
            return;
        }
        if(p.edges.x==0xFF) return;
        shaftbb geom(co, size, p.edges);
        if(s.inside(geom)) p.flags |= PVS_HIDE_GEOM;
    }

    ringbuf<shaftbb, 32> prevblockers;

    struct cullorder
    {
        int index, dist;

        cullorder() {}
        cullorder(int index, int dist) : index(index), dist(dist) {}
    };

    void cullpvs(pvsnode &p, const ivec &co = ivec(0, 0, 0), int size = worldsize)
    {
        if(p.flags&(PVS_HIDE_BB | PVS_HIDE_GEOM) || genpvs_canceled) return;
        if(p.children && !(p.flags&PVS_HIDE_BB))
        {
            pvsnode *children = &pvsnodes[p.children];
            int csize = size>>1;
            ivec dmin = ivec(co).add(csize>>1).sub(viewcellbb.min.toivec().add(viewcellbb.max.toivec()).shr(1)), dmax = ivec(dmin).add(csize);
            dmin.mul(dmin);
            dmax.mul(dmax);
            ivec diff = ivec(dmax).sub(dmin);
            cullorder order[8];
            int dir = 0;
            if(diff.x < 0) { diff.x = -diff.x; dir |= 1; }
            if(diff.y < 0) { diff.y = -diff.y; dir |= 2; }
            if(diff.z < 0) { diff.z = -diff.z; dir |= 4; }
            order[0] = cullorder(0, 0);
            order[7] = cullorder(7, diff.x + diff.y + diff.z);
            order[1] = cullorder(1, diff.x);
            order[2] = cullorder(2, diff.y);
            order[3] = cullorder(4, diff.z);
            if(order[2].dist < order[1].dist) swap(order[1], order[2]);
            if(order[3].dist < order[2].dist) swap(order[2], order[3]);
            if(order[2].dist < order[1].dist) swap(order[1], order[2]);
            cullorder dxy(order[1].index|order[2].index, order[1].dist+order[2].dist),
                      dxz(order[1].index|order[3].index, order[1].dist+order[3].dist),
                      dyz(order[2].index|order[3].index, order[2].dist+order[3].dist);
            int j;
            for(j = 4; j > 0 && dxy.dist < order[j-1].dist; --j) order[j] = order[j-1]; order[j] = dxy;
            for(j = 5; j > 0 && dxz.dist < order[j-1].dist; --j) order[j] = order[j-1]; order[j] = dxz;
            for(j = 6; j > 0 && dyz.dist < order[j-1].dist; --j) order[j] = order[j-1]; order[j] = dyz;
            loopi(8)
            {
                int index = order[i].index^dir;
                ivec o(index, co.x, co.y, co.z, csize);
                cullpvs(children[index], o, csize);
            }
            if(!(p.flags & PVS_HIDE_BB)) return;
        }
        bvec edges = p.children ? bvec(0x80, 0x80, 0x80) : p.edges;
        if(edges.x==0xFF) return;
        shaftbb geom(co, size, edges);
        ivec diff = geom.max.toivec().sub(viewcellbb.min.toivec()).abs();
        cullorder order[3] = { cullorder(0, diff.x), cullorder(1, diff.y), cullorder(2, diff.z) };
        if(order[1].dist > order[0].dist) swap(order[0], order[1]);
        if(order[2].dist > order[1].dist) swap(order[1], order[2]);
        if(order[1].dist > order[0].dist) swap(order[0], order[1]);
        loopi(6)
        {
            int dim = order[i >= 3 ? i-3 : i].index, dc = (i >= 3) != (geom.max[dim] <= viewcellbb.min[dim]) ? 1 : 0, r = R[dim], c = C[dim];
            int ccenter = geom.min[c];
            if(geom.min[r]==geom.max[r] || geom.min[c]==geom.max[c]) continue;
            while(ccenter < geom.max[c])
            {
                ivec rmin;
                rmin[dim] = geom[dim + 3*dc] + (dc ? -1 : 0);
                rmin[r] = geom.min[r];
                rmin[c] = ccenter;
                ivec rmax = rmin;
                rmax[r] = geom.max[r] - 1;
                int rcenter = (rmin[r] + rmax[r])/2;
                resetlevels();
                for(int minstep = -1, maxstep = 1; (minstep || maxstep) && rmax[r] - rmin[r] < maxpvsblocker;)
                {
                    if(minstep) minstep = hasvoxel(rmin, r, 0);
                    if(maxstep) maxstep = hasvoxel(rmax, r, 1);
                    rmin[r] += minstep;
                    rmax[r] += maxstep;
                }
                rmin[r] = rcenter + (rmin[r] - rcenter)/2;
                rmax[r] = rcenter + (rmax[r] - rcenter)/2;
                if(rmin[r]>=geom.min[r] && rmax[r]<geom.max[r]) { rmin[r] = geom.min[r]; rmax[r] = geom.max[r] - 1; }
                ivec cmin = rmin, cmax = rmin;
                if(rmin[r]>=geom.min[r] && rmax[r]<geom.max[r])
                {
                    cmin[c] = geom.min[c];
                    cmax[c] = geom.max[c]-1;
                }
                int cminstep = -1, cmaxstep = 1;
                for(; (cminstep || cmaxstep) && cmax[c] - cmin[c] < maxpvsblocker;)
                {
                    if(cminstep)
                    {
                        cmin[c] += cminstep; cminstep = INT_MIN;
                        cmin[r] = rmin[r];
                        resetlevels();
                        for(int rstep = 1; rstep && cmin[r] <= rmax[r];)
                        {
                            rstep = hasvoxel(cmin, r, 1, c, 0, &cminstep);
                            cmin[r] += rstep;
                        }
                        if(cmin[r] <= rmax[r]) cminstep = 0;
                    }
                    if(cmaxstep)
                    {
                        cmax[c] += cmaxstep; cmaxstep = INT_MAX;
                        cmax[r] = rmin[r];
                        resetlevels();
                        for(int rstep = 1; rstep && cmax[r] <= rmax[r];)
                        {
                            rstep = hasvoxel(cmax, r, 1, c, 1, &cmaxstep);
                            cmax[r] += rstep;
                        }
                        if(cmax[r] <= rmax[r]) cmaxstep = 0;
                    }
                }
                if(!cminstep) cmin[c]++;
                if(!cmaxstep) cmax[c]--;
                ivec emin = rmin, emax = rmax;
                if(cmin[c]>=geom.min[c] && cmax[c]<geom.max[c])
                {
                    if(emin[r]>geom.min[r]) emin[r] = geom.min[r];
                    if(emax[r]<geom.max[r]-1) emax[r] = geom.max[r]-1;
                }
                int rminstep = -1, rmaxstep = 1;
                for(; (rminstep || rmaxstep) && emax[r] - emin[r] < maxpvsblocker;)
                {
                    if(rminstep)
                    {
                        emin[r] += -1; rminstep = INT_MIN;
                        emin[c] = cmin[c];
                        resetlevels();
                        for(int cstep = 1; cstep && emin[c] <= cmax[c];)
                        {
                            cstep = hasvoxel(emin, c, 1, r, 0, &rminstep);
                            emin[c] += cstep;
                        }
                        if(emin[c] <= cmax[c]) rminstep = 0;
                    }
                    if(rmaxstep)
                    {
                        emax[r] += 1; rmaxstep = INT_MAX;
                        emax[c] = cmin[c];
                        resetlevels();
                        for(int cstep = 1; cstep && emax[c] <= cmax[c];)
                        {
                            cstep = hasvoxel(emax, c, 1, r, 1, &rmaxstep);
                            emax[c] += cstep;
                        }
                        if(emax[c] <= cmax[c]) rmaxstep = 0;
                    }
                }
                if(!rminstep) emin[r]++;
                if(!rmaxstep) emax[r]--;
                shaftbb bb;
                bb.min[dim] = rmin[dim];
                bb.max[dim] = rmin[dim]+1;
                bb.min[r] = emin[r];
                bb.max[r] = emax[r]+1;
                bb.min[c] = cmin[c];
                bb.max[c] = cmax[c]+1;
                if(bb.min[dim] >= viewcellbb.max[dim] || bb.max[dim] <= viewcellbb.min[dim])
                {
                    int ddir = bb.min[dim] >= viewcellbb.max[dim] ? 1 : -1,
                        dval = ddir>0 ? USHRT_MAX-1 : 0,
                        dlimit = maxpvsblocker,
                        numsides = 0;
                    loopj(4)
                    {
                        ivec dmax;
                        int odim = j < 2 ? c : r;
                        if(j&1)
                        {
                            if(bb.max[odim] >= viewcellbb.max[odim]) continue;
                            dmax[odim] = bb.max[odim]-1;
                        }
                        else
                        {
                            if(bb.min[odim] <= viewcellbb.min[odim]) continue;
                            dmax[odim] = bb.min[odim];
                        }
                        numsides++;
                        dmax[dim] = bb.min[dim];
                        int stepdim = j < 2 ? r : c, stepstart = bb.min[stepdim], stepend = bb.max[stepdim];
                        int dstep = ddir;
                        for(; dstep && ddir*(dmax[dim] - (int)bb.min[dim]) < dlimit;)
                        {
                            dmax[dim] += dstep; dstep = ddir > 0 ? INT_MAX : INT_MIN;
                            dmax[stepdim] = stepstart;
                            resetlevels();
                            for(int step = 1; step && dmax[stepdim] < stepend;)
                            {
                                step = hasvoxel(dmax, stepdim, 1, dim, (ddir+1)/2, &dstep);
                                dmax[stepdim] += step;
                            }
                            if(dmax[stepdim] < stepend) dstep = 0;
                        }
                        dlimit = min(dlimit, ddir*(dmax[dim] - (int)bb.min[dim]));
                        if(!dstep) dmax[dim] -= ddir;
                        if(ddir>0) dval = min(dval, dmax[dim]);
                        else dval = max(dval, dmax[dim]);
                    }
                    if(numsides>0)
                    {
                        if(ddir>0) bb.max[dim] = dval+1;
                        else bb.min[dim] = dval;
                    }
                    //printf("(%d,%d,%d) x %d,%d,%d, side %d, ccenter = %d, origin = (%d,%d,%d), size = %d\n", bb.min.x, bb.min.y, bb.min.z, bb.max.x-bb.min.x, bb.max.y-bb.min.y, bb.max.z-bb.min.z, i, ccenter, co.x, co.y, co.z, size);
                }
                bool dup = false;
                loopvj(prevblockers)
                {
                    if(prevblockers[j].contains(bb)) { dup = true; break; }
                }
                if(!dup)
                {
                    shaft s(viewcellbb, bb);
                    shaftcullpvs(s, pvsnodes[0]);
                    prevblockers.add(bb);
                }
                if(bb.contains(geom)) return;
                ccenter = cmax[c] + 1;
            }
        }
    }

    bool compresspvs(pvsnode &p, int size, int threshold)
    {
        if(!p.children) return true;
        if(p.flags&PVS_HIDE_BB) { p.children = 0; return true; }
        pvsnode *children = &pvsnodes[p.children];
        bool canreduce = true;
        loopi(8)
        {
            if(!compresspvs(children[i], size/2, threshold)) canreduce = false;
        }
        if(canreduce)
        {
            int hide = children[7].flags&PVS_HIDE_BB;
            loopi(7) if((children[i].flags&PVS_HIDE_BB)!=hide) canreduce = false;
            if(canreduce) 
            {
                p.flags = (p.flags & ~PVS_HIDE_BB) | hide;
                p.children = 0;
                return true;
            }
        }
        if(size <= threshold)
        {
            p.children = 0;
            return true;
        }
        return false;
    }
    
    vector<uchar> outbuf;

    bool serializepvs(pvsnode &p, int storage = -1)
    {
        if(!p.children)
        {
            outbuf.add(0xFF);
            loopi(8) outbuf.add(p.flags&PVS_HIDE_BB ? 0xFF : 0);
            return true;
        }
        int index = outbuf.length();
        pvsnode *children = &pvsnodes[p.children];
        int i = 0;
        uchar leafvalues = 0;
        if(storage>=0)
        {
            for(; i < 8; i++)
            {   
                pvsnode &child = children[i];
                if(child.flags&PVS_HIDE_BB) leafvalues |= 1<<i;
                else if(child.children) break;
            }
            if(i==8) { outbuf[storage] = leafvalues; return false; }
            // if offset won't fit, just mark the space as a visible to avoid problems
            int offset = (index - storage + 8)/9;
            if(offset>255) { outbuf[storage] = 0; return false; }
            outbuf[storage] = uchar(offset);
        }
        outbuf.add(0);
        loopj(8) outbuf.add(leafvalues&(1<<j) ? 0xFF : 0);
        uchar leafmask = (1<<i)-1;
        for(; i < 8; i++)
        {
            pvsnode &child = children[i];
            if(child.children) { if(!serializepvs(child, index+1+i)) leafmask |= 1<<i; }
            else { leafmask |= 1<<i; outbuf[index+1+i] = child.flags&PVS_HIDE_BB ? 0xFF : 0; }
        }
        outbuf[index] = leafmask;
        return true;
    }

    bool materialoccluded(pvsnode &p, const ivec &co, int size, const ivec &bborigin, const ivec &bbsize)
    {
        pvsnode *children = &pvsnodes[p.children];
        loopoctabox(co, size, bborigin, bbsize)
        {
            ivec o(i, co.x, co.y, co.z, size);
            if(children[i].flags & PVS_HIDE_BB) continue;
            if(!children[i].children || !materialoccluded(children[i], o, size/2, bborigin, bbsize)) return false;
        }
        return true;
    }

    bool materialoccluded(vector<materialsurface *> &matsurfs)
    {
        if(pvsnodes[0].flags & PVS_HIDE_BB) return true;
        if(!pvsnodes[0].children) return false;
        loopv(matsurfs)
        {
            materialsurface &m = *matsurfs[i];
            ivec bborigin(m.o), bbsize(0, 0, 0);
            int dim = dimension(m.orient);
            bbsize[C[dim]] = m.csize;
            bbsize[R[dim]] = m.rsize;
            bborigin[dim] -= 2;
            bbsize[dim] = 2;
            if(!materialoccluded(pvsnodes[0], vec(0, 0, 0), worldsize/2, bborigin, bbsize)) return false;
        }
        return true;
    }

    int wateroccluded, waterbytes;

    void calcpvs(const ivec &co, int size)
    {
        loopk(3)
        {
            viewcellbb.min[k] = co[k];
            viewcellbb.max[k] = co[k]+size;
        }
        memcpy(pvsnodes, origpvsnodes.getbuf(), origpvsnodes.length()*sizeof(pvsnode));
        prevblockers.clear();
        cullpvs(pvsnodes[0]);

        wateroccluded = 0;
        loopi(numwaterplanes)
        {
            if(waterplanes[i].height < 0)
            {
                if(waterfalls.length() && materialoccluded(waterfalls)) wateroccluded |= 1<<i;
            }
            else if(waterplanes[i].matsurfs.length() && materialoccluded(waterplanes[i].matsurfs)) wateroccluded |= 1<<i;
        }
        waterbytes = 0;
        loopi(4) if(wateroccluded&(0xFF<<(i*8))) waterbytes = i+1;

        compresspvs(pvsnodes[0], worldsize, pvsleafsize);
        outbuf.setsize(0);
        serializepvs(pvsnodes[0]);
    }

    uchar *testviewcell(const ivec &co, int size, int *waterpvs = NULL, int *len = NULL)
    {
        calcpvs(co, size);

        uchar *buf = new uchar[outbuf.length()];
        memcpy(buf, outbuf.getbuf(), outbuf.length());
        if(waterpvs) *waterpvs = wateroccluded;
        if(len) *len = outbuf.length();
        return buf;
    }

    int genviewcell(const ivec &co, int size)
    {
        calcpvs(co, size);

        if(pvsmutex) SDL_LockMutex(pvsmutex);
        numviewcells++;
        pvsdata key(pvsbuf.length(), waterbytes + outbuf.length());
        loopi(waterbytes) pvsbuf.add((wateroccluded>>(i*8))&0xFF);
        pvsbuf.put(outbuf.getbuf(), outbuf.length());
        int *val = pvscompress.access(key);
        if(val) pvsbuf.setsize(key.offset);
        else
        {
            val = &pvscompress[key];
            *val = pvs.length();
            pvs.add(key);
        }
        if(pvsmutex) SDL_UnlockMutex(pvsmutex);
        return *val;
    }

    static int run(void *data)
    {
        pvsworker *w = (pvsworker *)data;
        SDL_LockMutex(viewcellmutex);
        while(viewcellrequests.length())
        {
            viewcellrequest req = viewcellrequests.pop();
            SDL_UnlockMutex(viewcellmutex);
            int result = w->genviewcell(req.o, req.size);
            SDL_LockMutex(viewcellmutex);
            *req.result = result;
        }
        SDL_UnlockMutex(viewcellmutex);
        return 0;
    }
};

struct viewcellnode
{
    uchar leafmask;
    union viewcellchild
    {
        int pvs;
        viewcellnode *node;
    } children[8];

    viewcellnode() : leafmask(0xFF)
    {
        loopi(8) children[i].pvs = -1;
    }
    ~viewcellnode()
    {
        loopi(8) if(!(leafmask&(1<<i))) delete children[i].node;
    }
};

VARP(pvsthreads, 1, 1, 16);
static vector<pvsworker *> pvsworkers;

static volatile bool check_genpvs_progress = false;

static Uint32 genpvs_timer(Uint32 interval, void *param)
{
    check_genpvs_progress = true;
    return interval;
}

static int totalviewcells = 0;

static void show_genpvs_progress(int unique = pvs.length(), int processed = numviewcells)
{
    float bar1 = float(processed) / float(totalviewcells>0 ? totalviewcells : 1);

    defformatstring(text1)("%d%% - %d of %d view cells (%d unique)", int(bar1 * 100), processed, totalviewcells, unique);

    renderprogress(bar1, text1);

    if(interceptkey(SDLK_ESCAPE)) genpvs_canceled = true;
    check_genpvs_progress = false;
}

static shaftbb pvsbounds;

static void calcpvsbounds()
{
    loopk(3) pvsbounds.min[k] = USHRT_MAX;
    loopk(3) pvsbounds.max[k] = 0;
    extern vector<vtxarray *> valist;
    loopv(valist)
    {
        vtxarray *va = valist[i];
        loopk(3)
        {
            if(va->geommin[k]>va->geommax[k]) continue;
            pvsbounds.min[k] = min(pvsbounds.min[k], (ushort)va->geommin[k]);
            pvsbounds.max[k] = max(pvsbounds.max[k], (ushort)va->geommax[k]);
        }
    }
}

static inline bool isallclip(cube *c)
{
    loopi(8)
    {
        cube &h = c[i];
        if(h.children ? !isallclip(h.children) : (!isentirelysolid(h) && (h.material&MATF_CLIP)!=MAT_CLIP))
            return false;
    }
    return true;
}
   
static int countviewcells(cube *c, const ivec &co, int size, int threshold)
{
    int count = 0;
    loopi(8)
    {
        ivec o(i, co.x, co.y, co.z, size);
        if(pvsbounds.outside(o, size)) continue;
        cube &h = c[i];
        if(h.children)
        {
            if(size>threshold)
            {
                count += countviewcells(h.children, o, size>>1, threshold);
                continue;
            }
            if(isallclip(h.children)) continue;
        }
        else if(isentirelysolid(h) || (h.material&MATF_CLIP)==MAT_CLIP) continue;
        count++;
    }
    return count;
}

static void genviewcells(viewcellnode &p, cube *c, const ivec &co, int size, int threshold)
{
    if(genpvs_canceled) return;
    loopi(8)
    {
        ivec o(i, co.x, co.y, co.z, size);
        if(pvsbounds.outside(o, size)) continue;
        cube &h = c[i];
        if(h.children)
        {
            if(size>threshold)
            {
                p.leafmask &= ~(1<<i);
                p.children[i].node = new viewcellnode;
                genviewcells(*p.children[i].node, h.children, o, size>>1, threshold);
                continue;
            }
            if(isallclip(h.children)) continue;
        }
        else if(isentirelysolid(h) || (h.material&MATF_CLIP)==MAT_CLIP) continue;
        if(pvsthreads<=1)
        {
            if(genpvs_canceled) return;
            p.children[i].pvs = pvsworkers[0]->genviewcell(o, size);
            if(check_genpvs_progress) show_genpvs_progress();
        }
        else
        {
            viewcellrequest &req = viewcellrequests.add();
            req.result = &p.children[i].pvs;
            req.o = o;
            req.size = size;
        }
    }
}

static viewcellnode *viewcells = NULL;
static int lockedwaterplanes[MAXWATERPVS];
static uchar *curpvs = NULL, *lockedpvs = NULL;
static int curwaterpvs = 0, lockedwaterpvs = 0;

static inline pvsdata *lookupviewcell(const vec &p)
{
    uint x = uint(floor(p.x)), y = uint(floor(p.y)), z = uint(floor(p.z));
    if(!viewcells || (x|y|z)>=uint(worldsize)) return NULL;
    viewcellnode *vc = viewcells;
    for(int scale = worldscale-1; scale>=0; scale--)
    {
        int i = octastep(x, y, z, scale);
        if(vc->leafmask&(1<<i))
        {
            return vc->children[i].pvs>=0 ? &pvs[vc->children[i].pvs] : NULL;
        }
        vc = vc->children[i].node;
    }
    return NULL;
}

static void lockpvs_(bool lock)
{
    if(lockedpvs) DELETEA(lockedpvs);
    if(!lock) return;
    pvsdata *d = lookupviewcell(camera1->o);
    if(!d) return;
    int wbytes = d->len%9, len = d->len - wbytes;
    lockedpvs = new uchar[len];
    memcpy(lockedpvs, &pvsbuf[d->offset + wbytes], len);
    lockedwaterpvs = 0;
    loopi(wbytes) lockedwaterpvs |= pvsbuf[d->offset + i] << (i*8);
    loopi(MAXWATERPVS) lockedwaterplanes[i] = waterplanes[i].height;
    conoutf("locked view cell at %.1f, %.1f, %.1f", camera1->o.x, camera1->o.y, camera1->o.z);
}

VARF(lockpvs, 0, 0, 1, lockpvs_(lockpvs!=0));

VARN(pvs, usepvs, 0, 1, 1);
VARN(waterpvs, usewaterpvs, 0, 1, 1);

void setviewcell(const vec &p)
{
    if(!usepvs) curpvs = NULL;
    else if(lockedpvs) 
    {
        curpvs = lockedpvs;
        curwaterpvs = lockedwaterpvs;
    }
    else
    {
        pvsdata *d = lookupviewcell(p);
        curpvs = d ? &pvsbuf[d->offset] : NULL;
        curwaterpvs = 0;
        if(d)
        {
            loopi(d->len%9) curwaterpvs |= *curpvs++ << (i*8);
        }
    }
    if(!usepvs || !usewaterpvs) curwaterpvs = 0;
}

void clearpvs()
{
    DELETEP(viewcells);
    pvs.setsize(0);
    pvsbuf.setsize(0);
    curpvs = NULL;
    numwaterplanes = 0;
    lockpvs = 0;
    lockpvs_(false);
}

COMMAND(clearpvs, "");

static void findwaterplanes()
{
    extern vector<vtxarray *> valist;
    loopi(MAXWATERPVS)
    {
        waterplanes[i].height = -1;
        waterplanes[i].matsurfs.setsize(0);
    }
    waterfalls.setsize(0);
    numwaterplanes = 0;
    loopv(valist)
    {
        vtxarray *va = valist[i];
        loopj(va->matsurfs)
        {
            materialsurface &m = va->matbuf[j];
            if(m.material!=MAT_WATER || m.orient==O_BOTTOM) continue;
            if(m.orient!=O_TOP)
            {
                waterfalls.add(&m);
                continue;
            }
            loopk(numwaterplanes) if(waterplanes[k].height == m.o.z)
            {
                waterplanes[k].matsurfs.add(&m);
                goto nextmatsurf;
            }
            if(numwaterplanes < MAXWATERPVS)
            {
                waterplanes[numwaterplanes].height = m.o.z;
                waterplanes[numwaterplanes].matsurfs.add(&m);
                numwaterplanes++;
            }
        nextmatsurf:;
        }
    }
    if(waterfalls.length() > 0 && numwaterplanes < MAXWATERPVS) numwaterplanes++;
}

void testpvs(int *vcsize)
{
    lockpvs_(false);

    uint oldnumwaterplanes = numwaterplanes;
    int oldwaterplanes[MAXWATERPVS];
    loopi(numwaterplanes) oldwaterplanes[i] = waterplanes[i].height;

    findwaterplanes();

    pvsnode &root = origpvsnodes.add();
    memset(root.edges.v, 0xFF, 3);
    root.flags = 0;
    root.children = 0;
    genpvsnodes(worldroot);

    genpvs_canceled = false;
    check_genpvs_progress = false;

    int size = *vcsize>0 ? *vcsize : 32;
    for(int mask = 1; mask < size; mask <<= 1) size &= ~mask;

    ivec o = camera1->o;
    o.mask(~(size-1));
    pvsworker w;
    int len;
    lockedpvs = w.testviewcell(o, size, &lockedwaterpvs, &len);
    loopi(MAXWATERPVS) lockedwaterplanes[i] = waterplanes[i].height;
    lockpvs = 1;
    conoutf("generated test view cell of size %d at %.1f, %.1f, %.1f (%d B)", size, camera1->o.x, camera1->o.y, camera1->o.z, len);

    origpvsnodes.setsize(0);
    numwaterplanes = oldnumwaterplanes;
    loopi(numwaterplanes) waterplanes[i].height = oldwaterplanes[i];
}

COMMAND(testpvs, "i");

void genpvs(int *viewcellsize)
{
    if(worldsize > 1<<15)
    {
        conoutf(CON_ERROR, "map is too large for PVS");
        return;
    }

    renderbackground("generating PVS (esc to abort)");
    genpvs_canceled = false;
    Uint32 start = SDL_GetTicks();

    renderprogress(0, "finding view cells");

    clearpvs();
    calcpvsbounds();
    findwaterplanes();

    pvsnode &root = origpvsnodes.add();
    memset(root.edges.v, 0xFF, 3);
    root.flags = 0;
    root.children = 0;
    genpvsnodes(worldroot);

    totalviewcells = countviewcells(worldroot, ivec(0, 0, 0), worldsize>>1, *viewcellsize>0 ? *viewcellsize : 32);
    numviewcells = 0;
    genpvs_canceled = false;
    check_genpvs_progress = false;
    SDL_TimerID timer = NULL;
    if(pvsthreads<=1) 
    {
        pvsworkers.add(new pvsworker);
        timer = SDL_AddTimer(500, genpvs_timer, NULL);
    }
    viewcells = new viewcellnode;
    genviewcells(*viewcells, worldroot, ivec(0, 0, 0), worldsize>>1, *viewcellsize>0 ? *viewcellsize : 32);
    if(pvsthreads<=1)
    {
        SDL_RemoveTimer(timer);
    }
    else
    {
        renderprogress(0, "creating threads");
        if(!pvsmutex) pvsmutex = SDL_CreateMutex();
        if(!viewcellmutex) viewcellmutex = SDL_CreateMutex();
        loopi(pvsthreads)
        {
            pvsworker *w = pvsworkers.add(new pvsworker);
            w->thread = SDL_CreateThread(pvsworker::run, w);
        }
        show_genpvs_progress(0, 0);
        while(!genpvs_canceled)
        {
            SDL_Delay(500);
            SDL_LockMutex(viewcellmutex);
            int unique = pvs.length(), processed = numviewcells, remaining = viewcellrequests.length();
            SDL_UnlockMutex(viewcellmutex);
            show_genpvs_progress(unique, processed);
            if(!remaining) break;
        }        
        SDL_LockMutex(viewcellmutex);
        viewcellrequests.setsize(0);
        SDL_UnlockMutex(viewcellmutex);
        loopv(pvsworkers) SDL_WaitThread(pvsworkers[i]->thread, NULL);
    }
    pvsworkers.deletecontents();

    origpvsnodes.setsize(0);
    pvscompress.clear();

    Uint32 end = SDL_GetTicks();
    if(genpvs_canceled) 
    {
        clearpvs();
        conoutf("genpvs aborted");
    }
    else conoutf("generated %d unique view cells totaling %.1f kB and averaging %d B (%.1f seconds)", 
            pvs.length(), pvsbuf.length()/1024.0f, pvsbuf.length()/max(pvs.length(), 1), (end - start) / 1000.0f);
}

COMMAND(genpvs, "i");

void pvsstats()
{
    conoutf("%d unique view cells totaling %.1f kB and averaging %d B",          
        pvs.length(), pvsbuf.length()/1024.0f, pvsbuf.length()/max(pvs.length(), 1));
}

COMMAND(pvsstats, "");

static inline bool pvsoccluded(uchar *buf, const ivec &co, int size, const ivec &bborigin, const ivec &bbsize)
{
    uchar leafmask = buf[0];
    loopoctabox(co, size, bborigin, bbsize)
    {
        ivec o(i, co.x, co.y, co.z, size);
        if(leafmask&(1<<i))
        {
            uchar leafvalues = buf[1+i];
            if(!leafvalues || (leafvalues!=0xFF && octantrectangleoverlap(o, size>>1, bborigin, bbsize)&~leafvalues))
                return false;
        }
        else if(!pvsoccluded(buf+9*buf[1+i], o, size>>1, bborigin, bbsize)) return false;
    }
    return true;
}

static inline bool pvsoccluded(uchar *buf, const ivec &bborigin, const ivec &bbsize)
{
    int diff = (bborigin.x^(bborigin.x+bbsize.x)) | (bborigin.y^(bborigin.y+bbsize.y)) | (bborigin.z^(bborigin.z+bbsize.z));
    if(diff&~((1<<worldscale)-1)) return false;
    int scale = worldscale-1;
    while(!(diff&(1<<scale)))
    {
        int i = octastep(bborigin.x, bborigin.y, bborigin.z, scale);
        scale--;
        uchar leafmask = buf[0];
        if(leafmask&(1<<i))
        {
            uchar leafvalues = buf[1+i];
            return leafvalues && (leafvalues==0xFF || !(octantrectangleoverlap(ivec(bborigin).mask(~((2<<scale)-1)), 1<<scale, bborigin, bbsize)&~leafvalues));
        }
        buf += 9*buf[1+i];
    }
    return pvsoccluded(buf, ivec(bborigin).mask(~((2<<scale)-1)), 1<<scale, bborigin, bbsize);
}

bool pvsoccluded(const ivec &bborigin, const ivec &bbsize)
{
    return curpvs!=NULL && pvsoccluded(curpvs, bborigin, bbsize);
}

bool waterpvsoccluded(int height)
{
    if(!curwaterpvs) return false;
    if(lockedpvs)
    {
        loopi(MAXWATERPVS) if(lockedwaterplanes[i]==height) return (curwaterpvs&(1<<i))!=0;
    }
    else
    {
        loopi(numwaterplanes) if(waterplanes[i].height==height) return (curwaterpvs&(1<<i))!=0;
    }
    return false;
}

void saveviewcells(stream *f, viewcellnode &p)
{
    f->putchar(p.leafmask);
    loopi(8)
    {
        if(p.leafmask&(1<<i)) f->putlil<int>(p.children[i].pvs);
        else saveviewcells(f, *p.children[i].node);
    }
}

void savepvs(stream *f)
{
    uint totallen = pvsbuf.length() | (numwaterplanes>0 ? 0x80000000U : 0);
    f->putlil<uint>(totallen);
    if(numwaterplanes>0)
    {
        f->putlil<uint>(numwaterplanes);
        loopi(numwaterplanes)
        {
            f->putlil<int>(waterplanes[i].height);
            if(waterplanes[i].height < 0) break;
        }
    }
    loopv(pvs) f->putlil<ushort>(pvs[i].len);
    f->write(pvsbuf.getbuf(), pvsbuf.length());
    saveviewcells(f, *viewcells);
}

viewcellnode *loadviewcells(stream *f)
{
    viewcellnode *p = new viewcellnode;
    p->leafmask = f->getchar();
    loopi(8)
    {
        if(p->leafmask&(1<<i)) p->children[i].pvs = f->getlil<int>();
        else p->children[i].node = loadviewcells(f);
    }
    return p;
}

void loadpvs(stream *f, int numpvs)
{
    uint totallen = f->getlil<uint>();
    if(totallen & 0x80000000U)
    {
        totallen &= ~0x80000000U;
        numwaterplanes = f->getlil<uint>();
        loopi(numwaterplanes) waterplanes[i].height = f->getlil<int>();
    }
    int offset = 0;
    loopi(numpvs)
    {
        ushort len = f->getlil<ushort>();
        pvs.add(pvsdata(offset, len));
        offset += len;
    }
    f->read(pvsbuf.reserve(totallen).buf, totallen);
    pvsbuf.advance(totallen);
    viewcells = loadviewcells(f);
}

int getnumviewcells() { return pvs.length(); }

