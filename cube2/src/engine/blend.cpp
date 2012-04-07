#include "engine.h"

enum
{
    BM_BRANCH = 0,
    BM_SOLID,
    BM_IMAGE
};

struct BlendMapBranch;
struct BlendMapSolid;
struct BlendMapImage;

struct BlendMapNode
{
    union
    {
        BlendMapBranch *branch;
        BlendMapSolid *solid;
        BlendMapImage *image;
    };

    void cleanup(int type);
    void splitsolid(uchar &type, uchar val);
};

struct BlendMapBranch
{
    uchar type[4];
    BlendMapNode children[4];

    ~BlendMapBranch()
    {
        loopi(4) children[i].cleanup(type[i]);
    }

    uchar shrink(BlendMapNode &child, int quadrant);
};

struct BlendMapSolid
{
    uchar val;

    BlendMapSolid(uchar val) : val(val) {}
};

#define BM_SCALE 1
#define BM_IMAGE_SIZE 64

struct BlendMapImage
{
    uchar data[BM_IMAGE_SIZE*BM_IMAGE_SIZE];
};

void BlendMapNode::cleanup(int type)
{
    switch(type)
    {
        case BM_BRANCH: delete branch; break;
        case BM_IMAGE: delete image; break;
    }
}

#define DEFBMSOLIDS(n) n, n+1, n+2, n+3, n+4, n+5, n+6, n+7, n+8, n+9, n+10, n+11, n+12, n+13, n+14, n+15

static BlendMapSolid bmsolids[256] = 
{
    DEFBMSOLIDS(0x00), DEFBMSOLIDS(0x10), DEFBMSOLIDS(0x20), DEFBMSOLIDS(0x30),
    DEFBMSOLIDS(0x40), DEFBMSOLIDS(0x50), DEFBMSOLIDS(0x60), DEFBMSOLIDS(0x70),
    DEFBMSOLIDS(0x80), DEFBMSOLIDS(0x90), DEFBMSOLIDS(0xA0), DEFBMSOLIDS(0xB0),
    DEFBMSOLIDS(0xC0), DEFBMSOLIDS(0xD0), DEFBMSOLIDS(0xE0), DEFBMSOLIDS(0xF0),
};

void BlendMapNode::splitsolid(uchar &type, uchar val)
{
    cleanup(type);
    type = BM_BRANCH;
    branch = new BlendMapBranch;
    loopi(4)
    {
        branch->type[i] = BM_SOLID;
        branch->children[i].solid = &bmsolids[val];
    }
}

uchar BlendMapBranch::shrink(BlendMapNode &child, int quadrant)
{
    loopi(4) if(i != quadrant) children[i].cleanup(type[i]);
    uchar childtype = type[quadrant];
    child = children[quadrant];
    type[quadrant] = BM_SOLID;
    children[quadrant].solid = &bmsolids[0];
    return childtype;
}

struct BlendMapRoot : BlendMapNode
{
    uchar type;

    BlendMapRoot() : type(BM_SOLID) { solid = &bmsolids[0xFF]; }
    BlendMapRoot(uchar type, const BlendMapNode &node) : BlendMapNode(node), type(type) {}

    void cleanup() { BlendMapNode::cleanup(type); }

    void shrink(int quadrant)
    {
        if(type == BM_BRANCH) 
        {
            BlendMapRoot oldroot = *this;
            type = branch->shrink(*this, quadrant);
            oldroot.cleanup();
        }
    }
};

static BlendMapRoot blendmap;

struct BlendMapCache
{
    BlendMapRoot node;
    int scale;
    ivec origin;
};

BlendMapCache *newblendmapcache() { return new BlendMapCache; }

void freeblendmapcache(BlendMapCache *&cache) { delete cache; cache = NULL; }

