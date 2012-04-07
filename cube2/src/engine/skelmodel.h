VARP(gpuskel, 0, 1, 1);
VARP(matskel, 0, 1, 1);

#define BONEMASK_NOT  0x8000
#define BONEMASK_END  0xFFFF
#define BONEMASK_BONE 0x7FFF

struct skelmodel : animmodel
{
    struct vert { vec pos, norm; float u, v; int blend, interpindex; };
    struct vvert { vec pos; float u, v; };
    struct vvertn : vvert { vec norm; };
    struct vvertw : vvertn { uchar weights[4]; uchar bones[4]; };
    struct vvertbump : vvertn { vec tangent; float bitangent; };
    struct vvertbumpw : vvertw { vec tangent; float bitangent; };
    struct bumpvert { vec tangent; float bitangent; };
    struct tri { ushort vert[3]; };

    struct blendcombo
    {
        int uses, interpindex;
        float weights[4];
        uchar bones[4], interpbones[4];

        blendcombo() : uses(1)
        {
        }

        bool operator==(const blendcombo &c) const
        {
            loopk(4) if(bones[k] != c.bones[k]) return false;
            loopk(4) if(weights[k] != c.weights[k]) return false;
            return true;
        }

        int size() const
        {
            int i = 1;
            while(i < 4 && weights[i]) i++;
            return i;
        }

        static bool sortcmp(const blendcombo &x, const blendcombo &y)
        {
            loopi(4)
            {
                if(x.weights[i])
                {
                    if(!y.weights[i]) return true;
                }
                else if(y.weights[i]) return false;
                else break;
            }
            return false;
        }

        int addweight(int sorted, float weight, int bone)
        {
            if(weight <= 1e-3f) return sorted;
            loopk(sorted) if(weight > weights[k])
            {
                for(int l = min(sorted-1, 2); l >= k; l--)
                {
                    weights[l+1] = weights[l];
                    bones[l+1] = bones[l];
                }
                weights[k] = weight;
                bones[k] = bone;
                return sorted<4 ? sorted+1 : sorted;
            }
            if(sorted>=4) return sorted;
            weights[sorted] = weight;
            bones[sorted] = bone;
            return sorted+1;
        }
        
        void finalize(int sorted)
        {
            loopj(4-sorted) { weights[sorted+j] = 0; bones[sorted+j] = 0; }
            if(sorted <= 0) return;
            float total = 0;
            loopj(sorted) total += weights[j];
            total = 1.0f/total;
            loopj(sorted) weights[j] *= total;
        }

        void serialize(vvertw &v)
        {
            if(interpindex >= 0)
            {
                v.weights[0] = 255;
                loopk(3) v.weights[k+1] = 0;
                v.bones[0] = (matskel ? 3 : 2)*interpindex;
                loopk(3) v.bones[k+1] = v.bones[0];
            }
            else
            {
                int total = 0;
                loopk(4) total += (v.weights[k] = uchar(weights[k]*255));
                while(total > 255)
                {
                    loopk(4) if(v.weights[k] > 0 && total > 255) { v.weights[k]--; total--; } 
                }
                while(total < 255)
                {
                    loopk(4) if(v.weights[k] < 255 && total < 255) { v.weights[k]++; total++; }
                }
                loopk(4) v.bones[k] = (matskel ? 3 : 2)*interpbones[k];
            }
        }
    };


    struct animcacheentry
    {
        animstate as[MAXANIMPARTS];
        float pitch;
        int millis;
        uchar *partmask;
        ragdolldata *ragdoll;

        animcacheentry() : ragdoll(NULL)
        {
            loopk(MAXANIMPARTS) as[k].cur.fr1 = as[k].prev.fr1 = -1;
        }

        bool operator==(const animcacheentry &c) const
        {
            loopi(MAXANIMPARTS) if(as[i]!=c.as[i]) return false;
            return pitch==c.pitch && partmask==c.partmask && ragdoll==c.ragdoll && (!ragdoll || min(millis, c.millis) >= ragdoll->lastmove);
        }
    };

    struct vbocacheentry : animcacheentry
    {
        uchar *vdata;
        GLuint vbuf;
        int owner;

        vbocacheentry() : vdata(NULL), vbuf(0), owner(-1) {}
    };
    
    struct skelcacheentry : animcacheentry
    {
        dualquat *bdata;
        matrix3x4 *mdata;
        int version;
        GLuint ubuf;
        bool dirty;
 
        skelcacheentry() : bdata(NULL), mdata(NULL), version(-1), ubuf(0), dirty(false) {}
        
        void nextversion()
        {
            version = Shader::uniformlocversion();
            dirty = true;
        } 
    };

    struct blendcacheentry : skelcacheentry
    {
        int owner;

        blendcacheentry() : owner(-1) {}
    };

    struct skelmeshgroup;

    struct skelmesh : mesh
    {
        vert *verts;
        bumpvert *bumpverts;
        tri *tris;
        int numverts, numtris, maxweights;

        int voffset, eoffset, elen;
        ushort minvert, maxvert;

        skelmesh() : verts(NULL), bumpverts(NULL), tris(NULL), numverts(0), numtris(0), maxweights(0)
        {
        }

        virtual ~skelmesh()
        {
            DELETEA(verts);
            DELETEA(bumpverts);
            DELETEA(tris);
        }

        int addblendcombo(const blendcombo &c)
        {
            maxweights = max(maxweights, c.size());
            return ((skelmeshgroup *)group)->addblendcombo(c);
        }

        void smoothnorms(float limit = 0, bool areaweight = true)
        {
            mesh::smoothnorms(verts, numverts, tris, numtris, limit, areaweight);
        }

        void buildnorms(bool areaweight = true)
        {
            mesh::buildnorms(verts, numverts, tris, numtris, areaweight);
        }

        void calctangents(bool areaweight = true)
        {
            if(bumpverts) return;
            bumpverts = new bumpvert[numverts];
            mesh::calctangents(bumpverts, verts, verts, numverts, tris, numtris, areaweight);
        }

        void calcbb(int frame, vec &bbmin, vec &bbmax, const matrix3x4 &m)
        {
            loopj(numverts)
            {
                vec v = m.transform(verts[j].pos);
                loopi(3)
                {
                    bbmin[i] = min(bbmin[i], v[i]);
                    bbmax[i] = max(bbmax[i], v[i]);
                }
            }
        }

        void gentris(int frame, Texture *tex, vector<BIH::tri> *out, const matrix3x4 &m)
        {
            loopj(numtris)
            {
                BIH::tri &t = out[noclip ? 1 : 0].add();
                t.tex = tex;
                vert &av = verts[tris[j].vert[0]],
                     &bv = verts[tris[j].vert[1]],
                     &cv = verts[tris[j].vert[2]];
                t.a = m.transform(av.pos);
                t.b = m.transform(bv.pos);
                t.c = m.transform(cv.pos);
                t.tc[0] = av.u;
                t.tc[1] = av.v;
                t.tc[2] = bv.u;
                t.tc[3] = bv.v;
                t.tc[4] = cv.u;
                t.tc[5] = cv.v;
            }
        }

        static inline bool comparevert(vvert &w, int j, vert &v)
        {
            return v.u==w.u && v.v==w.v && v.pos==w.pos;
        }

        static inline bool comparevert(vvertn &w, int j, vert &v)
        {
            return v.u==w.u && v.v==w.v && v.pos==w.pos && v.norm==w.norm;
        }

        inline bool comparevert(vvertbump &w, int j, vert &v)
        {
            return v.u==w.u && v.v==w.v && v.pos==w.pos && v.norm==w.norm && (!bumpverts || (bumpverts[j].tangent==w.tangent && bumpverts[j].bitangent==w.bitangent));
        }

        static inline void assignvert(vvert &vv, int j, vert &v, blendcombo &c)
        {
            vv.pos = v.pos;
            vv.u = v.u;
            vv.v = v.v;
        }

        static inline void assignvert(vvertn &vv, int j, vert &v, blendcombo &c)
        {
            vv.pos = v.pos;
            vv.norm = v.norm;
            vv.u = v.u;
            vv.v = v.v;
        }

        inline void assignvert(vvertbump &vv, int j, vert &v, blendcombo &c)
        {
            vv.pos = v.pos;
            vv.norm = v.norm;
            vv.u = v.u;
            vv.v = v.v;
            if(bumpverts)
            {
                vv.tangent = bumpverts[j].tangent;
                vv.bitangent = bumpverts[j].bitangent;
            }
            else
            {
                vv.tangent = vec(0, 0, 0);
                vv.bitangent = 0;
            }
        }

        static inline void assignvert(vvertw &vv, int j, vert &v, blendcombo &c)
        {
            vv.pos = v.pos;
            vv.norm = v.norm;
            vv.u = v.u;
            vv.v = v.v;
            c.serialize(vv);
        }

        inline void assignvert(vvertbumpw &vv, int j, vert &v, blendcombo &c)
        {
            vv.pos = v.pos;
            vv.norm = v.norm;
            vv.u = v.u;
            vv.v = v.v;
            if(bumpverts)
            {
                vv.tangent = bumpverts[j].tangent;
                vv.bitangent = bumpverts[j].bitangent;
            }
            else
            {
                vv.tangent = vec(0, 0, 0);
                vv.bitangent = 0;
            }
            c.serialize(vv);
        }

        template<class T>
        int genvbo(vector<ushort> &idxs, int offset, vector<T> &vverts)
        {
            voffset = offset;
            eoffset = idxs.length();
            loopi(numverts)
            {
                vert &v = verts[i];
                assignvert(vverts.add(), i, v, ((skelmeshgroup *)group)->blendcombos[v.blend]);
            }
            loopi(numtris) loopj(3) idxs.add(voffset + tris[i].vert[j]);
            elen = idxs.length()-eoffset;
            minvert = voffset;
            maxvert = voffset + numverts-1;
            return numverts;
        }

