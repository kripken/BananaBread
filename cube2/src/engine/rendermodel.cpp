#include "engine.h"

VARP(oqdynent, 0, 1, 1);
VARP(animationinterpolationtime, 0, 150, 1000);

model *loadingmodel = NULL;

#include "ragdoll.h"
#include "animmodel.h"
#include "vertmodel.h"
#include "skelmodel.h"

static model *(__cdecl *modeltypes[NUMMODELTYPES])(const char *);

static int addmodeltype(int type, model *(__cdecl *loader)(const char *))
{
    modeltypes[type] = loader;
    return type;
}

#define MODELTYPE(modeltype, modelclass) \
static model *__loadmodel__##modelclass(const char *filename) \
{ \
    return new modelclass(filename); \
} \
static int __dummy__##modelclass = addmodeltype((modeltype), __loadmodel__##modelclass);
 
#include "md2.h"
#include "md3.h"
#include "md5.h"
#include "obj.h"
#include "smd.h"
#include "iqm.h"

MODELTYPE(MDL_MD2, md2);
MODELTYPE(MDL_MD3, md3);
MODELTYPE(MDL_MD5, md5);
MODELTYPE(MDL_OBJ, obj);
MODELTYPE(MDL_SMD, smd);
MODELTYPE(MDL_IQM, iqm);

#define checkmdl if(!loadingmodel) { conoutf(CON_ERROR, "not loading a model"); return; }

void mdlcullface(int *cullface)
{
    checkmdl;
    loadingmodel->setcullface(*cullface!=0);
}

COMMAND(mdlcullface, "i");

void mdlcollide(int *collide)
{
    checkmdl;
    loadingmodel->collide = *collide!=0;
}

COMMAND(mdlcollide, "i");

void mdlellipsecollide(int *collide)
{
    checkmdl;
    loadingmodel->ellipsecollide = *collide!=0;
}   
    
COMMAND(mdlellipsecollide, "i");

void mdlspec(int *percent)
{
    checkmdl;
    float spec = 1.0f; 
    if(*percent>0) spec = *percent/100.0f;
    else if(*percent<0) spec = 0.0f;
    loadingmodel->setspec(spec);
}

COMMAND(mdlspec, "i");

void mdlambient(int *percent)
{
    checkmdl;
    float ambient = 0.3f;
    if(*percent>0) ambient = *percent/100.0f;
    else if(*percent<0) ambient = 0.0f;
    loadingmodel->setambient(ambient);
}

COMMAND(mdlambient, "i");

void mdlalphatest(float *cutoff)
{   
    checkmdl;
    loadingmodel->setalphatest(max(0.0f, min(1.0f, *cutoff)));
}

COMMAND(mdlalphatest, "f");

void mdlalphablend(int *blend)
{   
    checkmdl;
    loadingmodel->setalphablend(*blend!=0);
}

COMMAND(mdlalphablend, "i");

void mdlalphadepth(int *depth)
{
    checkmdl;
    loadingmodel->alphadepth = *depth!=0;
}

COMMAND(mdlalphadepth, "i");

void mdldepthoffset(int *offset)
{
    checkmdl;
    loadingmodel->depthoffset = *offset!=0;
}

COMMAND(mdldepthoffset, "i");

void mdlglow(int *percent, int *delta, float *pulse)
{
    checkmdl;
    float glow = 3.0f, glowdelta = *delta/100.0f, glowpulse = *pulse > 0 ? *pulse/1000.0f : 0;
    if(*percent>0) glow = *percent/100.0f;
    else if(*percent<0) glow = 0.0f;
    glowdelta -= glow;
    loadingmodel->setglow(glow, glowdelta, glowpulse);
}

COMMAND(mdlglow, "iif");

void mdlglare(float *specglare, float *glowglare)
{
    checkmdl;
    loadingmodel->setglare(*specglare, *glowglare);
}

COMMAND(mdlglare, "ff");

void mdlenvmap(float *envmapmax, float *envmapmin, char *envmap)
{
    checkmdl;
    loadingmodel->setenvmap(*envmapmin, *envmapmax, envmap[0] ? cubemapload(envmap) : NULL);
}

COMMAND(mdlenvmap, "ffs");

void mdlfullbright(float *fullbright)
{
    checkmdl;
    loadingmodel->setfullbright(*fullbright);
}

COMMAND(mdlfullbright, "f");

void mdlshader(char *shader)
{
    checkmdl;
    loadingmodel->setshader(lookupshaderbyname(shader));
}

COMMAND(mdlshader, "s");

void mdlspin(float *yaw, float *pitch)
{
    checkmdl;
    loadingmodel->spinyaw = *yaw;
    loadingmodel->spinpitch = *pitch;
}

COMMAND(mdlspin, "ff");