bool setblendmaporigin(BlendMapCache *cache, const ivec &o, int size)
{
    if(blendmap.type!=BM_BRANCH)
    {
        cache->node = blendmap;
        cache->scale = worldscale-BM_SCALE;
        cache->origin = ivec(0, 0, 0);
        return cache->node.solid!=&bmsolids[0xFF];
    }

    BlendMapBranch *bm = blendmap.branch;
    int bmscale = worldscale-BM_SCALE, bmsize = 1<<bmscale,
        x = o.x>>BM_SCALE, y = o.y>>BM_SCALE,
        x1 = max(x-1, 0), y1 = max(y-1, 0),
        x2 = min(((o.x + size + (1<<BM_SCALE)-1)>>BM_SCALE) + 1, bmsize),
        y2 = min(((o.y + size + (1<<BM_SCALE)-1)>>BM_SCALE) + 1, bmsize),
        diff = (x1^x2)|(y1^y2);
    if(diff < bmsize) while(!(diff&(1<<(bmscale-1))))
    {
        bmscale--;
        int n = (((y1>>bmscale)&1)<<1) | ((x1>>bmscale)&1);
        if(bm->type[n]!=BM_BRANCH)
        {
            cache->node = BlendMapRoot(bm->type[n], bm->children[n]);
            cache->scale = bmscale;
            cache->origin = ivec(x1&(~0U<<bmscale), y1&(~0U<<bmscale), 0);
            return cache->node.solid!=&bmsolids[0xFF];
        }
        bm = bm->children[n].branch;
    }

    cache->node.type = BM_BRANCH;
    cache->node.branch = bm;
    cache->scale = bmscale;
    cache->origin = ivec(x1&(~0U<<bmscale), y1&(~0U<<bmscale), 0);
    return true;
}

bool hasblendmap(BlendMapCache *cache)
{
    return cache->node.solid!=&bmsolids[0xFF];
}

static uchar lookupblendmap(int x, int y, BlendMapBranch *bm, int bmscale)
{
    for(;;)
    {
        bmscale--;
        int n = (((y>>bmscale)&1)<<1) | ((x>>bmscale)&1);
        switch(bm->type[n])
        {
            case BM_SOLID: return bm->children[n].solid->val;
            case BM_IMAGE: return bm->children[n].image->data[(y&((1<<bmscale)-1))*BM_IMAGE_SIZE + (x&((1<<bmscale)-1))];
        }
        bm = bm->children[n].branch;
    }
}
    
uchar lookupblendmap(BlendMapCache *cache, const vec &pos)
{
    if(cache->node.type==BM_SOLID) return cache->node.solid->val;
    
    uchar vals[4], *val = vals;
    float bx = pos.x/(1<<BM_SCALE) - 0.5f, by = pos.y/(1<<BM_SCALE) - 0.5f;
    int ix = (int)floor(bx), iy = (int)floor(by),
        rx = ix-cache->origin.x, ry = iy-cache->origin.y;
    loop(vy, 2) loop(vx, 2)
    {
        int cx = clamp(rx+vx, 0, (1<<cache->scale)-1), cy = clamp(ry+vy, 0, (1<<cache->scale)-1);
        if(cache->node.type==BM_IMAGE)
            *val++ = cache->node.image->data[cy*BM_IMAGE_SIZE + cx];
        else *val++ = lookupblendmap(cx, cy, cache->node.branch, cache->scale);
    }
    float fx = bx - ix, fy = by - iy;
    return uchar((1-fy)*((1-fx)*vals[0] + fx*vals[1]) +
                 fy*((1-fx)*vals[2] + fx*vals[3]));
}