        template<class T>
        int genvbo(vector<ushort> &idxs, int offset, vector<T> &vverts, int *htdata, int htlen)
        {
            voffset = offset;
            eoffset = idxs.length();
            minvert = 0xFFFF;
            loopi(numtris)
            {
                tri &t = tris[i];
                loopj(3)
                {
                    int index = t.vert[j];
                    vert &v = verts[index];
                    int htidx = hthash(v.pos)&(htlen-1);
                    loopk(htlen)
                    {
                        int &vidx = htdata[(htidx+k)&(htlen-1)];
                        if(vidx < 0) { vidx = idxs.add(ushort(vverts.length())); assignvert(vverts.add(), index, v, ((skelmeshgroup *)group)->blendcombos[v.blend]); break; }
                        else if(comparevert(vverts[vidx], index, v)) { minvert = min(minvert, idxs.add(ushort(vidx))); break; }
                    }
                }
            }
            elen = idxs.length()-eoffset;
            minvert = min(minvert, ushort(voffset));
            maxvert = max(minvert, ushort(vverts.length()-1));
            return vverts.length()-voffset;
        }

        int genvbo(vector<ushort> &idxs, int offset)
        {
            loopi(numverts) verts[i].interpindex = ((skelmeshgroup *)group)->remapblend(verts[i].blend);
            
            voffset = offset;
            eoffset = idxs.length();
            loopi(numtris)
            {
                tri &t = tris[i];
                loopj(3) idxs.add(voffset+t.vert[j]);
            }
            minvert = voffset;
            maxvert = voffset + numverts-1;
            elen = idxs.length()-eoffset;
            return numverts;
        }

        void filltc(uchar *vdata, size_t stride)
        {
            vdata = (uchar *)&((vvert *)&vdata[voffset*stride])->u;
            loopi(numverts)
            {
                ((float *)vdata)[0] = verts[i].u;
                ((float *)vdata)[1] = verts[i].v; 
                vdata += stride;
            }
        }

        void fillbump(uchar *vdata, size_t stride)
        {
            if(stride==sizeof(vvertbumpw)) vdata = (uchar *)&((vvertbumpw *)&vdata[voffset*stride])->tangent;
            else vdata = (uchar *)&((vvertbump *)&vdata[voffset*stride])->tangent;
            if(bumpverts) loopi(numverts)
            {
                ((bumpvert *)vdata)->bitangent = bumpverts[i].bitangent;
                vdata += stride;
            }
            else loopi(numverts)
            {
                memset(vdata, 0, sizeof(bumpvert));
                vdata += stride;
            }
        }

        template<class M>
        void interpverts(const M * RESTRICT mdata1, const M * RESTRICT mdata2, bool norms, bool tangents, void * RESTRICT vdata, skin &s)
        {
            const int blendoffset = ((skelmeshgroup *)group)->skel->numinterpbones;
            mdata2 -= blendoffset;

            #define IPLOOP(type, dosetup, dotransform) \
                loopi(numverts) \
                { \
                    const vert &src = verts[i]; \
                    type &dst = ((type * RESTRICT)vdata)[i]; \
                    dosetup; \
                    const M &m = (src.interpindex < blendoffset ? mdata1 : mdata2)[src.interpindex]; \
                    dst.pos = m.transform(src.pos); \
                    dotransform; \
                }

            if(tangents)
            {
                if(bumpverts)
                {
                    IPLOOP(vvertbump, bumpvert &bsrc = bumpverts[i],
                    {
                        dst.norm = m.transformnormal(src.norm);
                        dst.tangent = m.transformnormal(bsrc.tangent);
                    });
                }
                else { IPLOOP(vvertbump, , dst.norm = m.transformnormal(src.norm)); }
            }
            else if(norms) { IPLOOP(vvertn, , dst.norm = m.transformnormal(src.norm)); }
            else { IPLOOP(vvert, , ); }

            #undef IPLOOP
        }

        void setshader(Shader *s)
        {
            skelmeshgroup *g = (skelmeshgroup *)group;
            if(glaring)
            {
                if(!g->skel->usegpuskel) s->setvariant(0, 2);
                else if(g->skel->usematskel) s->setvariant(min(maxweights, g->vweights), 2);
                else s->setvariant(min(maxweights, g->vweights)-1, 3);
            }
            else if(!g->skel->usegpuskel) s->set();
            else if(g->skel->usematskel) s->setvariant(min(maxweights, g->vweights)-1, 0);
            else s->setvariant(min(maxweights, g->vweights)-1, 1);
        }

        void render(const animstate *as, skin &s, vbocacheentry &vc)
        {
            if(!(as->cur.anim&ANIM_NOSKIN))
            {
                if(s.multitextured())
                {
                    if(!enablemtc || lastmtcbuf!=lastvbuf)
                    {
                        glClientActiveTexture_(GL_TEXTURE1_ARB);
                        if(!enablemtc) glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                        if(lastmtcbuf!=lastvbuf)
                        {
                            vvert *vverts = hasVBO ? 0 : (vvert *)vc.vdata;
                            glTexCoordPointer(2, GL_FLOAT, ((skelmeshgroup *)group)->vertsize, &vverts->u);
                        }
                        glClientActiveTexture_(GL_TEXTURE0_ARB);
                        lastmtcbuf = lastvbuf;
                        enablemtc = true;
                    }
                }
                else if(enablemtc) disablemtc();

                if(s.tangents())
                {
                    if(!enabletangents || lastxbuf!=lastvbuf)
                    {
                        if(!enabletangents) glEnableVertexAttribArray_(1);
                        if(lastxbuf!=lastvbuf)
                        {
                            if(((skelmeshgroup *)group)->vertsize==sizeof(vvertbumpw))
                            {
                                vvertbumpw *vverts = hasVBO ? 0 : (vvertbumpw *)vc.vdata;
                                glVertexAttribPointer_(1, 4, GL_FLOAT, GL_FALSE, ((skelmeshgroup *)group)->vertsize, &vverts->tangent.x);
                            }
                            else
                            {
                                vvertbump *vverts = hasVBO ? 0 : (vvertbump *)vc.vdata;
                                glVertexAttribPointer_(1, 4, GL_FLOAT, GL_FALSE, ((skelmeshgroup *)group)->vertsize, &vverts->tangent.x);
                            }
                        }
                        lastxbuf = lastvbuf;
                        enabletangents = true;
                    }
                }
                else if(enabletangents) disabletangents();

                if(renderpath==R_FIXEDFUNCTION && (s.scrollu || s.scrollv))
                {
                    glMatrixMode(GL_TEXTURE);
                    glPushMatrix();
                    glTranslatef(s.scrollu*lastmillis/1000.0f, s.scrollv*lastmillis/1000.0f, 0);

                    if(s.multitextured())
                    {
                        glActiveTexture_(GL_TEXTURE1_ARB);
                        glPushMatrix();
                        glTranslatef(s.scrollu*lastmillis/1000.0f, s.scrollv*lastmillis/1000.0f, 0);
                    }
                }
            }

            if(hasDRE) glDrawRangeElements_(GL_TRIANGLES, minvert, maxvert, elen, GL_UNSIGNED_SHORT, &((skelmeshgroup *)group)->edata[eoffset]);
            else glDrawElements(GL_TRIANGLES, elen, GL_UNSIGNED_SHORT, &((skelmeshgroup *)group)->edata[eoffset]);
            glde++;
            xtravertsva += numverts;

            if(renderpath==R_FIXEDFUNCTION && !(as->cur.anim&ANIM_NOSKIN) && (s.scrollu || s.scrollv))
            {
                if(s.multitextured())
                {
                    glPopMatrix();
                    glActiveTexture_(GL_TEXTURE0_ARB);
                }

                glPopMatrix();
                glMatrixMode(GL_MODELVIEW);
            }

            return;
        }
    };

       
    struct tag
    {
        char *name;
        int bone;
        matrix3x4 matrix;

        tag() : name(NULL) {}
        ~tag() { DELETEA(name); }
    };

    struct skelanimspec
    {
        char *name;
        int frame, range;

        skelanimspec() : name(NULL), frame(0), range(0) {}
        ~skelanimspec()
        {
            DELETEA(name);
        }
    };

    struct boneinfo
    {
        const char *name;
        int parent, children, next, group, scheduled, interpindex, interpparent, ragdollindex, correctindex;
        float pitchscale, pitchoffset, pitchmin, pitchmax;
        dualquat base, invbase;

        boneinfo() : name(NULL), parent(-1), children(-1), next(-1), group(INT_MAX), scheduled(-1), interpindex(-1), interpparent(-1), ragdollindex(-1), correctindex(-1), pitchscale(0), pitchoffset(0), pitchmin(0), pitchmax(0) {}
        ~boneinfo()
        {
            DELETEA(name);
        }
    };

    struct antipode
    {
        int parent, child;

        antipode(int parent, int child) : parent(parent), child(child) {}
    };

    struct pitchdep
    {
        int bone, parent;
        dualquat pose;
    };

    struct pitchtarget
    {
        int bone, frame, corrects, deps;
        float pitchmin, pitchmax, deviated;
        dualquat pose;
    };

    struct pitchcorrect
    {
        int bone, target, parent;
        float pitchmin, pitchmax, pitchscale, pitchangle, pitchtotal;
    };

    struct skeleton
    {
        char *name;
        int shared;
        vector<skelmeshgroup *> users;
        boneinfo *bones;
        int numbones, numinterpbones, numgpubones, numframes;
        dualquat *framebones;
        vector<skelanimspec> skelanims;
        vector<tag> tags;
        vector<antipode> antipodes;
        ragdollskel *ragdoll;
        vector<pitchdep> pitchdeps;
        vector<pitchtarget> pitchtargets;
        vector<pitchcorrect> pitchcorrects;

        bool usegpuskel, usematskel;
        vector<skelcacheentry> skelcache;

        skeleton() : name(NULL), shared(0), bones(NULL), numbones(0), numinterpbones(0), numgpubones(0), numframes(0), framebones(NULL), ragdoll(NULL), usegpuskel(false), usematskel(false)
        {
        }

