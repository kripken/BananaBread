struct md3;

struct md3frame
{
    vec bbmin, bbmax, origin;
    float radius;
    uchar name[16];
};

struct md3tag
{
    char name[64];
    vec pos;
    float rotation[3][3];
};

struct md3vertex
{
    short vertex[3];
    short normal;
};

struct md3triangle
{
    int vertexindices[3];
};

struct md3header
{
    char id[4];
    int version;
    char name[64];
    int flags;
    int numframes, numtags, nummeshes, numskins;
    int ofs_frames, ofs_tags, ofs_meshes, ofs_eof; // offsets
};

struct md3meshheader
{
    char id[4];
    char name[64];
    int flags;
    int numframes, numshaders, numvertices, numtriangles;
    int ofs_triangles, ofs_shaders, ofs_uv, ofs_vertices, meshsize; // offsets
};

struct md3 : vertmodel, vertloader<md3>
{
    md3(const char *name) : vertmodel(name) {}

    static const char *formatname() { return "md3"; }
    int type() const { return MDL_MD3; }

    struct md3meshgroup : vertmeshgroup
    {
        bool load(char *path)
        {
            stream *f = openfile(path, "rb");
            if(!f) return false;
            md3header header;
            f->read(&header, sizeof(md3header));
            lilswap(&header.version, 1);
            lilswap(&header.flags, 9);
            if(strncmp(header.id, "IDP3", 4) != 0 || header.version != 15) // header check
            { 
                delete f;
                conoutf("md3: corrupted header"); 
                return false; 
            }

            name = newstring(path);

            numframes = header.numframes;

            int mesh_offset = header.ofs_meshes;
            loopi(header.nummeshes)
            {
                vertmesh &m = *new vertmesh;
                m.group = this;
                meshes.add(&m);

                md3meshheader mheader;
                f->seek(mesh_offset, SEEK_SET);
                f->read(&mheader, sizeof(md3meshheader));
                lilswap(&mheader.flags, 10); 

                m.name = newstring(mheader.name);
               
                m.numtris = mheader.numtriangles; 
                m.tris = new tri[m.numtris];
                f->seek(mesh_offset + mheader.ofs_triangles, SEEK_SET);
                loopj(m.numtris)
                {
                    md3triangle tri;
                    f->read(&tri, sizeof(md3triangle)); // read the triangles
                    lilswap(tri.vertexindices, 3);
                    loopk(3) m.tris[j].vert[k] = (ushort)tri.vertexindices[k];
                }

                m.numverts = mheader.numvertices;
                m.tcverts = new tcvert[m.numverts];
                f->seek(mesh_offset + mheader.ofs_uv , SEEK_SET); 
                f->read(m.tcverts, m.numverts*2*sizeof(float)); // read the UV data
                lilswap(&m.tcverts[0].u, 2*m.numverts);
                
                m.verts = new vert[numframes*m.numverts];
                f->seek(mesh_offset + mheader.ofs_vertices, SEEK_SET); 
                loopj(numframes*m.numverts)
                {
                    md3vertex v;
                    f->read(&v, sizeof(md3vertex)); // read the vertices
                    lilswap(v.vertex, 4);

                    m.verts[j].pos.x = v.vertex[0]/64.0f;
                    m.verts[j].pos.y = -v.vertex[1]/64.0f;
                    m.verts[j].pos.z = v.vertex[2]/64.0f;

                    float lng = (v.normal&0xFF)*PI2/255.0f; // decode vertex normals
                    float lat = ((v.normal>>8)&0xFF)*PI2/255.0f;
                    m.verts[j].norm.x = cosf(lat)*sinf(lng);
                    m.verts[j].norm.y = -sinf(lat)*sinf(lng);
                    m.verts[j].norm.z = cosf(lng);
                }

                mesh_offset += mheader.meshsize;
            }

            numtags = header.numtags;
            if(numtags)
            {
                tags = new tag[numframes*numtags];
                f->seek(header.ofs_tags, SEEK_SET);
                md3tag tag;

                loopi(header.numframes*header.numtags)
                {
                    f->read(&tag, sizeof(md3tag));
                    lilswap(&tag.pos.x, 12);
                    if(tag.name[0] && i<header.numtags) tags[i].name = newstring(tag.name);
                    matrix3x4 &m = tags[i].transform;
                    tag.pos.y *= -1;
                    // undo the -y
                    loopj(3) tag.rotation[1][j] *= -1;
                    // then restore it
                    loopj(3) tag.rotation[j][1] *= -1;
                    m.a.w = tag.pos.x;
                    m.b.w = tag.pos.y;
                    m.c.w = tag.pos.z;
                    loopj(3)
                    {
                        m.a[j] = tag.rotation[j][0];
                        m.b[j] = tag.rotation[j][1];
                        m.c[j] = tag.rotation[j][2];
                    }
#if 0
                    tags[i].pos = vec(tag.pos.x, -tag.pos.y, tag.pos.z);
                    memcpy(tags[i].transform, tag.rotation, sizeof(tag.rotation));
                    // undo the -y
                    loopj(3) tags[i].transform[1][j] *= -1;
                    // then restore it
                    loopj(3) tags[i].transform[j][1] *= -1;
#endif
                }
            }

            delete f;
            return true;
        }
    };
    
    meshgroup *loadmeshes(char *name, va_list args)
    {
        md3meshgroup *group = new md3meshgroup;
        if(!group->load(name)) { delete group; return NULL; }
        return group;
    }

    bool loaddefaultparts()
    {
        const char *pname = parentdir(loadname);
        part &mdl = *new part;
        parts.add(&mdl);
        mdl.model = this;
        mdl.index = 0;
        defformatstring(name1)("packages/models/%s/tris.md3", loadname);
        mdl.meshes = sharemeshes(path(name1));
        if(!mdl.meshes)
        {
            defformatstring(name2)("packages/models/%s/tris.md3", pname);    // try md3 in parent folder (vert sharing)
            mdl.meshes = sharemeshes(path(name2));
            if(!mdl.meshes) return false;
        }
        Texture *tex, *masks;
        loadskin(loadname, pname, tex, masks);
        mdl.initskins(tex, masks);
        if(tex==notexture) conoutf("could not load model skin for %s", name1);
        return true;
    }

    bool load()
    {
        if(loaded) return true;
        formatstring(dir)("packages/models/%s", loadname);
        defformatstring(cfgname)("packages/models/%s/md3.cfg", loadname);

        loading = this;
        identflags &= ~IDF_PERSIST;
        if(execfile(cfgname, false) && parts.length()) // configured md3, will call the md3* commands below
        {
            identflags |= IDF_PERSIST;
            loading = NULL;
            loopv(parts) if(!parts[i]->meshes) return false;
        }
        else // md3 without configuration, try default tris and skin
        {
            identflags |= IDF_PERSIST;
            loading = NULL;
            if(!loaddefaultparts()) return false;
        }
        scale /= 4;
        translate.y = -translate.y;
        parts[0]->translate = translate;
        loopv(parts) parts[i]->meshes->shared++;
        preloadshaders();
        return loaded = true;
    }
};

vertcommands<md3> md3commands;