void mdlscale(int *percent)
{
    checkmdl;
    float scale = 0.3f;
    if(*percent>0) scale = *percent/100.0f;
    else if(*percent<0) scale = 0.0f;
    loadingmodel->scale = scale;
}  

COMMAND(mdlscale, "i");

void mdltrans(float *x, float *y, float *z)
{
    checkmdl;
    loadingmodel->translate = vec(*x, *y, *z);
} 

COMMAND(mdltrans, "fff");

void mdlyaw(float *angle)
{
    checkmdl;
    loadingmodel->offsetyaw = *angle;
}

COMMAND(mdlyaw, "f");

void mdlpitch(float *angle)
{
    checkmdl;
    loadingmodel->offsetpitch = *angle;
}

COMMAND(mdlpitch, "f");

void mdlshadow(int *shadow)
{
    checkmdl;
    loadingmodel->shadow = *shadow!=0;
}

COMMAND(mdlshadow, "i");

void mdlbb(float *rad, float *h, float *eyeheight)
{
    checkmdl;
    loadingmodel->collideradius = *rad;
    loadingmodel->collideheight = *h;
    loadingmodel->eyeheight = *eyeheight; 
}

COMMAND(mdlbb, "fff");

void mdlextendbb(float *x, float *y, float *z)
{
    checkmdl;
    loadingmodel->bbextend = vec(*x, *y, *z);
}

COMMAND(mdlextendbb, "fff");

void mdlname()
{
    checkmdl;
    result(loadingmodel->name());
}

COMMAND(mdlname, "");

#define checkragdoll \
    if(!loadingmodel->skeletal()) { conoutf(CON_ERROR, "not loading a skeletal model"); return; } \
    skelmodel *m = (skelmodel *)loadingmodel; \
    skelmodel::skelmeshgroup *meshes = (skelmodel::skelmeshgroup *)m->parts.last()->meshes; \
    if(!meshes) return; \
    skelmodel::skeleton *skel = meshes->skel; \
    if(!skel->ragdoll) skel->ragdoll = new ragdollskel; \
    ragdollskel *ragdoll = skel->ragdoll; \
    if(ragdoll->loaded) return;
    

void rdvert(float *x, float *y, float *z, float *radius)
{
    checkragdoll;
    ragdollskel::vert &v = ragdoll->verts.add();
    v.pos = vec(*x, *y, *z);
    v.radius = *radius > 0 ? *radius : 1;
}
COMMAND(rdvert, "ffff");

void rdeye(int *v)
{
    checkragdoll;
    ragdoll->eye = *v;
}
COMMAND(rdeye, "i");

void rdtri(int *v1, int *v2, int *v3)
{
    checkragdoll;
    ragdollskel::tri &t = ragdoll->tris.add();
    t.vert[0] = *v1;
    t.vert[1] = *v2;
    t.vert[2] = *v3;
}
COMMAND(rdtri, "iii");

void rdjoint(int *n, int *t, char *v1, char *v2, char *v3)
{
    checkragdoll;
    if(*n < 0 || *n >= skel->numbones) return;
    ragdollskel::joint &j = ragdoll->joints.add();
    j.bone = *n;
    j.tri = *t;
    j.vert[0] = v1[0] ? parseint(v1) : -1;
    j.vert[1] = v2[0] ? parseint(v2) : -1;
    j.vert[2] = v3[0] ? parseint(v3) : -1;
}
COMMAND(rdjoint, "iisss");
   
void rdlimitdist(int *v1, int *v2, float *mindist, float *maxdist)
{
    checkragdoll;
    ragdollskel::distlimit &d = ragdoll->distlimits.add();
    d.vert[0] = *v1;
    d.vert[1] = *v2;
    d.mindist = *mindist;
    d.maxdist = max(*maxdist, *mindist);
}
COMMAND(rdlimitdist, "iiff");

void rdlimitrot(int *t1, int *t2, float *maxangle, float *qx, float *qy, float *qz, float *qw)
{
    checkragdoll;
    ragdollskel::rotlimit &r = ragdoll->rotlimits.add();
    r.tri[0] = *t1;
    r.tri[1] = *t2;
    r.maxangle = *maxangle * RAD;
    r.middle = matrix3x3(quat(*qx, *qy, *qz, *qw));
}
COMMAND(rdlimitrot, "iifffff");

void rdanimjoints(int *on)
{
    checkragdoll;
    ragdoll->animjoints = *on!=0;
}
COMMAND(rdanimjoints, "i");

// mapmodels

vector<mapmodelinfo> mapmodels;

void mmodel(char *name)
{
    mapmodelinfo &mmi = mapmodels.add();
    copystring(mmi.name, name);
    mmi.m = NULL;
}

void mapmodelcompat(int *rad, int *h, int *tex, char *name, char *shadow)
{
    mmodel(name);
}

