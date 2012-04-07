static struct flaretype
{
    int type;             /* flaretex index, 0..5, -1 for 6+random shine */
    float loc;            /* postion on axis */
    float scale;          /* texture scaling */
    uchar alpha;          /* color alpha */
} flaretypes[] =
{
    {2,  1.30f, 0.04f, 153}, //flares
    {3,  1.00f, 0.10f, 102},
    {1,  0.50f, 0.20f, 77},
    {3,  0.20f, 0.05f, 77},
    {0,  0.00f, 0.04f, 77},
    {5, -0.25f, 0.07f, 127},
    {5, -0.40f, 0.02f, 153},
    {5, -0.60f, 0.04f, 102},
    {5, -1.00f, 0.03f, 51},
    {-1, 1.00f, 0.30f, 255}, //shine - red, green, blue
    {-2, 1.00f, 0.20f, 255},
    {-3, 1.00f, 0.25f, 255}
};

struct flare
{
    vec o, center;
    float size;
    uchar color[3];
    bool sparkle;
};

VAR(flarelights, 0, 0, 1);
VARP(flarecutoff, 0, 1000, 10000);
VARP(flaresize, 20, 100, 500);

struct flarerenderer : partrenderer
{
    int maxflares, numflares;
    unsigned int shinetime;
    flare *flares;

    flarerenderer(const char *texname, int maxflares)
        : partrenderer(texname, 3, PT_FLARE), maxflares(maxflares), shinetime(0)
    {
        flares = new flare[maxflares];
    }
    void reset()
    {
        numflares = 0;
    }

    void newflare(vec &o,  const vec &center, uchar r, uchar g, uchar b, float mod, float size, bool sun, bool sparkle)
    {
        if(numflares >= maxflares) return;
        vec target; //occlusion check (neccessary as depth testing is turned off)
        if(!raycubelos(o, camera1->o, target)) return;
        flare &f = flares[numflares++];
        f.o = o;
        f.center = center;
        f.size = size;
        f.color[0] = uchar(r*mod);
        f.color[1] = uchar(g*mod);
        f.color[2] = uchar(b*mod);
        f.sparkle = sparkle;
    }

    void addflare(vec &o, uchar r, uchar g, uchar b, bool sun, bool sparkle)
    {
        //frustrum + fog check
        if(isvisiblesphere(0.0f, o) > (sun?VFC_FOGGED:VFC_FULL_VISIBLE)) return;
        //find closest point between camera line of sight and flare pos
        vec viewdir;
        vecfromyawpitch(camera1->yaw, camera1->pitch, 1, 0, viewdir);
        vec flaredir = vec(o).sub(camera1->o);
        vec center = viewdir.mul(flaredir.dot(viewdir)).add(camera1->o);
        float mod, size;
        if(sun) //fixed size
        {
            mod = 1.0;
            size = flaredir.magnitude() * flaresize / 100.0f;
        }
        else
        {
            mod = (flarecutoff-vec(o).sub(center).squaredlen())/flarecutoff;
            if(mod < 0.0f) return;
            size = flaresize / 5.0f;
        }
        newflare(o, center, r, g, b, mod, size, sun, sparkle);
    }

    void makelightflares()
    {
        numflares = 0; //regenerate flarelist each frame
        shinetime = lastmillis/10;

        if(editmode || !flarelights) return;

        const vector<extentity *> &ents = entities::getents();
        vec viewdir;
        vecfromyawpitch(camera1->yaw, camera1->pitch, 1, 0, viewdir);
        extern const vector<int> &checklightcache(int x, int y);
        const vector<int> &lights = checklightcache(int(camera1->o.x), int(camera1->o.y));
        loopv(lights)
        {
            entity &e = *ents[lights[i]];
            if(e.type != ET_LIGHT) continue;
            bool sun = (e.attr1==0);
            float radius = float(e.attr1);
            vec flaredir = vec(e.o).sub(camera1->o);
            float len = flaredir.magnitude();
            if(!sun && (len > radius)) continue;
            if(isvisiblesphere(0.0f, e.o) > (sun?VFC_FOGGED:VFC_FULL_VISIBLE)) continue;
            vec center = vec(viewdir).mul(flaredir.dot(viewdir)).add(camera1->o);
            float mod, size;
            if(sun) //fixed size
            {
                mod = 1.0;
                size = len * flaresize / 100.0f;
            }
            else
            {
                mod = (radius-len)/radius;
                size = flaresize / 5.0f;
            }
            newflare(e.o, center, e.attr2, e.attr3, e.attr4, mod, size, sun, sun);
        }
    }

    int count()
    {
        return numflares;
    }

    bool haswork()
    {
        return (numflares != 0) && !glaring && !reflecting  && !refracting;
    }

    void render()
    {
        glDisable(GL_FOG);
        defaultshader->set();
        glDisable(GL_DEPTH_TEST);
        if(!tex) tex = textureload(texname);
        glBindTexture(GL_TEXTURE_2D, tex->id);
        glBegin(GL_QUADS);
        loopi(numflares)
        {
            flare *f = flares+i;
            vec center = f->center;
            vec axis = vec(f->o).sub(center);
            uchar color[4] = {f->color[0], f->color[1], f->color[2], 255};
            loopj(f->sparkle?12:9)
            {
                const flaretype &ft = flaretypes[j];
                vec o = vec(axis).mul(ft.loc).add(center);
                float sz = ft.scale * f->size;
                int tex = ft.type;
                if(ft.type < 0) //sparkles - always done last
                {
                    shinetime = (shinetime + 1) % 10;
                    tex = 6+shinetime;
                    color[0] = 0;
                    color[1] = 0;
                    color[2] = 0;
                    color[-ft.type-1] = f->color[-ft.type-1]; //only want a single channel
                }
                color[3] = ft.alpha;
                glColor4ubv(color);
                const float tsz = 0.25; //flares are aranged in 4x4 grid
                float tx = tsz*(tex&0x03);
                float ty = tsz*((tex>>2)&0x03);
                glTexCoord2f(tx,     ty+tsz); glVertex3f(o.x+(-camright.x+camup.x)*sz, o.y+(-camright.y+camup.y)*sz, o.z+(-camright.z+camup.z)*sz);
                glTexCoord2f(tx+tsz, ty+tsz); glVertex3f(o.x+( camright.x+camup.x)*sz, o.y+( camright.y+camup.y)*sz, o.z+( camright.z+camup.z)*sz);
                glTexCoord2f(tx+tsz, ty);     glVertex3f(o.x+( camright.x-camup.x)*sz, o.y+( camright.y-camup.y)*sz, o.z+( camright.z-camup.z)*sz);
                glTexCoord2f(tx,     ty);     glVertex3f(o.x+(-camright.x-camup.x)*sz, o.y+(-camright.y-camup.y)*sz, o.z+(-camright.z-camup.z)*sz);
            }
        }
        glEnd();
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_FOG);
    }

    //square per round hole - use addflare(..) instead
    particle *addpart(const vec &o, const vec &d, int fade, int color, float size, int gravity = 0) { return NULL; }
};
static flarerenderer flares("<grey>packages/particles/lensflares.png", 64);

