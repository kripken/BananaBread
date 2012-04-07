#include "engine.h"

VARP(ffdynlights, 0, min(5, DYNLIGHTMASK), DYNLIGHTMASK);
VARP(maxdynlights, 0, min(3, MAXDYNLIGHTS), MAXDYNLIGHTS);
VARP(dynlightdist, 0, 1024, 10000);

struct dynlight
{
    vec o, hud;
    float radius, initradius, curradius, dist;
    vec color, initcolor, curcolor;
    int fade, peak, expire, flags;
    physent *owner;

    void calcradius()
    {
        if(fade + peak > 0)
        {
            int remaining = expire - lastmillis;
            if(flags&DL_EXPAND)
                curradius = initradius + (radius - initradius) * (1.0f - remaining/float(fade + peak));
            else if(!(flags&DL_FLASH) && remaining > fade)
                curradius = initradius + (radius - initradius) * (1.0f - float(remaining - fade)/peak);
            else if(flags&DL_SHRINK)
                curradius = (radius*remaining)/fade;
            else curradius = radius;
        }
        else curradius = radius;
    }

    void calccolor()
    {
        if(flags&DL_FLASH || peak <= 0) curcolor = color;
        else
        {
            int peaking = expire - lastmillis - fade;
            if(peaking <= 0) curcolor = color;
            else curcolor.lerp(initcolor, color, 1.0f - float(peaking)/peak);
        }

        float intensity = 1.0f;
        if(fade > 0)
        {
            int fading = expire - lastmillis;
            if(fading < fade) intensity = float(fading)/fade;
        }
        curcolor.mul(intensity);
        // KLUGE: this prevents nvidia drivers from trying to recompile dynlight fragment programs
        loopk(3) if(fmod(curcolor[k], 1.0f/256) < 0.001f) curcolor[k] += 0.001f;
    }
};

vector<dynlight> dynlights;
vector<dynlight *> closedynlights;

void adddynlight(const vec &o, float radius, const vec &color, int fade, int peak, int flags, float initradius, const vec &initcolor, physent *owner)
{
    if(renderpath==R_FIXEDFUNCTION ? !ffdynlights || maxtmus<3 : !maxdynlights) return;
    if(o.dist(camera1->o) > dynlightdist || radius <= 0) return;

    int insert = 0, expire = fade + peak + lastmillis;
    loopvrev(dynlights) if(expire>=dynlights[i].expire) { insert = i+1; break; }
    dynlight d;
    d.o = d.hud = o;
    d.radius = radius;
    d.initradius = initradius;
    d.color = color;
    d.initcolor = initcolor;
    d.fade = fade;
    d.peak = peak;
    d.expire = expire;
    d.flags = flags;
    d.owner = owner;
    dynlights.insert(insert, d);
}

void cleardynlights()
{
    int faded = -1;
    loopv(dynlights) if(lastmillis<dynlights[i].expire) { faded = i; break; }
    if(faded<0) dynlights.setsize(0);
    else if(faded>0) dynlights.remove(0, faded);
}

void removetrackeddynlights(physent *owner)
{
    loopvrev(dynlights) if(owner ? dynlights[i].owner == owner : dynlights[i].owner != NULL) dynlights.remove(i);
}

void updatedynlights()
{
    cleardynlights();
    game::adddynlights();

    loopv(dynlights)
    {
        dynlight &d = dynlights[i];
        if(d.owner) game::dynlighttrack(d.owner, d.o, d.hud);
        d.calcradius();
        d.calccolor();
    }
}