void mapmodelreset(int *n) 
{ 
    if(!(identflags&IDF_OVERRIDDEN) && !game::allowedittoggle()) return;
    mapmodels.shrink(clamp(*n, 0, mapmodels.length())); 
}

mapmodelinfo *getmminfo(int i) { return mapmodels.inrange(i) ? &mapmodels[i] : 0; }
const char *mapmodelname(int i) { return mapmodels.inrange(i) ? mapmodels[i].name : NULL; }

COMMAND(mmodel, "s");
COMMANDN(mapmodel, mapmodelcompat, "iiiss");
COMMAND(mapmodelreset, "i");
ICOMMAND(mapmodelname, "i", (int *index), { result(mapmodels.inrange(*index) ? mapmodels[*index].name : ""); });
ICOMMAND(nummapmodels, "", (), { intret(mapmodels.length()); });

// model registry

hashtable<const char *, model *> mdllookup;
vector<const char *> preloadmodels;

void preloadmodel(const char *name)
{
    if(mdllookup.access(name)) return;
    preloadmodels.add(newstring(name));
}

// XXX EMSCRIPTEN: asynchronizing
static void flushpreloadedmodels_iter(void *);
static void (*flushpreloadedmodels_func)(void *);

void flushpreloadedmodels(void (*func)(void *))
{
    flushpreloadedmodels_func = func;
    flushpreloadedmodels_iter(NULL);
}

void flushpreloadedmodels_iter(void *)
{
    if (preloadmodels.length())
    {
        loadmodel(preloadmodels.pop(), -1, true);
        emscripten_push_uncounted_main_loop_blocker(flushpreloadedmodels_iter, NULL);
    }
    else emscripten_push_main_loop_blocker(flushpreloadedmodels_func, NULL);
}

void preloadusedmapmodels(bool msg, bool bih)
{
    vector<extentity *> &ents = entities::getents();
    vector<int> mapmodels;
    loopv(ents)
    {
        extentity &e = *ents[i];
        if(e.type==ET_MAPMODEL && e.attr2 >= 0 && mapmodels.find(e.attr2) < 0) mapmodels.add(e.attr2);
    }

    loopv(mapmodels)
    {
        loadprogress = float(i+1)/mapmodels.length();
        int mmindex = mapmodels[i];
        mapmodelinfo *mmi = getmminfo(mmindex);
        if(!mmi) { if(msg) conoutf(CON_WARN, "could not find map model: %d", mmindex); }
        else if(!loadmodel(NULL, mmindex, msg)) { if(msg) conoutf(CON_WARN, "could not load model: %s", mmi->name); }
        else if(mmi->m && bih) mmi->m->preloadBIH();
    }
    loadprogress = 0;
}

hashtable<const char *, model *> missingmodels; // XXX EMSCRIPTEN: models that fail to load once, we will never try to load again

model *loadmodel(const char *name, int i, bool msg)
{
    if(!name)
    {
        if(!mapmodels.inrange(i)) return NULL;
        mapmodelinfo &mmi = mapmodels[i];
        if(mmi.m) return mmi.m;
        name = mmi.name;
    }
    model **mm = mdllookup.access(name);
    model *m;
    if(mm) m = *mm;
    else
    { 
        if(lightmapping > 1) return NULL;
        if(missingmodels.access(name)) return NULL; // XXX EMSCRIPTEN
        if(msg)
        {
            defformatstring(filename)("packages/models/%s", name);
            renderprogress(loadprogress, filename);
        }
        loopi(NUMMODELTYPES)
        {
            m = modeltypes[i](name);
            if(!m) continue;
            loadingmodel = m;
            if(m->load()) break;
            DELETEP(m);
        }
        loadingmodel = NULL;
        if(!m)
        {
            missingmodels.access(name, NULL); // XXX EMSCRIPTEN
            return NULL;
        }
        mdllookup.access(m->name(), m);
    }
    if(mapmodels.inrange(i) && !mapmodels[i].m) mapmodels[i].m = m;
    return m;
}

void preloadmodelshaders()
{
    if(initing) return;
    enumerate(mdllookup, model *, m, m->preloadshaders());
}

void clear_mdls()
{
    enumerate(mdllookup, model *, m, delete m);
}

void cleanupmodels()
{
    enumerate(mdllookup, model *, m, m->cleanup());
}

void clearmodel(char *name)
{
    model **m = mdllookup.access(name);
    if(!m) { conoutf("model %s is not loaded", name); return; }
    loopv(mapmodels) if(mapmodels[i].m==*m) mapmodels[i].m = NULL;
    mdllookup.remove(name);
    (*m)->cleanup();
    delete *m;
    conoutf("cleared model %s", name);
}

COMMAND(clearmodel, "s");

