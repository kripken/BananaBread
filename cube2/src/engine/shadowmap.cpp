#include "engine.h"
#include "rendertarget.h"

VARP(shadowmap, 0, 0, 1);

extern void cleanshadowmap();
VARFP(shadowmapsize, 7, 9, 11, cleanshadowmap());
VARP(shadowmapradius, 64, 96, 256);
VAR(shadowmapheight, 0, 32, 128);
VARP(ffshadowmapdist, 128, 1024, 4096);
VARP(shadowmapdist, 128, 256, 512);
VARFP(fpshadowmap, 0, 0, 1, cleanshadowmap());
VARFP(shadowmapprecision, 0, 0, 1, cleanshadowmap());
bvec shadowmapambientcolor(0, 0, 0);
HVARFR(shadowmapambient, 0, 0, 0xFFFFFF,
{
    if(shadowmapambient <= 255) shadowmapambient |= (shadowmapambient<<8) | (shadowmapambient<<16);
    shadowmapambientcolor = bvec((shadowmapambient>>16)&0xFF, (shadowmapambient>>8)&0xFF, shadowmapambient&0xFF);
});
VARP(shadowmapintensity, 0, 40, 100);

VARP(blurshadowmap, 0, 1, 3);
VARP(blursmsigma, 1, 100, 200);

#define SHADOWSKEW 0.7071068f

vec shadowoffset(0, 0, 0), shadowfocus(0, 0, 0), shadowdir(0, SHADOWSKEW, 1);
VAR(shadowmapcasters, 1, 0, 0);
float shadowmapmaxz = 0;

void setshadowdir(int angle)
{
    shadowdir = vec(0, SHADOWSKEW, 1);
    shadowdir.rotate_around_z(angle*RAD);
}

VARFR(shadowmapangle, 0, 0, 360, setshadowdir(shadowmapangle));

void guessshadowdir()
{
    if(shadowmapangle) return;
    vec dir;
    extern int sunlight;
    extern vec sunlightdir;
    if(sunlight) dir = sunlightdir;
    else
    {
        vec lightpos(0, 0, 0), casterpos(0, 0, 0);
        int numlights = 0, numcasters = 0;
        const vector<extentity *> &ents = entities::getents();
        loopv(ents)
        {
            extentity &e = *ents[i];
            switch(e.type)
            {
                case ET_LIGHT:
                    if(!e.attr1) { lightpos.add(e.o); numlights++; }
                    break;

                case ET_MAPMODEL:
                    casterpos.add(e.o);
                    numcasters++;
                    break;

                default:
                    if(e.type<ET_GAMESPECIFIC) break;
                    casterpos.add(e.o);
                    numcasters++;
                    break;
            }
        }
        if(!numlights || !numcasters) return;
        lightpos.div(numlights);
        casterpos.div(numcasters);
        dir = vec(lightpos).sub(casterpos);
    }
    dir.z = 0;
    if(dir.iszero()) return;
    dir.normalize();
    dir.mul(SHADOWSKEW);
    dir.z = 1;
    shadowdir = dir;
}

bool shadowmapping = false;

static glmatrixf shadowmapmatrix;

VARP(shadowmapbias, 0, 5, 1024);
VARP(shadowmappeelbias, 0, 20, 1024);
VAR(smdepthpeel, 0, 1, 1);
VAR(smoothshadowmappeel, 1, 0, 0);

static struct shadowmaptexture : rendertarget
{
    GLenum attachment() const
    {
        return renderpath==R_FIXEDFUNCTION ? GL_DEPTH_ATTACHMENT_EXT : GL_COLOR_ATTACHMENT0_EXT;
    }

    const GLenum *colorformats() const
    {
        if(renderpath==R_FIXEDFUNCTION) 
        {
            static const GLenum depthtexfmts[] = { GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT32, GL_FALSE };
            return depthtexfmts;
        }
       
        static const GLenum rgbfmts[] = { GL_RGB, GL_RGB8, GL_FALSE }, rgbafmts[] = { GL_RGBA16F_ARB, GL_RGBA16, GL_RGBA, GL_RGBA8, GL_FALSE };
        return hasFBO ? &rgbafmts[fpshadowmap && hasTF ? 0 : (shadowmapprecision ? 1 : 2)] : rgbfmts;
    }