static void fillblendmap(uchar &type, BlendMapNode &node, int size, uchar val, int x1, int y1, int x2, int y2)
{
    if(max(x1, y1) <= 0 && min(x2, y2) >= size)
    {
        node.cleanup(type);
        type = BM_SOLID;
        node.solid = &bmsolids[val];
        return;
    }

    if(type==BM_BRANCH)
    {
        size /= 2;
        if(y1 < size)
        {
            if(x1 < size) fillblendmap(node.branch->type[0], node.branch->children[0], size, val,
                                        x1, y1, min(x2, size), min(y2, size));
            if(x2 > size) fillblendmap(node.branch->type[1], node.branch->children[1], size, val, 
                                        max(x1-size, 0), y1, x2-size, min(y2, size));
        }
        if(y2 > size)
        {
            if(x1 < size) fillblendmap(node.branch->type[2], node.branch->children[2], size, val,
                                        x1, max(y1-size, 0), min(x2, size), y2-size);
            if(x2 > size) fillblendmap(node.branch->type[3], node.branch->children[3], size, val,
                                        max(x1-size, 0), max(y1-size, 0), x2-size, y2-size);
        }
        loopi(4) if(node.branch->type[i]!=BM_SOLID || node.branch->children[i].solid->val!=val) return;
        node.cleanup(type);
        type = BM_SOLID;
        node.solid = &bmsolids[val];
        return;
    }             
    else if(type==BM_SOLID)
    {
        uchar oldval = node.solid->val;
        if(oldval==val) return;

        if(size > BM_IMAGE_SIZE)
        {
            node.splitsolid(type, oldval);
            fillblendmap(type, node, size, val, x1, y1, x2, y2);
            return;
        }
 
        type = BM_IMAGE;
        node.image = new BlendMapImage;
        memset(node.image->data, oldval, sizeof(node.image->data));
    }
    
    uchar *dst = &node.image->data[y1*BM_IMAGE_SIZE + x1];
    loopi(y2-y1)
    {
        memset(dst, val, x2-x1);
        dst += BM_IMAGE_SIZE;
    }
}

void fillblendmap(int x, int y, int w, int h, uchar val)
{
    int bmsize = worldsize>>BM_SCALE,
        x1 = clamp(x, 0, bmsize),
        y1 = clamp(y, 0, bmsize),
        x2 = clamp(x+w, 0, bmsize),
        y2 = clamp(y+h, 0, bmsize);
    if(max(x1, y1) >= bmsize || min(x2, y2) <= 0 || x1>=x2 || y1>=y2) return;
    fillblendmap(blendmap.type, blendmap, bmsize, val, x1, y1, x2, y2);
}

static void invertblendmap(uchar &type, BlendMapNode &node, int size, int x1, int y1, int x2, int y2)
{
    if(type==BM_BRANCH)
    {
        size /= 2;
        if(y1 < size)
        {
            if(x1 < size) invertblendmap(node.branch->type[0], node.branch->children[0], size,
                                        x1, y1, min(x2, size), min(y2, size));
            if(x2 > size) invertblendmap(node.branch->type[1], node.branch->children[1], size,
                                        max(x1-size, 0), y1, x2-size, min(y2, size));
        }
        if(y2 > size)
        {
            if(x1 < size) invertblendmap(node.branch->type[2], node.branch->children[2], size,
                                        x1, max(y1-size, 0), min(x2, size), y2-size);
            if(x2 > size) invertblendmap(node.branch->type[3], node.branch->children[3], size,
                                        max(x1-size, 0), max(y1-size, 0), x2-size, y2-size);
        }
        return;
    }
    else if(type==BM_SOLID)
    {
        fillblendmap(type, node, size, 255-node.solid->val, x1, y1, x2, y2);
    }
    else if(type==BM_IMAGE)
    {
        uchar *dst = &node.image->data[y1*BM_IMAGE_SIZE + x1];
        loopi(y2-y1)
        {
            loopj(x2-x1) dst[j] = 255-dst[j];
            dst += BM_IMAGE_SIZE;
        }
    }
}

void invertblendmap(int x, int y, int w, int h)
{
    int bmsize = worldsize>>BM_SCALE,
        x1 = clamp(x, 0, bmsize),
        y1 = clamp(y, 0, bmsize),
        x2 = clamp(x+w, 0, bmsize),
        y2 = clamp(y+h, 0, bmsize);
    if(max(x1, y1) >= bmsize || min(x2, y2) <= 0 || x1>=x2 || y1>=y2) return;
    invertblendmap(blendmap.type, blendmap, bmsize, x1, y1, x2, y2);
}

static void optimizeblendmap(uchar &type, BlendMapNode &node)
{
    switch(type)
    {
        case BM_IMAGE:
        {
            uint val = node.image->data[0];
            val |= val<<8;
            val |= val<<16;
            for(uint *data = (uint *)node.image->data, *end = &data[sizeof(node.image->data)/sizeof(uint)]; data < end; data++) 
                if(*data != val) return;
            node.cleanup(type);
            type = BM_SOLID;
            node.solid = &bmsolids[val&0xFF];
            break;
        }
        case BM_BRANCH:
        {
            loopi(4) optimizeblendmap(node.branch->type[i], node.branch->children[i]);
            if(node.branch->type[3]!=BM_SOLID) return;
            uint val = node.branch->children[3].solid->val;
            loopi(3) if(node.branch->type[i]!=BM_SOLID || node.branch->children[i].solid->val != val) return;
            node.cleanup(type);
            type = BM_SOLID;
            node.solid = &bmsolids[val];
            break;
        }
    }
}

