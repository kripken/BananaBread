struct obj;

struct obj : vertmodel, vertloader<obj>
{
    obj(const char *name) : vertmodel(name) {}

    static const char *formatname() { return "obj"; }
    static bool animated() { return false; }
    int type() const { return MDL_OBJ; }

    struct objmeshgroup : vertmeshgroup
    {
        void parsevert(char *s, vector<vec> &out)
        {
            vec &v = out.add(vec(0, 0, 0));
            while(isalpha(*s)) s++;
            loopi(3)
            {
                v[i] = strtod(s, &s);
                while(isspace(*s)) s++;
                if(!*s) break;
            }
        }

        bool load(char *filename, float smooth)
        {
            int len = strlen(filename);
            if(len < 4 || strcasecmp(&filename[len-4], ".obj")) return false;

            stream *file = openfile(filename, "rb");
            if(!file) return false;

            name = newstring(filename);

            numframes = 1;

            vector<vec> attrib[3];
            char buf[512];

            hashtable<ivec, int> verthash;
            vector<vert> verts;
            vector<tcvert> tcverts;
            vector<tri> tris;

            #define STARTMESH do { \
                vertmesh &m = *new vertmesh; \
                m.group = this; \
                m.name = meshname[0] ? newstring(meshname) : NULL; \
                meshes.add(&m); \
                curmesh = &m; \
                verthash.clear(); \
                verts.setsize(0); \
                tcverts.setsize(0); \
                tris.setsize(0); \
            } while(0)

            #define FLUSHMESH do { \
                curmesh->numverts = verts.length(); \
                if(verts.length()) \
                { \
                    curmesh->verts = new vert[verts.length()]; \
                    memcpy(curmesh->verts, verts.getbuf(), verts.length()*sizeof(vert)); \
                    curmesh->tcverts = new tcvert[verts.length()]; \
                    memcpy(curmesh->tcverts, tcverts.getbuf(), tcverts.length()*sizeof(tcvert)); \
                } \
                curmesh->numtris = tris.length(); \
                if(tris.length()) \
                { \
                    curmesh->tris = new tri[tris.length()]; \
                    memcpy(curmesh->tris, tris.getbuf(), tris.length()*sizeof(tri)); \
                } \
                if(attrib[2].empty()) \
                { \
                    if(smooth <= 1) curmesh->smoothnorms(smooth); \
                    else curmesh->buildnorms(); \
                } \
            } while(0)

            string meshname = "";
            vertmesh *curmesh = NULL;
            while(file->getline(buf, sizeof(buf)))
            {
                char *c = buf;
                while(isspace(*c)) c++;
                switch(*c)
                {
                    case '#': continue;
                    case 'v':
                        if(isspace(c[1])) parsevert(c, attrib[0]);
                        else if(c[1]=='t') parsevert(c, attrib[1]);
                        else if(c[1]=='n') parsevert(c, attrib[2]);
                        break;
                    case 'g':
                    {
                        while(isalpha(*c)) c++;
                        while(isspace(*c)) c++;
                        char *name = c;
                        size_t namelen = strlen(name);
                        while(namelen > 0 && isspace(name[namelen-1])) namelen--;
                        copystring(meshname, name, min(namelen+1, sizeof(meshname)));

                        if(curmesh) FLUSHMESH;
                        curmesh = NULL;
                        break;
                    }
                    case 'f':
                    {
                        if(!curmesh) STARTMESH;
                        int v0 = -1, v1 = -1;
                        while(isalpha(*c)) c++;
                        for(;;)
                        {
                            while(isspace(*c)) c++;
                            if(!*c) break; 
                            ivec vkey(-1, -1, -1);
                            loopi(3)
                            {
                                vkey[i] = strtol(c, &c, 10);
                                if(vkey[i] < 0) vkey[i] = attrib[i].length() + vkey[i];
                                else vkey[i]--;
                                if(!attrib[i].inrange(vkey[i])) vkey[i] = -1;
                                if(*c!='/') break;
                                c++;
                            }
                            int *index = verthash.access(vkey);
                            if(!index)
                            {
                                index = &verthash[vkey];
                                *index = verts.length();
                                vert &v = verts.add();
                                v.pos = vkey.x < 0 ? vec(0, 0, 0) : attrib[0][vkey.x];
                                v.pos = vec(v.pos.z, -v.pos.x, v.pos.y);
                                v.norm = vkey.z < 0 ? vec(0, 0, 0) : attrib[2][vkey.z];
                                v.norm = vec(v.norm.z, -v.norm.x, v.norm.y);
                                tcvert &tcv = tcverts.add();
                                if(vkey.y < 0) tcv.u = tcv.v = 0;
                                else { tcv.u = attrib[1][vkey.y].x; tcv.v = 1-attrib[1][vkey.y].y; }
                            }
                            if(v0 < 0) v0 = *index;
                            else if(v1 < 0) v1 = *index;
                            else
                            {
                                tri &t = tris.add();
                                t.vert[0] = ushort(*index);
                                t.vert[1] = ushort(v1);
                                t.vert[2] = ushort(v0);
                                v1 = *index;
                            }
                        }
                        break;
                    }
                }
            }

            if(curmesh) FLUSHMESH;

            delete file;

            return true;
        }
    };

    meshgroup *loadmeshes(char *name, va_list args)
    {
        objmeshgroup *group = new objmeshgroup;
        if(!group->load(name, va_arg(args, double))) { delete group; return NULL; }
        return group;
    }

    bool loaddefaultparts()
    {
        part &mdl = *new part;
        parts.add(&mdl);
        mdl.model = this;
        mdl.index = 0;
        const char *pname = parentdir(loadname);
        defformatstring(name1)("packages/models/%s/tris.obj", loadname);
        mdl.meshes = sharemeshes(path(name1), 2.0);
        if(!mdl.meshes)
        {
            defformatstring(name2)("packages/models/%s/tris.obj", pname);    // try obj in parent folder (vert sharing)
            mdl.meshes = sharemeshes(path(name2), 2.0);
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
        defformatstring(cfgname)("packages/models/%s/obj.cfg", loadname);

        loading = this;
        identflags &= ~IDF_PERSIST;
        if(execfile(cfgname, false) && parts.length()) // configured obj, will call the obj* commands below
        {
            identflags |= IDF_PERSIST;
            loading = NULL;
            loopv(parts) if(!parts[i]->meshes) return false;
        }
        else // obj without configuration, try default tris and skin
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

vertcommands<obj> objcommands;