        ~skeleton()
        {
            DELETEA(name);
            DELETEA(bones);
            DELETEA(framebones);
            DELETEP(ragdoll);
            loopv(skelcache)
            {
                DELETEA(skelcache[i].bdata);
                DELETEA(skelcache[i].mdata);
                if(skelcache[i].ubuf) glDeleteBuffers_(1, &skelcache[i].ubuf);
            }
        }

        skelanimspec *findskelanim(const char *name, char sep = '\0')
        {
            int len = sep ? strlen(name) : 0;
            loopv(skelanims)
            {
                if(skelanims[i].name)
                {
                    if(sep)
                    {
                        const char *end = strchr(skelanims[i].name, ':');
                        if(end && end - skelanims[i].name == len && !memcmp(name, skelanims[i].name, len)) return &skelanims[i];
                    }
                    if(!strcmp(name, skelanims[i].name)) return &skelanims[i];
                }
            }
            return NULL;
        }

        skelanimspec &addskelanim(const char *name)
        {
            skelanimspec &sa = skelanims.add();
            sa.name = name ? newstring(name) : NULL;
            return sa;
        }

        int findbone(const char *name)
        {
            loopi(numbones) if(bones[i].name && !strcmp(bones[i].name, name)) return i;
            return -1;
        }

        int findtag(const char *name)
        {
            loopv(tags) if(!strcmp(tags[i].name, name)) return i;
            return -1;
        }

        bool addtag(const char *name, int bone, const matrix3x4 &matrix)
        {
            if(findtag(name) >= 0) return false;
            tag &t = tags.add();
            t.name = newstring(name);
            t.bone = bone;
            t.matrix = matrix;
            return true;
        }

        void calcantipodes()
        {
            antipodes.shrink(0);
            vector<int> schedule;
            loopi(numbones) 
            {
                if(bones[i].group >= numbones) 
                {
                    bones[i].scheduled = schedule.length();
                    schedule.add(i);
                }
                else bones[i].scheduled = -1;
            }
            loopv(schedule)
            {
                int bone = schedule[i];
                const boneinfo &info = bones[bone];
                loopj(numbones) if(abs(bones[j].group) == bone && bones[j].scheduled < 0)
                {
                    antipodes.add(antipode(info.interpindex, bones[j].interpindex));
                    bones[j].scheduled = schedule.length();
                    schedule.add(j);
                }
                if(i + 1 == schedule.length())
                {
                    int conflict = INT_MAX;
                    loopj(numbones) if(bones[j].group < numbones && bones[j].scheduled < 0) conflict = min(conflict, abs(bones[j].group));
                    if(conflict < numbones)
                    {
                        bones[conflict].scheduled = schedule.length();
                        schedule.add(conflict);
                    }
                }
            }
        }

        void remapbones()
        {
            loopi(numbones) 
            {
                boneinfo &info = bones[i];
                info.interpindex = -1;
                info.ragdollindex = -1;
            }
            numgpubones = 0;
            loopv(users)
            {
                skelmeshgroup *group = users[i];
                loopvj(group->blendcombos)
                {
                    blendcombo &c = group->blendcombos[j];
                    loopk(4) 
                    {
                        if(!c.weights[k]) { c.interpbones[k] = k > 0 ? c.interpbones[k-1] : 0; continue; } 
                        boneinfo &info = bones[c.bones[k]];
                        if(info.interpindex < 0) info.interpindex = numgpubones++;
                        c.interpbones[k] = info.interpindex;
                        if(info.group < 0) continue;
                        loopl(4)
                        {
                            if(!c.weights[l]) break;
                            if(l == k) continue;
                            int parent = c.bones[l];
                            if(info.parent == parent || (info.parent >= 0 && info.parent == bones[parent].parent)) { info.group = -info.parent; break; }
                            if(info.group <= parent) continue;
                            int child = c.bones[k];
                            while(parent > child) parent = bones[parent].parent;
                            if(parent != child) info.group = c.bones[l];
                        }
                    }
                }
            }
            numinterpbones = numgpubones;
            loopv(tags)
            {
                boneinfo &info = bones[tags[i].bone];
                if(info.interpindex < 0) info.interpindex = numinterpbones++;
            }
            if(ragdoll)
            {
                loopv(ragdoll->joints) 
                {
                    boneinfo &info = bones[ragdoll->joints[i].bone];
                    if(info.interpindex < 0) info.interpindex = numinterpbones++;
                    info.ragdollindex = i;
                }
            }
            loopi(numbones)
            {
                boneinfo &info = bones[i];
                if(info.interpindex < 0) continue;
                for(int parent = info.parent; parent >= 0 && bones[parent].interpindex < 0; parent = bones[parent].parent)
                    bones[parent].interpindex = numinterpbones++;
            }
            loopi(numbones)
            {
                boneinfo &info = bones[i];
                if(info.interpindex < 0) continue;
                info.interpparent = info.parent >= 0 ? bones[info.parent].interpindex : -1;
            }
            if(ragdoll)
            {
                loopi(numbones)
                {
                    boneinfo &info = bones[i];
                    if(info.interpindex < 0 || info.ragdollindex >= 0) continue;
                    for(int parent = info.parent; parent >= 0; parent = bones[parent].parent)
                    {
                        if(bones[parent].ragdollindex >= 0) { ragdoll->addreljoint(i, bones[parent].ragdollindex); break; }
                    }
                }
            }
            calcantipodes();
        }


        void addpitchdep(int bone, int frame)
        {
            for(; bone >= 0; bone = bones[bone].parent)
            {
                int pos = pitchdeps.length();
                loopvj(pitchdeps) if(bone <= pitchdeps[j].bone)
                { 
                    if(bone == pitchdeps[j].bone) goto nextbone;
                    pos = j;
                    break;
                }
                {
                    pitchdep d;
                    d.bone = bone;
                    d.parent = -1;
                    d.pose = framebones[frame*numbones + bone];
                    pitchdeps.insert(pos, d);
                }
            nextbone:;
            }
        }

        int findpitchdep(int bone)
        {
            loopv(pitchdeps) if(bone <= pitchdeps[i].bone) return bone == pitchdeps[i].bone ? i : -1;
            return -1;
        }

        int findpitchcorrect(int bone)
        {
            loopv(pitchcorrects) if(bone <= pitchcorrects[i].bone) return bone == pitchcorrects[i].bone ? i : -1;
            return -1;
        }

        void initpitchdeps()
        {
            pitchdeps.setsize(0);
            if(pitchtargets.empty()) return;
            loopv(pitchtargets)
            {
                pitchtarget &t = pitchtargets[i];
                t.deps = -1;
                addpitchdep(t.bone, t.frame);
            }
            loopv(pitchdeps)
            {
                pitchdep &d = pitchdeps[i];
                int parent = bones[d.bone].parent;
                if(parent >= 0) 
                {
                    int j = findpitchdep(parent);
                    if(j >= 0)
                    {
                        d.parent = j;
                        d.pose.mul(pitchdeps[j].pose, dualquat(d.pose));
                    }
                }
            }
            loopv(pitchtargets)
            {
                pitchtarget &t = pitchtargets[i];
                int j = findpitchdep(t.bone);
                if(j >= 0)
                {
                    t.deps = j;
                    t.pose = pitchdeps[j].pose;
                }    
                t.corrects = -1;
                for(int parent = t.bone; parent >= 0; parent = bones[parent].parent)
                {
                    t.corrects = findpitchcorrect(parent);
                    if(t.corrects >= 0) break;
                }
            }
            loopv(pitchcorrects)
            {
                pitchcorrect &c = pitchcorrects[i];
                bones[c.bone].correctindex = i;
                c.parent = -1;
                for(int parent = c.bone;;)
                {
                    parent = bones[parent].parent;
                    if(parent < 0) break;
                    c.parent = findpitchcorrect(parent);
                    if(c.parent >= 0) break;
                }
            }
        }

        void optimize()
        {
            cleanup();
            if(ragdoll) ragdoll->setup();
            remapbones();
            initpitchdeps();
        }

        void expandbonemask(uchar *expansion, int bone, int val)
        {
            expansion[bone] = val;
            bone = bones[bone].children;
            while(bone>=0) { expandbonemask(expansion, bone, val); bone = bones[bone].next; }
        }

        void applybonemask(ushort *mask, uchar *partmask, int partindex)
        {
            if(!mask || *mask==BONEMASK_END) return;
            uchar *expansion = new uchar[numbones];
            memset(expansion, *mask&BONEMASK_NOT ? 1 : 0, numbones);
            while(*mask!=BONEMASK_END)
            {
                expandbonemask(expansion, *mask&BONEMASK_BONE, *mask&BONEMASK_NOT ? 0 : 1);
                mask++;
            }
            loopi(numbones) if(expansion[i]) partmask[i] = partindex;
            delete[] expansion;
        }

        void linkchildren()
        {
            loopi(numbones)
            {
                boneinfo &b = bones[i];
                b.children = -1;
                if(b.parent<0) b.next = -1;
                else
                {
                    b.next = bones[b.parent].children;
                    bones[b.parent].children = i;
                }
            }
        }

        int maxgpuparams() const
        {
            switch(renderpath)
            {
                case R_GLSLANG: return maxvsuniforms;
                case R_ASMGLSLANG:
                case R_ASMSHADER: return maxvpenvparams;
                default: return 0;
            }
        }
        int availgpubones() const { return (min(maxgpuparams() - reservevpparams, 256) - 10) / (matskel ? 3 : 2); }
        bool gpuaccelerate() const { return renderpath!=R_FIXEDFUNCTION && numframes && gpuskel && numgpubones<=availgpubones(); }

        float calcdeviation(const vec &axis, const vec &forward, const dualquat &pose1, const dualquat &pose2)
        {
            vec forward1 = pose1.transformnormal(forward).project(axis).normalize(),
                forward2 = pose2.transformnormal(forward).project(axis).normalize(),
                daxis = vec().cross(forward1, forward2);
            float dx = clamp(forward1.dot(forward2), -1.0f, 1.0f), dy = clamp(daxis.magnitude(), -1.0f, 1.0f);
            if(daxis.dot(axis) < 0) dy = -dy;
            return atan2f(dy, dx)/RAD;
        }

