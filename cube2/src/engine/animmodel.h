VARFP(lightmodels, 0, 1, 1, preloadmodelshaders());
VARFP(envmapmodels, 0, 1, 1, preloadmodelshaders());
VARFP(glowmodels, 0, 1, 1, preloadmodelshaders());
VARFP(bumpmodels, 0, 1, 1, preloadmodelshaders());
VARP(fullbrightmodels, 0, 0, 200);

struct animmodel : model
{
    struct animspec
    {
        int frame, range;
        float speed;
        int priority;
    };

    struct animpos
    {
        int anim, fr1, fr2;
        float t;

        void setframes(const animinfo &info)
        {
            anim = info.anim;
            if(info.range<=1) 
            {
                fr1 = 0;
                t = 0;
            }
            else
            {
                int time = info.anim&ANIM_SETTIME ? info.basetime : lastmillis-info.basetime;
                fr1 = (int)(time/info.speed); // round to full frames
                t = (time-fr1*info.speed)/info.speed; // progress of the frame, value from 0.0f to 1.0f
            }
            if(info.anim&ANIM_LOOP)
            {
                fr1 = fr1%info.range+info.frame;
                fr2 = fr1+1;
                if(fr2>=info.frame+info.range) fr2 = info.frame;
            }
            else
            {
                fr1 = min(fr1, info.range-1)+info.frame;
                fr2 = min(fr1+1, info.frame+info.range-1);
            }
            if(info.anim&ANIM_REVERSE)
            {
                fr1 = (info.frame+info.range-1)-(fr1-info.frame);
                fr2 = (info.frame+info.range-1)-(fr2-info.frame);
            }
        }

        bool operator==(const animpos &a) const { return fr1==a.fr1 && fr2==a.fr2 && (fr1==fr2 || t==a.t); }
        bool operator!=(const animpos &a) const { return fr1!=a.fr1 || fr2!=a.fr2 || (fr1!=fr2 && t!=a.t); }
    };

    struct part;

    struct animstate
    {
        part *owner;
        animpos cur, prev;
        float interp;

        bool operator==(const animstate &a) const { return cur==a.cur && (interp<1 ? interp==a.interp && prev==a.prev : a.interp>=1); }
        bool operator!=(const animstate &a) const { return cur!=a.cur || (interp<1 ? interp!=a.interp || prev!=a.prev : a.interp<1); }
    };

    struct linkedpart;
    struct mesh;

    struct skin
    {
        part *owner;
        Texture *tex, *masks, *envmap, *unlittex, *normalmap;
        Shader *shader;
        float spec, ambient, glow, glowdelta, glowpulse, specglare, glowglare, fullbright, envmapmin, envmapmax, scrollu, scrollv, alphatest;
        bool alphablend, cullface;

        // XXX EMSCRIPTEN: alphatest set to 0 instead of 0.9f, until we emulate alphatest in rewritten shaders
        skin() : owner(0), tex(notexture), masks(notexture), envmap(NULL), unlittex(NULL), normalmap(NULL), shader(NULL), spec(1.0f), ambient(0.3f), glow(3.0f), glowdelta(0), glowpulse(0), specglare(1), glowglare(1), fullbright(0), envmapmin(0), envmapmax(0), scrollu(0), scrollv(0), alphatest(0), alphablend(true), cullface(true) {}

        bool multitextured() { return enableglow; }
        bool envmapped() { return hasCM && envmapmax>0 && envmapmodels && (renderpath!=R_FIXEDFUNCTION || maxtmus >= 3); }
        bool bumpmapped() { return renderpath!=R_FIXEDFUNCTION && normalmap && bumpmodels; }
        bool normals() { return renderpath!=R_FIXEDFUNCTION || (lightmodels && !fullbright) || envmapped() || bumpmapped(); }
        bool tangents() { return bumpmapped(); }

        void setuptmus(const animstate *as, bool masked)
        {
            if(fullbright)
            {
                if(enablelighting) { glDisable(GL_LIGHTING); enablelighting = false; }
            }
            else if(lightmodels && !enablelighting)
            {
                glEnable(GL_LIGHTING); 
                enablelighting = true;
                if(!enablerescale) { glEnable(hasRN ? GL_RESCALE_NORMAL_EXT : GL_NORMALIZE); enablerescale = true; }
            }
            if(masked!=enableglow) lasttex = lastmasks = NULL;
            float mincolor = as->cur.anim&ANIM_FULLBRIGHT ? fullbrightmodels/100.0f : 0.0f;
            vec color = vec(lightcolor).max(mincolor), matcolor(1, 1, 1);
            if(masked)
            {
                bool needenvmap = envmaptmu>=0 && envmapmax>0;
                if(enableoverbright) disableoverbright();
                if(!enableglow || enableenvmap!=needenvmap) setuptmu(0, "1 , C @ T", needenvmap ? "= Ta" : "= Ca");
                int glowscale = glow>2 ? 4 : (glow>1 || mincolor>1 ? 2 : 1);
                matcolor.div(glowscale);

                glActiveTexture_(GL_TEXTURE1_ARB);
                if(!enableglow || enableenvmap!=needenvmap)
                {
                    if(!enableglow) glEnable(GL_TEXTURE_2D);
                    setuptmu(1, "P * T", needenvmap ? "= Pa" : "Pa * Ta");
                }
                scaletmu(1, glowscale);
                glActiveTexture_(GL_TEXTURE0_ARB);

                enableglow = true;
            }
            else
            {
                if(enableglow) disableglow();
                if(mincolor>1 && maxtmus>=1)
                {
                    matcolor.div(2);
                    if(!enableoverbright) { setuptmu(0, "C * T x 2"); enableoverbright = true; }
                }
                else if(enableoverbright) disableoverbright();
            }
            if(fullbright) glColor4f(matcolor.x*fullbright, matcolor.y*fullbright, matcolor.z*fullbright, transparent);
            else if(lightmodels)
            {
                GLfloat material[4] = { matcolor.x, matcolor.y, matcolor.z, transparent };
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
            }
            else glColor4f(matcolor.x*color.x, matcolor.y*color.y, matcolor.z*color.z, transparent);
            if(lightmodels && !fullbright)
            {
                float ambientk = min(max(ambient, mincolor)*0.75f, 1.0f),
                      diffusek = 1-ambientk;
                GLfloat ambientcol[4] = { color.x*ambientk, color.y*ambientk, color.z*ambientk, 1 },
                        diffusecol[4] = { color.x*diffusek, color.y*diffusek, color.z*diffusek, 1 };
                float ambientmax = max(ambientcol[0], max(ambientcol[1], ambientcol[2])),
                      diffusemax = max(diffusecol[0], max(diffusecol[1], diffusecol[2]));
                if(ambientmax>1e-3f) loopk(3) ambientcol[k] *= min(1.5f, 1.0f/min(ambientmax, 1.0f));
                if(diffusemax>1e-3f) loopk(3) diffusecol[k] *= min(1.5f, 1.0f/min(diffusemax, 1.0f));
                glLightfv(GL_LIGHT0, GL_AMBIENT, ambientcol);
                glLightfv(GL_LIGHT0, GL_DIFFUSE, diffusecol);
            }
        }

