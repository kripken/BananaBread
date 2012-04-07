struct vertmodel : animmodel
{
    struct vert { vec pos, norm; };
    struct vvertff { vec pos; float u, v; };
    struct vvert : vvertff { vec norm; };
    struct vvertbump : vvert { vec tangent; float bitangent; };
    struct tcvert { float u, v; };
    struct bumpvert { vec tangent; float bitangent; };
    struct tri { ushort vert[3]; };

    struct vbocacheentry
    {
        uchar *vdata;
        GLuint vbuf;
        animstate as;
        int millis;
 
        vbocacheentry() : vdata(NULL), vbuf(0) { as.cur.fr1 = as.prev.fr1 = -1; }
    };

    struct vertmesh : mesh
    {
        vert *verts;
        tcvert *tcverts;
        bumpvert *bumpverts;
        tri *tris;
        int numverts, numtris;

        int voffset, eoffset, elen;
        ushort minvert, maxvert;

        vertmesh() : verts(0), tcverts(0), bumpverts(0), tris(0)
        {
        }

        virtual ~vertmesh()
        {
            DELETEA(verts);
            DELETEA(tcverts);
            DELETEA(bumpverts);
            DELETEA(tris);
        }

        void smoothnorms(float limit = 0, bool areaweight = true)
        {
            if(((vertmeshgroup *)group)->numframes != 1)
            {
                buildnorms(areaweight);
                return;
            }
            mesh::smoothnorms(verts, numverts, tris, numtris, limit, areaweight);
        }

        void buildnorms(bool areaweight = true)
        {
            loopk(((vertmeshgroup *)group)->numframes)
                mesh::buildnorms(&verts[k*numverts], numverts, tris, numtris, areaweight); 
        }

        void calctangents(bool areaweight = true)
        {
            if(bumpverts) return;
            bumpverts = new bumpvert[((vertmeshgroup *)group)->numframes*numverts];
            loopk(((vertmeshgroup *)group)->numframes)
                mesh::calctangents(&bumpverts[k*numverts], &verts[k*numverts], tcverts, numverts, tris, numtris, areaweight);
        }

        void calcbb(int frame, vec &bbmin, vec &bbmax, const matrix3x4 &m)
        {
            vert *fverts = &verts[frame*numverts];
            loopj(numverts)
            {
                vec v = m.transform(fverts[j].pos);
                loopi(3)
                {
                    bbmin[i] = min(bbmin[i], v[i]);
                    bbmax[i] = max(bbmax[i], v[i]);
                }
            }
        }

        void gentris(int frame, Texture *tex, vector<BIH::tri> *out, const matrix3x4 &m)
        {
            vert *fverts = &verts[frame*numverts];
            loopj(numtris)
            {
                BIH::tri &t = out[noclip ? 1 : 0].add();
                t.tex = tex;
                t.a = m.transform(fverts[tris[j].vert[0]].pos);
                t.b = m.transform(fverts[tris[j].vert[1]].pos);
                t.c = m.transform(fverts[tris[j].vert[2]].pos);
                tcvert &av = tcverts[tris[j].vert[0]],
                       &bv = tcverts[tris[j].vert[1]],
                       &cv = tcverts[tris[j].vert[2]];
                t.tc[0] = av.u;
                t.tc[1] = av.v;
                t.tc[2] = bv.u;
                t.tc[3] = bv.v;
                t.tc[4] = cv.u;
                t.tc[5] = cv.v;
            }
        }

        static inline bool comparevert(vvertff &w, int j, tcvert &tc, vert &v)
        {
            return tc.u==w.u && tc.v==w.v && v.pos==w.pos;
        }

        static inline bool comparevert(vvert &w, int j, tcvert &tc, vert &v)
        {
            return tc.u==w.u && tc.v==w.v && v.pos==w.pos && v.norm==w.norm;
        }

        inline bool comparevert(vvertbump &w, int j, tcvert &tc, vert &v)
        {
            return tc.u==w.u && tc.v==w.v && v.pos==w.pos && v.norm==w.norm && (!bumpverts || (bumpverts[j].tangent==w.tangent && bumpverts[j].bitangent==w.bitangent));
        }

        static inline void assignvert(vvertff &vv, int j, tcvert &tc, vert &v)
        {
            vv.pos = v.pos;
            vv.u = tc.u;
            vv.v = tc.v;
        }

        static inline void assignvert(vvert &vv, int j, tcvert &tc, vert &v)
        {
            vv.pos = v.pos;
            vv.norm = v.norm;
            vv.u = tc.u;
            vv.v = tc.v;
        }

        inline void assignvert(vvertbump &vv, int j, tcvert &tc, vert &v)
        {
            vv.pos = v.pos;
            vv.norm = v.norm;
            vv.u = tc.u;
            vv.v = tc.v;
            if(bumpverts)
            {
                vv.tangent = bumpverts[j].tangent;
                vv.bitangent = bumpverts[j].bitangent;
            }
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
                    tcvert &tc = tcverts[index];
                    vert &v = verts[index];
                    int htidx = hthash(v.pos)&(htlen-1);
                    loopk(htlen)
                    {
                        int &vidx = htdata[(htidx+k)&(htlen-1)];
                        if(vidx < 0) { vidx = idxs.add(ushort(vverts.length())); assignvert(vverts.add(), index, tc, v); break; }
                        else if(comparevert(vverts[vidx], index, tc, v)) { minvert = min(minvert, idxs.add(ushort(vidx))); break; }
                    }
                }
            }
            minvert = min(minvert, ushort(voffset));
            maxvert = max(minvert, ushort(vverts.length()-1));
            elen = idxs.length()-eoffset;
            return vverts.length()-voffset;
        }

        int genvbo(vector<ushort> &idxs, int offset)
        {
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
            vdata = (uchar *)&((vvertff *)&vdata[voffset*stride])->u;
            loopi(numverts)
            {
                *(tcvert *)vdata = tcverts[i];
                vdata += stride;
            }
        }

        void interpverts(const animstate &as, bool norms, bool tangents, void * RESTRICT vdata, skin &s)
        {
            const vert * RESTRICT vert1 = &verts[as.cur.fr1 * numverts],
                       * RESTRICT vert2 = &verts[as.cur.fr2 * numverts],
                       * RESTRICT pvert1 = as.interp<1 ? &verts[as.prev.fr1 * numverts] : NULL, 
                       * RESTRICT pvert2 = as.interp<1 ? &verts[as.prev.fr2 * numverts] : NULL;
            #define ipvert(attrib)   v.attrib.lerp(vert1[i].attrib, vert2[i].attrib, as.cur.t)
            #define ipbvert(attrib)  v.attrib.lerp(bvert1[i].attrib, bvert2[i].attrib, as.cur.t)
            #define ipvertp(attrib)  v.attrib.lerp(pvert1[i].attrib, pvert2[i].attrib, as.prev.t).lerp(vec().lerp(vert1[i].attrib, vert2[i].attrib, as.cur.t), as.interp)
            #define ipbvertp(attrib) v.attrib.lerp(bpvert1[i].attrib, bpvert2[i].attrib, as.prev.t).lerp(vec().lerp(bvert1[i].attrib, bvert2[i].attrib, as.cur.t), as.interp)
            #define iploop(type, body) \
                loopi(numverts) \
                { \
                    type &v = ((type * RESTRICT)vdata)[i]; \
                    body; \
                }
            if(tangents)
            {
                const bumpvert * RESTRICT bvert1 = &bumpverts[as.cur.fr1 * numverts],
                               * RESTRICT bvert2 = &bumpverts[as.cur.fr2 * numverts],
                               * RESTRICT bpvert1 = as.interp<1 ? &bumpverts[as.prev.fr1 * numverts] : NULL, 
                               * RESTRICT bpvert2 = as.interp<1 ? &bumpverts[as.prev.fr2 * numverts] : NULL;
                if(as.interp<1) iploop(vvertbump, { ipvertp(pos); ipvertp(norm); ipbvertp(tangent); v.bitangent = bvert1[i].bitangent; })
                else iploop(vvertbump, { ipvert(pos); ipvert(norm); ipbvert(tangent); v.bitangent = bvert1[i].bitangent; })
            }
            else if(norms)
            {
                if(as.interp<1) iploop(vvert, { ipvertp(pos); ipvertp(norm); })
                else iploop(vvert, { ipvert(pos); ipvert(norm); })
            }
            else if(as.interp<1) iploop(vvertff, ipvertp(pos))
            else iploop(vvertff, ipvert(pos))
            #undef iploop
            #undef ipvert
            #undef ipbvert
            #undef ipvertp
            #undef ipbvertp
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
                            vvertff *vverts = hasVBO ? 0 : (vvertff *)vc.vdata;
                            glTexCoordPointer(2, GL_FLOAT, ((vertmeshgroup *)group)->vertsize, &vverts->u);
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
                            vvertbump *vverts = hasVBO ? 0 : (vvertbump *)vc.vdata;
                            glVertexAttribPointer_(1, 4, GL_FLOAT, GL_FALSE, ((vertmeshgroup *)group)->vertsize, &vverts->tangent.x);
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

            if(hasDRE) glDrawRangeElements_(GL_TRIANGLES, minvert, maxvert, elen, GL_UNSIGNED_SHORT, &((vertmeshgroup *)group)->edata[eoffset]);
            else glDrawElements(GL_TRIANGLES, elen, GL_UNSIGNED_SHORT, &((vertmeshgroup *)group)->edata[eoffset]);
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
        matrix3x4 transform;

        tag() : name(NULL) {}
        ~tag() { DELETEA(name); }
    };

    struct vertmeshgroup : meshgroup
    {
        int numframes;
        tag *tags;
        int numtags;

        static const int MAXVBOCACHE = 16;
        vbocacheentry vbocache[MAXVBOCACHE];

        ushort *edata;
        GLuint ebuf;
        bool vnorms, vtangents;
        int vlen, vertsize;
        uchar *vdata;

        vertmeshgroup() : numframes(0), tags(NULL), numtags(0), edata(NULL), ebuf(0), vdata(NULL) 
        {
        }

        virtual ~vertmeshgroup()
        {
            DELETEA(tags);
            if(ebuf) glDeleteBuffers_(1, &ebuf);
            loopi(MAXVBOCACHE) 
            {
                DELETEA(vbocache[i].vdata);
                if(vbocache[i].vbuf) glDeleteBuffers_(1, &vbocache[i].vbuf);
            }
            DELETEA(vdata);
        }

        int findtag(const char *name)
        {
            loopi(numtags) if(!strcmp(tags[i].name, name)) return i;
            return -1;
        }

        int totalframes() const { return numframes; }

        void concattagtransform(part *p, int frame, int i, const matrix3x4 &m, matrix3x4 &n)
        {
            n.mul(m, tags[frame*numtags + i].transform);
            n.translate(m.transformnormal(p->translate).mul(p->model->scale));
        }

        void calctagmatrix(part *p, int i, const animstate &as, glmatrixf &matrix)
        {
            const matrix3x4 &tag1 = tags[as.cur.fr1*numtags + i].transform, 
                            &tag2 = tags[as.cur.fr2*numtags + i].transform;
            matrix3x4 tag;
            tag.lerp(tag1, tag2, as.cur.t);
            if(as.interp<1)
            {
                const matrix3x4 &tag1p = tags[as.prev.fr1*numtags + i].transform, 
                                &tag2p = tags[as.prev.fr2*numtags + i].transform;
                matrix3x4 tagp;
                tagp.lerp(tag1p, tag2p, as.prev.t);
                tag.lerp(tagp, tag, as.interp);
            }
            matrix = glmatrixf(tag);
            matrix[12] = (matrix[12] + p->translate.x) * p->model->scale;
            matrix[13] = (matrix[13] + p->translate.y) * p->model->scale;
            matrix[14] = (matrix[14] + p->translate.z) * p->model->scale;
        }

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
                        loopv(meshes) ((vertmesh *)meshes[i])->filltc(vdata, vertsize); \
                    } while(0)
                if(!vc.vdata) ALLOCVDATA(vc.vdata);
                return;
            }

            vector<ushort> idxs;

            vnorms = norms;
            vtangents = tangents;
            vertsize = tangents ? sizeof(vvertbump) : (norms ? sizeof(vvert) : sizeof(vvertff));
            vlen = 0;
            if(numframes>1)
            {
                loopv(meshes) vlen += ((vertmesh *)meshes[i])->genvbo(idxs, vlen);
                DELETEA(vdata);
                if(hasVBO) ALLOCVDATA(vdata); 
                else ALLOCVDATA(vc.vdata);
            } 
            else 
            {
                if(hasVBO) glBindBuffer_(GL_ARRAY_BUFFER_ARB, vc.vbuf);
                #define GENVBO(type) \
                    do \
                    { \
                        vector<type> vverts; \
                        loopv(meshes) vlen += ((vertmesh *)meshes[i])->genvbo(idxs, vlen, vverts, htdata, htlen); \
                        if(hasVBO) glBufferData_(GL_ARRAY_BUFFER_ARB, vverts.length()*sizeof(type), vverts.getbuf(), GL_STATIC_DRAW_ARB); \
                        else \
                        { \
                            DELETEA(vc.vdata); \
                            vc.vdata = new uchar[vverts.length()*sizeof(type)]; \
                            memcpy(vc.vdata, vverts.getbuf(), vverts.length()*sizeof(type)); \
                        } \
                    } while(0)
                int numverts = 0, htlen = 128;
                loopv(meshes) numverts += ((vertmesh *)meshes[i])->numverts;
                while(htlen < numverts) htlen *= 2;
                if(numverts*4 > htlen*3) htlen *= 2; 
                int *htdata = new int[htlen];
                memset(htdata, -1, htlen*sizeof(int));
                if(tangents) GENVBO(vvertbump);
                else if(norms) GENVBO(vvert);
                else GENVBO(vvertff);
                delete[] htdata;
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
            #undef ALLOCVDATA
        }

        void bindvbo(const animstate *as, vbocacheentry &vc)
        {
            vvert *vverts = hasVBO ? 0 : (vvert *)vc.vdata;
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
            if(enablebones) disablebones();
        }

        void cleanup()
        {
            loopi(MAXVBOCACHE)
            {
                vbocacheentry &c = vbocache[i];
                if(c.vbuf) { glDeleteBuffers_(1, &c.vbuf); c.vbuf = 0; }
                DELETEA(c.vdata);
                c.as.cur.fr1 = -1;
            }
            if(hasVBO) { if(ebuf) { glDeleteBuffers_(1, &ebuf); ebuf = 0; } }
            else DELETEA(vdata);
        }

        void render(const animstate *as, float pitch, const vec &axis, const vec &forward, dynent *d, part *p)
        {
            if(as->cur.anim&ANIM_NORENDER)
            {
                loopv(p->links) calctagmatrix(p, p->links[i].tag, *as, p->links[i].matrix);
                return;
            }

            bool norms = false, tangents = false;
            loopv(p->skins) 
            {
                if(p->skins[i].normals()) norms = true;
                if(p->skins[i].tangents()) tangents = true;
            }
            if(norms!=vnorms || tangents!=vtangents) { cleanup(); disablevbo(); }
            vbocacheentry *vc = NULL;
            if(numframes<=1) vc = vbocache;
            else
            {
                loopi(MAXVBOCACHE)
                {
                    vbocacheentry &c = vbocache[i];
                    if(hasVBO ? !c.vbuf : !c.vdata) continue;
                    if(c.as==*as) { vc = &c; break; }
                }
                if(!vc) loopi(MAXVBOCACHE) { vc = &vbocache[i]; if((hasVBO ? !vc->vbuf : !vc->vdata) || vc->millis < lastmillis) break; }
            }
            if(hasVBO ? !vc->vbuf : !vc->vdata) genvbo(norms, tangents, *vc);
            if(numframes>1)
            {
                if(vc->as!=*as)
                {
                    vc->as = *as;
                    vc->millis = lastmillis;
                    loopv(meshes) 
                    {
                        vertmesh &m = *(vertmesh *)meshes[i];
                        m.interpverts(*as, norms, tangents, (hasVBO ? vdata : vc->vdata) + m.voffset*vertsize, p->skins[i]);
                    }
                    if(hasVBO)
                    {
                        glBindBuffer_(GL_ARRAY_BUFFER_ARB, vc->vbuf);
                        glBufferData_(GL_ARRAY_BUFFER_ARB, vlen*vertsize, vdata, GL_STREAM_DRAW_ARB);    
                    }
                }
                vc->millis = lastmillis;
            }
        
            bindvbo(as, *vc);
            loopv(meshes)
            {
                vertmesh *m = (vertmesh *)meshes[i];
                p->skins[i].bind(m, as);
                m->render(as, p->skins[i], *vc);
            }
            
            loopv(p->links) calctagmatrix(p, p->links[i].tag, *as, p->links[i].matrix);
        }
    };

    vertmodel(const char *name) : animmodel(name)
    {
    }
};