        void calcpitchcorrects(float pitch, const vec &axis, const vec &forward)
        {
            loopv(pitchtargets)
            {
                pitchtarget &t = pitchtargets[i];
                t.deviated = calcdeviation(axis, forward, t.pose, pitchdeps[t.deps].pose);
            }
            loopv(pitchcorrects)
            {
                pitchcorrect &c = pitchcorrects[i];
                c.pitchangle = c.pitchtotal = 0;
            }
            loopvj(pitchtargets)
            {
                pitchtarget &t = pitchtargets[j];
                float tpitch = pitch - t.deviated;
                for(int parent = t.corrects; parent >= 0; parent = pitchcorrects[parent].parent)
                    tpitch -= pitchcorrects[parent].pitchangle;
                if(t.pitchmin || t.pitchmax) tpitch = clamp(tpitch, t.pitchmin, t.pitchmax);
                loopv(pitchcorrects)
                {
                    pitchcorrect &c = pitchcorrects[i];
                    if(c.target != j) continue;
                    float total = c.parent >= 0 ? pitchcorrects[c.parent].pitchtotal : 0, 
                          avail = tpitch - total, 
                          used = tpitch*c.pitchscale;
                    if(c.pitchmin || c.pitchmax)
                    {
                        if(used < 0) used = clamp(c.pitchmin, used, 0.0f);
                        else used = clamp(c.pitchmax, 0.0f, used);
                    }
                    if(used < 0) used = clamp(avail, used, 0.0f);
                    else used = clamp(avail, 0.0f, used);
                    c.pitchangle = used;
                    c.pitchtotal = used + total;
                }
            }
        }

        #define INTERPBONE(bone) \
            const animstate &s = as[partmask[bone]]; \
            const framedata &f = partframes[partmask[bone]]; \
            dualquat d; \
            (d = f.fr1[bone]).mul((1-s.cur.t)*s.interp); \
            d.accumulate(f.fr2[bone], s.cur.t*s.interp); \
            if(s.interp<1) \
            { \
                d.accumulate(f.pfr1[bone], (1-s.prev.t)*(1-s.interp)); \
                d.accumulate(f.pfr2[bone], s.prev.t*(1-s.interp)); \
            }

        #define INTERPBONES(outbody, rotbody) \
            sc.nextversion(); \
            struct framedata \
            { \
                const dualquat *fr1, *fr2, *pfr1, *pfr2; \
            } partframes[MAXANIMPARTS]; \
            loopi(numanimparts) \
            { \
                partframes[i].fr1 = &framebones[as[i].cur.fr1*numbones]; \
                partframes[i].fr2 = &framebones[as[i].cur.fr2*numbones]; \
                if(as[i].interp<1) \
                { \
                    partframes[i].pfr1 = &framebones[as[i].prev.fr1*numbones]; \
                    partframes[i].pfr2 = &framebones[as[i].prev.fr2*numbones]; \
                } \
            } \
            loopv(pitchdeps) \
            { \
                pitchdep &p = pitchdeps[i]; \
                INTERPBONE(p.bone); \
                d.normalize(); \
                if(p.parent >= 0) p.pose.mul(pitchdeps[p.parent].pose, d); \
                else p.pose = d; \
            } \
            calcpitchcorrects(pitch, axis, forward); \
            loopi(numbones) if(bones[i].interpindex>=0) \
            { \
                INTERPBONE(i); \
                const boneinfo &b = bones[i]; \
                outbody; \
                float angle; \
                if(b.pitchscale) { angle = b.pitchscale*pitch + b.pitchoffset; if(b.pitchmin || b.pitchmax) angle = clamp(angle, b.pitchmin, b.pitchmax); } \
                else if(b.correctindex >= 0) angle = pitchcorrects[b.correctindex].pitchangle; \
                else continue; \
                if(as->cur.anim&ANIM_NOPITCH || (as->interp < 1 && as->prev.anim&ANIM_NOPITCH)) \
                    angle *= (as->cur.anim&ANIM_NOPITCH ? 0 : as->interp) + (as->interp < 1 && as->prev.anim&ANIM_NOPITCH ? 0 : 1-as->interp); \
                rotbody; \
            }

        void interpmatbones(const animstate *as, float pitch, const vec &axis, const vec &forward, int numanimparts, const uchar *partmask, skelcacheentry &sc)
        {
            if(!sc.mdata) sc.mdata = new matrix3x4[numinterpbones];
            if(lastsdata == sc.mdata) lastsdata = NULL;
            INTERPBONES(
            {
                matrix3x4 m(d);
                if(b.interpparent<0) sc.mdata[b.interpindex] = m;
                else sc.mdata[b.interpindex].mul(sc.mdata[b.interpparent], m);
            },
            {
                sc.mdata[b.interpindex].mulorient(matrix3x3(angle*RAD, axis), b.base);
            });
        }

        void interpbones(const animstate *as, float pitch, const vec &axis, const vec &forward, int numanimparts, const uchar *partmask, skelcacheentry &sc)
        {
            if(!sc.bdata) sc.bdata = new dualquat[numinterpbones];
            if(lastsdata == sc.bdata) lastsdata = NULL;
            INTERPBONES(
            {
                d.normalize();
                if(b.interpparent<0) sc.bdata[b.interpindex] = d;
                else sc.bdata[b.interpindex].mul(sc.bdata[b.interpparent], d);
            },
            {
                sc.bdata[b.interpindex].mulorient(quat(axis, angle*RAD), b.base);
            });
            loopv(antipodes) sc.bdata[antipodes[i].child].fixantipodal(sc.bdata[antipodes[i].parent]);
        }

        #define INITRAGDOLL(ptype, pdata, relbody) \
            const ptype *pdata = sc.pdata; \
            loopv(ragdoll->joints) \
            { \
                const ragdollskel::joint &j = ragdoll->joints[i]; \
                const boneinfo &b = bones[j.bone]; \
                const ptype &p = pdata[b.interpindex]; \
                loopk(3) if(j.vert[k] >= 0) \
                { \
                    ragdollskel::vert &v = ragdoll->verts[j.vert[k]]; \
                    ragdolldata::vert &dv = d.verts[j.vert[k]]; \
                    dv.pos.add(p.transform(v.pos).mul(v.weight)); \
                } \
            } \
            if(ragdoll->animjoints) loopv(ragdoll->joints) \
            { \
                const ragdollskel::joint &j = ragdoll->joints[i]; \
                const boneinfo &b = bones[j.bone]; \
                const ptype &p = pdata[b.interpindex]; \
                d.calcanimjoint(i, p); \
            } \
            loopv(ragdoll->verts) \
            { \
                ragdolldata::vert &dv = d.verts[i]; \
                matrixstack[matrixpos].transform(vec(dv.pos).add(p->translate).mul(p->model->scale), dv.pos); \
            } \
            loopv(ragdoll->reljoints) \
            { \
                const ragdollskel::reljoint &r = ragdoll->reljoints[i]; \
                const ragdollskel::joint &j = ragdoll->joints[r.parent]; \
                const boneinfo &br = bones[r.bone], &bj = bones[j.bone]; \
                relbody; \
            }
  
        void initmatragdoll(ragdolldata &d, skelcacheentry &sc, part *p)
        {
            INITRAGDOLL(matrix3x4, mdata,
            {
                d.reljoints[i].transposemul(mdata[bj.interpindex], mdata[br.interpindex]);
            });
        }

        void initragdoll(ragdolldata &d, skelcacheentry &sc, part *p)
        {
            INITRAGDOLL(dualquat, bdata,
            {
                dualquat q = bdata[bj.interpindex];
                q.invert().mul(bdata[br.interpindex]);
                d.reljoints[i] = matrix3x4(q);
            });
        }

        #define GENRAGDOLLBONES(outbody, relbody) \
            sc.nextversion(); \
            loopv(ragdoll->joints) \
            { \
                const ragdollskel::joint &j = ragdoll->joints[i]; \
                const boneinfo &b = bones[j.bone]; \
                vec pos(0, 0, 0); \
                loopk(3) if(j.vert[k]>=0) pos.add(d.verts[j.vert[k]].pos); \
                pos.mul(j.weight/p->model->scale).sub(p->translate); \
                outbody; \
            } \
            loopv(ragdoll->reljoints) \
            { \
                const ragdollskel::reljoint &r = ragdoll->reljoints[i]; \
                const ragdollskel::joint &j = ragdoll->joints[r.parent]; \
                const boneinfo &br = bones[r.bone], &bj = bones[j.bone]; \
                relbody; \
            }

        void genmatragdollbones(ragdolldata &d, skelcacheentry &sc, part *p)
        {
            if(!sc.mdata) sc.mdata = new matrix3x4[numinterpbones];
            if(lastsdata == sc.mdata) lastsdata = NULL;
            GENRAGDOLLBONES(
            {
                sc.mdata[b.interpindex].transposemul(d.tris[j.tri], pos, d.animjoints ? d.animjoints[i] : j.orient);
            },
            {
                sc.mdata[br.interpindex].mul(sc.mdata[bj.interpindex], d.reljoints[i]);
            });
        }

        void genragdollbones(ragdolldata &d, skelcacheentry &sc, part *p)
        {
            if(!sc.bdata) sc.bdata = new dualquat[numinterpbones];
            if(lastsdata == sc.bdata) lastsdata = NULL;
            GENRAGDOLLBONES(
            {
                matrix3x4 m;
                m.transposemul(d.tris[j.tri], pos, d.animjoints ? d.animjoints[i] : j.orient);
                sc.bdata[b.interpindex] = dualquat(m);
            },
            {
                sc.bdata[br.interpindex].mul(sc.bdata[bj.interpindex], dualquat(d.reljoints[i]));
            });
            loopv(antipodes) sc.bdata[antipodes[i].child].fixantipodal(sc.bdata[antipodes[i].parent]);
        }