    bool shadowcompare() const { return renderpath==R_FIXEDFUNCTION; }
    bool filter() const { return renderpath!=R_FIXEDFUNCTION || hasNVPCF; }
    bool swaptexs() const { return renderpath!=R_FIXEDFUNCTION; }

    bool scissorblur(int &x, int &y, int &w, int &h)
    {
        x = max(int(floor((scissorx1+1)/2*vieww)) - 2*blursize, 2);
        y = max(int(floor((scissory1+1)/2*viewh)) - 2*blursize, 2);
        w = min(int(ceil((scissorx2+1)/2*vieww)) + 2*blursize, vieww-2) - x;
        h = min(int(ceil((scissory2+1)/2*viewh)) + 2*blursize, viewh-2) - y;
        return true;
    }

    bool scissorrender(int &x, int &y, int &w, int &h)
    {
        x = y = 2;
        w = vieww - 2*2;
        h = viewh - 2*2;
        return true;
    }

    void doclear()
    {
        if(!hasFBO && rtscissor)
        {
            glEnable(GL_SCISSOR_TEST);
            glScissor(screen->w-vieww, screen->h-viewh, vieww, viewh);
        }
        glClearColor(0, 0, 0, 0);
        glClear(GL_DEPTH_BUFFER_BIT | (renderpath!=R_FIXEDFUNCTION ? GL_COLOR_BUFFER_BIT : 0));
        if(!hasFBO && rtscissor) glDisable(GL_SCISSOR_TEST);
    }

    bool dorender()
    {
        // nvidia bug, must push modelview here, then switch to projection, then back to modelview before can safely modify it
        glPushMatrix();

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(-shadowmapradius, shadowmapradius, -shadowmapradius, shadowmapradius, 
            renderpath==R_FIXEDFUNCTION ? 0 : -shadowmapdist, 
            renderpath==R_FIXEDFUNCTION ? ffshadowmapdist : shadowmapdist);

        glMatrixMode(GL_MODELVIEW);

        vec skewdir(shadowdir);
        skewdir.rotate_around_z(-camera1->yaw*RAD);

        vec dir;
        vecfromyawpitch(camera1->yaw, camera1->pitch, 1, 0, dir);
        dir.z = 0;
        dir.mul(shadowmapradius);

        vec dirx, diry;
        vecfromyawpitch(camera1->yaw, 0, 0, 1, dirx);
        vecfromyawpitch(camera1->yaw, 0, 1, 0, diry);
        shadowoffset.x = -fmod(dirx.dot(camera1->o) - skewdir.x*camera1->o.z, 2.0f*shadowmapradius/vieww);
        shadowoffset.y = -fmod(diry.dot(camera1->o) - skewdir.y*camera1->o.z, 2.0f*shadowmapradius/viewh);

        GLfloat skew[] =
        {
            1, 0, 0, 0,
            0, 1, 0, 0,
            skewdir.x, skewdir.y, 1, 0,
            0, 0, 0, 1
        };
        glLoadMatrixf(skew);
        glTranslatef(skewdir.x*shadowmapheight + shadowoffset.x, skewdir.y*shadowmapheight + shadowoffset.y + dir.magnitude(), -shadowmapheight);
        glRotatef(camera1->yaw+180, 0, 0, -1);
        glTranslatef(-camera1->o.x, -camera1->o.y, -camera1->o.z);
        shadowfocus = camera1->o;
        shadowfocus.add(dir);
        shadowfocus.add(vec(shadowdir).mul(shadowmapheight));
        shadowfocus.add(dirx.mul(shadowoffset.x));
        shadowfocus.add(diry.mul(shadowoffset.y));

        glmatrixf proj, mv;
        glGetFloatv(GL_PROJECTION_MATRIX, proj.v);
        glGetFloatv(GL_MODELVIEW_MATRIX, mv.v);
        shadowmapmatrix.mul(proj, mv);
        if(renderpath==R_FIXEDFUNCTION) shadowmapmatrix.projective();
        else shadowmapmatrix.projective(-1, 1-shadowmapbias/float(shadowmapdist));

        glColor3f(0, 0, 0);
        glDisable(GL_TEXTURE_2D);

        if(renderpath!=R_FIXEDFUNCTION) setenvparamf("shadowmapbias", SHPARAM_VERTEX, 0, -shadowmapbias/float(shadowmapdist), 1 - (shadowmapbias + (smoothshadowmappeel ? 0 : shadowmappeelbias))/float(shadowmapdist));
        else glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        shadowmapcasters = 0;
        shadowmapmaxz = shadowfocus.z - shadowmapdist;
        shadowmapping = true;
        rendergame();
        shadowmapping = false;
        shadowmapmaxz = min(shadowmapmaxz, shadowfocus.z);

        glEnable(GL_TEXTURE_2D);

        if(renderpath==R_FIXEDFUNCTION) glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        else if(shadowmapcasters && smdepthpeel) 
        {
            int sx, sy, sw, sh;
            bool scissoring = rtscissor && scissorblur(sx, sy, sw, sh) && sw > 0 && sh > 0;
            if(scissoring) 
            {
                if(!hasFBO)
                {
                    sx += screen->w-vieww;
                    sy += screen->h-viewh;
                }
                glScissor(sx, sy, sw, sh);
            }
            if(!rtscissor || scissoring) rendershadowmapreceivers();
        }

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        return shadowmapcasters>0;
    }

