struct md5;

struct md5joint
{
    vec pos;
    quat orient;
};

struct md5weight
{
    int joint;
    float bias;
    vec pos;
};  

struct md5vert
{
    float u, v;
    ushort start, count;
};

struct md5hierarchy
{
    string name;
    int parent, flags, start;
};

struct md5 : skelmodel, skelloader<md5>
{
    md5(const char *name) : skelmodel(name) {}

    static const char *formatname() { return "md5"; }
    int type() const { return MDL_MD5; }

    struct md5mesh : skelmesh
    {
        md5weight *weightinfo;
        int numweights;
        md5vert *vertinfo;

        md5mesh() : weightinfo(NULL), numweights(0), vertinfo(NULL)
        {
        }

        ~md5mesh()
        {
            cleanup();
        }

        void cleanup()
        {
            DELETEA(weightinfo);
            DELETEA(vertinfo);
        }

        void buildverts(vector<md5joint> &joints)
        {
            loopi(numverts)
            {
                md5vert &v = vertinfo[i];
                vec pos(0, 0, 0);
                loopk(v.count)
                {
                    md5weight &w = weightinfo[v.start+k];
                    md5joint &j = joints[w.joint];
                    pos.add(j.orient.rotate(w.pos).add(j.pos).mul(w.bias));
                }
                vert &vv = verts[i];
                vv.pos = pos;
                vv.u = v.u;
                vv.v = v.v;

                blendcombo c;
                int sorted = 0;
                loopj(v.count)
                {
                    md5weight &w = weightinfo[v.start+j];
                    sorted = c.addweight(sorted, w.bias, w.joint); 
                }
                c.finalize(sorted);
                vv.blend = addblendcombo(c);
            }
        }

        void load(stream *f, char *buf, size_t bufsize)
        {
            md5weight w;
            md5vert v;
            tri t;
            int index;

            while(f->getline(buf, bufsize) && buf[0]!='}')
            {
                if(strstr(buf, "// meshes:"))
                {
                    char *start = strchr(buf, ':')+1;
                    if(*start==' ') start++; 
                    char *end = start + strlen(start)-1;
                    while(end >= start && isspace(*end)) end--;
                    name = newstring(start, end+1-start);
                }
                else if(strstr(buf, "shader"))
                {
                    char *start = strchr(buf, '"'), *end = start ? strchr(start+1, '"') : NULL;
                    if(start && end) 
                    {
                        char *texname = newstring(start+1, end-(start+1));
                        part *p = loading->parts.last();
                        p->initskins(notexture, notexture, group->meshes.length());
                        skin &s = p->skins.last();
                        s.tex = textureload(makerelpath(dir, texname), 0, true, false);
                        delete[] texname;
                    }
                }
                else if(sscanf(buf, " numverts %d", &numverts)==1)
                {
                    numverts = max(numverts, 0);        
                    if(numverts)
                    {
                        vertinfo = new md5vert[numverts];
                        verts = new vert[numverts];
                    }
                }
                else if(sscanf(buf, " numtris %d", &numtris)==1)
                {
                    numtris = max(numtris, 0);
                    if(numtris) tris = new tri[numtris];
                }
                else if(sscanf(buf, " numweights %d", &numweights)==1)
                {
                    numweights = max(numweights, 0);
                    if(numweights) weightinfo = new md5weight[numweights];
                }
                else if(sscanf(buf, " vert %d ( %f %f ) %hu %hu", &index, &v.u, &v.v, &v.start, &v.count)==5)
                {
                    if(index>=0 && index<numverts) vertinfo[index] = v;
                }
                else if(sscanf(buf, " tri %d %hu %hu %hu", &index, &t.vert[0], &t.vert[1], &t.vert[2])==4)
                {
                    if(index>=0 && index<numtris) tris[index] = t;
                }
                else if(sscanf(buf, " weight %d %d %f ( %f %f %f ) ", &index, &w.joint, &w.bias, &w.pos.x, &w.pos.y, &w.pos.z)==6)
                {
                    w.pos.y = -w.pos.y;
                    if(index>=0 && index<numweights) weightinfo[index] = w;
                }
            }
        }
    };

    struct md5meshgroup : skelmeshgroup
    {
        md5meshgroup() 
        {
        }

