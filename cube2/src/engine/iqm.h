struct iqm;

struct iqmheader
{
    char magic[16];
    uint version;
    uint filesize;
    uint flags;
    uint num_text, ofs_text;
    uint num_meshes, ofs_meshes;
    uint num_vertexarrays, num_vertexes, ofs_vertexarrays;
    uint num_triangles, ofs_triangles, ofs_adjacency;
    uint num_joints, ofs_joints;
    uint num_poses, ofs_poses;
    uint num_anims, ofs_anims;
    uint num_frames, num_framechannels, ofs_frames, ofs_bounds;
    uint num_comment, ofs_comment;
    uint num_extensions, ofs_extensions;
};

struct iqmmesh
{
    uint name;
    uint material;
    uint first_vertex, num_vertexes;
    uint first_triangle, num_triangles;
};

enum
{        
    IQM_POSITION     = 0,
    IQM_TEXCOORD     = 1,
    IQM_NORMAL       = 2,
    IQM_TANGENT      = 3,
    IQM_BLENDINDEXES = 4,
    IQM_BLENDWEIGHTS = 5,
    IQM_COLOR        = 6,
    IQM_CUSTOM       = 0x10
};  

enum
{
    IQM_BYTE   = 0,
    IQM_UBYTE  = 1,
    IQM_SHORT  = 2,
    IQM_USHORT = 3,
    IQM_INT    = 4,
    IQM_UINT   = 5,
    IQM_HALF   = 6,
    IQM_FLOAT  = 7,
    IQM_DOUBLE = 8,
};

struct iqmtriangle
{
    uint vertex[3];
};

struct iqmjoint
{
    uint name;
    int parent;
    vec pos; 
    quat orient; 
    vec size;
};

struct iqmpose
{
    int parent;
    uint mask;
    vec offsetpos;
    vec4 offsetorient;
    vec offsetsize;
    vec scalepos;
    vec4 scaleorient;
    vec scalesize;
};

struct iqmanim
{
    uint name;
    uint first_frame, num_frames;
    float framerate;
    uint flags;
};

struct iqmvertexarray
{
    uint type;
    uint flags;
    uint format;
    uint size;
    uint offset;
};

struct iqm : skelmodel, skelloader<iqm>
{
    iqm(const char *name) : skelmodel(name) {}

    static const char *formatname() { return "iqm"; }
    int type() const { return MDL_IQM; }

    struct iqmmeshgroup : skelmeshgroup
    {
        iqmmeshgroup() 
        {
        }