    bool flipdebug() const { return false; }

    void dodebug(int w, int h)
    {
        if(shadowmapcasters)
        {
            glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE);
            debugscissor(w, h);
            glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_FALSE);
            debugblurtiles(w, h);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        }
    }
} shadowmaptex;

void cleanshadowmap()
{
    shadowmaptex.cleanup(true);
}

VAR(ffsmscissor, 0, 1, 1);

static void calcscissorbox()
{
    int smx, smy, smw, smh;
    shadowmaptex.scissorblur(smx, smy, smw, smh);

    vec forward, right;
    vecfromyawpitch(camera1->yaw, 0, -1, 0, forward);
    vecfromyawpitch(camera1->yaw, 0, 0, -1, right);
    forward.mul(shadowmapradius*2.0f/shadowmaptex.viewh);
    right.mul(shadowmapradius*2.0f/shadowmaptex.vieww);

    vec bottom(shadowfocus);
    bottom.sub(vec(shadowdir).mul(shadowmapdist));
    bottom.add(vec(forward).mul(smy - shadowmaptex.viewh/2)).add(vec(right).mul(smx - shadowmaptex.vieww/2));
    vec top(bottom);
    top.add(vec(shadowdir).mul(shadowmapmaxz - (shadowfocus.z - shadowmapdist)));
   
    vec4 v[8];
    float sx1 = 1, sy1 = 1, sx2 = -1, sy2 = -1;
    loopi(8)
    {
        vec c = i&4 ? top : bottom;
        if(i&1) c.add(vec(right).mul(smw));
        if(i&2) c.add(vec(forward).mul(smh));
        if(reflecting) c.z = 2*reflectz - c.z;
        vec4 &p = v[i];
        mvpmatrix.transform(c, p);
        if(p.z >= -p.w)
        {
            float x = p.x / p.w, y = p.y / p.w;
            sx1 = min(sx1, x);
            sy1 = min(sy1, y);
            sx2 = max(sx2, x);
            sy2 = max(sy2, y);
        }
    }
    if(sx1 >= sx2 || sy1 >= sy2) return;
    loopi(8)
    {
        const vec4 &p = v[i];
        if(p.z >= -p.w) continue;
        loopj(3)
        {
            const vec4 &o = v[i^(1<<j)];
            if(o.z <= -o.w) continue;
            float t = (p.z + p.w)/(p.z + p.w - o.z - o.w),
                  w = p.w + t*(o.w - p.w),
                  x = (p.x + t*(o.x - p.x))/w,
                  y = (p.y + t*(o.y - p.y))/w;
            sx1 = min(sx1, x);
            sy1 = min(sy1, y);
            sx2 = max(sx2, x);
            sy2 = max(sy2, y);
        }
    }
    pushscissor(sx1, sy1, sx2, sy2);
}