template<class MDL> struct vertloader : modelloader<MDL>
{
};

template<class MDL> struct vertcommands : modelcommands<MDL, struct MDL::vertmesh>
{
    typedef struct MDL::part part;
    typedef struct MDL::skin skin;

    static void loadpart(char *model, float *smooth)
    {
        if(!MDL::loading) { conoutf("not loading an %s", MDL::formatname()); return; }
        defformatstring(filename)("%s/%s", MDL::dir, model);
        part &mdl = *new part;
        MDL::loading->parts.add(&mdl);
        mdl.model = MDL::loading;
        mdl.index = MDL::loading->parts.length()-1;
        if(mdl.index) mdl.pitchscale = mdl.pitchoffset = mdl.pitchmin = mdl.pitchmax = 0;
        mdl.meshes = MDL::loading->sharemeshes(path(filename), double(*smooth > 0 ? cos(clamp(*smooth, 0.0f, 180.0f)*RAD) : 2));
        if(!mdl.meshes) conoutf("could not load %s", filename);
        else mdl.initskins();
    }
    
    static void setpitch(float *pitchscale, float *pitchoffset, float *pitchmin, float *pitchmax)
    {
        if(!MDL::loading || MDL::loading->parts.empty()) { conoutf("not loading an %s", MDL::formatname()); return; }
        part &mdl = *MDL::loading->parts.last();
    
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

    static void setanim(char *anim, int *frame, int *range, float *speed, int *priority)
    {
        if(!MDL::loading || MDL::loading->parts.empty()) { conoutf("not loading an %s", MDL::formatname()); return; }
        vector<int> anims;
        findanims(anim, anims);
        if(anims.empty()) conoutf("could not find animation %s", anim);
        else loopv(anims)
        {
            MDL::loading->parts.last()->setanim(0, anims[i], *frame, *range, *speed, *priority);
        }
    }

    vertcommands()
    {
        if(MDL::multiparted()) this->modelcommand(loadpart, "load", "sf"); 
        this->modelcommand(setpitch, "pitch", "ffff");
        if(MDL::animated()) this->modelcommand(setanim, "anim", "siiff");
    }
};

