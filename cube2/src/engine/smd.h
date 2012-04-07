struct smd;

struct smdbone
{
    string name;
    int parent;
    smdbone() : parent(-1) { name[0] = '\0'; }
};

struct smd : skelmodel, skelloader<smd>
{
    smd(const char *name) : skelmodel(name) {}

    static const char *formatname() { return "smd"; }
    int type() const { return MDL_SMD; }

    struct smdmesh : skelmesh
    {
    };

    struct smdmeshgroup : skelmeshgroup
    {
        smdmeshgroup() 
        {
        }

        bool skipcomment(char *&curbuf)
        {
            while(*curbuf && isspace(*curbuf)) curbuf++;
            switch(*curbuf)
            {
                case '#':
                case ';':
                case '\r':
                case '\n':
                case '\0':
                    return true;
                case '/':
                    if(curbuf[1] == '/') return true;
                    break;
            }
            return false;
        }

        void skipsection(stream *f, char *buf, size_t bufsize)
        {
            while(f->getline(buf, bufsize))
            {
                char *curbuf = buf;
                if(skipcomment(curbuf)) continue;
                if(!strncmp(curbuf, "end", 3)) break;
            }
        }

        void readname(char *&curbuf, char *name, size_t namesize)
        {
            char *curname = name;
            while(*curbuf && isspace(*curbuf)) curbuf++;
            bool allowspace = false;
            if(*curbuf == '"') { curbuf++; allowspace = true; }
            while(*curbuf)
            {
                char c = *curbuf++;
                if(c == '"') break;      
                if(isspace(c) && !allowspace) break;
                if(curname < &name[namesize-1]) *curname++ = c;
            } 
            *curname = '\0';
        }

        void readnodes(stream *f, char *buf, size_t bufsize, vector<smdbone> &bones)
        {
            while(f->getline(buf, bufsize))
            {
                char *curbuf = buf;
                if(skipcomment(curbuf)) continue;
                if(!strncmp(curbuf, "end", 3)) break;
                int id = strtol(curbuf, &curbuf, 10);
                string name;
                readname(curbuf, name, sizeof(name));
                int parent = strtol(curbuf, &curbuf, 10);
                if(id < 0 || id > 255 || parent > 255 || !name[0]) continue; 
                while(!bones.inrange(id)) bones.add();
                smdbone &bone = bones[id];
                copystring(bone.name, name);
                bone.parent = parent;
            }
        }

        void readmaterial(char *&curbuf, char *name, size_t namesize)
        {
            char *curname = name;
            while(*curbuf && isspace(*curbuf)) curbuf++;
            while(*curbuf)
            {
                char c = *curbuf++;
                if(isspace(c)) break;
                if(c == '.')
                {
                    while(*curbuf && !isspace(*curbuf)) curbuf++;
                    break;
                }
                if(curname < &name[namesize-1]) *curname++ = c;
            }
            *curname = '\0';
        }

        struct smdmeshdata
        {
            smdmesh *mesh;
            vector<vert> verts;
            vector<tri> tris;

            void finalize()
            {
                if(verts.empty() || tris.empty()) return;
                vert *mverts = new vert[mesh->numverts + verts.length()];
                if(mesh->numverts) 
                {
                    memcpy(mverts, mesh->verts, mesh->numverts*sizeof(vert));
                    delete[] mesh->verts;
                }
                memcpy(&mverts[mesh->numverts], verts.getbuf(), verts.length()*sizeof(vert));
                mesh->numverts += verts.length();
                mesh->verts = mverts;
                tri *mtris = new tri[mesh->numtris + tris.length()];
                if(mesh->numtris) 
                {
                    memcpy(mtris, mesh->tris, mesh->numtris*sizeof(tri));
                    delete[] mesh->tris;
                }
                memcpy(&mtris[mesh->numtris], tris.getbuf(), tris.length()*sizeof(tri));
                mesh->numtris += tris.length();
                mesh->tris = mtris;
            }
        };

        struct smdvertkey : vert
        {
            smdmeshdata *mesh;
            