        void concattagtransform(part *p, int frame, int i, const matrix3x4 &m, matrix3x4 &n)
        {
            matrix3x4 t;
            t.mul(bones[tags[i].bone].base, tags[i].matrix);
            t.translate(vec(p->translate).mul(p->model->scale));
            n.mul(m, t);
        }

        void calctags(part *p, skelcacheentry *sc = NULL)
        {
            loopv(p->links)
            {
                linkedpart &l = p->links[i];
                tag &t = tags[l.tag];
                matrix3x4 m;
                m.mul(bones[t.bone].base, t.matrix);
                if(sc)
                {
                    int interpindex = bones[t.bone].interpindex;
                    m.mul(usematskel ? sc->mdata[interpindex] : sc->bdata[interpindex], matrix3x4(m));
                }
                l.matrix = m;
                l.matrix[12] = (l.matrix[12] + p->translate.x) * p->model->scale;
                l.matrix[13] = (l.matrix[13] + p->translate.y) * p->model->scale;
                l.matrix[14] = (l.matrix[14] + p->translate.z) * p->model->scale;
            }
        }

        void cleanup(bool full = true)
        {
            loopv(skelcache)
            {
                skelcacheentry &sc = skelcache[i];
                loopj(MAXANIMPARTS) sc.as[j].cur.fr1 = -1;
                DELETEA(sc.bdata);
                DELETEA(sc.mdata);
                if(sc.ubuf) { glDeleteBuffers_(1, &sc.ubuf); sc.ubuf = 0; }
            }
            skelcache.setsize(0);
            lastsdata = lastbdata = NULL;
            if(full) loopv(users) users[i]->cleanup();
        }

        skelcacheentry &checkskelcache(part *p, const animstate *as, float pitch, const vec &axis, const vec &forward, ragdolldata *rdata)
        {
            if(skelcache.empty()) 
            {
                usegpuskel = gpuaccelerate();
                usematskel = matskel!=0;
            }

            int numanimparts = ((skelpart *)as->owner)->numanimparts;
            uchar *partmask = ((skelpart *)as->owner)->partmask;
            skelcacheentry *sc = NULL;
            bool match = false;
            loopv(skelcache)
            {
                skelcacheentry &c = skelcache[i];
                loopj(numanimparts) if(c.as[j]!=as[j]) goto mismatch;
                if(c.pitch != pitch || c.partmask != partmask || c.ragdoll != rdata || (rdata && c.millis < rdata->lastmove)) goto mismatch;
                match = true;
                sc = &c;
                break;
            mismatch:
                if(c.millis < lastmillis) { sc = &c; break; }
            }
            if(!sc) sc = &skelcache.add();
            if(!match)
            {
                loopi(numanimparts) sc->as[i] = as[i];
                sc->pitch = pitch;
                sc->partmask = partmask;
                sc->ragdoll = rdata;
                if(rdata)
                {
                    if(matskel) genmatragdollbones(*rdata, *sc, p);
                    else genragdollbones(*rdata, *sc, p);
                }
                else if(matskel) interpmatbones(as, pitch, axis, forward, numanimparts, partmask, *sc);
                else interpbones(as, pitch, axis, forward, numanimparts, partmask, *sc);
            }
            sc->millis = lastmillis;
            return *sc;
        }

        void setasmbones(skelcacheentry &sc, int count = 0)
        {
            if(sc.dirty) sc.dirty = false;
            else if((count ? lastbdata : lastsdata) == (usematskel ? (void *)sc.mdata : (void *)sc.bdata)) return;
            int offset = count ? numgpubones : 0;
            if(!offset) count = numgpubones;
            if(hasPP)
            {
                if(usematskel) glProgramEnvParameters4fv_(GL_VERTEX_PROGRAM_ARB, 10 + 3*offset, 3*count, sc.mdata[offset].a.v);
                else glProgramEnvParameters4fv_(GL_VERTEX_PROGRAM_ARB, 10 + 2*offset, 2*count, sc.bdata[offset].real.v);
            }
            else if(usematskel) loopi(count)
            {
                glProgramEnvParameter4fv_(GL_VERTEX_PROGRAM_ARB, 10 + 3*(offset+i), sc.mdata[offset+i].a.v);
                glProgramEnvParameter4fv_(GL_VERTEX_PROGRAM_ARB, 11 + 3*(offset+i), sc.mdata[offset+i].b.v);
                glProgramEnvParameter4fv_(GL_VERTEX_PROGRAM_ARB, 12 + 3*(offset+i), sc.mdata[offset+i].c.v);
            }
            else loopi(count)
            {
                glProgramEnvParameter4fv_(GL_VERTEX_PROGRAM_ARB, 10 + 2*(offset+i), sc.bdata[offset+i].real.v);
                glProgramEnvParameter4fv_(GL_VERTEX_PROGRAM_ARB, 11 + 2*(offset+i), sc.bdata[offset+i].dual.v);
            }
            if(offset) lastbdata = usematskel ? (void *)sc.mdata : (void *)sc.bdata;
            else lastsdata = usematskel ? (void *)sc.mdata : (void *)sc.bdata;
        }

        void bindubo(UniformLoc &u, skelcacheentry &sc, skelcacheentry &bc, int count)
        {
            if(hasUBO)
            {
                if(!lastsdata && lastbdata == &bc.ubuf && !bc.dirty) return;
            }
            else if(u.version == bc.version && u.data == &bc.ubuf) return;
            if(!bc.ubuf) { glGenBuffers_(1, &bc.ubuf); bc.dirty = true; }
            if(bc.dirty)
            {
                GLenum target = hasUBO ? GL_UNIFORM_BUFFER : GL_UNIFORM_BUFFER_EXT;
                glBindBuffer_(target, bc.ubuf);
                glBufferData_(target, u.size, NULL, GL_STREAM_DRAW_ARB);
                int bsize = usematskel ? sizeof(matrix3x4) : sizeof(dualquat), boffset = numgpubones*bsize;
                glBufferSubData_(target, u.offset, boffset, usematskel ? (void *)sc.mdata : (void *)sc.bdata);
                if(count > 0) glBufferSubData_(target, u.offset + boffset, count*bsize, usematskel ? (void *)&bc.mdata[numgpubones] : (void *)&bc.bdata[numgpubones]);
                glBindBuffer_(target, 0);
                bc.dirty = false;
            }
            if(hasUBO)
            {
                glBindBufferBase_(GL_UNIFORM_BUFFER, u.binding, bc.ubuf);
                lastsdata = NULL;
                lastbdata = &bc.ubuf;
            }
            else
            {
                glUniformBuffer_(Shader::lastshader->program, u.loc, bc.ubuf); 
                u.version = bc.version;
                u.data = &bc.ubuf;
            }
        }

        void setglslbones(UniformLoc &u, skelcacheentry &sc, skelcacheentry &bc, int count)
        {
            if(u.version == bc.version && u.data == (usematskel ? (void *)bc.mdata : (void *)bc.bdata)) return;
            count += numgpubones;
            if(usematskel) 
            {
                if(count > numgpubones && bc.dirty) 
                {
                    memcpy(bc.mdata, sc.mdata, numgpubones*sizeof(matrix3x4));
                    bc.dirty = false;
                }
                glUniform4fv_(u.loc, 3*count, bc.mdata[0].a.v);
            }
            else 
            {
                if(count > numgpubones && bc.dirty) 
                {
                    memcpy(bc.bdata, sc.bdata, numgpubones*sizeof(dualquat));
                    bc.dirty = false;
                }
                glUniform4fv_(u.loc, 2*count, bc.bdata[0].real.v);
            }
            u.version = bc.version;
            u.data = usematskel ? (void *)bc.mdata : (void *)bc.bdata;
        }
        
        void setgpubones(skelcacheentry &sc, blendcacheentry *bc, int count)
        {
            if(!Shader::lastshader) return;
            if(Shader::lastshader->type & SHADER_GLSLANG) 
            {
                if(Shader::lastshader->uniformlocs.length() < 1) return;
                UniformLoc &u = Shader::lastshader->uniformlocs[0];
                if(u.size > 0 && (hasUBO || hasBUE)) bindubo(u, sc, bc ? *bc : sc, count);
                else setglslbones(u, sc, bc ? *bc : sc, count);
            }
            else
            {
                setasmbones(sc);
                if(bc) setasmbones(*bc, count);
            }
        }
    
        bool shouldcleanup() const
        {
            return numframes && (skelcache.empty() || gpuaccelerate()!=usegpuskel || (matskel!=0)!=usematskel);
        }
    };

    struct skelmeshgroup : meshgroup
    {
        skeleton *skel;

        vector<blendcombo> blendcombos;
        int numblends[4];

        static const int MAXBLENDCACHE = 16;
        blendcacheentry blendcache[MAXBLENDCACHE];

        static const int MAXVBOCACHE = 16;
        vbocacheentry vbocache[MAXVBOCACHE];
 
        ushort *edata;
        GLuint ebuf;
        bool vnorms, vtangents;
        int vlen, vertsize, vblends, vweights;
        uchar *vdata;

        skelmeshgroup() : skel(NULL), edata(NULL), ebuf(0), vdata(NULL)
        {
            memset(numblends, 0, sizeof(numblends));
        }

        virtual ~skelmeshgroup()
        {
            if(skel)
            {
                if(skel->shared) skel->users.removeobj(this);
                else DELETEP(skel);
            }
            if(ebuf) glDeleteBuffers_(1, &ebuf);
            loopi(MAXBLENDCACHE)
            {
                DELETEA(blendcache[i].bdata);
                DELETEA(blendcache[i].mdata);
                if(blendcache[i].ubuf) glDeleteBuffers_(1, &blendcache[i].ubuf); 
            }
            loopi(MAXVBOCACHE)
            {
                DELETEA(vbocache[i].vdata);
                if(vbocache[i].vbuf) glDeleteBuffers_(1, &vbocache[i].vbuf);
            }
            DELETEA(vdata);
        }