        bool loadmesh(const char *filename, float smooth)
        {
            stream *f = openfile(filename, "r");
            if(!f) return false;

            char buf[512];
            vector<md5joint> basejoints;
            while(f->getline(buf, sizeof(buf)))
            {
                int tmp;
                if(sscanf(buf, " MD5Version %d", &tmp)==1)
                {
                    if(tmp!=10) { delete f; return false; }
                }
                else if(sscanf(buf, " numJoints %d", &tmp)==1)
                {
                    if(tmp<1) { delete f; return false; }
                    if(skel->numbones>0) continue;
                    skel->numbones = tmp;
                    skel->bones = new boneinfo[skel->numbones];
                }
                else if(sscanf(buf, " numMeshes %d", &tmp)==1)
                {
                    if(tmp<1) { delete f; return false; }
                }
                else if(strstr(buf, "joints {"))
                {
                    string name;
                    int parent;
                    md5joint j;
                    while(f->getline(buf, sizeof(buf)) && buf[0]!='}')
                    {
                        char *curbuf = buf, *curname = name;
                        bool allowspace = false;
                        while(*curbuf && isspace(*curbuf)) curbuf++;
                        if(*curbuf == '"') { curbuf++; allowspace = true; }
                        while(*curbuf && curname < &name[sizeof(name)-1])
                        {
                            char c = *curbuf++;
                            if(c == '"') break; 
                            if(isspace(c) && !allowspace) break;
                            *curname++ = c;
                        } 
                        *curname = '\0'; 
                        if(sscanf(curbuf, " %d ( %f %f %f ) ( %f %f %f )",
                            &parent, &j.pos.x, &j.pos.y, &j.pos.z,
                            &j.orient.x, &j.orient.y, &j.orient.z)==7)
                        {
                            j.pos.y = -j.pos.y;
                            j.orient.x = -j.orient.x;
                            j.orient.z = -j.orient.z;
                            if(basejoints.length()<skel->numbones) 
                            {
                                if(!skel->bones[basejoints.length()].name) 
                                    skel->bones[basejoints.length()].name = newstring(name);
                                skel->bones[basejoints.length()].parent = parent;
                            }
                            j.orient.restorew();
                            basejoints.add(j);
                        }
                    }
                    if(basejoints.length()!=skel->numbones) { delete f; return false; }
                }
                else if(strstr(buf, "mesh {"))
                {
                    md5mesh *m = new md5mesh;
                    m->group = this;
                    meshes.add(m);
                    m->load(f, buf, sizeof(buf));
                    if(!m->numtris || !m->numverts)
                    {
                        conoutf("empty mesh in %s", filename);
                        meshes.removeobj(m);
                        delete m;
                    }
                }
            }
        
            if(skel->shared <= 1) 
            {
                skel->linkchildren();
                loopv(basejoints) 
                {
                    boneinfo &b = skel->bones[i];
                    b.base = dualquat(basejoints[i].orient, basejoints[i].pos);
                    (b.invbase = b.base).invert();
                }
            }

            loopv(meshes)
            {
                md5mesh &m = *(md5mesh *)meshes[i];
                m.buildverts(basejoints);
                if(smooth <= 1) m.smoothnorms(smooth);
                else m.buildnorms();
                m.cleanup();
            }
            
            sortblendcombos();

            delete f;
            return true;
        }