bool modeloccluded(const vec &center, float radius)
{
    int br = int(radius*2)+1;
    return pvsoccluded(ivec(int(center.x-radius), int(center.y-radius), int(center.z-radius)), ivec(br, br, br)) ||
           bboccluded(ivec(int(center.x-radius), int(center.y-radius), int(center.z-radius)), ivec(br, br, br));
}

VAR(showboundingbox, 0, 0, 2);

void render2dbox(vec &o, float x, float y, float z)
{
    glBegin(GL_LINE_LOOP);
    glVertex3f(o.x, o.y, o.z);
    glVertex3f(o.x, o.y, o.z+z);
    glVertex3f(o.x+x, o.y+y, o.z+z);
    glVertex3f(o.x+x, o.y+y, o.z);
    glEnd();
}

void render3dbox(vec &o, float tofloor, float toceil, float xradius, float yradius)
{
    if(yradius<=0) yradius = xradius;
    vec c = o;
    c.sub(vec(xradius, yradius, tofloor));
    float xsz = xradius*2, ysz = yradius*2;
    float h = tofloor+toceil;
    lineshader->set();
    glDisable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);
    render2dbox(c, xsz, 0, h);
    render2dbox(c, 0, ysz, h);
    c.add(vec(xsz, ysz, 0));
    render2dbox(c, -xsz, 0, h);
    render2dbox(c, 0, -ysz, h);
    xtraverts += 16;
    glEnable(GL_TEXTURE_2D);
}

void renderellipse(vec &o, float xradius, float yradius, float yaw)
{
    lineshader->set();
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_LINE_LOOP);
    loopi(16)
    {
        vec p(xradius*cosf(2*M_PI*i/16.0f), yradius*sinf(2*M_PI*i/16.0f), 0);
        p.rotate_around_z((yaw+90)*RAD);
        p.add(o);
        glVertex3fv(p.v);
    }
    glEnd();
    glEnable(GL_TEXTURE_2D);
}

struct batchedmodel
{
    vec pos, color, dir;
    int anim;
    float yaw, pitch, transparent;
    int basetime, basetime2, flags;
    dynent *d;
    int attached;
    occludequery *query;
};  
struct modelbatch
{
    model *m;
    int flags;
    vector<batchedmodel> batched;
};  
static vector<modelbatch *> batches;
static vector<modelattach> modelattached;
static int numbatches = -1;
static occludequery *modelquery = NULL;

void startmodelbatches()
{
    numbatches = 0;
    modelattached.setsize(0);
}

modelbatch &addbatchedmodel(model *m)
{
    modelbatch *b = NULL;
    if(m->batch>=0 && m->batch<numbatches && batches[m->batch]->m==m) b = batches[m->batch];
    else
    {
        if(numbatches<batches.length())
        {
            b = batches[numbatches];
            b->batched.setsize(0);
        }
        else b = batches.add(new modelbatch);
        b->m = m;
        b->flags = 0;
        m->batch = numbatches++;
    }
    return *b;
}

void renderbatchedmodel(model *m, batchedmodel &b)
{
    modelattach *a = NULL;
    if(b.attached>=0) a = &modelattached[b.attached];

    int anim = b.anim;
    if(shadowmapping)
    {
        anim |= ANIM_NOSKIN; 
        if(renderpath!=R_FIXEDFUNCTION) setenvparamf("shadowintensity", SHPARAM_VERTEX, 1, b.transparent);
    }
    else 
    {
        if(b.flags&MDL_FULLBRIGHT) anim |= ANIM_FULLBRIGHT;
        if(b.flags&MDL_GHOST) anim |= ANIM_GHOST;
    }

    m->render(anim, b.basetime, b.basetime2, b.pos, b.yaw, b.pitch, b.d, a, b.color, b.dir, b.transparent);
}

struct transparentmodel
{
    model *m;
    batchedmodel *batched;
    float dist;
};

static inline bool sorttransparentmodels(const transparentmodel &x, const transparentmodel &y)
{
    return x.dist < y.dist;
}