        void shareskeleton(char *name)
        {
            if(!name)
            {
                skel = new skeleton;
                skel->users.add(this);
                return;
            }

            static hashtable<char *, skeleton *> skeletons;
            if(skeletons.access(name)) skel = skeletons[name];
            else
            {
                skel = new skeleton;
                skel->name = newstring(name);
                skeletons[skel->name] = skel;
            }
            skel->users.add(this);
            skel->shared++;
        }

        int findtag(const char *name)
        {
            return skel->findtag(name);
        }

        int totalframes() const { return max(skel->numframes, 1); }

        virtual skelanimspec *loadanim(const char *filename) { return NULL; }

        void genvbo(bool norms, bool tangents, vbocacheentry &vc)
        {
            if(hasVBO)
            {
                if(!vc.vbuf) glGenBuffers_(1, &vc.vbuf);
                if(ebuf) return;
            }
            else if(edata)
            {
                #define ALLOCVDATA(vdata) \
                    do \
                    { \
                        DELETEA(vdata); \
                        vdata = new uchar[vlen*vertsize]; \
                        loopv(meshes) \
                        { \
                            skelmesh &m = *(skelmesh *)meshes[i]; \
                            m.filltc(vdata, vertsize); \
                            if(tangents) m.fillbump(vdata, vertsize); \
                        } \
                    } while(0)
                if(!vc.vdata) ALLOCVDATA(vc.vdata);
                return;
            }

            vector<ushort> idxs;

            vnorms = norms;
            vtangents = tangents;
            vlen = 0;
            vblends = 0;
            if(skel->numframes && !skel->usegpuskel)
            {
                vweights = 1;
                loopv(blendcombos)
                {
                    blendcombo &c = blendcombos[i];
                    c.interpindex = c.weights[1] ? skel->numinterpbones + vblends++ : -1;
                }

                vertsize = tangents ? sizeof(vvertbump) : (norms ? sizeof(vvertn) : sizeof(vvert));
                loopv(meshes) vlen += ((skelmesh *)meshes[i])->genvbo(idxs, vlen);
                DELETEA(vdata);
                if(hasVBO) ALLOCVDATA(vdata);
                else ALLOCVDATA(vc.vdata);
            }
            else
            {
                if(skel->numframes)
                {
                    vweights = 4;
                    int availbones = skel->availgpubones() - skel->numgpubones;
                    while(vweights > 1 && availbones >= numblends[vweights-1]) availbones -= numblends[--vweights];
                    loopv(blendcombos)
                    {
                        blendcombo &c = blendcombos[i];
                        c.interpindex = c.size() > vweights ? skel->numgpubones + vblends++ : -1;
                    }
                }
                else
                {
                    vweights = 0;
                    loopv(blendcombos) blendcombos[i].interpindex = -1;
                }

                if(hasVBO) glBindBuffer_(GL_ARRAY_BUFFER_ARB, vc.vbuf);
                #define GENVBO(type, args) \
                    do \
                    { \
                        vertsize = sizeof(type); \
                        vector<type> vverts; \
                        loopv(meshes) vlen += ((skelmesh *)meshes[i])->genvbo args; \
                        if(hasVBO) glBufferData_(GL_ARRAY_BUFFER_ARB, vverts.length()*sizeof(type), vverts.getbuf(), GL_STATIC_DRAW_ARB); \
                        else \
                        { \
                            DELETEA(vc.vdata); \
                            vc.vdata = new uchar[vverts.length()*sizeof(type)]; \
                            memcpy(vc.vdata, vverts.getbuf(), vverts.length()*sizeof(type)); \
                        } \
                    } while(0)
                #define GENVBOANIM(type) GENVBO(type, (idxs, vlen, vverts))
                #define GENVBOSTAT(type) GENVBO(type, (idxs, vlen, vverts, htdata, htlen))
                if(skel->numframes)
                {
                    if(tangents) GENVBOANIM(vvertbumpw);
                    else GENVBOANIM(vvertw);
                }
                else 
                {
                    int numverts = 0, htlen = 128;
                    loopv(meshes) numverts += ((skelmesh *)meshes[i])->numverts;
                    while(htlen < numverts) htlen *= 2;
                    if(numverts*4 > htlen*3) htlen *= 2;  
                    int *htdata = new int[htlen];
                    memset(htdata, -1, htlen*sizeof(int));
                    if(tangents) GENVBOSTAT(vvertbump);
                    else if(norms) GENVBOSTAT(vvertn);
                    else GENVBOSTAT(vvert);
                    delete[] htdata;
                }
                if(hasVBO) glBindBuffer_(GL_ARRAY_BUFFER_ARB, 0);
            }

            if(hasVBO)
            {
                glGenBuffers_(1, &ebuf);
                glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER_ARB, ebuf);
                glBufferData_(GL_ELEMENT_ARRAY_BUFFER_ARB, idxs.length()*sizeof(ushort), idxs.getbuf(), GL_STATIC_DRAW_ARB);
                glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
            }
            else
            {
                edata = new ushort[idxs.length()];
                memcpy(edata, idxs.getbuf(), idxs.length()*sizeof(ushort));
            }
            #undef GENVBO
            #undef GENVBOANIM
            #undef GENVBOSTAT
            #undef ALLOCVDATA
        }