int finddynlights()
{
    closedynlights.setsize(0);
    if(renderpath==R_FIXEDFUNCTION ? !ffdynlights || maxtmus<3 : !maxdynlights) return 0;
    physent e;
    e.type = ENT_CAMERA;
    e.collidetype = COLLIDE_AABB;
    loopvj(dynlights)
    {
        dynlight &d = dynlights[j];
        if(d.curradius <= 0) continue;
        d.dist = camera1->o.dist(d.o) - d.curradius;
        if(d.dist > dynlightdist || isfoggedsphere(d.curradius, d.o) || pvsoccluded(d.o, 2*int(d.curradius+1))) 
            continue;
        if(reflecting || refracting > 0)
        {
            if(d.o.z + d.curradius < reflectz) continue;
        }
        else if(refracting < 0 && d.o.z - d.curradius > reflectz) continue;
        e.o = d.o;
        e.radius = e.xradius = e.yradius = e.eyeheight = e.aboveeye = d.curradius;
        if(collide(&e, vec(0, 0, 0), 0, false)) continue;

        int insert = 0;
        loopvrev(closedynlights) if(d.dist >= closedynlights[i]->dist) { insert = i+1; break; }
        closedynlights.insert(insert, &d);
        if(closedynlights.length() >= DYNLIGHTMASK) break;
    }
    if(renderpath==R_FIXEDFUNCTION && closedynlights.length() > ffdynlights)
        closedynlights.setsize(ffdynlights);
    return closedynlights.length();
}

bool getdynlight(int n, vec &o, float &radius, vec &color)
{
    if(!closedynlights.inrange(n)) return false;
    dynlight &d = *closedynlights[n];
    o = d.o;
    radius = d.curradius;
    color = d.curcolor;
    return true;
}

void dynlightreaching(const vec &target, vec &color, vec &dir, bool hud)
{
    vec dyncolor(0, 0, 0);//, dyndir(0, 0, 0);
    loopv(dynlights)
    {
        dynlight &d = dynlights[i];
        if(d.curradius<=0) continue;

        vec ray(hud ? d.hud : d.o);
        ray.sub(target);
        float mag = ray.squaredlen();
        if(mag >= d.curradius*d.curradius) continue;

        vec color = d.curcolor;
        color.mul(1 - sqrtf(mag)/d.curradius);
        dyncolor.add(color);
        //dyndir.add(ray.mul(intensity/mag));
    }
#if 0
    if(!dyndir.iszero())
    {
        dyndir.normalize();
        float x = dyncolor.magnitude(), y = color.magnitude();
        if(x+y>0)
        {
            dir.mul(x);
            dyndir.mul(y); 
            dir.add(dyndir).div(x+y);
            if(dir.iszero()) dir = vec(0, 0, 1);
            else dir.normalize();
        }
    }
#endif
    color.add(dyncolor);
}

void calcdynlightmask(vtxarray *va)
{
    uint mask = 0;
    int offset = 0;
    loopv(closedynlights)
    {
        dynlight &d = *closedynlights[i];
        if(d.o.dist_to_bb(va->geommin, va->geommax) >= d.curradius) continue;

        mask |= (i+1)<<offset;
        offset += DYNLIGHTBITS;
        if(offset >= maxdynlights*DYNLIGHTBITS) break;
    }
    va->dynlightmask = mask;
}

int setdynlights(vtxarray *va)
{
    if(closedynlights.empty() || !va->dynlightmask) return 0;

    static string posparams[MAXDYNLIGHTS] = { "" }, colorparams[MAXDYNLIGHTS] = { "" }, offsetparams[MAXDYNLIGHTS] = { "" };
    if(!*posparams[0]) loopi(MAXDYNLIGHTS)
    {
        formatstring(posparams[i])("dynlight%dpos", i);
        formatstring(colorparams[i])("dynlight%dcolor", i);
        formatstring(offsetparams[i])("dynlight%doffset", i);
    }

    int index = 0;
    float scale0 = 1;
    vec origin0(0, 0, 0);
    for(uint mask = va->dynlightmask; mask; mask >>= DYNLIGHTBITS, index++)
    {
        dynlight &d = *closedynlights[(mask&DYNLIGHTMASK)-1];

        float scale = 1.0f/d.curradius;
        vec origin = vec(d.o).mul(-scale);
        setenvparamf(posparams[index], SHPARAM_VERTEX, 10+index, origin.x, origin.y, origin.z, scale);

        if(index<=0) { scale0 = scale; origin0 = origin; }
        else
        {
            scale /= scale0;
            origin.sub(vec(origin0).mul(scale));
            setenvparamf(offsetparams[index], SHPARAM_PIXEL, index-1, origin.x, origin.y, origin.z, scale);
        }

        setenvparamf(colorparams[index], SHPARAM_PIXEL, 10+index, d.curcolor.x, d.curcolor.y, d.curcolor.z);
    }

    return index;
}