        bool loadiqmmeshes(const char *filename, const iqmheader &hdr, uchar *buf)
        {
            lilswap((uint *)&buf[hdr.ofs_vertexarrays], hdr.num_vertexarrays*sizeof(iqmvertexarray)/sizeof(uint));
            lilswap((uint *)&buf[hdr.ofs_triangles], hdr.num_triangles*sizeof(iqmtriangle)/sizeof(uint));
            lilswap((uint *)&buf[hdr.ofs_meshes], hdr.num_meshes*sizeof(iqmmesh)/sizeof(uint));
            lilswap((uint *)&buf[hdr.ofs_joints], hdr.num_joints*sizeof(iqmjoint)/sizeof(uint));

            const char *str = hdr.ofs_text ? (char *)&buf[hdr.ofs_text] : "";
            float *vpos = NULL, *vnorm = NULL, *vtan = NULL, *vtc = NULL;
            uchar *vindex = NULL, *vweight = NULL;
            iqmvertexarray *vas = (iqmvertexarray *)&buf[hdr.ofs_vertexarrays];
            loopi(hdr.num_vertexarrays)
            {
                iqmvertexarray &va = vas[i];
                switch(va.type)
                {
                    case IQM_POSITION: if(va.format != IQM_FLOAT || va.size != 3) return false; vpos = (float *)&buf[va.offset]; lilswap(vpos, 3*hdr.num_vertexes); break;
                    case IQM_NORMAL: if(va.format != IQM_FLOAT || va.size != 3) return false; vnorm = (float *)&buf[va.offset]; lilswap(vnorm, 3*hdr.num_vertexes); break;
                    case IQM_TANGENT: if(va.format != IQM_FLOAT || va.size != 4) return false; vtan = (float *)&buf[va.offset]; lilswap(vtan, 4*hdr.num_vertexes); break;
                    case IQM_TEXCOORD: if(va.format != IQM_FLOAT || va.size != 2) return false; vtc = (float *)&buf[va.offset]; lilswap(vtc, 2*hdr.num_vertexes); break;
                    case IQM_BLENDINDEXES: if(va.format != IQM_UBYTE || va.size != 4) return false; vindex = (uchar *)&buf[va.offset]; break;
                    case IQM_BLENDWEIGHTS: if(va.format != IQM_UBYTE || va.size != 4) return false; vweight = (uchar *)&buf[va.offset]; break;
                }
            }
            iqmtriangle *tris = (iqmtriangle *)&buf[hdr.ofs_triangles];
            iqmmesh *imeshes = (iqmmesh *)&buf[hdr.ofs_meshes];
            iqmjoint *joints = (iqmjoint *)&buf[hdr.ofs_joints];

            if(hdr.num_joints)
            {
                if(skel->numbones <= 0)
                {
                    skel->numbones = hdr.num_joints;
                    skel->bones = new boneinfo[skel->numbones]; 
                    loopi(hdr.num_joints)
                    {
                        iqmjoint &j = joints[i]; 
                        boneinfo &b = skel->bones[i];
                        if(!b.name) b.name = newstring(&str[j.name]);
                        b.parent = j.parent;
                        if(skel->shared <= 1)
                        {
                            j.pos.y = -j.pos.y;
                            j.orient.x = -j.orient.x;
                            j.orient.z = -j.orient.z;
                            j.orient.normalize();
                            b.base = dualquat(j.orient, j.pos);      
                            if(b.parent >= 0) b.base.mul(skel->bones[b.parent].base, dualquat(b.base));
                            (b.invbase = b.base).invert();
                        }
                    }
                }

                if(skel->shared <= 1)
                    skel->linkchildren();
            }

            loopi(hdr.num_meshes)
            {
                iqmmesh &im = imeshes[i];
                skelmesh *m = new skelmesh;
                m->group = this;   
                meshes.add(m);
                m->name = newstring(&str[im.name]);
                m->numverts = im.num_vertexes;
                if(m->numverts) 
                {
                    m->verts = new vert[m->numverts];
                    if(vtan) m->bumpverts = new bumpvert[m->numverts];
                }
                loopj(im.num_vertexes)
                {
                    int fj = j + im.first_vertex;
                    vert &v = m->verts[j];
                    loopk(3) v.pos[k] = vpos[3*fj + k];    
                    v.pos.y = -v.pos.y;
                    v.u = vtc[2*fj + 0];
                    v.v = vtc[2*fj + 1];
                    if(vnorm) 
                    {
                        loopk(3) v.norm[k] = vnorm[3*fj + k];
                        v.norm.y = -v.norm.y;
                        if(vtan)
                        {
                            bumpvert &bv = m->bumpverts[j];
                            loopk(3) bv.tangent[k] = vtan[4*fj + k];
                            bv.tangent.y = -bv.tangent.y;
                            bv.bitangent = vtan[4*fj + 3];
                        }
                    } 
                    blendcombo c;
                    int sorted = 0;
                    if(vindex && vweight) loopk(4) sorted = c.addweight(sorted, vweight[4*fj + k], vindex[4*fj + k]);
                    c.finalize(sorted);
                    v.blend = m->addblendcombo(c);
                }
                m->numtris = im.num_triangles;
                if(m->numtris) m->tris = new tri[m->numtris]; 
                loopj(im.num_triangles)
                {
                    int fj = j + im.first_triangle;
                    loopk(3) m->tris[j].vert[k] = tris[fj].vertex[k] - im.first_vertex;
                }
                if(!m->numtris || !m->numverts)
                {
                    conoutf("empty mesh in %s", filename);
                    meshes.removeobj(m);
                    delete m;
                }
            }

            sortblendcombos();
                
            return true;
        }