            smdvertkey(smdmeshdata *mesh) : mesh(mesh) {}
        };
     
        void readtriangles(stream *f, char *buf, size_t bufsize)
        {
            smdmeshdata *curmesh = NULL;
            hashtable<const char *, smdmeshdata> materials(1<<6);
            hashset<int> verts(1<<12); 
            while(f->getline(buf, bufsize))
            {
                char *curbuf = buf;
                if(skipcomment(curbuf)) continue;
                if(!strncmp(curbuf, "end", 3)) break;
                string material;
                readmaterial(curbuf, material, sizeof(material)); 
                if(!curmesh || strcmp(curmesh->mesh->name, material))
                {
                    curmesh = materials.access(material);
                    if(!curmesh)
                    {
                        smdmesh *m = new smdmesh;
                        m->group = this;
                        m->name = newstring(material);
                        meshes.add(m);
                        curmesh = &materials[m->name];
                        curmesh->mesh = m;
                    }
                }
                tri curtri;
                loopi(3)                        
                {
                    char *curbuf;
                    do
                    {
                        if(!f->getline(buf, bufsize)) goto endsection;
                        curbuf = buf;
                    } while(skipcomment(curbuf));
                    smdvertkey key(curmesh);     
                    int parent = -1, numlinks = 0, len = 0;
                    if(sscanf(curbuf, " %d %f %f %f %f %f %f %f %f %d%n", &parent, &key.pos.x, &key.pos.y, &key.pos.z, &key.norm.x, &key.norm.y, &key.norm.z, &key.u, &key.v, &numlinks, &len) < 9) goto endsection;    
                    curbuf += len;
                    key.pos.y = -key.pos.y;
                    key.norm.y = -key.norm.y;
                    key.v = 1 - key.v;
                    blendcombo c;
                    int sorted = 0;
                    float pweight = 0, tweight = 0;
                    for(; numlinks > 0; numlinks--)
                    {
                        int bone = -1, len = 0;
                        float weight = 0;
                        if(sscanf(curbuf, " %d %f%n", &bone, &weight, &len) < 2) break;
                        curbuf += len;
                        tweight += weight;
                        if(bone == parent) pweight += weight;                       
                        else sorted = c.addweight(sorted, weight, bone);
                    }
                    if(tweight < 1) pweight += 1 - tweight;
                    if(pweight > 0) sorted = c.addweight(sorted, pweight, parent);
                    c.finalize(sorted);
                    key.blend = curmesh->mesh->addblendcombo(c);
                    int index = verts.access(key, curmesh->verts.length());
                    if(index == curmesh->verts.length()) curmesh->verts.add(key);
                    curtri.vert[2-i] = index;
                }
                curmesh->tris.add(curtri);
            }
        endsection:
            enumerate(materials, smdmeshdata, data, data.finalize());
        }

        void readskeleton(stream *f, char *buf, size_t bufsize)
        {
            int frame = -1;
            while(f->getline(buf, bufsize))
            {
                char *curbuf = buf;
                if(skipcomment(curbuf)) continue;
                if(sscanf(curbuf, " time %d", &frame) == 1) continue;
                else if(!strncmp(curbuf, "end", 3)) break;
                else if(frame != 0) continue;
                int bone;
                vec pos, rot;
                if(sscanf(curbuf, " %d %f %f %f %f %f %f", &bone, &pos.x, &pos.y, &pos.z, &rot.x, &rot.y, &rot.z) != 7)
                    continue;
                if(bone < 0 || bone >= skel->numbones)
                    continue;
                rot.x = -rot.x;
                rot.z = -rot.z;
                float cx = cosf(rot.x/2), sx = sinf(rot.x/2),
                      cy = cosf(rot.y/2), sy = sinf(rot.y/2),
                      cz = cosf(rot.z/2), sz = sinf(rot.z/2);
                pos.y = -pos.y;
                dualquat dq(quat(sx*cy*cz - cx*sy*sz,
                                 cx*sy*cz + sx*cy*sz,
                                 cx*cy*sz - sx*sy*cz,
                                 cx*cy*cz + sx*sy*sz),
                            pos);
                boneinfo &b = skel->bones[bone];
                if(b.parent < 0) b.base = dq;
                else b.base.mul(skel->bones[b.parent].base, dq);
                (b.invbase = b.base).invert();
            }
        }