        void setshaderparams(mesh *m, const animstate *as, bool masked)
        {
            if(fullbright)
            {
                glColor4f(fullbright/2, fullbright/2, fullbright/2, transparent);
                setenvparamf("lightscale", SHPARAM_VERTEX, 2, 0, 0, 2);
                setenvparamf("lightscale", SHPARAM_PIXEL, 2, 0, 0, 2);
            }
            else
            {
                float mincolor = as->cur.anim&ANIM_FULLBRIGHT ? fullbrightmodels/100.0f : 0.0f, 
                      bias = max(mincolor-1.0f, 0.2f), scale = 0.5f*max(0.8f-bias, 0.0f), 
                      minshade = scale*max(ambient, mincolor);
                vec color = vec(lightcolor).max(mincolor);
                glColor4f(color.x, color.y, color.z, transparent);
                setenvparamf("lightscale", SHPARAM_VERTEX, 2, scale - minshade, scale, minshade + bias);
                setenvparamf("lightscale", SHPARAM_PIXEL, 2, scale - minshade, scale, minshade + bias);
            }
            float curglow = glow;
            if(glowpulse > 0)
            {
                float curpulse = lastmillis*glowpulse;
                curpulse -= floor(curpulse);
                curglow += glowdelta*2*fabs(curpulse - 0.5f);
            }
            setenvparamf("maskscale", SHPARAM_PIXEL, 4, 0.5f*spec*lightmodels, 0.5f*curglow*glowmodels, 16*specglare, 4*glowglare);
            setenvparamf("texscroll", SHPARAM_VERTEX, 5, lastmillis/1000.0f, scrollu*lastmillis/1000.0f, scrollv*lastmillis/1000.0f);
            if(envmaptmu>=0 && envmapmax>0) setenvparamf("envmapscale", bumpmapped() ? SHPARAM_PIXEL : SHPARAM_VERTEX, 3, envmapmin-envmapmax, envmapmax);
        }