        bool loadiqmanims(const char *filename, const iqmheader &hdr, uchar *buf)
        {
            lilswap((uint *)&buf[hdr.ofs_poses], hdr.num_poses*sizeof(iqmpose)/sizeof(uint));
            lilswap((uint *)&buf[hdr.ofs_anims], hdr.num_anims*sizeof(iqmanim)/sizeof(uint));
            lilswap((ushort *)&buf[hdr.ofs_frames], hdr.num_frames*hdr.num_framechannels);

            const char *str = hdr.ofs_text ? (char *)&buf[hdr.ofs_text] : "";
            iqmpose *poses = (iqmpose *)&buf[hdr.ofs_poses];
            iqmanim *anims = (iqmanim *)&buf[hdr.ofs_anims];
            ushort *frames = (ushort *)&buf[hdr.ofs_frames];
            loopi(hdr.num_anims)
            {
                iqmanim &a = anims[i];
                string name;
                copystring(name, filename);
                concatstring(name, ":");
                concatstring(name, &str[a.name]);
                skelanimspec *sa = skel->findskelanim(name);
                if(sa) continue;
                sa = &skel->addskelanim(name);
                sa->frame = skel->numframes;
                sa->range = a.num_frames;
                dualquat *animbones = new dualquat[(skel->numframes+a.num_frames)*skel->numbones];
                if(skel->bones)
                {
                    memcpy(animbones, skel->framebones, skel->numframes*skel->numbones*sizeof(dualquat));
                    delete[] skel->framebones;
                }
                skel->framebones = animbones;
                animbones += skel->numframes*skel->numbones;
                skel->numframes += a.num_frames;
                ushort *animdata = &frames[a.first_frame*hdr.num_framechannels];
                loopj(a.num_frames)
                {
                    dualquat *frame = &animbones[j*skel->numbones];
                    loopk(skel->numbones)
                    {
                        iqmpose &p = poses[k];
                        vec pos;
                        quat orient;
                        pos.x = p.offsetpos.x; if(p.mask&0x01) pos.x += *animdata++ * p.scalepos.x;
                        pos.y = -p.offsetpos.y; if(p.mask&0x02) pos.y -= *animdata++ * p.scalepos.y;
                        pos.z = p.offsetpos.z; if(p.mask&0x04) pos.z += *animdata++ * p.scalepos.z;
                        orient.x = -p.offsetorient.x; if(p.mask&0x08) orient.x -= *animdata++ * p.scaleorient.x;
                        orient.y = p.offsetorient.y; if(p.mask&0x10) orient.y += *animdata++ * p.scaleorient.y;
                        orient.z = -p.offsetorient.z; if(p.mask&0x20) orient.z -= *animdata++ * p.scaleorient.z;
                        orient.w = p.offsetorient.w; if(p.mask&0x40) orient.w += *animdata++ * p.scaleorient.w;
                        orient.normalize();
                        if(p.mask&0x380)
                        {
                            if(p.mask&0x80) animdata++;
                            if(p.mask&0x100) animdata++;
                            if(p.mask&0x200) animdata++;
                        }
                        frame[k] = dualquat(orient, pos);
                        if(adjustments.inrange(k)) adjustments[k].adjust(frame[k]);
                        boneinfo &b = skel->bones[k];
                        frame[k].mul(b.invbase);
                        if(b.parent >= 0) frame[k].mul(skel->bones[b.parent].base, dualquat(frame[k]));
                        frame[k].fixantipodal(skel->framebones[k]);
                    }
                } 
            }
     
            return true;
        }

        bool loadiqm(const char *filename, bool doloadmesh, bool doloadanim)
        {
            stream *f = openfile(filename, "rb");
            if(!f) return false;

            uchar *buf = NULL;
            iqmheader hdr;
            if(f->read(&hdr, sizeof(hdr)) != sizeof(hdr) || memcmp(hdr.magic, "INTERQUAKEMODEL", sizeof(hdr.magic))) goto error;
            lilswap(&hdr.version, (sizeof(hdr) - sizeof(hdr.magic))/sizeof(uint));
            if(hdr.version != 2) goto error;
            if(hdr.filesize > (16<<20)) goto error; // sanity check... don't load files bigger than 16 MB
            buf = new uchar[hdr.filesize];
            if(f->read(buf + sizeof(hdr), hdr.filesize - sizeof(hdr)) != int(hdr.filesize - sizeof(hdr))) goto error;

            if(doloadmesh && !loadiqmmeshes(filename, hdr, buf)) goto error;
            if(doloadanim && !loadiqmanims(filename, hdr, buf)) goto error;

            delete[] buf;
            delete f;
            return true;

        error:
            if(buf) delete[] buf;
            delete f;
            return false;
        }

        bool loadmesh(const char *filename)
        {
            name = newstring(filename);

            return loadiqm(filename, true, false);
        }

        skelanimspec *loadanim(const char *animname)
        {
            const char *sep = strchr(animname, ':');
            skelanimspec *sa = skel->findskelanim(animname, sep ? '\0' : ':');
            if(!sa)
            {
                string filename;
                copystring(filename, animname);
                if(sep) filename[sep - animname] = '\0';
                if(loadiqm(filename, false, true))
                    sa = skel->findskelanim(animname, sep ? '\0' : ':');
            }
            return sa;
        }
    };            

    meshgroup *loadmeshes(char *name, va_list args)
    {
        iqmmeshgroup *group = new iqmmeshgroup;
        group->shareskeleton(va_arg(args, char *));
        if(!group->loadmesh(name)) { delete group; return NULL; }
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
        defformatstring(meshname)("packages/models/%s/%s.iqm", loadname, fname);
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
        defformatstring(cfgname)("packages/models/%s/iqm.cfg", loadname);

        loading = this;
        identflags &= ~IDF_PERSIST;
        if(execfile(cfgname, false) && parts.length()) // configured iqm, will call the iqm* commands below
        {
            identflags |= IDF_PERSIST;
            loading = NULL;
            loopv(parts) if(!parts[i]->meshes) return false;
        }
        else // iqm without configuration, try default tris and skin 
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

skelcommands<iqm> iqmcommands;