        void bindvbo(const animstate *as, vbocacheentry &vc, skelcacheentry *sc = NULL, blendcacheentry *bc = NULL)
        {
            vvertn *vverts = hasVBO ? 0 : (vvertn *)vc.vdata;
            if(hasVBO && lastebuf!=ebuf)
            {
                glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER_ARB, ebuf);
                lastebuf = ebuf;
            }
            if(lastvbuf != (hasVBO ? (void *)(size_t)vc.vbuf : vc.vdata))
            {
                if(hasVBO) glBindBuffer_(GL_ARRAY_BUFFER_ARB, vc.vbuf);
                if(!lastvbuf) glEnableClientState(GL_VERTEX_ARRAY);
                glVertexPointer(3, GL_FLOAT, vertsize, &vverts->pos);
                lastvbuf = hasVBO ? (void *)(size_t)vc.vbuf : vc.vdata;
            }
            if(as->cur.anim&ANIM_NOSKIN)
            {
                if(enabletc) disabletc();
                if(enablenormals) disablenormals();
            }
            else
            {
                if(vnorms || vtangents)
                {
                    if(!enablenormals)
                    {
                        glEnableClientState(GL_NORMAL_ARRAY);
                        enablenormals = true;
                    }
                    if(lastnbuf!=lastvbuf)
                    {
                        glNormalPointer(GL_FLOAT, vertsize, &vverts->norm);
                        lastnbuf = lastvbuf;
                    }
                }
                else if(enablenormals) disablenormals();

                if(!enabletc)
                {
                    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                    enabletc = true;
                }
                if(lasttcbuf!=lastvbuf)
                {
                    glTexCoordPointer(2, GL_FLOAT, vertsize, &vverts->u);
                    lasttcbuf = lastvbuf;
                }
            }
            if(!sc || !skel->usegpuskel)
            {
                if(enablebones) disablebones();
                return;
            }
            if(!enablebones)
            {
                glEnableVertexAttribArray_(6);
                glEnableVertexAttribArray_(7);
                enablebones = true;
            }
            if(lastbbuf!=lastvbuf)
            {
                glVertexAttribPointer_(6, 4, GL_UNSIGNED_BYTE, GL_TRUE, vertsize, &((vvertw *)vverts)->weights);
                glVertexAttribPointer_(7, 4, GL_UNSIGNED_BYTE, GL_FALSE, vertsize, &((vvertw *)vverts)->bones);
                lastbbuf = lastvbuf;
            }
        }

        void concattagtransform(part *p, int frame, int i, const matrix3x4 &m, matrix3x4 &n)
        {
            skel->concattagtransform(p, frame, i, m, n);
        }

        int addblendcombo(const blendcombo &c)
        {
            loopv(blendcombos) if(blendcombos[i]==c)
            {
                blendcombos[i].uses += c.uses;
                return i;
            }
            numblends[c.size()-1]++;
            blendcombo &a = blendcombos.add(c);
            return a.interpindex = blendcombos.length()-1; 
        }

        void sortblendcombos()
        {
            blendcombos.sort(blendcombo::sortcmp);
            int *remap = new int[blendcombos.length()];
            loopv(blendcombos) remap[blendcombos[i].interpindex] = i;
            loopv(meshes)
            {
                skelmesh *m = (skelmesh *)meshes[i];
                loopj(m->numverts)
                {
                    vert &v = m->verts[j];
                    v.blend = remap[v.blend];
                }
            }
            delete[] remap;
        }

        int remapblend(int blend)
        {
            const blendcombo &c = blendcombos[blend];
            return c.weights[1] ? c.interpindex : c.interpbones[0];
        }

        template<class B>
        static inline void blendbones(B &d, const B *bdata, const blendcombo &c)
        {
            d = bdata[c.interpbones[0]];
            d.mul(c.weights[0]);
            d.accumulate(bdata[c.interpbones[1]], c.weights[1]);
            if(c.weights[2])
            {
                d.accumulate(bdata[c.interpbones[2]], c.weights[2]);
                if(c.weights[3]) d.accumulate(bdata[c.interpbones[3]], c.weights[3]);
            }
        }

        void blendmatbones(const skelcacheentry &sc, blendcacheentry &bc)
        {
            bc.nextversion();
            if(!bc.mdata) bc.mdata = new matrix3x4[(skel->usegpuskel ? skel->numgpubones : 0) + vblends];
            if(lastbdata == bc.mdata) lastbdata = NULL;
            matrix3x4 *dst = bc.mdata - (skel->usegpuskel ? 0 : skel->numinterpbones);
            loopv(blendcombos)
            {
                const blendcombo &c = blendcombos[i];
                if(c.interpindex<0) break;
                blendbones(dst[c.interpindex], sc.mdata, c);
            }
        }

        void blendbones(const skelcacheentry &sc, blendcacheentry &bc)
        {
            bc.nextversion();
            if(!bc.bdata) bc.bdata = new dualquat[(skel->usegpuskel ? skel->numgpubones : 0) + vblends];
            if(lastbdata == bc.bdata) lastbdata = NULL;
            dualquat *dst = bc.bdata - (skel->usegpuskel ? 0 : skel->numinterpbones);
            bool normalize = !skel->usegpuskel || vweights<=1;
            loopv(blendcombos)
            {
                const blendcombo &c = blendcombos[i];
                if(c.interpindex<0) break;
                dualquat &d = dst[c.interpindex];
                blendbones(d, sc.bdata, c);
                if(normalize) d.normalize();
            }
        }

        void cleanup()
        {
            loopi(MAXBLENDCACHE)
            {
                blendcacheentry &c = blendcache[i];
                DELETEA(c.bdata);
                DELETEA(c.mdata);
                if(c.ubuf) { glDeleteBuffers_(1, &c.ubuf); c.ubuf = 0; }
                c.owner = -1;
            }
            loopi(MAXVBOCACHE)
            {
                vbocacheentry &c = vbocache[i];
                if(c.vbuf) { glDeleteBuffers_(1, &c.vbuf); c.vbuf = 0; }
                DELETEA(c.vdata);
                c.owner = -1;
            }
            if(hasVBO) { if(ebuf) { glDeleteBuffers_(1, &ebuf); ebuf = 0; } }
            else DELETEA(vdata);
            if(skel) skel->cleanup(false);
        }

        #define SEARCHCACHE(cachesize, cacheentry, cache, reusecheck) \
            loopi(cachesize) \
            { \
                cacheentry &c = cache[i]; \
                if(c.owner==owner) \
                { \
                     if(c==sc) return c; \
                     else c.owner = -1; \
                     break; \
                } \
            } \
            loopi(cachesize-1) \
            { \
                cacheentry &c = cache[i]; \
                if(reusecheck c.owner < 0 || c.millis < lastmillis) \
                    return c; \
            } \
            return cache[cachesize-1];

        vbocacheentry &checkvbocache(skelcacheentry &sc, int owner)
        {
            SEARCHCACHE(MAXVBOCACHE, vbocacheentry, vbocache, (hasVBO ? !c.vbuf : !c.vdata) || );
        }

        blendcacheentry &checkblendcache(skelcacheentry &sc, int owner)
        {
            SEARCHCACHE(MAXBLENDCACHE, blendcacheentry, blendcache, )
        }

        void render(const animstate *as, float pitch, const vec &axis, const vec &forward, dynent *d, part *p)
        {
            bool norms = false, tangents = false;
            loopv(p->skins)
            {
                if(p->skins[i].normals()) norms = true;
                if(p->skins[i].tangents()) tangents = true;
            }
            if(skel->shouldcleanup()) { skel->cleanup(); disablevbo(); }
            else if(norms!=vnorms || tangents!=vtangents) { cleanup(); disablevbo(); }

            if(!skel->numframes)
            {
                if(!(as->cur.anim&ANIM_NORENDER))
                {
                    if(hasVBO ? !vbocache->vbuf : !vbocache->vdata) genvbo(norms, tangents, *vbocache);
                    bindvbo(as, *vbocache);
                    loopv(meshes) 
                    {
                        skelmesh *m = (skelmesh *)meshes[i];
                        p->skins[i].bind(m, as);
                        m->render(as, p->skins[i], *vbocache);
                    }
                }
                skel->calctags(p);
                return;
            }

            skelcacheentry &sc = skel->checkskelcache(p, as, pitch, axis, forward, as->cur.anim&ANIM_RAGDOLL || !d || !d->ragdoll || d->ragdoll->skel != skel->ragdoll ? NULL : d->ragdoll);
            if(!(as->cur.anim&ANIM_NORENDER))
            {
                int owner = &sc-&skel->skelcache[0];
                vbocacheentry &vc = skel->usegpuskel ? *vbocache : checkvbocache(sc, owner);
                vc.millis = lastmillis;
                if(hasVBO ? !vc.vbuf : !vc.vdata) genvbo(norms, tangents, vc);
                blendcacheentry *bc = NULL;
                if(vblends)
                {
                    bc = &checkblendcache(sc, owner);
                    bc->millis = lastmillis;
                    if(bc->owner!=owner)
                    {
                        bc->owner = owner;
                        *(animcacheentry *)bc = sc;
                        if(skel->usematskel) blendmatbones(sc, *bc);
                        else blendbones(sc, *bc);
                    }
                }
                if(!skel->usegpuskel && vc.owner!=owner)
                { 
                    vc.owner = owner;
                    (animcacheentry &)vc = sc;
                    loopv(meshes)
                    {
                        skelmesh &m = *(skelmesh *)meshes[i];
                        if(skel->usematskel) m.interpverts(sc.mdata, bc ? bc->mdata : NULL, norms, tangents, (hasVBO ? vdata : vc.vdata) + m.voffset*vertsize, p->skins[i]);
                        else m.interpverts(sc.bdata, bc ? bc->bdata : NULL, norms, tangents, (hasVBO ? vdata : vc.vdata) + m.voffset*vertsize, p->skins[i]);
                    }
                    if(hasVBO)
                    {
                        glBindBuffer_(GL_ARRAY_BUFFER_ARB, vc.vbuf);
                        glBufferData_(GL_ARRAY_BUFFER_ARB, vlen*vertsize, vdata, GL_STREAM_DRAW_ARB);
                    }
                }

                bindvbo(as, vc, &sc, bc);
                loopv(meshes) 
                {
                    skelmesh *m = (skelmesh *)meshes[i];
                    p->skins[i].bind(m, as);
                    if(skel->usegpuskel) skel->setgpubones(sc, bc, vblends);
                    m->render(as, p->skins[i], vc);
                }
            }

            skel->calctags(p, &sc);

            if(as->cur.anim&ANIM_RAGDOLL && skel->ragdoll && !d->ragdoll)
            {
                d->ragdoll = new ragdolldata(skel->ragdoll, p->model->scale);
                if(matskel) skel->initmatragdoll(*d->ragdoll, sc, p);
                else skel->initragdoll(*d->ragdoll, sc, p);
                d->ragdoll->init(d);
            }
        }
    };

    struct animpartmask
    {
        animpartmask *next;
        int numbones;
        uchar bones[1];
    };

    struct skelpart : part
    {
        animpartmask *buildingpartmask;

        uchar *partmask;
        
        skelpart() : buildingpartmask(NULL), partmask(NULL)
        {
        }

        virtual ~skelpart()
        {
            DELETEA(buildingpartmask);
        }

        uchar *sharepartmask(animpartmask *o)
        {
            static animpartmask *partmasks = NULL;
            animpartmask *p = partmasks;
            for(; p; p = p->next) if(p->numbones==o->numbones && !memcmp(p->bones, o->bones, p->numbones))
            {
                delete[] (uchar *)o;
                return p->bones;
            }

            o->next = p;
            partmasks = o;
            return o->bones;
        }

        animpartmask *newpartmask()
        {
            animpartmask *p = (animpartmask *)new uchar[sizeof(animpartmask) + ((skelmeshgroup *)meshes)->skel->numbones-1];
            p->numbones = ((skelmeshgroup *)meshes)->skel->numbones;
            memset(p->bones, 0, p->numbones);
            return p;
        }

        void initanimparts()
        {
            DELETEA(buildingpartmask);
            buildingpartmask = newpartmask();
        }

        bool addanimpart(ushort *bonemask)
        {
            if(!buildingpartmask || numanimparts>=MAXANIMPARTS) return false;
            ((skelmeshgroup *)meshes)->skel->applybonemask(bonemask, buildingpartmask->bones, numanimparts);
            numanimparts++;
            return true;
        }

        void endanimparts()
        {
            if(buildingpartmask)
            {
                partmask = sharepartmask(buildingpartmask);
                buildingpartmask = NULL;
            }

            ((skelmeshgroup *)meshes)->skel->optimize();
        }
    };

    skelmodel(const char *name) : animmodel(name)
    {
    }

    int linktype(animmodel *m) const
    {
        return type()==m->type() &&
            ((skelmeshgroup *)parts[0]->meshes)->skel == ((skelmeshgroup *)m->parts[0]->meshes)->skel ? 
                LINK_REUSE : 
                LINK_TAG;
    }
    
    bool skeletal() const { return true; }
};

struct skeladjustment
{
    float yaw, pitch, roll;
    vec translate;

    skeladjustment(float yaw, float pitch, float roll, const vec &translate) : yaw(yaw), pitch(pitch), roll(roll), translate(translate) {}

    void adjust(dualquat &dq)
    {
        if(yaw) dq.mulorient(quat(vec(0, 0, 1), yaw*RAD));
        if(pitch) dq.mulorient(quat(vec(0, -1, 0), pitch*RAD));
        if(roll) dq.mulorient(quat(vec(-1, 0, 0), roll*RAD));
        if(!translate.iszero()) dq.translate(translate);
    }
};

template<class MDL> struct skelloader : modelloader<MDL>
{
    static vector<skeladjustment> adjustments;
};

template<class MDL> vector<skeladjustment> skelloader<MDL>::adjustments;