void optimizeblendmap()
{
    optimizeblendmap(blendmap.type, blendmap);
}

VARF(blendpaintmode, 0, 0, 5,
{
    if(!blendpaintmode) stoppaintblendmap();
});

static void blitblendmap(uchar &type, BlendMapNode &node, int bmx, int bmy, int bmsize, uchar *src, int sx, int sy, int sw, int sh)
{
    if(type==BM_BRANCH)
    {
        bmsize /= 2;
        if(sy < bmy + bmsize)
        {
            if(sx < bmx + bmsize) blitblendmap(node.branch->type[0], node.branch->children[0], bmx, bmy, bmsize, src, sx, sy, sw, sh);
            if(sx + sw > bmx + bmsize) blitblendmap(node.branch->type[1], node.branch->children[1], bmx+bmsize, bmy, bmsize, src, sx, sy, sw, sh);
        }
        if(sy + sh > bmy + bmsize)
        {
            if(sx < bmx + bmsize) blitblendmap(node.branch->type[2], node.branch->children[2], bmx, bmy+bmsize, bmsize, src, sx, sy, sw, sh);
            if(sx + sw > bmx + bmsize) blitblendmap(node.branch->type[3], node.branch->children[3], bmx+bmsize, bmy+bmsize, bmsize, src, sx, sy, sw, sh);
        }
        return;
    }
    if(type==BM_SOLID)
    {
        uchar val = node.solid->val;
        if(bmsize > BM_IMAGE_SIZE)
        {
            node.splitsolid(type, val);
            blitblendmap(type, node, bmx, bmy, bmsize, src, sx, sy, sw, sh);
            return;
        }

        type = BM_IMAGE;
        node.image = new BlendMapImage;
        memset(node.image->data, val, sizeof(node.image->data));
    }

    int x1 = clamp(sx - bmx, 0, bmsize), y1 = clamp(sy - bmy, 0, bmsize),
        x2 = clamp(sx+sw - bmx, 0, bmsize), y2 = clamp(sy+sh - bmy, 0, bmsize);
    uchar *dst = &node.image->data[y1*BM_IMAGE_SIZE + x1];
    src += max(bmy - sy, 0)*sw + max(bmx - sx, 0);
    loopi(y2-y1)
    {
        switch(blendpaintmode)
        {
            case 1:
                memcpy(dst, src, x2 - x1);
                break;

            case 2:
                loopi(x2 - x1) dst[i] = min(dst[i], src[i]); 
                break;

            case 3:
                loopi(x2 - x1) dst[i] = max(dst[i], src[i]);
                break;

            case 4:
                loopi(x2 - x1) dst[i] = min(dst[i], uchar(0xFF - src[i]));
                break;

            case 5:
                loopi(x2 - x1) dst[i] = max(dst[i], uchar(0xFF - src[i]));
                break;
        }
        dst += BM_IMAGE_SIZE;
        src += sw;
    } 
}

void blitblendmap(uchar *src, int sx, int sy, int sw, int sh)
{
    int bmsize = worldsize>>BM_SCALE;
    if(max(sx, sy) >= bmsize || min(sx+sw, sy+sh) <= 0 || min(sw, sh) <= 0) return;
    blitblendmap(blendmap.type, blendmap, 0, 0, bmsize, src, sx, sy, sw, sh);
}
        
void resetblendmap()
{
    blendmap.cleanup();
    blendmap.type = BM_SOLID;
    blendmap.solid = &bmsolids[0xFF];
}

void enlargeblendmap()
{
    if(blendmap.type == BM_SOLID) return;
    BlendMapBranch *branch = new BlendMapBranch;
    branch->type[0] = blendmap.type;
    branch->children[0] = blendmap;
    loopi(3)
    {
        branch->type[i+1] = BM_SOLID;
        branch->children[i+1].solid = &bmsolids[0xFF];
    }
    blendmap.type = BM_BRANCH;
    blendmap.branch = branch;
}