void endmodelbatches()
{
    vector<transparentmodel> transparent;
    loopi(numbatches)
    {
        modelbatch &b = *batches[i];
        if(b.batched.empty()) continue;
        if(b.flags&(MDL_SHADOW|MDL_DYNSHADOW))
        {
            vec center, bbradius;
            b.m->boundbox(0/*frame*/, center, bbradius); // FIXME
            loopvj(b.batched)
            {
                batchedmodel &bm = b.batched[j];
                if(bm.flags&(MDL_SHADOW|MDL_DYNSHADOW))
                    renderblob(bm.flags&MDL_DYNSHADOW ? BLOB_DYNAMIC : BLOB_STATIC, bm.d && bm.d->ragdoll ? bm.d->ragdoll->center : bm.pos, bm.d ? bm.d->radius : max(bbradius.x, bbradius.y), bm.transparent);
            }
            flushblobs();
        }
        bool rendered = false;
        occludequery *query = NULL;
        if(b.flags&MDL_GHOST)
        {
            loopvj(b.batched)
            {
                batchedmodel &bm = b.batched[j];
                if((bm.flags&(MDL_CULL_VFC|MDL_GHOST))!=MDL_GHOST || bm.query) continue;
                if(!rendered) { b.m->startrender(); rendered = true; }
                renderbatchedmodel(b.m, bm);
            }
            if(rendered) 
            {
                b.m->endrender();
                rendered = false;
            }
        }
        loopvj(b.batched) 
        {
            batchedmodel &bm = b.batched[j];
            if(bm.flags&(MDL_CULL_VFC|MDL_GHOST)) continue;
            if(bm.query!=query)
            {
                if(query) endquery(query);
                query = bm.query;
                if(query) startquery(query);
            }
            if(bm.transparent < 1 && (!query || query->owner==bm.d) && !shadowmapping)
            {
                transparentmodel &tm = transparent.add();
                tm.m = b.m;
                tm.batched = &bm;
                tm.dist = camera1->o.dist(bm.d && bm.d->ragdoll ? bm.d->ragdoll->center : bm.pos);
                continue;
            }
            if(!rendered) { b.m->startrender(); rendered = true; }
            renderbatchedmodel(b.m, bm);
        }
        if(query) endquery(query);
        if(rendered) b.m->endrender();
    }
    if(transparent.length())
    {
        transparent.sort(sorttransparentmodels);
        model *lastmodel = NULL;
        occludequery *query = NULL;
        loopv(transparent)
        {
            transparentmodel &tm = transparent[i];
            if(lastmodel!=tm.m)
            {
                if(lastmodel) lastmodel->endrender();
                (lastmodel = tm.m)->startrender();
            }
            if(query!=tm.batched->query)
            {
                if(query) endquery(query);
                query = tm.batched->query;
                if(query) startquery(query);
            }
            renderbatchedmodel(tm.m, *tm.batched);
        }
        if(query) endquery(query);
        if(lastmodel) lastmodel->endrender();
    }
    numbatches = -1;
}

void startmodelquery(occludequery *query)
{
    modelquery = query;
}

void endmodelquery()
{
    int querybatches = 0;
    loopi(numbatches)
    {
        modelbatch &b = *batches[i];
        if(b.batched.empty() || b.batched.last().query!=modelquery) continue;
        querybatches++;
    }
    if(querybatches<=1)
    {
        if(!querybatches) modelquery->fragments = 0;
        modelquery = NULL;
        return;
    }
    int minattached = modelattached.length();
    startquery(modelquery);
    loopi(numbatches)
    {
        modelbatch &b = *batches[i];
        if(b.batched.empty() || b.batched.last().query!=modelquery) continue;
        b.m->startrender();
        do
        {
            batchedmodel &bm = b.batched.pop();
            if(bm.attached>=0) minattached = min(minattached, bm.attached);
            renderbatchedmodel(b.m, bm);
        }
        while(b.batched.length() && b.batched.last().query==modelquery);
        b.m->endrender();
    }
    endquery(modelquery);
    modelquery = NULL;
    modelattached.setsize(minattached);
}

VARP(maxmodelradiusdistance, 10, 200, 1000);

void rendermodelquery(model *m, dynent *d, const vec &center, float radius)
{
    if(fabs(camera1->o.x-center.x) < radius+1 &&
       fabs(camera1->o.y-center.y) < radius+1 &&
       fabs(camera1->o.z-center.z) < radius+1)
    {
        d->query = NULL;
        return;
    }
    d->query = newquery(d);
    if(!d->query) return;
    nocolorshader->set();
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);
    startquery(d->query);
    int br = int(radius*2)+1;
    drawbb(ivec(int(center.x-radius), int(center.y-radius), int(center.z-radius)), ivec(br, br, br));
    endquery(d->query);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, fading ? GL_FALSE : GL_TRUE);
    glDepthMask(GL_TRUE);
}   

extern int oqfrags;