        skelanimspec *loadanim(const char *filename)
        {
            skelanimspec *sa = skel->findskelanim(filename);
            if(sa) return sa;

            stream *f = openfile(filename, "r");
            if(!f) return NULL;

            vector<md5hierarchy> hierarchy;
            vector<md5joint> basejoints;
            int animdatalen = 0, animframes = 0;
            float *animdata = NULL;
            dualquat *animbones = NULL;
            char buf[512];
            while(f->getline(buf, sizeof(buf)))
            {
                int tmp;
                if(sscanf(buf, " MD5Version %d", &tmp)==1)
                {
                    if(tmp!=10) { delete f; return NULL; }
                }
                else if(sscanf(buf, " numJoints %d", &tmp)==1)
                {
                    if(tmp!=skel->numbones) { delete f; return NULL; }
                }
                else if(sscanf(buf, " numFrames %d", &animframes)==1)
                {
                    if(animframes<1) { delete f; return NULL; }
                }
                else if(sscanf(buf, " frameRate %d", &tmp)==1);
                else if(sscanf(buf, " numAnimatedComponents %d", &animdatalen)==1)
                {
                    if(animdatalen>0) animdata = new float[animdatalen];
                }
                else if(strstr(buf, "bounds {"))
                {
                    while(f->getline(buf, sizeof(buf)) && buf[0]!='}');
                }
                else if(strstr(buf, "hierarchy {"))
                {
                    while(f->getline(buf, sizeof(buf)) && buf[0]!='}')
                    {
                        md5hierarchy h;
                        if(sscanf(buf, " %s %d %d %d", h.name, &h.parent, &h.flags, &h.start)==4)
                            hierarchy.add(h);
                    }
                }
                else if(strstr(buf, "baseframe {"))
                {
                    while(f->getline(buf, sizeof(buf)) && buf[0]!='}')
                    {
                        md5joint j;
                        if(sscanf(buf, " ( %f %f %f ) ( %f %f %f )", &j.pos.x, &j.pos.y, &j.pos.z, &j.orient.x, &j.orient.y, &j.orient.z)==6)
                        {
                            j.pos.y = -j.pos.y;
                            j.orient.x = -j.orient.x;
                            j.orient.z = -j.orient.z;
                            j.orient.restorew();
                            basejoints.add(j);
                        }
                    }
                    if(basejoints.length()!=skel->numbones) { delete f; return NULL; }
                    animbones = new dualquat[(skel->numframes+animframes)*skel->numbones];
                    if(skel->framebones)
                    {
                        memcpy(animbones, skel->framebones, skel->numframes*skel->numbones*sizeof(dualquat));
                        delete[] skel->framebones;
                    }
                    skel->framebones = animbones;
                    animbones += skel->numframes*skel->numbones;

                    sa = &skel->addskelanim(filename);
                    sa->frame = skel->numframes;
                    sa->range = animframes;

                    skel->numframes += animframes;
                }
                else if(sscanf(buf, " frame %d", &tmp)==1)
                {
                    for(int numdata = 0; f->getline(buf, sizeof(buf)) && buf[0]!='}';)
                    {
                        for(char *src = buf, *next = src; numdata < animdatalen; numdata++, src = next)
                        {
                            animdata[numdata] = strtod(src, &next);
                            if(next <= src) break;
                        }
                    }
                    dualquat *frame = &animbones[tmp*skel->numbones];
                    loopv(basejoints)
                    {
                        md5hierarchy &h = hierarchy[i];
                        md5joint j = basejoints[i];
                        if(h.start < animdatalen && h.flags)
                        {
                            float *jdata = &animdata[h.start];
                            if(h.flags&1) j.pos.x = *jdata++;
                            if(h.flags&2) j.pos.y = -*jdata++;
                            if(h.flags&4) j.pos.z = *jdata++;
                            if(h.flags&8) j.orient.x = -*jdata++;
                            if(h.flags&16) j.orient.y = *jdata++;
                            if(h.flags&32) j.orient.z = -*jdata++;
                            j.orient.restorew();
                        }
                        frame[i] = dualquat(j.orient, j.pos);
                        if(adjustments.inrange(i)) adjustments[i].adjust(frame[i]);
                        frame[i].mul(skel->bones[i].invbase);
                        if(h.parent >= 0) frame[i].mul(skel->bones[h.parent].base, dualquat(frame[i]));
                        frame[i].fixantipodal(skel->framebones[i]);
                    }
                }    
            }

            DELETEA(animdata);
            delete f;

            return sa;
        }

        bool load(const char *meshfile, float smooth)
        {
            name = newstring(meshfile);

            if(!loadmesh(meshfile, smooth)) return false;
            
            return true;
        }
    };            

    meshgroup *loadmeshes(char *name, va_list args)
    {
        md5meshgroup *group = new md5meshgroup;
        group->shareskeleton(va_arg(args, char *));
        if(!group->load(name, va_arg(args, double))) { delete group; return NULL; }
        return group;
    }

    bool loaddefaultparts()
    {
        skelpart &mdl = *new skelpart;
        parts.add(&mdl);
        mdl.model = this;
        mdl.index = 0;
        mdl.pitchscale = mdl.pitchoffset = mdl.pitchmin = mdl.pitchmax = 0;
        adjustments.setsize(0);
        const char *fname = loadname + strlen(loadname);
        do --fname; while(fname >= loadname && *fname!='/' && *fname!='\\');
        fname++;
        defformatstring(meshname)("packages/models/%s/%s.md5mesh", loadname, fname);
        mdl.meshes = sharemeshes(path(meshname), NULL, 2.0);
        if(!mdl.meshes) return false;
        mdl.initanimparts();
        mdl.initskins();
        defformatstring(animname)("packages/models/%s/%s.md5anim", loadname, fname);
        ((md5meshgroup *)mdl.meshes)->loadanim(path(animname));
        return true;
    }

    bool load()
    {
        if(loaded) return true;
        formatstring(dir)("packages/models/%s", loadname);
        defformatstring(cfgname)("packages/models/%s/md5.cfg", loadname);

        loading = this;
        identflags &= ~IDF_PERSIST;
        if(execfile(cfgname, false) && parts.length()) // configured md5, will call the md5* commands below
        {
            identflags |= IDF_PERSIST;
            loading = NULL;
            loopv(parts) if(!parts[i]->meshes) return false;
        }
        else // md5 without configuration, try default tris and skin 
        {
            identflags |= IDF_PERSIST;
            if(!loaddefaultparts()) 
            {
                loading = NULL;
                return false;
            }
            loading = NULL;
        }
        scale /= 4;
        parts[0]->translate = translate;
        loopv(parts) 
        {
            skelpart *p = (skelpart *)parts[i];
            p->endanimparts();
            p->meshes->shared++;
        }
        preloadshaders();
        return loaded = true;
    }
};

skelcommands<md5> md5commands;