void shrinkblendmap(int octant)
{
    blendmap.shrink(octant&3);
}
 
struct BlendBrush
{
    char *name;
    int w, h;
    uchar *data;
    GLuint tex;
   
    BlendBrush(const char *name, int w, int h) :
      name(newstring(name)), w(w), h(h), data(new uchar[w*h]), tex(0)
    {}

    ~BlendBrush()
    {
        cleanup();
        delete[] name;
        if(data) delete[] data;
    }

    void cleanup()
    {
        if(tex) { glDeleteTextures(1, &tex); tex = 0; }
    }

    void gentex()
    {
        if(!tex) glGenTextures(1, &tex);
        uchar *buf = new uchar[2*(w+2)*(h+2)];
        memset(buf, 0, 2*(w+2));
        uchar *dst = &buf[2*(w+2)], *src = data;
        loopi(h)
        {
            *dst++ = 0; 
            *dst++ = 0;
            loopj(w)
            {
                *dst++ = 255 - *src;
                *dst++ = 255 - *src++;
            }
            *dst++ = 0;
            *dst++ = 0;
        }
        memset(dst, 0, 2*(w+2));
        createtexture(tex, w+2, h+2, buf, 3, 1, GL_LUMINANCE_ALPHA);
        delete[] buf;
    }
    
    void reorient(bool flipx, bool flipy, bool swapxy)
    {
        uchar *rdata = new uchar[w*h];
        int stridex = 1, stridey = 1;
        if(swapxy) stridex *= h; else stridey *= w;
        uchar *src = data, *dst = rdata;
        if(flipx) { dst += (w-1)*stridex; stridex = -stridex; }
        if(flipy) { dst += (h-1)*stridey; stridey = -stridey; }
        loopi(h)
        {
            uchar *curdst = dst;
            loopj(w) 
            {
                *curdst = *src++;
                curdst += stridex;
            }
            dst += stridey;
        }
        if(swapxy) swap(w, h);
        delete[] data;
        data = rdata;
        if(tex) gentex();
    } 
};

static vector<BlendBrush *> brushes;
static int curbrush = -1;

void cleanupblendmap()
{
    loopv(brushes) brushes[i]->cleanup();
}

void clearblendbrushes()
{
    while(brushes.length()) delete brushes.pop();
    curbrush = -1;
}

void delblendbrush(const char *name)
{
    loopv(brushes) if(!strcmp(brushes[i]->name, name)) 
    {
        delete brushes[i];
        brushes.remove(i--);
    }
    curbrush = brushes.empty() ? -1 : clamp(curbrush, 0, brushes.length()-1);
}

void addblendbrush(const char *name, const char *imgname)
{
    delblendbrush(name);

    ImageData s;
    if(!loadimage(imgname, s)) { conoutf(CON_ERROR, "could not load blend brush image %s", imgname); return; }
    if(max(s.w, s.h) > (1<<12))
    {
        conoutf(CON_ERROR, "blend brush image size exceeded %dx%d pixels: %s", 1<<12, 1<<12, imgname);
        return;
    }
    
    BlendBrush *brush = new BlendBrush(name, s.w, s.h);

    uchar *dst = brush->data, *srcrow = s.data;
    loopi(s.h)
    {
        for(uchar *src = srcrow, *end = &srcrow[s.w*s.bpp]; src < end; src += s.bpp)
            *dst++ = src[0];
        srcrow += s.pitch;
    }

    brushes.add(brush);
    if(curbrush < 0) curbrush = 0;
    else if(curbrush >= brushes.length()) curbrush = brushes.length()-1;

}

void nextblendbrush(int *dir)
{
    curbrush += *dir < 0 ? -1 : 1;
    if(brushes.empty()) curbrush = -1;
    else if(!brushes.inrange(curbrush)) curbrush = *dir < 0 ? brushes.length()-1 : 0;
}

void setblendbrush(const char *name)
{
    loopv(brushes) if(!strcmp(brushes[i]->name, name)) { curbrush = i; break; }
}

