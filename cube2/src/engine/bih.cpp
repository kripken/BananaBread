#include "engine.h"

bool BIH::triintersect(tri &t, const vec &o, const vec &ray, float maxdist, float &dist, int mode, tri *noclip)
{
    vec p;
    p.cross(ray, t.c);
    float det = t.b.dot(p);
    if(det == 0) return false;
    vec r(o); 
    r.sub(t.a);
    float u = r.dot(p) / det; 
    if(u < 0 || u > 1) return false;
    vec q; 
    q.cross(r, t.b);
    float v = ray.dot(q) / det;
    if(v < 0 || u + v > 1) return false;
    float f = t.c.dot(q) / det;
    if(f < 0 || f > maxdist) return false;
    if(!(mode&RAY_SHADOW) && &t >= noclip) return false;
    if(t.tex && (mode&RAY_ALPHAPOLY)==RAY_ALPHAPOLY && (t.tex->alphamask || (lightmapping <= 1 && (loadalphamask(t.tex), t.tex->alphamask))))
    {
        int si = clamp(int(t.tex->xs * (t.tc[0] + u*(t.tc[2] - t.tc[0]) + v*(t.tc[4] - t.tc[0]))), 0, t.tex->xs-1),
            ti = clamp(int(t.tex->ys * (t.tc[1] + u*(t.tc[3] - t.tc[1]) + v*(t.tc[5] - t.tc[1]))), 0, t.tex->ys-1);
        if(!(t.tex->alphamask[ti*((t.tex->xs+7)/8) + si/8] & (1<<(si%8)))) return false;
    }
    dist = f;
    return true;
}

struct BIHStack
{
    BIHNode *node;
    float tmin, tmax;
};

inline bool BIH::traverse(const vec &o, const vec &ray, const vec &invray, float maxdist, float &dist, int mode, BIHNode *curnode, float tmin, float tmax)
{
    BIHStack stack[128];
    int stacksize = 0;
    ivec order(ray.x>0 ? 0 : 1, ray.y>0 ? 0 : 1, ray.z>0 ? 0 : 1);
    for(;;)
    {
        int axis = curnode->axis();
        int nearidx = order[axis], faridx = nearidx^1;
        float nearsplit = (curnode->split[nearidx] - o[axis])*invray[axis],
              farsplit = (curnode->split[faridx] - o[axis])*invray[axis];

        if(nearsplit <= tmin)
        {
            if(farsplit < tmax)
            {
                if(!curnode->isleaf(faridx))
                {
                    curnode = &nodes[curnode->childindex(faridx)];
                    tmin = max(tmin, farsplit);
                    continue;
                }
                else if(triintersect(tris[curnode->childindex(faridx)], o, ray, maxdist, dist, mode, noclip)) return true;
            }
        }
        else if(curnode->isleaf(nearidx))
        {
            if(triintersect(tris[curnode->childindex(nearidx)], o, ray, maxdist, dist, mode, noclip)) return true;
            if(farsplit < tmax)
            {
                if(!curnode->isleaf(faridx))
                {
                    curnode = &nodes[curnode->childindex(faridx)];
                    tmin = max(tmin, farsplit);
                    continue;
                }
                else if(triintersect(tris[curnode->childindex(faridx)], o, ray, maxdist, dist, mode, noclip)) return true;
            }
        }
        else
        {
            if(farsplit < tmax)
            {
                if(!curnode->isleaf(faridx))
                {
                    if(stacksize < int(sizeof(stack)/sizeof(stack[0])))
                    {
                        BIHStack &save = stack[stacksize++];
                        save.node = &nodes[curnode->childindex(faridx)];
                        save.tmin = max(tmin, farsplit);
                        save.tmax = tmax;
                    }
                    else 
                    {
                        if(traverse(o, ray, invray, maxdist, dist, mode, &nodes[curnode->childindex(nearidx)], tmin, min(tmax, nearsplit))) return true;
                        curnode = &nodes[curnode->childindex(faridx)];
                        tmin = max(tmin, farsplit);
                        continue;
                    }
                }
                else if(triintersect(tris[curnode->childindex(faridx)], o, ray, maxdist, dist, mode, noclip)) return true;
            }
            curnode = &nodes[curnode->childindex(nearidx)];
            tmax = min(tmax, nearsplit);
            continue;
        }
        if(stacksize <= 0) return false;
        BIHStack &restore = stack[--stacksize];
        curnode = restore.node;
        tmin = restore.tmin;
        tmax = restore.tmax;
    }
}