void rendermodel(entitylight *light, const char *mdl, int anim, const vec &o, float yaw, float pitch, int flags, dynent *d, modelattach *a, int basetime, int basetime2, float trans)
{
    if(shadowmapping && !(flags&(MDL_SHADOW|MDL_DYNSHADOW))) return;
    model *m = loadmodel(mdl); 
    if(!m) return;
    vec center(0, 0, 0), bbradius(0, 0, 0);
    float radius = 0;
    bool shadow = !shadowmap && !glaring && (flags&(MDL_SHADOW|MDL_DYNSHADOW)) && showblobs,
         doOQ = flags&MDL_CULL_QUERY && hasOQ && oqfrags && oqdynent;
    if(flags&(MDL_CULL_VFC|MDL_CULL_DIST|MDL_CULL_OCCLUDED|MDL_CULL_QUERY|MDL_SHADOW|MDL_DYNSHADOW))
    {
        m->boundbox(0/*frame*/, center, bbradius); // FIXME
        radius = bbradius.magnitude();
        if(d && d->ragdoll)
        {
            radius = max(radius, d->ragdoll->radius);
            center = d->ragdoll->center;
        }
        else
        {
            center.rotate_around_z(yaw*RAD);
            center.add(o);
        }
        if(flags&MDL_CULL_DIST && center.dist(camera1->o)/radius>maxmodelradiusdistance) return;
        if(flags&MDL_CULL_VFC)
        {
            if(reflecting || refracting)
            {
                if(reflecting || refracting>0) 
                {
                    if(center.z+radius<=reflectz) return;
                }
                else
                {
                    if(fogging && center.z+radius<reflectz-waterfog) return;
                    if(!shadow && center.z-radius>=reflectz) return;
                }
                if(center.dist(camera1->o)-radius>reflectdist) return;
            }
            if(isfoggedsphere(radius, center)) return;
            if(shadowmapping && !isshadowmapcaster(center, radius)) return;
        }
        if(shadowmapping)
        {
            if(d)
            {
                if(flags&MDL_CULL_OCCLUDED && d->occluded>=OCCLUDE_PARENT) return;
                if(doOQ && d->occluded+1>=OCCLUDE_BB && d->query && d->query->owner==d && checkquery(d->query)) return;
            }
            if(!addshadowmapcaster(center, radius, radius)) return;
        }
        else if(flags&MDL_CULL_OCCLUDED && modeloccluded(center, radius))
        {
            if(!reflecting && !refracting && d)
            {
                d->occluded = OCCLUDE_PARENT;
                if(doOQ) rendermodelquery(m, d, center, radius);
            }
            return;
        }
        else if(doOQ && d && d->query && d->query->owner==d && checkquery(d->query))
        {
            if(!reflecting && !refracting) 
            {
                if(d->occluded<OCCLUDE_BB) d->occluded++;
                rendermodelquery(m, d, center, radius);
            }
            return;
        }
    }

    if(flags&MDL_NORENDER) anim |= ANIM_NORENDER;
    else if(showboundingbox && !shadowmapping && !reflecting && !refracting && editmode)
    {
        if(d && showboundingbox==1) 
        {
            render3dbox(d->o, d->eyeheight, d->aboveeye, d->radius);
            renderellipse(d->o, d->xradius, d->yradius, d->yaw);
        }
        else
        {
            vec center, radius;
            if(showboundingbox==1) m->collisionbox(0, center, radius);
            else m->boundbox(0, center, radius);
            rotatebb(center, radius, int(yaw));
            center.add(o);
            render3dbox(center, radius.z, radius.z, radius.x, radius.y);
        }
    }

    vec lightcolor(1, 1, 1), lightdir(0, 0, 1);
    if(!shadowmapping)
    {
        vec pos = o;
        if(d) 
        {
            if(!reflecting && !refracting) d->occluded = OCCLUDE_NOTHING;
            if(!light) light = &d->light;
            if(flags&MDL_LIGHT && light->millis!=lastmillis)
            {
                if(d->ragdoll)
                {
                    pos = d->ragdoll->center;
                    pos.z += radius/2;
                }
                else if(d->type < ENT_CAMERA) pos.z += 0.75f*(d->eyeheight + d->aboveeye);
                lightreaching(pos, light->color, light->dir, (flags&MDL_LIGHT_FAST)!=0);
                dynlightreaching(pos, light->color, light->dir, (flags&MDL_HUD)!=0);
                game::lighteffects(d, light->color, light->dir);
                light->millis = lastmillis;
            }
        }
        else if(flags&MDL_LIGHT)
        {
            if(!light) 
            {
                lightreaching(pos, lightcolor, lightdir, (flags&MDL_LIGHT_FAST)!=0);
                dynlightreaching(pos, lightcolor, lightdir, (flags&MDL_HUD)!=0);
            }
            else if(light->millis!=lastmillis)
            {
                lightreaching(pos, light->color, light->dir, (flags&MDL_LIGHT_FAST)!=0);
                dynlightreaching(pos, light->color, light->dir, (flags&MDL_HUD)!=0);
                light->millis = lastmillis;
            }
        }
        if(light) { lightcolor = light->color; lightdir = light->dir; }
        if(flags&MDL_DYNLIGHT) dynlightreaching(pos, lightcolor, lightdir, (flags&MDL_HUD)!=0);
    }

    if(a) for(int i = 0; a[i].tag; i++)
    {
        if(a[i].name) a[i].m = loadmodel(a[i].name);
        //if(a[i].m && a[i].m->type()!=m->type()) a[i].m = NULL;
    }

    if(!d || reflecting || refracting || shadowmapping) doOQ = false;
  
    if(numbatches>=0)
    {
        modelbatch &mb = addbatchedmodel(m);
        batchedmodel &b = mb.batched.add();
        b.query = modelquery;
        b.pos = o;
        b.color = lightcolor;
        b.dir = lightdir;
        b.anim = anim;
        b.yaw = yaw;
        b.pitch = pitch;
        b.basetime = basetime;
        b.basetime2 = basetime2;
        b.transparent = trans;
        b.flags = flags & ~(MDL_CULL_VFC | MDL_CULL_DIST | MDL_CULL_OCCLUDED);
        if(!shadow || reflecting || refracting>0) 
        {
            b.flags &= ~(MDL_SHADOW|MDL_DYNSHADOW);
            if((flags&MDL_CULL_VFC) && refracting<0 && center.z-radius>=reflectz) b.flags |= MDL_CULL_VFC;
        }
        mb.flags |= b.flags;
        b.d = d;
        b.attached = a ? modelattached.length() : -1;
        if(a) for(int i = 0;; i++) { modelattached.add(a[i]); if(!a[i].tag) break; }
        if(doOQ) d->query = b.query = newquery(d);
        return;
    }

    if(shadow && !reflecting && refracting<=0)
    {
        renderblob(flags&MDL_DYNSHADOW ? BLOB_DYNAMIC : BLOB_STATIC, d && d->ragdoll ? center : o, d ? d->radius : max(bbradius.x, bbradius.y), trans);
        flushblobs();
        if((flags&MDL_CULL_VFC) && refracting<0 && center.z-radius>=reflectz) return;
    }

    m->startrender();

    if(shadowmapping)
    {
        anim |= ANIM_NOSKIN;
        if(renderpath!=R_FIXEDFUNCTION) setenvparamf("shadowintensity", SHPARAM_VERTEX, 1, trans);
    }
    else 
    {
        if(flags&MDL_FULLBRIGHT) anim |= ANIM_FULLBRIGHT;
        if(flags&MDL_GHOST) anim |= ANIM_GHOST;
    }

    if(doOQ)
    {
        d->query = newquery(d);
        if(d->query) startquery(d->query);
    }

    m->render(anim, basetime, basetime2, o, yaw, pitch, d, a, lightcolor, lightdir, trans);

    if(doOQ && d->query) endquery(d->query);

    m->endrender();
}