void getblendbrushname(int *n)
{
    result(brushes.inrange(*n) ? brushes[*n]->name : "");
}

void curblendbrush()
{
    intret(curbrush);
}

COMMAND(clearblendbrushes, "");
COMMAND(delblendbrush, "s");
COMMAND(addblendbrush, "ss");
COMMAND(nextblendbrush, "i");
COMMAND(setblendbrush, "s");
COMMAND(getblendbrushname, "i");
COMMAND(curblendbrush, "");

extern int nompedit;

bool canpaintblendmap(bool brush = true, bool sel = false, bool msg = true)
{
    if(noedit(!sel, msg) || (nompedit && multiplayer())) return false;
    if(!blendpaintmode)
    {
        if(msg) conoutf(CON_ERROR, "operation only allowed in blend paint mode");
        return false;
    }
    if(brush && !brushes.inrange(curbrush))
    {
        if(msg) conoutf(CON_ERROR, "no blend brush selected");
        return false;
    }
    return true;
}

void rotateblendbrush(int *val)
{
    if(!canpaintblendmap()) return;
    
    int numrots = *val < 0 ? 3 : clamp(*val, 1, 5);
    BlendBrush *brush = brushes[curbrush];
    brush->reorient(numrots>=2 && numrots<=4, numrots<=2 || numrots==5, (numrots&5)==1);
}

COMMAND(rotateblendbrush, "i");

void paintblendmap(bool msg)
{
    if(!canpaintblendmap(true, false, msg)) return;

    BlendBrush *brush = brushes[curbrush];
    int x = (int)floor(clamp(worldpos.x, 0.0f, float(worldsize))/(1<<BM_SCALE) - 0.5f*brush->w),
        y = (int)floor(clamp(worldpos.y, 0.0f, float(worldsize))/(1<<BM_SCALE) - 0.5f*brush->h);
    blitblendmap(brush->data, x, y, brush->w, brush->h);
    previewblends(ivec((x-1)<<BM_SCALE, (y-1)<<BM_SCALE, 0),
                  ivec((brush->w+2)<<BM_SCALE, (brush->h+2)<<BM_SCALE, worldsize));
}

VAR(paintblendmapdelay, 1, 500, 3000);
VAR(paintblendmapinterval, 1, 30, 3000);

int paintingblendmap = 0, lastpaintblendmap = 0;

void stoppaintblendmap()
{
    paintingblendmap = 0;
    lastpaintblendmap = 0;
}

void trypaintblendmap()
{
    if(!paintingblendmap || totalmillis - paintingblendmap < paintblendmapdelay) return;
    if(lastpaintblendmap)
    {
        int diff = totalmillis - lastpaintblendmap;
        if(diff < paintblendmapinterval) return;
        lastpaintblendmap = (diff - diff%paintblendmapinterval) + lastpaintblendmap;
    }
    else lastpaintblendmap = totalmillis;
    paintblendmap(false);
}

ICOMMAND(paintblendmap, "D", (int *isdown),
{
    if(*isdown)
    {
        if(!paintingblendmap) { paintblendmap(true); paintingblendmap = totalmillis; }
    }
    else stoppaintblendmap();
});
    
void clearblendmapsel()
{
    if(noedit(false) || (nompedit && multiplayer())) return;
    extern selinfo sel;
    int x1 = sel.o.x>>BM_SCALE, y1 = sel.o.y>>BM_SCALE,
        x2 = (sel.o.x+sel.s.x*sel.grid+(1<<BM_SCALE)-1)>>BM_SCALE,
        y2 = (sel.o.y+sel.s.y*sel.grid+(1<<BM_SCALE)-1)>>BM_SCALE;
    fillblendmap(x1, y1, x2-x1, y2-y1, 0xFF);
    previewblends(ivec(x1<<BM_SCALE, y1<<BM_SCALE, 0),
                  ivec((x2-x1)<<BM_SCALE, (y2-y1)<<BM_SCALE, worldsize));
}

COMMAND(clearblendmapsel, "");