        Shader *loadshader(bool shouldenvmap, bool masked)
        {
            #define DOMODELSHADER(name, body) \
                do { \
                    static Shader *name##shader = NULL; \
                    if(!name##shader) name##shader = useshaderbyname(#name); \
                    body; \
                } while(0)
            #define LOADMODELSHADER(name) DOMODELSHADER(name, return name##shader)
            #define SETMODELSHADER(m, name) DOMODELSHADER(name, (m)->setshader(name##shader))
            if(shader) return shader;
            else if(bumpmapped())
            {
                if(shouldenvmap)
                {
                    if(lightmodels && !fullbright && (masked || spec>=0.01f)) LOADMODELSHADER(bumpenvmapmodel);
                    else LOADMODELSHADER(bumpenvmapnospecmodel);
                }
                else if(masked && lightmodels && !fullbright) LOADMODELSHADER(bumpmasksmodel);
                else if(masked && glowmodels) LOADMODELSHADER(bumpmasksnospecmodel);
                else if(spec>=0.01f && lightmodels && !fullbright) LOADMODELSHADER(bumpmodel);
                else LOADMODELSHADER(bumpnospecmodel);
            }
            else if(shouldenvmap)
            {
                if(lightmodels && !fullbright && (masked || spec>=0.01f)) LOADMODELSHADER(envmapmodel);
                else LOADMODELSHADER(envmapnospecmodel);
            }
            else if(masked && lightmodels && !fullbright) LOADMODELSHADER(masksmodel);
            else if(masked && glowmodels) LOADMODELSHADER(masksnospecmodel);
            else if(spec>=0.01f && lightmodels && !fullbright) LOADMODELSHADER(stdmodel);
            else LOADMODELSHADER(nospecmodel);
        }

        void preloadBIH()
        {
            if(tex && tex->type&Texture::ALPHA && !tex->alphamask) loadalphamask(tex);
        }
 
        void preloadshader()
        {
            bool shouldenvmap = envmapped();
            if(masks->type&Texture::STUB && !strncmp(masks->name, "<stub>", 6))
            {
                float glowscale = glow/(glow>2 ? 4 : (glow>1 ? 2 : 1)),
                      envscale = envmapmax > 0 ? 0.2f*envmapmax + 0.8f*envmapmin : 0;
                defformatstring(ffmask)("<ffmask:%.2f,%.2f>", floor(glowscale*16 + 0.5f)/16, floor(envscale*16 + 0.5f)/16);
                masks = textureload(makerelpath(NULL, masks->name + 6, NULL, ffmask), 0, true, false);
            }
            loadshader(shouldenvmap, masks!=notexture && !(masks->type&Texture::STUB) && (lightmodels || glowmodels || shouldenvmap));
        }
 
        void setshader(mesh *m, const animstate *as, bool masked)
        {
            m->setshader(loadshader(envmaptmu>=0 && envmapmax>0, masked));
        }

        void bind(mesh *b, const animstate *as)
        {
            if(!cullface && enablecullface) { glDisable(GL_CULL_FACE); enablecullface = false; }
            else if(cullface && !enablecullface) { glEnable(GL_CULL_FACE); enablecullface = true; }

            if(as->cur.anim&ANIM_NOSKIN)
            {
                if(enablealphatest) { glDisable(GL_ALPHA_TEST); enablealphatest = false; }
                if(enablealphablend) { glDisable(GL_BLEND); enablealphablend = false; }
                if(enableglow) disableglow();
                if(enableenvmap) disableenvmap();
                if(enablelighting) { glDisable(GL_LIGHTING); enablelighting = false; }
                if(enablerescale) { glDisable(hasRN ? GL_RESCALE_NORMAL_EXT : GL_NORMALIZE); enablerescale = false; }
                if(shadowmapping) SETMODELSHADER(b, shadowmapcaster);
                else /*if(as->cur.anim&ANIM_SHADOW)*/ SETMODELSHADER(b, notexturemodel);
                return;
            }
            Texture *s = bumpmapped() && unlittex ? unlittex : tex, 
                    *m = masks->type&Texture::STUB ? notexture : masks, 
                    *n = bumpmapped() ? normalmap : NULL;
            if((renderpath==R_FIXEDFUNCTION || !lightmodels) &&
               !glowmodels && (!envmapmodels || envmaptmu<0 || envmapmax<=0))
                m = notexture;
            if(renderpath==R_FIXEDFUNCTION) setuptmus(as, m!=notexture);
            else
            {
                setshaderparams(b, as, m!=notexture);
                setshader(b, as, m!=notexture);
            }
            if(s!=lasttex)
            {
                if(enableglow) glActiveTexture_(GL_TEXTURE1_ARB);
                glBindTexture(GL_TEXTURE_2D, s->id);
                if(enableglow) glActiveTexture_(GL_TEXTURE0_ARB);
                lasttex = s;
            }
            if(n && n!=lastnormalmap)
            {
                glActiveTexture_(GL_TEXTURE3_ARB);
                glBindTexture(GL_TEXTURE_2D, n->id);
                glActiveTexture_(GL_TEXTURE0_ARB);
            }
            if(s->type&Texture::ALPHA)
            {
                if(alphablend)
                {
                    if(!enablealphablend && !reflecting && !refracting)
                    {
                        glEnable(GL_BLEND);
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                        enablealphablend = true;
                    }
                }
                else if(enablealphablend) { glDisable(GL_BLEND); enablealphablend = false; }
                if(alphatest>0)
                {
                    if(!enablealphatest) { glEnable(GL_ALPHA_TEST); enablealphatest = true; }
                    if(lastalphatest!=alphatest)
                    {
                        glAlphaFunc(GL_GREATER, alphatest);
                        lastalphatest = alphatest;
                    }
                }
                else if(enablealphatest) { glDisable(GL_ALPHA_TEST); enablealphatest = false; }
            }
            else
            {
                if(enablealphatest) { glDisable(GL_ALPHA_TEST); enablealphatest = false; }
                if(enablealphablend && transparent>=1) { glDisable(GL_BLEND); enablealphablend = false; }
            }
            if(m!=lastmasks && m!=notexture)
            {
                if(!enableglow) glActiveTexture_(GL_TEXTURE1_ARB);
                glBindTexture(GL_TEXTURE_2D, m->id);
                if(!enableglow) glActiveTexture_(GL_TEXTURE0_ARB);
                lastmasks = m;
            }
            if((renderpath!=R_FIXEDFUNCTION || m!=notexture) && envmaptmu>=0 && envmapmax>0)
            {
                GLuint emtex = envmap ? envmap->id : closestenvmaptex;
                if(!enableenvmap || lastenvmaptex!=emtex)
                {
                    glActiveTexture_(GL_TEXTURE0_ARB+envmaptmu);
                    if(!enableenvmap)
                    {
                        glEnable(GL_TEXTURE_CUBE_MAP_ARB);
                        if(!lastenvmaptex && renderpath==R_FIXEDFUNCTION)
                        {
                            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB);
                            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB);
                            glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB);
                            glEnable(GL_TEXTURE_GEN_S);
                            glEnable(GL_TEXTURE_GEN_T);
                            glEnable(GL_TEXTURE_GEN_R);
                        }
                        enableenvmap = true;
                        if(!enablerescale) { glEnable(hasRN ? GL_RESCALE_NORMAL_EXT : GL_NORMALIZE); enablerescale = true; }
                    }
                    if(lastenvmaptex!=emtex) { glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, emtex); lastenvmaptex = emtex; }
                    glActiveTexture_(GL_TEXTURE0_ARB);
                }
            }
            else if(enableenvmap) disableenvmap();
        }
    };

    struct meshgroup;

    struct mesh
    {
        meshgroup *group;
        char *name;
        bool noclip;

        mesh() : group(NULL), name(NULL), noclip(false)
        {
        }

        virtual ~mesh()
        {
            DELETEA(name);
        }

        virtual void calcbb(int frame, vec &bbmin, vec &bbmax, const matrix3x4 &m) {}
        virtual void gentris(int frame, Texture *tex, vector<BIH::tri> *out, const matrix3x4 &m) {}

        virtual void setshader(Shader *s) 
        { 
            if(glaring) s->setvariant(0, 2);
            else s->set(); 
        }

        template<class V, class T> void smoothnorms(V *verts, int numverts, T *tris, int numtris, float limit, bool areaweight)
        {
            hashtable<vec, int> share;
            int *next = new int[numverts];
            memset(next, -1, numverts*sizeof(int));
            loopi(numverts)
            {
                V &v = verts[i];
                v.norm = vec(0, 0, 0);
                int idx = share.access(v.pos, i);
                if(idx != i) { next[i] = next[idx]; next[idx] = i; }
            }
            loopi(numtris)
            {
                T &t = tris[i];
                V &v1 = verts[t.vert[0]], &v2 = verts[t.vert[1]], &v3 = verts[t.vert[2]];
                vec norm;
                norm.cross(vec(v2.pos).sub(v1.pos), vec(v3.pos).sub(v1.pos));
                if(!areaweight) norm.normalize();
                v1.norm.add(norm);
                v2.norm.add(norm);
                v3.norm.add(norm);
            }
            vec *norms = new vec[numverts];
            memset(norms, 0, numverts*sizeof(vec));
            loopi(numverts)
            {
                V &v = verts[i];
                norms[i].add(v.norm);
                if(next[i] >= 0)
                {
                    float vlimit = limit*v.norm.magnitude();
                    for(int j = next[i]; j >= 0; j = next[j])
                    {
                        V &o = verts[j];
                        if(v.norm.dot(o.norm) >= vlimit*o.norm.magnitude())
                        {
                            norms[i].add(o.norm);
                            norms[j].add(v.norm);
                        }
                    }
                }
            }
            loopi(numverts) verts[i].norm = norms[i].normalize();
            delete[] next;
            delete[] norms;
        }

        template<class V, class T> void buildnorms(V *verts, int numverts, T *tris, int numtris, bool areaweight)
        {
            loopi(numverts) verts[i].norm = vec(0, 0, 0);
            loopi(numtris)
            {
                T &t = tris[i];
                V &v1 = verts[t.vert[0]], &v2 = verts[t.vert[1]], &v3 = verts[t.vert[2]];
                vec norm;
                norm.cross(vec(v2.pos).sub(v1.pos), vec(v3.pos).sub(v1.pos));
                if(!areaweight) norm.normalize();
                v1.norm.add(norm);
                v2.norm.add(norm);
                v3.norm.add(norm);
            }
            loopi(numverts) verts[i].norm.normalize();
        }
        
        template<class B, class V, class TC, class T> void calctangents(B *bumpverts, V *verts, TC *tcverts, int numverts, T *tris, int numtris, bool areaweight)
        {
            vec *tangent = new vec[2*numverts], *bitangent = tangent+numverts;
            memset(tangent, 0, 2*numverts*sizeof(vec));
            loopi(numtris)
            {
                const T &t = tris[i];
                const vec &e0 = verts[t.vert[0]].pos;
                vec e1 = vec(verts[t.vert[1]].pos).sub(e0), e2 = vec(verts[t.vert[2]].pos).sub(e0);

                const TC &tc0 = tcverts[t.vert[0]],
                         &tc1 = tcverts[t.vert[1]],
                         &tc2 = tcverts[t.vert[2]];
                float u1 = tc1.u - tc0.u, v1 = tc1.v - tc0.v,
                      u2 = tc2.u - tc0.u, v2 = tc2.v - tc0.v;
                vec u(e2), v(e2);
                u.mul(v1).sub(vec(e1).mul(v2));
                v.mul(u1).sub(vec(e1).mul(u2));

                if(vec().cross(e2, e1).dot(vec().cross(v, u)) >= 0)
                {
                    u.neg();
                    v.neg();
                }

                if(!areaweight)
                {
                    u.normalize();
                    v.normalize();
                }

                loopj(3)
                {
                    tangent[t.vert[j]].sub(u);
                    bitangent[t.vert[j]].add(v);
                }
            }
            loopi(numverts)
            {
                const vec &n = verts[i].norm,
                          &t = tangent[i],
                          &bt = bitangent[i];
                B &bv = bumpverts[i];
                (bv.tangent = t).sub(vec(n).mul(n.dot(t))).normalize();
                bv.bitangent = vec().cross(n, t).dot(bt) < 0 ? -1 : 1;
            }
            delete[] tangent;
        }
    
    };

    struct meshgroup
    {
        meshgroup *next;
        int shared;
        char *name;
        vector<mesh *> meshes;
        float scale;
        vec translate;

        meshgroup() : next(NULL), shared(0), name(NULL), scale(1), translate(0, 0, 0)
        {
        }

        virtual ~meshgroup()
        {
            DELETEA(name);
            meshes.deletecontents();
            DELETEP(next);
        }            

        virtual int findtag(const char *name) { return -1; }
        virtual void concattagtransform(part *p, int frame, int i, const matrix3x4 &m, matrix3x4 &n) {}

        void calcbb(int frame, vec &bbmin, vec &bbmax, const matrix3x4 &m)
        {
            loopv(meshes) meshes[i]->calcbb(frame, bbmin, bbmax, m);
        }

        void gentris(int frame, vector<skin> &skins, vector<BIH::tri> *tris, const matrix3x4 &m)
        {
            loopv(meshes) meshes[i]->gentris(frame, skins[i].tex && skins[i].tex->type&Texture::ALPHA ? skins[i].tex : NULL, tris, m);
        }

        virtual int totalframes() const { return 1; }
        bool hasframe(int i) const { return i>=0 && i<totalframes(); }
        bool hasframes(int i, int n) const { return i>=0 && i+n<=totalframes(); }
        int clipframes(int i, int n) const { return min(n, totalframes() - i); }

        virtual void cleanup() {}
        virtual void render(const animstate *as, float pitch, const vec &axis, const vec &forward, dynent *d, part *p) {}
    };

    virtual meshgroup *loadmeshes(char *name, va_list args) { return NULL; }

    meshgroup *sharemeshes(char *name, ...)
    {
        static hashtable<char *, meshgroup *> meshgroups;
        if(!meshgroups.access(name))
        {
            va_list args;
            va_start(args, name);
            meshgroup *group = loadmeshes(name, args);
            va_end(args);
            if(!group) return NULL;
            meshgroups[group->name] = group;
        }
        return meshgroups[name];
    }

    struct linkedpart
    {
        part *p;
        int tag, anim, basetime;
        vec translate;
        vec *pos;
        glmatrixf matrix;

        linkedpart() : p(NULL), tag(-1), anim(-1), basetime(0), translate(0, 0, 0), pos(NULL) {}
    };

    struct part
    {
        animmodel *model;
        int index;
        meshgroup *meshes;
        vector<linkedpart> links;
        vector<skin> skins;
        vector<animspec> *anims[MAXANIMPARTS];
        int numanimparts;
        float pitchscale, pitchoffset, pitchmin, pitchmax;
        vec translate;

        part() : meshes(NULL), numanimparts(1), pitchscale(1), pitchoffset(0), pitchmin(0), pitchmax(0), translate(0, 0, 0)
        {
            loopk(MAXANIMPARTS) anims[k] = NULL;
        }
        virtual ~part()
        {
            loopk(MAXANIMPARTS) DELETEA(anims[k]);
        }

        virtual void cleanup()
        {
            if(meshes) meshes->cleanup();
        }

        void calcbb(int frame, vec &bbmin, vec &bbmax, const matrix3x4 &m)
        {
            matrix3x4 t = m;
            t.translate(translate);
            t.scale(model->scale);
            meshes->calcbb(frame, bbmin, bbmax, t);
            loopv(links)
            {
                matrix3x4 n;
                meshes->concattagtransform(this, frame, links[i].tag, m, n);
                n.transformedtranslate(links[i].translate, model->scale);
                links[i].p->calcbb(frame, bbmin, bbmax, n);
            }
        }

        void gentris(int frame, vector<BIH::tri> *tris, const matrix3x4 &m)
        {
            matrix3x4 t = m;
            t.translate(translate);
            t.scale(model->scale);
            meshes->gentris(frame, skins, tris, t);
            loopv(links)
            {
                matrix3x4 n;
                meshes->concattagtransform(this, frame, links[i].tag, m, n);
                n.transformedtranslate(links[i].translate, model->scale);
                links[i].p->gentris(frame, tris, n);
            }
        }

        bool link(part *p, const char *tag, const vec &translate = vec(0, 0, 0), int anim = -1, int basetime = 0, vec *pos = NULL)
        {
            int i = meshes ? meshes->findtag(tag) : -1;
            if(i<0)
            {
                loopv(links) if(links[i].p && links[i].p->link(p, tag, translate, anim, basetime, pos)) return true;
                return false;
            }
            linkedpart &l = links.add();
            l.p = p;
            l.tag = i;
            l.anim = anim;
            l.basetime = basetime;
            l.translate = translate;
            l.pos = pos;
            return true;
        }

        bool unlink(part *p)
        {
            loopvrev(links) if(links[i].p==p) { links.remove(i, 1); return true; }
            loopv(links) if(links[i].p && links[i].p->unlink(p)) return true;
            return false;
        }

        void initskins(Texture *tex = notexture, Texture *masks = notexture, int limit = 0)
        {
            if(!limit)
            {
                if(!meshes) return;
                limit = meshes->meshes.length();
            }
            while(skins.length() < limit)
            {
                skin &s = skins.add();
                s.owner = this;
                s.tex = tex;
                s.masks = masks;
            }
        }

        void preloadBIH()
        {
            loopv(skins) skins[i].preloadBIH();
        }

        void preloadshaders()
        {
            loopv(skins) skins[i].preloadshader();
        }

        virtual void getdefaultanim(animinfo &info, int anim, uint varseed, dynent *d)
        {
            info.frame = 0;
            info.range = 1;
        }

        bool calcanim(int animpart, int anim, int basetime, int basetime2, dynent *d, int interp, animinfo &info, int &aitime)
        {
            uint varseed = uint((size_t)d);
            info.anim = anim;
            info.basetime = basetime;
            info.varseed = varseed;
            info.speed = anim&ANIM_SETSPEED ? basetime2 : 100.0f;
            if((anim&ANIM_INDEX)==ANIM_ALL)
            {
                info.frame = 0;
                info.range = meshes->totalframes();
            }
            else 
            {
                animspec *spec = NULL;
                if(anims[animpart])
                {
                    vector<animspec> &primary = anims[animpart][anim&ANIM_INDEX];
                    if(&primary < &anims[animpart][NUMANIMS] && primary.length()) spec = &primary[uint(varseed + basetime)%primary.length()];
                    if((anim>>ANIM_SECONDARY)&(ANIM_INDEX|ANIM_DIR))
                    {
                        vector<animspec> &secondary = anims[animpart][(anim>>ANIM_SECONDARY)&ANIM_INDEX];
                        if(&secondary < &anims[animpart][NUMANIMS] && secondary.length())
                        {
                            animspec &spec2 = secondary[uint(varseed + basetime2)%secondary.length()];
                            if(!spec || spec2.priority > spec->priority)
                            {
                                spec = &spec2;
                                info.anim >>= ANIM_SECONDARY;
                                info.basetime = basetime2;
                            }
                        }
                    }
                }
                if(spec)
                {
                    info.frame = spec->frame;
                    info.range = spec->range;
                    if(spec->speed>0) info.speed = 1000.0f/spec->speed;
                }
                else getdefaultanim(info, anim, uint(varseed + info.basetime), d);
            }

            info.anim &= (1<<ANIM_SECONDARY)-1;
            info.anim |= anim&ANIM_FLAGS;
            if((info.anim&ANIM_CLAMP) != ANIM_CLAMP)
            {
                if(info.anim&(ANIM_LOOP|ANIM_START|ANIM_END))
                {
                    info.anim &= ~ANIM_SETTIME;
                    if(!info.basetime) info.basetime = -((int)(size_t)d&0xFFF);
                }
                if(info.anim&(ANIM_START|ANIM_END))
                {
                    if(info.anim&ANIM_END) info.frame += info.range-1;
                    info.range = 1;
                }
            }

            if(!meshes->hasframes(info.frame, info.range))
            {
                if(!meshes->hasframe(info.frame)) return false;
                info.range = meshes->clipframes(info.frame, info.range);
            }

            if(d && interp>=0)
            {
                animinterpinfo &ai = d->animinterp[interp];
                if((info.anim&ANIM_CLAMP)==ANIM_CLAMP) aitime = min(aitime, int(info.range*info.speed*0.5e-3f));
                if(d->ragdoll && !(anim&ANIM_RAGDOLL)) 
                {
                    ai.prev.range = ai.cur.range = 0;
                    ai.lastswitch = -1;
                }
                else if(ai.lastmodel!=this || ai.lastswitch<0 || lastmillis-d->lastrendered>aitime)
                {
                    ai.prev = ai.cur = info;
                    ai.lastswitch = lastmillis-aitime*2;
                }
                else if(ai.cur!=info)
                {
                    if(lastmillis-ai.lastswitch>aitime/2) ai.prev = ai.cur;
                    ai.cur = info;
                    ai.lastswitch = lastmillis;
                }
                else if(info.anim&ANIM_SETTIME) ai.cur.basetime = info.basetime;
                ai.lastmodel = this;
            }
            return true;
        }

        void render(int anim, int basetime, int basetime2, float pitch, const vec &axis, const vec &forward, dynent *d)
        {
            animstate as[MAXANIMPARTS];
            render(anim, basetime, basetime2, pitch, axis, forward, d, as);
        }

        void render(int anim, int basetime, int basetime2, float pitch, const vec &axis, const vec &forward, dynent *d, animstate *as)
        {
            if(!(anim&ANIM_REUSE)) loopi(numanimparts)
            {
                animinfo info;
                int interp = d && index+numanimparts<=MAXANIMPARTS ? index+i : -1, aitime = animationinterpolationtime;
                if(!calcanim(i, anim, basetime, basetime2, d, interp, info, aitime)) return;
                animstate &p = as[i];
                p.owner = this;
                p.cur.setframes(info);
                p.interp = 1;
                if(interp>=0 && d->animinterp[interp].prev.range>0)
                {
                    int diff = lastmillis-d->animinterp[interp].lastswitch;
                    if(diff<aitime)
                    {
                        p.prev.setframes(d->animinterp[interp].prev);
                        p.interp = diff/float(aitime);
                    }
                }
            }

            vec oaxis, oforward;
            matrixstack[matrixpos].transposedtransformnormal(axis, oaxis);
            float pitchamount = pitchscale*pitch + pitchoffset;
            if(pitchmin || pitchmax) pitchamount = clamp(pitchamount, pitchmin, pitchmax);
            if(as->cur.anim&ANIM_NOPITCH || (as->interp < 1 && as->prev.anim&ANIM_NOPITCH))
                pitchamount *= (as->cur.anim&ANIM_NOPITCH ? 0 : as->interp) + (as->interp < 1 && as->prev.anim&ANIM_NOPITCH ? 0 : 1-as->interp);
            if(pitchamount)
            {
                ++matrixpos;
                matrixstack[matrixpos] = matrixstack[matrixpos-1];
                matrixstack[matrixpos].rotate(pitchamount*RAD, oaxis);
            }
            matrixstack[matrixpos].transposedtransformnormal(forward, oforward);

            if(!(anim&ANIM_NORENDER))
            {
                glPushMatrix();
                glMultMatrixf(matrixstack[matrixpos].v);
                if(model->scale!=1) glScalef(model->scale, model->scale, model->scale);
                if(!translate.iszero()) glTranslatef(translate.x, translate.y, translate.z);
                if(renderpath!=R_FIXEDFUNCTION && envmaptmu>=0)
                {
                    glMatrixMode(GL_TEXTURE);
                    glLoadMatrixf(matrixstack[matrixpos].v);
                    glMatrixMode(GL_MODELVIEW);
                }
            }

            if(!(anim&(ANIM_NOSKIN|ANIM_NORENDER)))
            {
                if(renderpath!=R_FIXEDFUNCTION)
                {
                    vec odir, ocampos;
                    matrixstack[matrixpos].transposedtransformnormal(lightdir, odir);
                    setenvparamf("lightdir", SHPARAM_VERTEX, 0, odir.x, odir.y, odir.z);
                    setenvparamf("lightdir", SHPARAM_PIXEL, 0, odir.x, odir.y, odir.z);
                    matrixstack[matrixpos].transposedtransform(camera1->o, ocampos);
                    ocampos.div(model->scale).sub(translate);
                    setenvparamf("camera", SHPARAM_VERTEX, 1, ocampos.x, ocampos.y, ocampos.z, 1);
                }
            }

            meshes->render(as, pitch, oaxis, oforward, d, this);

            if(!(anim&ANIM_NORENDER))
            {
                glPopMatrix();
            }

            if(!(anim&ANIM_REUSE)) 
            {
                loopv(links)
                {
                    linkedpart &link = links[i];
                    link.matrix.transformedtranslate(links[i].translate, model->scale);

                    matrixpos++;
                    matrixstack[matrixpos].mul(matrixstack[matrixpos-1], link.matrix);

                    if(link.pos) *link.pos = matrixstack[matrixpos].gettranslation();

                    if(!link.p)
                    {
                        matrixpos--;
                        continue;
                    }

                    int nanim = anim, nbasetime = basetime, nbasetime2 = basetime2;
                    if(link.anim>=0)
                    {
                        nanim = link.anim | (anim&ANIM_FLAGS);
                        nbasetime = link.basetime;
                        nbasetime2 = 0;
                    }
                    link.p->render(nanim, nbasetime, nbasetime2, pitch, axis, forward, d);

                    matrixpos--;
                }
            }

            if(pitchamount) matrixpos--;
        }

        void setanim(int animpart, int num, int frame, int range, float speed, int priority = 0)
        {
            if(animpart<0 || animpart>=MAXANIMPARTS) return;
            if(frame<0 || range<=0 || !meshes || !meshes->hasframes(frame, range))
            {
                conoutf("invalid frame %d, range %d in model %s", frame, range, model->loadname);
                return;
            }
            if(!anims[animpart]) anims[animpart] = new vector<animspec>[NUMANIMS];
            animspec &spec = anims[animpart][num].add();
            spec.frame = frame;
            spec.range = range;
            spec.speed = speed;
            spec.priority = priority;
        }
    };

    enum
    {
        LINK_TAG = 0,
        LINK_COOP,
        LINK_REUSE
    };

    virtual int linktype(animmodel *m) const { return LINK_TAG; }

    void render(int anim, int basetime, int basetime2, float pitch, const vec &axis, const vec &forward, dynent *d, modelattach *a)
    {
        if(!loaded) return;

        int numtags = 0;
        if(a)
        {
            int index = parts.last()->index + parts.last()->numanimparts;
            for(int i = 0; a[i].tag; i++)
            {
                numtags++;

                animmodel *m = (animmodel *)a[i].m;
                if(!m || !m->loaded)
                {
                    if(a[i].pos) link(NULL, a[i].tag, vec(0, 0, 0), 0, 0, a[i].pos);
                    continue;
                }
                part *p = m->parts[0];
                switch(linktype(m))
                {
                    case LINK_TAG:
                        p->index = link(p, a[i].tag, vec(0, 0, 0), a[i].anim, a[i].basetime, a[i].pos) ? index : -1;
                        break;

                    case LINK_COOP:
                        p->index = index;
                        break;

                    default:
                        continue;
                }
                index += p->numanimparts;
            }
        }

        animstate as[MAXANIMPARTS];
        parts[0]->render(anim, basetime, basetime2, pitch, axis, forward, d, as);

        if(a) for(int i = numtags-1; i >= 0; i--)
        {
            animmodel *m = (animmodel *)a[i].m;
            if(!m || !m->loaded)
            {
                if(a[i].pos) unlink(NULL);
                continue;
            }
            part *p = m->parts[0];
            switch(linktype(m))
            {
                case LINK_TAG:    
                    if(p->index >= 0) unlink(p);
                    p->index = 0;
                    break;

                case LINK_COOP:
                    p->render(anim, basetime, basetime2, pitch, axis, forward, d);
                    p->index = 0;
                    break;

                case LINK_REUSE:
                    p->render(anim | ANIM_REUSE, basetime, basetime2, pitch, axis, forward, d, as); 
                    break;
            }
        }
    }

    void render(int anim, int basetime, int basetime2, const vec &o, float yaw, float pitch, dynent *d, modelattach *a, const vec &color, const vec &dir, float trans)
    {
        if(!loaded) return;

        yaw += spinyaw*lastmillis/1000.0f;
        pitch += offsetpitch + spinpitch*lastmillis/1000.0f;

        vec axis(0, -1, 0), forward(1, 0, 0);

        matrixpos = 0;
        matrixstack[0].identity();
        if(!d || !d->ragdoll || anim&ANIM_RAGDOLL)
        {
            matrixstack[0].translate(o);
            matrixstack[0].rotate_around_z(yaw*RAD);
            matrixstack[0].transformnormal(vec(axis), axis);
            matrixstack[0].transformnormal(vec(forward), forward);
            if(offsetyaw) matrixstack[0].rotate_around_z(offsetyaw*RAD);
        }
        else pitch = 0;

        if(anim&ANIM_NORENDER)
        {
            render(anim, basetime, basetime2, pitch, axis, forward, d, a);
            if(d) d->lastrendered = lastmillis;
            return;
        }

        if(!(anim&ANIM_NOSKIN))
        {
            if(renderpath==R_FIXEDFUNCTION && lightmodels)
            {
                GLfloat pos[4] = { dir.x*1000, dir.y*1000, dir.z*1000, 0 };
                glLightfv(GL_LIGHT0, GL_POSITION, pos);
            }
                
            transparent = trans;
            lightdir = dir;
            lightcolor = color;

            if(envmapped()) envmaptmu = 2;
            else if(a) for(int i = 0; a[i].tag; i++) if(a[i].m && a[i].m->envmapped())
            {
                envmaptmu = 2;
                break;
            }
            if(envmaptmu>=0) closestenvmaptex = lookupenvmap(closestenvmap(o));
        }

        if(depthoffset && !enabledepthoffset)
        {
            enablepolygonoffset(GL_POLYGON_OFFSET_FILL);
            enabledepthoffset = true;
        }

        if(envmaptmu>=0)
        {
            if(renderpath==R_FIXEDFUNCTION)
            {
                glActiveTexture_(GL_TEXTURE0_ARB+envmaptmu);
                setuptmu(envmaptmu, "T , P @ Pa", hasTEX || hasTE4 ? "Ca * $1a" : "= Ca");

                glMatrixMode(GL_TEXTURE);
                glLoadMatrixf(envmatrix.v);
                glMatrixMode(GL_MODELVIEW);
                glActiveTexture_(GL_TEXTURE0_ARB);
            }
            else
            {
                setenvparamf("lightdirworld", SHPARAM_PIXEL, 1, dir.x, dir.y, dir.z);
            }
        }

        if(transparent<1)
        {
            if(anim&ANIM_GHOST) 
            {
                glDepthFunc(GL_GREATER);
                glDepthMask(GL_FALSE);
            }
            else if(alphadepth)
            {
                glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                render(anim|ANIM_NOSKIN, basetime, basetime2, pitch, axis, forward, d, a);
                glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, fading ? GL_FALSE : GL_TRUE);

                glDepthFunc(GL_LEQUAL);
            }

            if(!enablealphablend)
            {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                enablealphablend = true;
            }
        }

        render(anim, basetime, basetime2, pitch, axis, forward, d, a);

        if(envmaptmu>=0)
        {
            if(renderpath==R_FIXEDFUNCTION) glActiveTexture_(GL_TEXTURE0_ARB+envmaptmu);
            glMatrixMode(GL_TEXTURE);
            glLoadIdentity();
            glMatrixMode(GL_MODELVIEW);
            if(renderpath==R_FIXEDFUNCTION) glActiveTexture_(GL_TEXTURE0_ARB);
        }

        if(transparent<1 && (alphadepth || anim&ANIM_GHOST)) 
        {
            glDepthFunc(GL_LESS);
            if(anim&ANIM_GHOST) glDepthMask(GL_TRUE);
        }

        if(d) d->lastrendered = lastmillis;
    }

    bool loaded;
    char *loadname;
    vector<part *> parts;

    animmodel(const char *name) : loaded(false)
    {
        loadname = newstring(name);
    }

    virtual ~animmodel()
    {
        delete[] loadname;
        parts.deletecontents();
    }

    const char *name() const { return loadname; }

    void cleanup()
    {
        loopv(parts) parts[i]->cleanup();
        enablelight0 = false;
    }

    void initmatrix(matrix3x4 &m)
    {
        m.identity();
        if(offsetyaw) m.rotate_around_z(offsetyaw*RAD);
        if(offsetpitch) m.rotate_around_y(-offsetpitch*RAD);
    }

    void gentris(int frame, vector<BIH::tri> *tris)
    {
        if(parts.empty()) return;
        matrix3x4 m;
        initmatrix(m);
        parts[0]->gentris(frame, tris, m);
    }

    void preloadBIH()
    {
        model::preloadBIH();
        if(bih) loopv(parts) parts[i]->preloadBIH();
    }

    BIH *setBIH()
    {
        if(bih) return bih;
        vector<BIH::tri> tris[2];
        gentris(0, tris);
        bih = new BIH(tris);
        return bih;
    }

    bool link(part *p, const char *tag, const vec &translate = vec(0, 0, 0), int anim = -1, int basetime = 0, vec *pos = NULL)
    {
        if(parts.empty()) return false;
        return parts[0]->link(p, tag, translate, anim, basetime, pos);
    }

    bool unlink(part *p)
    {
        if(parts.empty()) return false;
        return parts[0]->unlink(p);
    }

    bool envmapped()
    {
        loopv(parts) loopvj(parts[i]->skins) if(parts[i]->skins[j].envmapped()) return true;
        return false;
    }

    virtual bool loaddefaultparts()
    {
        return true;
    }

    void preloadshaders()
    {
        loopv(parts) parts[i]->preloadshaders();
    }

    void setshader(Shader *shader)
    {
        if(parts.empty()) loaddefaultparts();
        loopv(parts) loopvj(parts[i]->skins) parts[i]->skins[j].shader = shader;
    }

    void setenvmap(float envmapmin, float envmapmax, Texture *envmap)
    {
        if(parts.empty()) loaddefaultparts();
        loopv(parts) loopvj(parts[i]->skins)
        {
            skin &s = parts[i]->skins[j];
            if(envmapmax)
            {
                s.envmapmin = envmapmin;
                s.envmapmax = envmapmax;
            }
            if(envmap) s.envmap = envmap;
        }
    }

    void setspec(float spec)
    {
        if(parts.empty()) loaddefaultparts();
        loopv(parts) loopvj(parts[i]->skins) parts[i]->skins[j].spec = spec;
    }

    void setambient(float ambient)
    {
        if(parts.empty()) loaddefaultparts();
        loopv(parts) loopvj(parts[i]->skins) parts[i]->skins[j].ambient = ambient;
    }

    void setglow(float glow, float delta, float pulse)
    {
        if(parts.empty()) loaddefaultparts();
        loopv(parts) loopvj(parts[i]->skins) 
        {
            skin &s = parts[i]->skins[j];
            s.glow = glow;
            s.glowdelta = delta;
            s.glowpulse = pulse;
        }
    }

    void setglare(float specglare, float glowglare)
    {
        if(parts.empty()) loaddefaultparts();
        loopv(parts) loopvj(parts[i]->skins)
        {
            skin &s = parts[i]->skins[j];
            s.specglare = specglare;
            s.glowglare = glowglare;
        }
    }

    void setalphatest(float alphatest)
    {
        if(parts.empty()) loaddefaultparts();
        loopv(parts) loopvj(parts[i]->skins) parts[i]->skins[j].alphatest = alphatest;
    }

    void setalphablend(bool alphablend)
    {
        if(parts.empty()) loaddefaultparts();
        loopv(parts) loopvj(parts[i]->skins) parts[i]->skins[j].alphablend = alphablend;
    }

    void setfullbright(float fullbright)
    {
        if(parts.empty()) loaddefaultparts();
        loopv(parts) loopvj(parts[i]->skins) parts[i]->skins[j].fullbright = fullbright;
    }

    void setcullface(bool cullface)
    {
        if(parts.empty()) loaddefaultparts();
        loopv(parts) loopvj(parts[i]->skins) parts[i]->skins[j].cullface = cullface;
    }

    void calcbb(int frame, vec &center, vec &radius)
    {
        if(parts.empty()) return;
        vec bbmin(1e16f, 1e16f, 1e16f), bbmax(-1e16f, -1e16f, -1e16f);
        matrix3x4 m;
        initmatrix(m); 
        parts[0]->calcbb(frame, bbmin, bbmax, m);
        radius = bbmax;
        radius.sub(bbmin);
        radius.mul(0.5f);
        center = bbmin;
        center.add(radius);
    }

    static bool enabletc, enablemtc, enablealphatest, enablealphablend, enableenvmap, enableglow, enableoverbright, enablelighting, enablelight0, enablecullface, enablenormals, enabletangents, enablebones, enablerescale, enabledepthoffset;
    static vec lightdir, lightcolor;
    static float transparent, lastalphatest;
    static void *lastvbuf, *lasttcbuf, *lastmtcbuf, *lastnbuf, *lastxbuf, *lastbbuf, *lastsdata, *lastbdata;
    static GLuint lastebuf, lastenvmaptex, closestenvmaptex;
    static Texture *lasttex, *lastmasks, *lastnormalmap;
    static int envmaptmu, matrixpos;
    static glmatrixf matrixstack[64];

    void startrender()
    {
        enabletc = enablemtc = enablealphatest = enablealphablend = enableenvmap = enableglow = enableoverbright = enablelighting = enablenormals = enabletangents = enablebones = enablerescale = enabledepthoffset = false;
        enablecullface = true;
        lastalphatest = -1;
        lastvbuf = lasttcbuf = lastmtcbuf = lastxbuf = lastnbuf = lastbbuf = lastsdata = lastbdata = NULL;
        lastebuf = lastenvmaptex = closestenvmaptex = 0;
        lasttex = lastmasks = lastnormalmap = NULL;
        envmaptmu = -1;
        transparent = 1;

        if(renderpath==R_FIXEDFUNCTION && lightmodels && !enablelight0)
        {
            glEnable(GL_LIGHT0);
            static const GLfloat zero[4] = { 0, 0, 0, 0 };
            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, zero);
            glLightfv(GL_LIGHT0, GL_SPECULAR, zero);
            glMaterialfv(GL_FRONT, GL_SPECULAR, zero);
            glMaterialfv(GL_FRONT, GL_EMISSION, zero);
            enablelight0 = true;
        }
    }

    static void disablebones()
    {
        glDisableVertexAttribArray_(6);
        glDisableVertexAttribArray_(7);
        enablebones = false;
    }

    static void disabletangents()
    {
        glDisableVertexAttribArray_(1);
        enabletangents = false;
    }

    static void disablemtc()
    {
        glClientActiveTexture_(GL_TEXTURE1_ARB);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glClientActiveTexture_(GL_TEXTURE0_ARB);
        enablemtc = false;
    }

    static void disabletc()
    {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        if(enablemtc) disablemtc();
        enabletc = false;
    }

    static void disablenormals()
    {
        glDisableClientState(GL_NORMAL_ARRAY);
        enablenormals = false;
    }

    static void disablevbo()
    {
        if(hasVBO)
        {
            glBindBuffer_(GL_ARRAY_BUFFER_ARB, 0);
            glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
        }
        glDisableClientState(GL_VERTEX_ARRAY);
        if(enabletc) disabletc();
        if(enablenormals) disablenormals();
        if(enabletangents) disabletangents();
        if(enablebones) disablebones();
        lastvbuf = lasttcbuf = lastmtcbuf = lastxbuf = lastnbuf = lastbbuf = NULL;
        lastebuf = 0;
    }

    static void disableoverbright()
    {
        resettmu(0);
        enableoverbright = false;
    }

    static void disableglow()
    {
        resettmu(0);
        glActiveTexture_(GL_TEXTURE1_ARB);
        resettmu(1);
        glDisable(GL_TEXTURE_2D);
        glActiveTexture_(GL_TEXTURE0_ARB);
        lasttex = lastmasks = NULL;
        enableglow = false;
    }

    static void disableenvmap(bool cleanup = false)
    {
        glActiveTexture_(GL_TEXTURE0_ARB+envmaptmu);
        if(enableenvmap) glDisable(GL_TEXTURE_CUBE_MAP_ARB);
        if(cleanup && renderpath==R_FIXEDFUNCTION)
        {
            resettmu(envmaptmu);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
            glDisable(GL_TEXTURE_GEN_R);
        }
        glActiveTexture_(GL_TEXTURE0_ARB);
        enableenvmap = false;
    }

    void endrender()
    {
        if(lastvbuf || lastebuf) disablevbo();
        if(enablealphatest) glDisable(GL_ALPHA_TEST);
        if(enablealphablend) glDisable(GL_BLEND);
        if(enableglow) disableglow();
        if(enableoverbright) disableoverbright();
        if(enablelighting) glDisable(GL_LIGHTING);
        if(lastenvmaptex) disableenvmap(true);
        if(enablerescale) glDisable(hasRN ? GL_RESCALE_NORMAL_EXT : GL_NORMALIZE);
        if(!enablecullface) glEnable(GL_CULL_FACE);
        if(enabledepthoffset) disablepolygonoffset(GL_POLYGON_OFFSET_FILL);
    }
};