template<class MDL> struct skelcommands : modelcommands<MDL, struct MDL::skelmesh>
{
    typedef modelcommands<MDL, struct MDL::skelmesh> commands;
    typedef struct MDL::skeleton skeleton;
    typedef struct MDL::skelmeshgroup meshgroup;
    typedef struct MDL::skelpart part;
    typedef struct MDL::skin skin;
    typedef struct MDL::boneinfo boneinfo;
    typedef struct MDL::skelanimspec animspec;
    typedef struct MDL::pitchdep pitchdep;
    typedef struct MDL::pitchtarget pitchtarget;
    typedef struct MDL::pitchcorrect pitchcorrect;

    static void loadpart(char *meshfile, char *skelname, float *smooth)
    {
        if(!MDL::loading) { conoutf("not loading an %s", MDL::formatname()); return; }
        defformatstring(filename)("%s/%s", MDL::dir, meshfile);
        part &mdl = *new part;
        MDL::loading->parts.add(&mdl);
        mdl.model = MDL::loading;
        mdl.index = MDL::loading->parts.length()-1;
        mdl.pitchscale = mdl.pitchoffset = mdl.pitchmin = mdl.pitchmax = 0;
        MDL::adjustments.setsize(0);
        mdl.meshes = MDL::loading->sharemeshes(path(filename), skelname[0] ? skelname : NULL, double(*smooth > 0 ? cos(clamp(*smooth, 0.0f, 180.0f)*RAD) : 2));
        if(!mdl.meshes) conoutf("could not load %s", filename);
        else
        {
            mdl.initanimparts();
            mdl.initskins();
        }
    }
   
    static void settag(char *name, char *tagname, float *tx, float *ty, float *tz, float *rx, float *ry, float *rz)
    {
        if(!MDL::loading || MDL::loading->parts.empty()) { conoutf("not loading an %s", MDL::formatname()); return; }
        part &mdl = *(part *)MDL::loading->parts.last();
        int i = mdl.meshes ? ((meshgroup *)mdl.meshes)->skel->findbone(name) : -1;
        if(i >= 0)
        {
            float cx = *rx ? cosf(*rx/2*RAD) : 1, sx = *rx ? sinf(*rx/2*RAD) : 0,
                  cy = *ry ? cosf(*ry/2*RAD) : 1, sy = *ry ? sinf(*ry/2*RAD) : 0,
                  cz = *rz ? cosf(*rz/2*RAD) : 1, sz = *rz ? sinf(*rz/2*RAD) : 0;
            matrix3x4 m(matrix3x3(quat(sx*cy*cz - cx*sy*sz, cx*sy*cz + sx*cy*sz, cx*cy*sz - sx*sy*cz, cx*cy*cz + sx*sy*sz)),
                        vec(*tx, *ty, *tz));
            ((meshgroup *)mdl.meshes)->skel->addtag(tagname, i, m);
            return;
        }
        conoutf("could not find bone %s for tag %s", name, tagname);
    }

    static void setpitch(char *name, float *pitchscale, float *pitchoffset, float *pitchmin, float *pitchmax)
    {
        if(!MDL::loading || MDL::loading->parts.empty()) { conoutf("not loading an %s", MDL::formatname()); return; }
        part &mdl = *(part *)MDL::loading->parts.last();
    
        if(name[0])
        {
            int i = mdl.meshes ? ((meshgroup *)mdl.meshes)->skel->findbone(name) : -1;
            if(i>=0)
            {
                boneinfo &b = ((meshgroup *)mdl.meshes)->skel->bones[i];
                b.pitchscale = *pitchscale;
                b.pitchoffset = *pitchoffset;
                if(*pitchmin || *pitchmax)
                {
                    b.pitchmin = *pitchmin;
                    b.pitchmax = *pitchmax;
                }
                else
                {
                    b.pitchmin = -360*fabs(b.pitchscale) + b.pitchoffset;
                    b.pitchmax = 360*fabs(b.pitchscale) + b.pitchoffset;
                }
                return;
            }
            conoutf("could not find bone %s to pitch", name);
            return;
        }
    
        mdl.pitchscale = *pitchscale;
        mdl.pitchoffset = *pitchoffset;
        if(*pitchmin || *pitchmax)
        {
            mdl.pitchmin = *pitchmin;
            mdl.pitchmax = *pitchmax;
        }
        else
        {
            mdl.pitchmin = -360*fabs(mdl.pitchscale) + mdl.pitchoffset;
            mdl.pitchmax = 360*fabs(mdl.pitchscale) + mdl.pitchoffset;
        }
    }

    static void setpitchtarget(char *name, char *animfile, int *frameoffset, float *pitchmin, float *pitchmax)
    {
        if(!MDL::loading || MDL::loading->parts.empty()) { conoutf("\frnot loading an %s", MDL::formatname()); return; }
        part &mdl = *(part *)MDL::loading->parts.last();
        if(!mdl.meshes) return;
        defformatstring(filename)("%s/%s", MDL::dir, animfile);
        animspec *sa = ((meshgroup *)mdl.meshes)->loadanim(path(filename));
        if(!sa) { conoutf("\frcould not load %s anim file %s", MDL::formatname(), filename); return; }
        skeleton *skel = ((meshgroup *)mdl.meshes)->skel;
        int bone = skel ? skel->findbone(name) : -1;
        if(bone < 0)
        {
            conoutf("\frcould not find bone %s to pitch target", name);
            return;
        }
        loopv(skel->pitchtargets) if(skel->pitchtargets[i].bone == bone) return;
        pitchtarget &t = skel->pitchtargets.add();
        t.bone = bone;
        t.frame = sa->frame + clamp(*frameoffset, 0, sa->range-1);
        t.pitchmin = *pitchmin;
        t.pitchmax = *pitchmax;
    }

    static void setpitchcorrect(char *name, char *targetname, float *scale, float *pitchmin, float *pitchmax)
    {
        if(!MDL::loading || MDL::loading->parts.empty()) { conoutf("\frnot loading an %s", MDL::formatname()); return; }
        part &mdl = *(part *)MDL::loading->parts.last();
        if(!mdl.meshes) return;
        skeleton *skel = ((meshgroup *)mdl.meshes)->skel;
        int bone = skel ? skel->findbone(name) : -1;
        if(bone < 0)
        {
            conoutf("\frcould not find bone %s to pitch correct", name);
            return;
        }
        if(skel->findpitchcorrect(bone) >= 0) return;
        int targetbone = skel->findbone(targetname), target = -1;
        if(targetbone >= 0) loopv(skel->pitchtargets) if(skel->pitchtargets[i].bone == targetbone) { target = i; break; }
        if(target < 0)
        {
            conoutf("\frcould not find pitch target %s to pitch correct %s", targetname, name);
            return;
        }
        pitchcorrect c;
        c.bone = bone;
        c.target = target;
        c.pitchmin = *pitchmin;
        c.pitchmax = *pitchmax;
        c.pitchscale = *scale;
        int pos = skel->pitchcorrects.length();
        loopv(skel->pitchcorrects) if(bone <= skel->pitchcorrects[i].bone) { pos = i; break; break; }
        skel->pitchcorrects.insert(pos, c); 
    }

    static void setanim(char *anim, char *animfile, float *speed, int *priority, int *startoffset, int *endoffset)
    {
        if(!MDL::loading || MDL::loading->parts.empty()) { conoutf("not loading an %s", MDL::formatname()); return; }
    
        vector<int> anims;
        findanims(anim, anims);
        if(anims.empty()) conoutf("could not find animation %s", anim);
        else
        {
            part *p = (part *)MDL::loading->parts.last();
            if(!p->meshes) return;
            defformatstring(filename)("%s/%s", MDL::dir, animfile);
            animspec *sa = ((meshgroup *)p->meshes)->loadanim(path(filename));
            if(!sa) conoutf("could not load %s anim file %s", MDL::formatname(), filename);
            else loopv(anims)
            {
                int start = sa->frame, end = sa->range;
                if(*startoffset > 0) start += min(*startoffset, end-1);
                else if(*startoffset < 0) start += max(end + *startoffset, 0);
                end -= start - sa->frame;
                if(*endoffset > 0) end = min(end, *endoffset);
                else if(*endoffset < 0) end = max(end + *endoffset, 1); 
                MDL::loading->parts.last()->setanim(p->numanimparts-1, anims[i], start, end, *speed, *priority);
            }
        }
    }
    
    static void setanimpart(char *maskstr)
    {
        if(!MDL::loading || MDL::loading->parts.empty()) { conoutf("not loading an %s", MDL::formatname()); return; }
    
        part *p = (part *)MDL::loading->parts.last();
    
        vector<char *> bonestrs;
        explodelist(maskstr, bonestrs);
        vector<ushort> bonemask;
        loopv(bonestrs)
        {
            char *bonestr = bonestrs[i];
            int bone = p->meshes ? ((meshgroup *)p->meshes)->skel->findbone(bonestr[0]=='!' ? bonestr+1 : bonestr) : -1;
            if(bone<0) { conoutf("could not find bone %s for anim part mask [%s]", bonestr, maskstr); bonestrs.deletearrays(); return; }
            bonemask.add(bone | (bonestr[0]=='!' ? BONEMASK_NOT : 0));
        }
        bonestrs.deletearrays();
        bonemask.sort();
        if(bonemask.length()) bonemask.add(BONEMASK_END);
    
        if(!p->addanimpart(bonemask.getbuf())) conoutf("too many animation parts");
    }

    static void setadjust(char *name, float *yaw, float *pitch, float *roll, float *tx, float *ty, float *tz)
    {
        if(!MDL::loading || MDL::loading->parts.empty()) { conoutf("not loading an %s", MDL::formatname()); return; }
        part &mdl = *(part *)MDL::loading->parts.last();

        if(!name[0]) return;
        int i = mdl.meshes ? ((meshgroup *)mdl.meshes)->skel->findbone(name) : -1;
        if(i < 0) {  conoutf("could not find bone %s to adjust", name); return; }
        while(!MDL::adjustments.inrange(i)) MDL::adjustments.add(skeladjustment(0, 0, 0, vec(0, 0, 0)));
        MDL::adjustments[i] = skeladjustment(*yaw, *pitch, *roll, vec(*tx/4, *ty/4, *tz/4));
    }
    
    skelcommands()
    {
        if(MDL::multiparted()) this->modelcommand(loadpart, "load", "ssf");
        this->modelcommand(settag, "tag", "ssffffff");
        this->modelcommand(setpitch, "pitch", "sffff");
        this->modelcommand(setpitchtarget, "pitchtarget", "ssiff");
        this->modelcommand(setpitchcorrect, "pitchcorrect", "ssfff");
        if(MDL::animated())
        {
            this->modelcommand(setanim, "anim", "ssfiii");
            this->modelcommand(setanimpart, "animpart", "s");
            this->modelcommand(setadjust, "adjust", "sffffff");
        }
    }
};