void calcshadowmapbb(const vec &o, float xyrad, float zrad, float &x1, float &y1, float &x2, float &y2)
{
    vec skewdir(shadowdir);
    skewdir.rotate_around_z(-camera1->yaw*RAD);

    vec ro(o);
    ro.sub(camera1->o);
    ro.rotate_around_z(-(camera1->yaw+180)*RAD);
    ro.x += ro.z * skewdir.x + shadowoffset.x;
    ro.y += ro.z * skewdir.y + shadowmapradius * cosf(camera1->pitch*RAD) + shadowoffset.y;

    vec high(ro), low(ro);
    high.x += zrad * skewdir.x;
    high.y += zrad * skewdir.y;
    low.x -= zrad * skewdir.x;
    low.y -= zrad * skewdir.y;

    x1 = (min(high.x, low.x) - xyrad) / shadowmapradius;
    y1 = (min(high.y, low.y) - xyrad) / shadowmapradius;
    x2 = (max(high.x, low.x) + xyrad) / shadowmapradius;
    y2 = (max(high.y, low.y) + xyrad) / shadowmapradius;
}

bool addshadowmapcaster(const vec &o, float xyrad, float zrad)
{
    if(o.z + zrad <= shadowfocus.z - shadowmapdist || o.z - zrad >= shadowfocus.z) return false;

    shadowmapmaxz = max(shadowmapmaxz, o.z + zrad);

    float x1, y1, x2, y2;
    calcshadowmapbb(o, xyrad, zrad, x1, y1, x2, y2);

    if(!shadowmaptex.addblurtiles(x1, y1, x2, y2, 2)) return false;

    shadowmapcasters++;
    return true;
}

bool isshadowmapreceiver(vtxarray *va)
{
    if(!shadowmap || !shadowmapcasters) return false;

    if(va->shadowmapmax.z <= shadowfocus.z - shadowmapdist || va->shadowmapmin.z >= shadowmapmaxz) return false;

    float xyrad = SQRT2*0.5f*max(va->shadowmapmax.x-va->shadowmapmin.x, va->shadowmapmax.y-va->shadowmapmin.y),
          zrad = 0.5f*(va->shadowmapmax.z-va->shadowmapmin.z),
          x1, y1, x2, y2;
    if(xyrad<0 || zrad<0) return false;

    vec center(va->shadowmapmin.tovec());
    center.add(va->shadowmapmax.tovec()).mul(0.5f);
    calcshadowmapbb(center, xyrad, zrad, x1, y1, x2, y2);

    return shadowmaptex.checkblurtiles(x1, y1, x2, y2, 2);

#if 0
    // cheaper inexact test
    float dz = va->o.z + va->size/2 - shadowfocus.z;
    float cx = shadowfocus.x + dz*shadowdir.x, cy = shadowfocus.y + dz*shadowdir.y;
    float skew = va->size/2*SHADOWSKEW;
    if(!shadowmap || !shadowmaptex ||
       va->o.z + va->size <= shadowfocus.z - shadowmapdist || va->o.z >= shadowmapmaxz ||
       va->o.x + va->size <= cx - shadowmapradius-skew || va->o.x >= cx + shadowmapradius+skew || 
       va->o.y + va->size <= cy - shadowmapradius-skew || va->o.y >= cy + shadowmapradius+skew) 
        return false;
    return true;
#endif
}

bool isshadowmapcaster(const vec &o, float rad)
{
    // cheaper inexact test
    float dz = o.z - shadowfocus.z;
    float cx = shadowfocus.x + dz*shadowdir.x, cy = shadowfocus.y + dz*shadowdir.y;
    float skew = rad*SHADOWSKEW;
    if(!shadowmapping ||
       o.z + rad <= shadowfocus.z - shadowmapdist || o.z - rad >= shadowfocus.z ||
       o.x + rad <= cx - shadowmapradius-skew || o.x - rad >= cx + shadowmapradius+skew ||
       o.y + rad <= cy - shadowmapradius-skew || o.y - rad >= cy + shadowmapradius+skew)
        return false;
    return true;
}