bool animmodel::enabletc = false, animmodel::enablemtc = false, animmodel::enablealphatest = false, animmodel::enablealphablend = false,
     animmodel::enableenvmap = false, animmodel::enableglow = false, animmodel::enableoverbright = false, animmodel::enablelighting = false, animmodel::enablelight0 = false, animmodel::enablecullface = true,
     animmodel::enablenormals = false, animmodel::enabletangents = false, animmodel::enablebones = false, animmodel::enablerescale = false, animmodel::enabledepthoffset = false;
vec animmodel::lightdir(0, 0, 1), animmodel::lightcolor(1, 1, 1);
float animmodel::transparent = 1, animmodel::lastalphatest = -1;
void *animmodel::lastvbuf = NULL, *animmodel::lasttcbuf = NULL, *animmodel::lastmtcbuf = NULL, *animmodel::lastnbuf = NULL, *animmodel::lastxbuf = NULL, *animmodel::lastbbuf = NULL, *animmodel::lastsdata = NULL, *animmodel::lastbdata = NULL;
GLuint animmodel::lastebuf = 0, animmodel::lastenvmaptex = 0, animmodel::closestenvmaptex = 0;
Texture *animmodel::lasttex = NULL, *animmodel::lastmasks = NULL, *animmodel::lastnormalmap = NULL;
int animmodel::envmaptmu = -1, animmodel::matrixpos = 0;
glmatrixf animmodel::matrixstack[64];