        bool loadmesh(const char *filename)
        {
            stream *f = openfile(filename, "r");
            if(!f) return false;
            
            char buf[512];
            int version = -1;
            while(f->getline(buf, sizeof(buf)))
            {
                char *curbuf = buf;
                if(skipcomment(curbuf)) continue;
                if(sscanf(curbuf, " version %d", &version) == 1)
                {
                    if(version != 1) { delete f; return false; }
                }
                else if(!strncmp(curbuf, "nodes", 5))
                {
                    if(skel->numbones > 0) { skipsection(f, buf, sizeof(buf)); continue; }
                    vector<smdbone> bones;
                    readnodes(f, buf, sizeof(buf), bones); 
                    if(bones.empty()) continue;
                    skel->numbones = bones.length();
                    skel->bones = new boneinfo[skel->numbones];
                    loopv(bones)
                    {
                        boneinfo &dst = skel->bones[i];
                        smdbone &src = bones[i];
                        dst.name = newstring(src.name);
                        dst.parent = src.parent;
                    }
                    skel->linkchildren();
                }
                else if(!strncmp(curbuf, "triangles", 9))
                    readtriangles(f, buf, sizeof(buf));
                else if(!strncmp(curbuf, "skeleton", 8))
                {
                    if(skel->shared > 1) skipsection(f, buf, sizeof(buf));
                    else readskeleton(f, buf, sizeof(buf));
                }
                else if(!strncmp(curbuf, "vertexanimation", 15))
                    skipsection(f, buf, sizeof(buf));
            }

            sortblendcombos();

            delete f;
            return true;
        }

        int readframes(stream *f, char *buf, size_t bufsize, vector<dualquat> &animbones)
        {
            int frame = -1, numframes = 0, lastbone = skel->numbones;
            while(f->getline(buf, bufsize))
            {
                char *curbuf = buf;
                if(skipcomment(curbuf)) continue;
                int nextframe = -1;
                if(sscanf(curbuf, " time %d", &nextframe) == 1)
                {
                    for(; lastbone < skel->numbones; lastbone++) animbones[frame*skel->numbones + lastbone] = animbones[lastbone];
                    if(nextframe >= numframes)
                    {
                        databuf<dualquat> framebones = animbones.reserve(skel->numbones * (nextframe + 1 - numframes));
                        loopi(nextframe - numframes) framebones.put(animbones.getbuf(), skel->numbones);
                        animbones.addbuf(framebones);
                        animbones.advance(skel->numbones);
                        numframes = nextframe + 1;
                    }
                    frame = nextframe;
                    lastbone = 0;
                    continue;
                }
                else if(!strncmp(curbuf, "end", 3)) break;
                int bone;
                vec pos, rot;
                if(sscanf(curbuf, " %d %f %f %f %f %f %f", &bone, &pos.x, &pos.y, &pos.z, &rot.x, &rot.y, &rot.z) != 7)
                    continue;
                if(bone < 0 || bone >= skel->numbones)
                    continue;
                for(; lastbone < bone; lastbone++) animbones[frame*skel->numbones + lastbone] = animbones[lastbone];
                lastbone++;
                float cx = cosf(rot.x/2), sx = sinf(rot.x/2),
                      cy = cosf(rot.y/2), sy = sinf(rot.y/2),
                      cz = cosf(rot.z/2), sz = sinf(rot.z/2);
                pos.y = -pos.y;
                dualquat dq(quat(-(sx*cy*cz - cx*sy*sz),
                                 cx*sy*cz + sx*cy*sz,
                                 -(cx*cy*sz - sx*sy*cz),
                                 cx*cy*cz + sx*sy*sz),
                            pos);
                if(adjustments.inrange(bone)) adjustments[bone].adjust(dq);
                dq.mul(skel->bones[bone].invbase);
                dualquat &dst = animbones[frame*skel->numbones + bone];
                if(skel->bones[bone].parent < 0) dst = dq;
                else dst.mul(skel->bones[skel->bones[bone].parent].base, dq);
                dst.fixantipodal(skel->numframes > 0 ? skel->framebones[bone] : animbones[bone]);
            }
            for(; lastbone < skel->numbones; lastbone++) animbones[frame*skel->numbones + lastbone] = animbones[lastbone];
            return numframes;
        }