void pushshadowmap()
{
    if(!shadowmap || !shadowmaptex.rendertex) return;

    if(renderpath==R_FIXEDFUNCTION)
    {
        glBindTexture(GL_TEXTURE_2D, shadowmaptex.rendertex);

        const GLfloat *v = shadowmapmatrix.v;
        GLfloat texgenS[4] = { v[0], v[4], v[8], v[12] },
                texgenT[4] = { v[1], v[5], v[9], v[13] },
                texgenR[4] = { v[2], v[6], v[10], v[14] };

        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        glTexGenfv(GL_S, GL_OBJECT_PLANE, texgenS);
        glEnable(GL_TEXTURE_GEN_S);

        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        glTexGenfv(GL_T, GL_OBJECT_PLANE, texgenT);
        glEnable(GL_TEXTURE_GEN_T);

        glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        glTexGenfv(GL_R, GL_OBJECT_PLANE, texgenR);
        glEnable(GL_TEXTURE_GEN_R);

        // intel driver bug workaround: when R texgen is enabled, it uses the value of Q, even if not enabled!
        // MUST set Q with glTexCoord4f, glTexCoord3f does not work
        glTexCoord4f(0, 0, 0, 1);

        glColor3f(shadowmapintensity/100.0f, shadowmapintensity/100.0f, shadowmapintensity/100.0f);

        if(ffsmscissor) calcscissorbox();
        return;
    }

    glActiveTexture_(GL_TEXTURE7_ARB);
    glBindTexture(GL_TEXTURE_2D, shadowmaptex.rendertex);

    glActiveTexture_(GL_TEXTURE2_ARB);
    glMatrixMode(GL_TEXTURE);
    glLoadMatrixf(shadowmapmatrix.v);
    glMatrixMode(GL_MODELVIEW);

    glActiveTexture_(GL_TEXTURE0_ARB);
    glClientActiveTexture_(GL_TEXTURE0_ARB);

    float r, g, b;
	if(!shadowmapambient)
	{
		if(skylightcolor[0] || skylightcolor[1] || skylightcolor[2])
		{
			r = max(25.0f, 0.4f*ambientcolor[0] + 0.6f*max(ambientcolor[0], skylightcolor[0]));
			g = max(25.0f, 0.4f*ambientcolor[1] + 0.6f*max(ambientcolor[1], skylightcolor[1]));
			b = max(25.0f, 0.4f*ambientcolor[2] + 0.6f*max(ambientcolor[2], skylightcolor[2]));
		}
		else 
        {
            r = max(25.0f, 2.0f*ambientcolor[0]);
            g = max(25.0f, 2.0f*ambientcolor[1]);
            b = max(25.0f, 2.0f*ambientcolor[2]);
        }
	}
    else { r = shadowmapambientcolor[0]; g = shadowmapambientcolor[1]; b = shadowmapambientcolor[2]; }
    setenvparamf("shadowmapambient", SHPARAM_PIXEL, 7, r/255.0f, g/255.0f, b/255.0f);
}

void popshadowmap()
{
    if(!shadowmap || !shadowmaptex.rendertex) return;

    if(renderpath==R_FIXEDFUNCTION) 
    {
        popscissor();

        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
        glDisable(GL_TEXTURE_GEN_R);
    }
}

void rendershadowmap()
{
    if(!shadowmap || (renderpath==R_FIXEDFUNCTION && (!hasSGIDT || !hasSGISH))) return;

    // Apple/ATI bug - fixed-function fog state can force software fallback even when fragment program is enabled
    glDisable(GL_FOG); 
    shadowmaptex.render(1<<shadowmapsize, 1<<shadowmapsize, renderpath!=R_FIXEDFUNCTION ? blurshadowmap : 0, blursmsigma/100.0f);
    glEnable(GL_FOG);
}

VAR(debugsm, 0, 0, 1);

void viewshadowmap()
{
    if(!shadowmap) return;
    shadowmaptex.debug();
}