template<class MDL> struct modelloader
{
    static MDL *loading;
    static string dir;

    static bool animated() { return true; }
    static bool multiparted() { return true; }
    static bool multimeshed() { return true; } 
};

template<class MDL> MDL *modelloader<MDL>::loading = NULL;
template<class MDL> string modelloader<MDL>::dir = {'\0'}; // crashes clang if "" is used here

template<class MDL, class MESH> struct modelcommands
{
    typedef struct MDL::part part;
    typedef struct MDL::skin skin;

    static void setdir(char *name)
    {
        if(!MDL::loading) { conoutf("not loading an %s", MDL::formatname()); return; }
        formatstring(MDL::dir)("packages/models/%s", name);
    }

    #define loopmeshes(meshname, m, body) \
        if(!MDL::loading || MDL::loading->parts.empty()) { conoutf("not loading an %s", MDL::formatname()); return; } \
        part &mdl = *MDL::loading->parts.last(); \
        if(!mdl.meshes) return; \
        loopv(mdl.meshes->meshes) \
        { \
            MESH &m = *(MESH *)mdl.meshes->meshes[i]; \
            if(!strcmp(meshname, "*") || (m.name && !strcmp(m.name, meshname))) \
            { \
                body; \
            } \
        }

    #define loopskins(meshname, s, body) loopmeshes(meshname, m, { skin &s = mdl.skins[i]; body; })
    
    static void setskin(char *meshname, char *tex, char *masks, float *envmapmax, float *envmapmin)
    {
        loopskins(meshname, s,
            s.tex = textureload(makerelpath(MDL::dir, tex), 0, true, false);
            if(*masks)
            {
                s.masks = textureload(makerelpath(MDL::dir, masks, "<stub>"), 0, true, false);
                s.envmapmax = *envmapmax;
                s.envmapmin = *envmapmin;
            }
        );
    }
    
    static void setspec(char *meshname, int *percent)
    {
        float spec = 1.0f;
        if(*percent>0) spec = *percent/100.0f;
        else if(*percent<0) spec = 0.0f;
        loopskins(meshname, s, s.spec = spec);
    }
    
    static void setambient(char *meshname, int *percent)
    {
        float ambient = 0.3f;
        if(*percent>0) ambient = *percent/100.0f;
        else if(*percent<0) ambient = 0.0f;
        loopskins(meshname, s, s.ambient = ambient);
    }
    
    static void setglow(char *meshname, int *percent, int *delta, float *pulse)
    {
        float glow = 3.0f, glowdelta = *delta/100.0f, glowpulse = *pulse > 0 ? *pulse/1000.0f : 0;
        if(*percent>0) glow = *percent/100.0f;
        else if(*percent<0) glow = 0.0f;
        glowdelta -= glow;
        loopskins(meshname, s, { s.glow = glow; s.glowdelta = glowdelta; s.glowpulse = glowpulse; });
    }
    
    static void setglare(char *meshname, float *specglare, float *glowglare)
    {
        loopskins(meshname, s, { s.specglare = *specglare; s.glowglare = *glowglare; });
    }
    
    static void setalphatest(char *meshname, float *cutoff)
    {
        loopskins(meshname, s, s.alphatest = max(0.0f, min(1.0f, *cutoff)));
    }
    
    static void setalphablend(char *meshname, int *blend)
    {
        loopskins(meshname, s, s.alphablend = *blend!=0);
    }
    
    static void setcullface(char *meshname, int *cullface)
    {
        loopskins(meshname, s, s.cullface = *cullface!=0);
    }
    
    static void setenvmap(char *meshname, char *envmap)
    {
        Texture *tex = cubemapload(envmap);
        loopskins(meshname, s, s.envmap = tex);
    }
    
    static void setbumpmap(char *meshname, char *normalmapfile, char *skinfile)
    {
        Texture *normalmaptex = NULL, *skintex = NULL;
        normalmaptex = textureload(makerelpath(MDL::dir, normalmapfile, "<noff>"), 0, true, false);
        if(skinfile[0]) skintex = textureload(makerelpath(MDL::dir, skinfile, "<noff>"), 0, true, false);
        loopskins(meshname, s, { s.unlittex = skintex; s.normalmap = normalmaptex; m.calctangents(); });
    }
    
    static void setfullbright(char *meshname, float *fullbright)
    {
        loopskins(meshname, s, s.fullbright = *fullbright);
    }
    
    static void setshader(char *meshname, char *shader)
    {
        loopskins(meshname, s, s.shader = lookupshaderbyname(shader));
    }
    
    static void setscroll(char *meshname, float *scrollu, float *scrollv)
    {
        loopskins(meshname, s, { s.scrollu = *scrollu; s.scrollv = *scrollv; });
    }
    
    static void setnoclip(char *meshname, int *noclip)
    {
        loopmeshes(meshname, m, m.noclip = *noclip!=0);
    }
  
    static void setlink(int *parent, int *child, char *tagname, float *x, float *y, float *z)
    {
        if(!MDL::loading) { conoutf("not loading an %s", MDL::formatname()); return; }
        if(!MDL::loading->parts.inrange(*parent) || !MDL::loading->parts.inrange(*child)) { conoutf("no models loaded to link"); return; }
        if(!MDL::loading->parts[*parent]->link(MDL::loading->parts[*child], tagname, vec(*x, *y, *z))) conoutf("could not link model %s", MDL::loading->loadname);
    }
 
    template<class F> void modelcommand(F *fun, const char *suffix, const char *args)
    {
        defformatstring(name)("%s%s", MDL::formatname(), suffix);
        addcommand(newstring(name), (void (*)())fun, args);
    }

    modelcommands()
    {
        modelcommand(setdir, "dir", "s");
        if(MDL::multimeshed())
        {
            modelcommand(setskin, "skin", "sssff");
            modelcommand(setspec, "spec", "si");
            modelcommand(setambient, "ambient", "si");
            modelcommand(setglow, "glow", "siif");
            modelcommand(setglare, "glare", "sff");
            modelcommand(setalphatest, "alphatest", "sf");
            modelcommand(setalphablend, "alphablend", "si");
            modelcommand(setcullface, "cullface", "si");
            modelcommand(setenvmap, "envmap", "ss");
            modelcommand(setbumpmap, "bumpmap", "sss");
            modelcommand(setfullbright, "fullbright", "sf");
            modelcommand(setshader, "shader", "ss");
            modelcommand(setscroll, "scroll", "sff");
            modelcommand(setnoclip, "noclip", "si");
        }
        if(MDL::multiparted()) modelcommand(setlink, "link", "iisfff");
    }
};