void abovemodel(vec &o, const char *mdl)
{
    model *m = loadmodel(mdl);
    if(!m) return;
    o.z += m->above(0/*frame*/);
}

bool matchanim(const char *name, const char *pattern)
{
    for(;; pattern++)
    {
        const char *s = name;
        char c;
        for(;; pattern++)
        {
            c = *pattern;
            if(!c || c=='|') break;
            else if(c=='*') 
            {
                if(!*s || iscubespace(*s)) break;
                do s++; while(*s && !iscubespace(*s));
            }
            else if(c!=*s) break;
            else s++;
        }
        if(!*s && (!c || c=='|')) return true;
        pattern = strchr(pattern, '|');
        if(!pattern) break;
    }
    return false;
}

void findanims(const char *pattern, vector<int> &anims)
{
    loopi(sizeof(animnames)/sizeof(animnames[0])) if(matchanim(animnames[i], pattern)) anims.add(i);
}

ICOMMAND(findanims, "s", (char *name),
{
    vector<int> anims;
    findanims(name, anims);
    vector<char> buf;
    string num;
    loopv(anims)
    {
        formatstring(num)("%d", anims[i]);
        if(i > 0) buf.add(' ');
        buf.put(num, strlen(num));
    }
    buf.add('\0');
    result(buf.getbuf());
});

void loadskin(const char *dir, const char *altdir, Texture *&skin, Texture *&masks) // model skin sharing
{
#define ifnoload(tex, path) if((tex = textureload(path, 0, true, false))==notexture)
#define tryload(tex, prefix, cmd, name) \
    ifnoload(tex, makerelpath(mdir, name ".jpg", prefix, cmd)) \
    { \
        ifnoload(tex, makerelpath(mdir, name ".png", prefix, cmd)) \
        { \
            ifnoload(tex, makerelpath(maltdir, name ".jpg", prefix, cmd)) \
            { \
                ifnoload(tex, makerelpath(maltdir, name ".png", prefix, cmd)) return; \
            } \
        } \
    }
   
    defformatstring(mdir)("packages/models/%s", dir);
    defformatstring(maltdir)("packages/models/%s", altdir);
    masks = notexture;
    tryload(skin, NULL, NULL, "skin");
    tryload(masks, "<stub>", NULL, "masks");
}

// convenient function that covers the usual anims for players/monsters/npcs