void invertblendmapsel()
{
    if(noedit(false) || (nompedit && multiplayer())) return;
    extern selinfo sel;
    int x1 = sel.o.x>>BM_SCALE, y1 = sel.o.y>>BM_SCALE,
        x2 = (sel.o.x+sel.s.x*sel.grid+(1<<BM_SCALE)-1)>>BM_SCALE,
        y2 = (sel.o.y+sel.s.y*sel.grid+(1<<BM_SCALE)-1)>>BM_SCALE;
    invertblendmap(x1, y1, x2-x1, y2-y1);
    previewblends(ivec(x1<<BM_SCALE, y1<<BM_SCALE, 0),
                  ivec((x2-x1)<<BM_SCALE, (y2-y1)<<BM_SCALE, worldsize));
}

COMMAND(invertblendmapsel, "");

void invertblendmap()
{
    if(noedit(false) || (nompedit && multiplayer())) return;
    invertblendmap(0, 0, worldsize>>BM_SCALE, worldsize>>BM_SCALE);
    previewblends(ivec(0, 0, 0), ivec(worldsize, worldsize, worldsize));
}

COMMAND(invertblendmap, "");

void showblendmap()
{
    if(noedit(true) || (nompedit && multiplayer())) return;
    previewblends(ivec(0, 0, 0), ivec(worldsize, worldsize, worldsize));
}

COMMAND(showblendmap, "");
COMMAND(optimizeblendmap, "");
ICOMMAND(clearblendmap, "", (),
{
    if(noedit(true) || (nompedit && multiplayer())) return;
    resetblendmap();
    showblendmap();
});

void renderblendbrush()
{
    if(!blendpaintmode || !brushes.inrange(curbrush)) return;

    BlendBrush *brush = brushes[curbrush];
    int x1 = (int)floor(clamp(worldpos.x, 0.0f, float(worldsize))/(1<<BM_SCALE) - 0.5f*(brush->w+2)) << BM_SCALE,
        y1 = (int)floor(clamp(worldpos.y, 0.0f, float(worldsize))/(1<<BM_SCALE) - 0.5f*(brush->h+2)) << BM_SCALE,
        x2 = x1 + ((brush->w+2) << BM_SCALE),
        y2 = y1 + ((brush->h+2) << BM_SCALE);

    if(max(x1, y1) >= worldsize || min(x2, y2) <= 0 || x1>=x2 || y1>=y2) return;

    if(!brush->tex) brush->gentex();
    renderblendbrush(brush->tex, x1, y1, x2 - x1, y2 - y1);
}

bool loadblendmap(stream *f, uchar &type, BlendMapNode &node)
{
    type = f->getchar();
    switch(type)
    {
        case BM_SOLID:
        {
            int val = f->getchar();
            if(val<0 || val>0xFF) return false;
            node.solid = &bmsolids[val];
            break;
        }

        case BM_IMAGE:
            node.image = new BlendMapImage;
            if(f->read(node.image->data, sizeof(node.image->data)) != sizeof(node.image->data))
                return false;
            break;

        case BM_BRANCH:
            node.branch = new BlendMapBranch;
            loopi(4) { node.branch->type[i] = BM_SOLID; node.branch->children[i].solid = &bmsolids[0xFF]; }
            loopi(4) if(!loadblendmap(f, node.branch->type[i], node.branch->children[i]))
                return false;
            break;

        default:
            type = BM_SOLID;
            node.solid = &bmsolids[0xFF];
            return false;
    }
    return true;
}

bool loadblendmap(stream *f, int info)
{
    resetblendmap();
    return loadblendmap(f, blendmap.type, blendmap);
}

void saveblendmap(stream *f, uchar type, BlendMapNode &node)
{
    f->putchar(type);
    switch(type)
    {
        case BM_SOLID:
            f->putchar(node.solid->val);
            break;
        
        case BM_IMAGE:
            f->write(node.image->data, sizeof(node.image->data));
            break;

        case BM_BRANCH:
            loopi(4) saveblendmap(f, node.branch->type[i], node.branch->children[i]);
            break;
    }
}

void saveblendmap(stream *f)
{
    saveblendmap(f, blendmap.type, blendmap);
}

uchar shouldsaveblendmap()
{
    return blendmap.solid!=&bmsolids[0xFF] ? 1 : 0;
}
    