inline bool BIH::traverse(const vec &o, const vec &ray, float maxdist, float &dist, int mode)
{
    if(!numnodes) return false;

    vec invray(ray.x ? 1/ray.x : 1e16f, ray.y ? 1/ray.y : 1e16f, ray.z ? 1/ray.z : 1e16f);
    float tmin, tmax;
    float t1 = (bbmin.x - o.x)*invray.x,
          t2 = (bbmax.x - o.x)*invray.x;
    if(invray.x > 0) { tmin = t1; tmax = t2; } else { tmin = t2; tmax = t1; }
    t1 = (bbmin.y - o.y)*invray.y;
    t2 = (bbmax.y - o.y)*invray.y;
    if(invray.y > 0) { tmin = max(tmin, t1); tmax = min(tmax, t2); } else { tmin = max(tmin, t2); tmax = min(tmax, t1); }
    t1 = (bbmin.z - o.z)*invray.z;
    t2 = (bbmax.z - o.z)*invray.z;
    if(invray.z > 0) { tmin = max(tmin, t1); tmax = min(tmax, t2); } else { tmin = max(tmin, t2); tmax = min(tmax, t1); }
    if(tmin >= maxdist || tmin>=tmax) return false;
    tmax = min(tmax, maxdist);

    return BIH::traverse(o, ray, invray, maxdist, dist, mode, &nodes[0], tmin, tmax); 
}

void BIH::build(vector<BIHNode> &buildnodes, ushort *indices, int numindices, const vec &vmin, const vec &vmax, int depth)
{
    maxdepth = max(maxdepth, depth);
   
    int axis = 2;
    loopk(2) if(vmax[k] - vmin[k] > vmax[axis] - vmin[axis]) axis = k;

    vec leftmin, leftmax, rightmin, rightmax;
    float splitleft, splitright;
    int left, right;
    loopk(3)
    {
        leftmin = rightmin = vec(1e16f, 1e16f, 1e16f);
        leftmax = rightmax = vec(-1e16f, -1e16f, -1e16f);
        float split = 0.5f*(vmax[axis] + vmin[axis]);
        for(left = 0, right = numindices, splitleft = SHRT_MIN, splitright = SHRT_MAX; left < right;)
        {
            tri &tri = tris[indices[left]];
            float amin = min(tri.a[axis], min(tri.b[axis], tri.c[axis])),
                  amax = max(tri.a[axis], max(tri.b[axis], tri.c[axis]));
            if(max(split - amin, 0.0f) > max(amax - split, 0.0f)) 
            {
                ++left;
                splitleft = max(splitleft, amax);
                leftmin.min(tri.a).min(tri.b).min(tri.c);
                leftmax.max(tri.a).max(tri.b).max(tri.c);
            }
            else 
            {    
                --right; 
                swap(indices[left], indices[right]); 
                splitright = min(splitright, amin);
                rightmin.min(tri.a).min(tri.b).min(tri.c);
                rightmax.max(tri.a).max(tri.b).max(tri.c);
            }
        }
        if(left > 0 && right < numindices) break;
        axis = (axis+1)%3;
    }

    if(!left || right==numindices) 
    {
        leftmin = rightmin = vec(1e16f, 1e16f, 1e16f);
        leftmax = rightmax = vec(-1e16f, -1e16f, -1e16f);
        left = right = numindices/2;
        splitleft = SHRT_MIN;
        splitright = SHRT_MAX;
        loopi(numindices)
        {
            tri &tri = tris[indices[i]];
            if(i < left) 
            {
                splitleft = max(splitleft, max(tri.a[axis], max(tri.b[axis], tri.c[axis])));
                leftmin.min(tri.a).min(tri.b).min(tri.c);
                leftmax.max(tri.a).max(tri.b).max(tri.c);
            }
            else 
            {
                splitright = min(splitright, min(tri.a[axis], min(tri.b[axis], tri.c[axis])));
                rightmin.min(tri.a).min(tri.b).min(tri.c);
                rightmax.max(tri.a).max(tri.b).max(tri.c);
            }
        }
    }

    int node = buildnodes.length();
    buildnodes.add();
    buildnodes[node].split[0] = short(ceil(splitleft));
    buildnodes[node].split[1] = short(floor(splitright));

    if(left==1) buildnodes[node].child[0] = (axis<<14) | indices[0];
    else
    {
        buildnodes[node].child[0] = (axis<<14) | buildnodes.length();
        build(buildnodes, indices, left, leftmin, leftmax, depth+1);
    }

    if(numindices-right==1) buildnodes[node].child[1] = (1<<15) | (left==1 ? 1<<14 : 0) | indices[right];
    else 
    {
        buildnodes[node].child[1] = (left==1 ? 1<<14 : 0) | buildnodes.length();
        build(buildnodes, &indices[right], numindices-right, rightmin, rightmax, depth+1);
    }
}