VAR(animoverride, -1, 0, NUMANIMS-1);
VAR(testanims, 0, 0, 1);
VAR(testpitch, -90, 0, 90);

void renderclient(dynent *d, const char *mdlname, modelattach *attachments, int hold, int attack, int attackdelay, int lastaction, int lastpain, float fade, bool ragdoll)
{
    int anim = hold ? hold : ANIM_IDLE|ANIM_LOOP;
    float yaw = testanims && d==player ? 0 : d->yaw+90,
          pitch = testpitch && d==player ? testpitch : d->pitch;
    vec o = d->feetpos();
    int basetime = 0;
    if(animoverride) anim = (animoverride<0 ? ANIM_ALL : animoverride)|ANIM_LOOP;
    else if(d->state==CS_DEAD)
    {
        anim = ANIM_DYING|ANIM_NOPITCH;
        basetime = lastpain;
        if(ragdoll)
        {
            if(!d->ragdoll || d->ragdoll->millis < basetime) anim |= ANIM_RAGDOLL;
        }
        else if(lastmillis-basetime>1000) anim = ANIM_DEAD|ANIM_LOOP|ANIM_NOPITCH;
    }
    else if(d->state==CS_EDITING || d->state==CS_SPECTATOR) anim = ANIM_EDIT|ANIM_LOOP;
    else if(d->state==CS_LAGGED)                            anim = ANIM_LAG|ANIM_LOOP;
    else
    {
        if(lastmillis-lastpain < 300) 
        { 
            anim = ANIM_PAIN;
            basetime = lastpain;
        }
        else if(lastpain < lastaction && (attack < 0 || (d->type != ENT_AI && lastmillis-lastaction < attackdelay)))
        { 
            anim = attack < 0 ? -attack : attack; 
            basetime = lastaction; 
        }

        if(d->inwater && d->physstate<=PHYS_FALL) anim |= (((game::allowmove(d) && (d->move || d->strafe)) || d->vel.z+d->falling.z>0 ? ANIM_SWIM : ANIM_SINK)|ANIM_LOOP)<<ANIM_SECONDARY;
        else if(d->timeinair>100) anim |= (ANIM_JUMP|ANIM_END)<<ANIM_SECONDARY;
        else if(game::allowmove(d) && (d->move || d->strafe)) 
        {
            if(d->move>0) anim |= (ANIM_FORWARD|ANIM_LOOP)<<ANIM_SECONDARY;
            else if(d->strafe) anim |= ((d->strafe>0 ? ANIM_LEFT : ANIM_RIGHT)|ANIM_LOOP)<<ANIM_SECONDARY;
            else if(d->move<0) anim |= (ANIM_BACKWARD|ANIM_LOOP)<<ANIM_SECONDARY;
        }
        
        if((anim&ANIM_INDEX)==ANIM_IDLE && (anim>>ANIM_SECONDARY)&ANIM_INDEX) anim >>= ANIM_SECONDARY;
    }
    if(d->ragdoll && (!ragdoll || (anim&ANIM_INDEX)!=ANIM_DYING)) DELETEP(d->ragdoll);
    if(!((anim>>ANIM_SECONDARY)&ANIM_INDEX)) anim |= (ANIM_IDLE|ANIM_LOOP)<<ANIM_SECONDARY;
    int flags = MDL_LIGHT;
    if(d!=player && !(anim&ANIM_RAGDOLL)) flags |= MDL_CULL_VFC | MDL_CULL_OCCLUDED | MDL_CULL_QUERY;
    if(d->type==ENT_PLAYER) flags |= MDL_FULLBRIGHT;
    else flags |= MDL_CULL_DIST;
    if(d->state==CS_LAGGED) fade = min(fade, 0.3f);
    else flags |= MDL_DYNSHADOW;
    rendermodel(NULL, mdlname, anim, o, yaw, pitch, flags, d, attachments, basetime, 0, fade);
}

void setbbfrommodel(dynent *d, const char *mdl)
{
    model *m = loadmodel(mdl); 
    if(!m) return;
    vec center, radius;
    m->collisionbox(0, center, radius);
    if(d->type==ENT_INANIMATE && !m->ellipsecollide)
    {
        d->collidetype = COLLIDE_OBB;
        //d->collidetype = COLLIDE_AABB;
        //rotatebb(center, radius, int(d->yaw));
    }
    d->xradius   = radius.x + fabs(center.x);
    d->yradius   = radius.y + fabs(center.y);
    d->radius    = d->collidetype==COLLIDE_OBB ? sqrtf(d->xradius*d->xradius + d->yradius*d->yradius) : max(d->xradius, d->yradius);
    d->eyeheight = (center.z-radius.z) + radius.z*2*m->eyeheight;
    d->aboveeye  = radius.z*2*(1.0f-m->eyeheight);
}