        skelanimspec *loadanim(const char *filename)
        {
            skelanimspec *sa = skel->findskelanim(filename);
            if(sa || skel->numbones <= 0) return sa;

            stream *f = openfile(filename, "r");
            if(!f) return NULL;

            char buf[512];
            int version = -1;
            vector<dualquat> animbones;
            while(f->getline(buf, sizeof(buf)))
            {
                char *curbuf = buf;
                if(skipcomment(curbuf)) continue;
                if(sscanf(curbuf, " version %d", &version) == 1)
                {
                    if(version != 1) { delete f; return NULL; }
                }
                else if(!strncmp(curbuf, "nodes", 5))
                {
                    vector<smdbone> bones;
                    readnodes(f, buf, sizeof(buf), bones);
                    if(bones.length() != skel->numbones) { delete f; return NULL; }
                }
                else if(!strncmp(curbuf, "triangles", 9))
                    skipsection(f, buf, sizeof(buf));
                else if(!strncmp(curbuf, "skeleton", 8))
                    readframes(f, buf, sizeof(buf), animbones);
                else if(!strncmp(curbuf, "vertexanimation", 15))
                    skipsection(f, buf, sizeof(buf));
            }
            int numframes = animbones.length() / skel->numbones;
            dualquat *framebones = new dualquat[(skel->numframes+numframes)*skel->numbones];             
            if(skel->framebones)
            {
                memcpy(framebones, skel->framebones, skel->numframes*skel->numbones*sizeof(dualquat));
                delete[] skel->framebones;
            }
            memcpy(&framebones[skel->numframes*skel->numbones], animbones.getbuf(), numframes*skel->numbones*sizeof(dualquat));
            skel->framebones = framebones;
            sa = &skel->addskelanim(filename);
            sa->frame = skel->numframes;
            sa->range = numframes;
            skel->numframes += numframes;

            delete f;

            return sa;
        }

        bool load(const char *meshfile)
        {
            name = newstring(meshfile);

            if(!loadmesh(meshfile)) return false;
            
            return true;
        }
    };            

    meshgroup *loadmeshes(char *name, va_list args)
    {
        smdmeshgroup *group = new smdmeshgroup;
        group->shareskeleton(va_arg(args, char *));
        if(!group->load(name)) { delete group; return NULL; }
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
        defformatstring(meshname)("packages/models/%s/%s.smd", loadname, fname);
        mdl.meshes = sharemeshes(path(meshname), NULL);
        if(!mdl.meshes) return false;
        mdl.initanimparts();
        mdl.initskins();
        return true;
    }

    bool load()
    {
        if(loaded) return true;
        formatstring(dir)("packages/models/%s", loadname);
        defformatstring(cfgname)("packages/models/%s/smd.cfg", loadname);

        loading = this;
        identflags &= ~IDF_PERSIST;
        if(execfile(cfgname, false) && parts.length()) // configured smd, will call the smd* commands below
        {
            identflags |= IDF_PERSIST;
            loading = NULL;
            loopv(parts) if(!parts[i]->meshes) return false;
        }
        else // smd without configuration, try default tris and skin 
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

static inline uint hthash(const smd::smdmeshgroup::smdvertkey &k)
{
    return hthash(k.pos);
}

static inline bool htcmp(const smd::smdmeshgroup::smdvertkey &k, int index)
{
    if(!k.mesh->verts.inrange(index)) return false;
    const smd::vert &v = k.mesh->verts[index];
    return k.pos == v.pos && k.norm == v.norm && k.u == v.u && k.v == v.v && k.blend == v.blend;
}

skelcommands<smd> smdcommands;