BIH::BIH(vector<tri> *t)
  : maxdepth(0), numnodes(0), nodes(NULL), numtris(0), tris(NULL), noclip(NULL), bbmin(1e16f, 1e16f, 1e16f), bbmax(-1e16f, -1e16f, -1e16f)
{
    numtris = t[0].length() + t[1].length();
    if(!numtris) return; 

    tris = new tri[numtris];
    noclip = &tris[t[0].length()];
    memcpy(tris, t[0].getbuf(), t[0].length()*sizeof(tri));
    memcpy(noclip, t[1].getbuf(), t[1].length()*sizeof(tri));

    loopi(numtris)
    {
        tri &tri = tris[i];
        bbmin.min(tri.a).min(tri.b).min(tri.c);
        bbmax.max(tri.a).max(tri.b).max(tri.c);
    }
    
    radius = max(max(max(fabs(bbmin.x), fabs(bbmin.y)), fabs(bbmin.z)),
                 max(max(fabs(bbmax.x), fabs(bbmax.y)), fabs(bbmax.z)));
    radius *= radius;

    vector<BIHNode> buildnodes;
    ushort *indices = new ushort[numtris];
    loopi(numtris) indices[i] = i;

    maxdepth = 0;

    build(buildnodes, indices, numtris, bbmin, bbmax);

    delete[] indices;

    numnodes = buildnodes.length();
    nodes = new BIHNode[numnodes];
    memcpy(nodes, buildnodes.getbuf(), numnodes*sizeof(BIHNode));

    // convert tri.b/tri.c to edges
    loopi(numtris)
    {
        tri &tri = tris[i];
        tri.b.sub(tri.a);
        tri.c.sub(tri.a);
    }
}

bool mmintersect(const extentity &e, const vec &o, const vec &ray, float maxdist, int mode, float &dist)
{
    extern vector<mapmodelinfo> mapmodels;
    if(!mapmodels.inrange(e.attr2)) return false;
    model *m = mapmodels[e.attr2].m;
    if(!m)
    {
        m = loadmodel(NULL, e.attr2);
        if(!m) return false;
    }
    if(mode&RAY_SHADOW)
    {
        if(!m->shadow || e.flags&extentity::F_NOSHADOW) return false;
    }
    else if((mode&RAY_ENTS)!=RAY_ENTS && (!m->collide || e.flags&extentity::F_NOCOLLIDE)) return false;
    if(!m->bih && (lightmapping > 1 || !m->setBIH())) return false;
    vec mo = vec(o).sub(e.o), mray(ray);
    float v = mo.dot(mray), inside = m->bih->radius - mo.squaredlen();
    if((inside < 0 && v > 0) || inside + v*v < 0) return false;
    int yaw = e.attr1;
    if(yaw != 0) 
    {
        if(yaw < 0) yaw = 360 + yaw%360;
        else if(yaw >= 360) yaw %= 360;
        const vec2 &rot = sincos360[yaw];
        mo.rotate_around_z(rot.x, -rot.y);
        mray.rotate_around_z(rot.x, -rot.y);
    }
    return m->bih->traverse(mo, mray, maxdist ? maxdist : 1e16f, dist, mode);
}

