// shader.cpp: OpenGL assembly/GLSL shader management

#include "engine.h"

struct GlobalShaderParamState : ShaderParamState
{
    uint version;

    GlobalShaderParamState() : version(0) {}
};

Shader *Shader::lastshader = NULL;

Shader *defaultshader = NULL, *rectshader = NULL, *cubemapshader = NULL, *notextureshader = NULL, *nocolorshader = NULL, *foggedshader = NULL, *foggednotextureshader = NULL, *stdworldshader = NULL, *lineshader = NULL, *foggedlineshader = NULL;

static hashtable<const char *, Shader> shaders;
static Shader *curshader = NULL;
static vector<ShaderParam> curparams;
static GlobalShaderParamState vertexparamstate[RESERVEDSHADERPARAMS + MAXSHADERPARAMS], pixelparamstate[RESERVEDSHADERPARAMS + MAXSHADERPARAMS];
static bool dirtyenvparams = false, standardshader = false, initshaders = false, forceshaders = true;
static uint paramversion = 0;

VAR(reservevpparams, 1, 16, 0);
VAR(maxvpenvparams, 1, 0, 0);
VAR(maxvplocalparams, 1, 0, 0);
VAR(maxfpenvparams, 1, 0, 0);
VAR(maxfplocalparams, 1, 0, 0);
VAR(maxtexcoords, 1, 0, 0);
VAR(maxvsuniforms, 1, 0, 0);
VAR(maxfsuniforms, 1, 0, 0);
VAR(maxvaryings, 1, 0, 0);
VAR(dbgshader, 0, 0, 2);

// XXX EMSCRIPTEN: split up shader loading, it takes a while
static void loadshaders2(void *); 
static void loadshaders3(void *);
static char *loadshaders_glsl;
static int loadshaders_glsl_len;
static const int loadshaders_glsl_chunks = 12;
static char *loadshaders_glsl_curr;

void loadshaders(bool firstload)
{
    if(renderpath==R_ASMSHADER || renderpath==R_ASMGLSLANG)
    {
        GLint val;
        glGetProgramiv_(GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_ENV_PARAMETERS_ARB, &val);
        maxvpenvparams = val; 
        glGetProgramiv_(GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB, &val);
        maxvplocalparams = val;
        glGetProgramiv_(GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_ENV_PARAMETERS_ARB, &val);
        maxfpenvparams = val;
        glGetProgramiv_(GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB, &val);
        maxfplocalparams = val;
    }
    if(renderpath==R_GLSLANG || renderpath==R_ASMGLSLANG)
    {
        GLint val;
        glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB, &val);
        maxvsuniforms = val/4;
        glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB, &val);
        maxfsuniforms = val/4;
        glGetIntegerv(GL_MAX_VARYING_FLOATS_ARB, &val);
        maxvaryings = val;
    }
    if(renderpath != R_FIXEDFUNCTION)
    {
        GLint val;
        glGetIntegerv(GL_MAX_TEXTURE_COORDS_ARB, &val);
        maxtexcoords = val;
    }

    initshaders = true;
    standardshader = true;

    // XXX EMSCRIPTEN: load glsl.cfg to split it up
    loadshaders_glsl = loadfile(renderpath==R_GLSLANG ? "data/glsl.cfg" : "data/stdshader.cfg", &loadshaders_glsl_len);
    loadshaders_glsl_curr = loadshaders_glsl;

    if (firstload)
    {
        loopi(loadshaders_glsl_chunks) emscripten_push_main_loop_blocker(loadshaders2, NULL);
        emscripten_push_main_loop_blocker(loadshaders3, NULL);
    } else {
        loopi(loadshaders_glsl_chunks) loadshaders2(NULL);
        loadshaders3(NULL);
    }
}

void loadshaders2(void *)
{
    if (!loadshaders_glsl_curr) return;

    int chunk_size = loadshaders_glsl_len / loadshaders_glsl_chunks;

    // Find next split point
    char *next = NULL;
    if (loadshaders_glsl_curr + int(chunk_size*1.25) < loadshaders_glsl + loadshaders_glsl_len) { // avoid making a final tiny chunk
      next = strstr(loadshaders_glsl_curr + chunk_size, "\n]\n");
      if (next) {
        next += 2;
        *next = 0; // we can put a 0 on the second \n
        next++; // beginning of next chunk
      }
    }

    execute(loadshaders_glsl_curr);
    loadshaders_glsl_curr = next;
}

void loadshaders3(void *)
{
    free(loadshaders_glsl);

    standardshader = false;
    initshaders = false;
    defaultshader = lookupshaderbyname("default");
    stdworldshader = lookupshaderbyname("stdworld");
    if(!defaultshader || !stdworldshader) fatal("cannot find shader definitions");

    extern Slot dummyslot;
    dummyslot.shader = stdworldshader;

    extern int ati_line_bug;
    rectshader = lookupshaderbyname("rect");
    cubemapshader = lookupshaderbyname("cubemap");
    notextureshader = lookupshaderbyname("notexture");
    nocolorshader = lookupshaderbyname("nocolor");
    foggedshader = lookupshaderbyname("fogged");
    foggednotextureshader = lookupshaderbyname("foggednotexture");
    lineshader = lookupshaderbyname(ati_line_bug && renderpath == R_ASMGLSLANG ? "notextureglsl" : "notexture");
    foggedlineshader = lookupshaderbyname(ati_line_bug && renderpath == R_ASMGLSLANG ? "foggednotextureglsl" : "foggednotexture");
    
    if(renderpath==R_ASMSHADER || renderpath==R_ASMGLSLANG)
    {
        glEnable(GL_VERTEX_PROGRAM_ARB);
        glEnable(GL_FRAGMENT_PROGRAM_ARB);
    }
    
    defaultshader->set();
}

Shader *lookupshaderbyname(const char *name) 
{ 
    Shader *s = shaders.access(name);
    return s && s->detailshader ? s : NULL;
}

static bool compileasmshader(GLenum type, GLuint &idx, const char *def, const char *tname, const char *name, bool msg = true, bool nativeonly = false)
{
    glGenPrograms_(1, &idx);
    glBindProgram_(type, idx);
    def += strspn(def, " \t\r\n");
    glProgramString_(type, GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei)strlen(def), def);
    GLint err = -1, native = 1;
    glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &err);
    extern int apple_vp_bug;
    if(type!=GL_VERTEX_PROGRAM_ARB || !apple_vp_bug)
        glGetProgramiv_(type, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &native);
    if(msg && err!=-1)
    {
        conoutf(CON_ERROR, "COMPILE ERROR (%s:%s) - %s", tname, name, glGetString(GL_PROGRAM_ERROR_STRING_ARB));
        if(err>=0 && err<(int)strlen(def))
        {
            loopi(err) putchar(*def++);
            puts(" <<HERE>> ");
            while(*def) putchar(*def++);
        }
    }
    else if(msg && !native) conoutf(CON_ERROR, "%s:%s EXCEEDED NATIVE LIMITS", tname, name);
    glBindProgram_(type, 0);
    if(err!=-1 || (!native && nativeonly))
    {
        glDeletePrograms_(1, &idx);
        idx = 0;
    }
    return native!=0;
}

static void showglslinfo(GLhandleARB obj, const char *tname, const char *name, const char *source)
{
    GLint length = 0;
    glGetObjectParameteriv_(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length);
    if(length > 1)
    {
        GLcharARB *log = new GLcharARB[length];
        glGetInfoLog_(obj, length, &length, log);
        conoutf(CON_ERROR, "GLSL ERROR (%s:%s)", tname, name);
        puts(log);
        if(source) loopi(1000)
        {
            const char *next = strchr(source, '\n');
            printf("%d: ", i+1);
            fwrite(source, 1, next ? next - source + 1 : strlen(source), stdout); 
            if(!next) { putchar('\n'); break; }
            source = next + 1;
        } 
        delete[] log;
    }
}

static void compileglslshader(GLenum type, GLhandleARB &obj, const char *def, const char *tname, const char *name, bool msg = true) 
{
    const GLcharARB *source = (const GLcharARB*)(def + strspn(def, " \t\r\n")); 
    obj = glCreateShaderObject_(type);
    glShaderSource_(obj, 1, &source, NULL);
    glCompileShader_(obj);
    GLint success;
    glGetObjectParameteriv_(obj, GL_OBJECT_COMPILE_STATUS_ARB, &success);
    if(!success) 
    {
        if(msg) showglslinfo(obj, tname, name, source);
        glDeleteObject_(obj);
        obj = 0;
    }
    else if(dbgshader > 1 && msg) showglslinfo(obj, tname, name, source);
}  

VAR(dbgubo, 0, 0, 1);

static void bindglsluniform(Shader &s, UniformLoc &u)
{
    u.loc = glGetUniformLocation_(s.program, u.name);
    if(!u.blockname) return;
    if(hasUBO)
    {
        GLuint bidx = glGetUniformBlockIndex_(s.program, u.blockname);
        GLuint uidx = GL_INVALID_INDEX;
        glGetUniformIndices_(s.program, 1, &u.name, &uidx);
        if(bidx != GL_INVALID_INDEX && uidx != GL_INVALID_INDEX)
        {
            GLint sizeval = 0, offsetval = 0, strideval = 0;
            glGetActiveUniformBlockiv_(s.program, bidx, GL_UNIFORM_BLOCK_DATA_SIZE, &sizeval);
            if(sizeval <= 0) return;
            glGetActiveUniformsiv_(s.program, 1, &uidx, GL_UNIFORM_OFFSET, &offsetval);
            if(u.stride > 0)
            {
                glGetActiveUniformsiv_(s.program, 1, &uidx, GL_UNIFORM_ARRAY_STRIDE, &strideval);
                if(strideval > u.stride) return;
            }
            u.offset = offsetval;
            u.size = sizeval;
            glUniformBlockBinding_(s.program, bidx, u.binding);
            if(dbgubo) conoutf(CON_DEBUG, "UBO: %s:%s:%d, offset: %d, size: %d, stride: %d", u.name, u.blockname, u.binding, offsetval, sizeval, strideval);
        }
    }
    else if(hasBUE)
    {
        GLint size = glGetUniformBufferSize_(s.program, u.loc), stride = 0;
        if(size <= 0) return;
        if(u.stride > 0)
        {
            defformatstring(elem1name)("%s[1]", u.name);
            GLint elem1loc = glGetUniformLocation_(s.program, elem1name);
            if(elem1loc == -1) return;
            GLintptr elem0off = glGetUniformOffset_(s.program, u.loc),
                     elem1off = glGetUniformOffset_(s.program, elem1loc);
            stride = elem1off - elem0off;
            if(stride > u.stride) return;
        }
        u.offset = 0;
        u.size = size;
        if(dbgubo) conoutf(CON_DEBUG, "BUE: %s:%s:%d, offset: %d, size: %d, stride: %d", u.name, u.blockname, u.binding, 0, size, stride);
    }
}

static void linkglslprogram(Shader &s, bool msg = true)
{
    s.program = s.vsobj && s.psobj ? glCreateProgramObject_() : 0;
    GLint success = 0;
    if(s.program)
    {
        glAttachObject_(s.program, s.vsobj);
        glAttachObject_(s.program, s.psobj);
        loopv(s.attriblocs)
        {
            AttribLoc &a = s.attriblocs[i];
            glBindAttribLocation_(s.program, a.loc, a.name);
        }
        glLinkProgram_(s.program);
        glGetObjectParameteriv_(s.program, GL_OBJECT_LINK_STATUS_ARB, &success);
    }
    if(success)
    {
        glUseProgramObject_(s.program);
        loopi(8)
        {
            defformatstring(arg)("tex%d", i);
            GLint loc = glGetUniformLocation_(s.program, arg);
            if(loc != -1) glUniform1i_(loc, i);
        }
        loopv(s.defaultparams)
        {
            ShaderParam &param = s.defaultparams[i];
            string pname;
            if(param.type==SHPARAM_UNIFORM) copystring(pname, param.name);
            else formatstring(pname)("%s%d", param.type==SHPARAM_VERTEX ? "v" : "p", param.index);
            param.loc = glGetUniformLocation_(s.program, pname);
        }
        loopv(s.uniformlocs) bindglsluniform(s, s.uniformlocs[i]);
        glUseProgramObject_(0);
    }
    else if(s.program)
    {
        if(msg) showglslinfo(s.program, "PROG", s.name, NULL);
        glDeleteObject_(s.program);
        s.program = 0;
    }
}

bool checkglslsupport()
{
#if 0
    /* check if GLSL profile supports loops
     */
    const GLcharARB *source = 
        "uniform int N;\n"
        "uniform vec4 delta;\n"
        "void main(void) {\n"
        "   vec4 test = vec4(0.0, 0.0, 0.0, 0.0);\n"
        "   for(int i = 0; i < N; i++)  test += delta;\n"
        "   gl_FragColor = test;\n"
        "}\n";
#else
    const GLcharARB *source =
        "void main(void) {\n"
        "   gl_FragColor = vec4(0.0);\n"
        "}\n";
#endif
    GLhandleARB obj = glCreateShaderObject_(GL_FRAGMENT_SHADER_ARB);
    if(!obj) return false;
    glShaderSource_(obj, 1, &source, NULL);
    glCompileShader_(obj);
    GLint success;
    glGetObjectParameteriv_(obj, GL_OBJECT_COMPILE_STATUS_ARB, &success);
    if(!success)
    {
        conoutf(CON_WARN, "Cannot compile fragment shader");
        glDeleteObject_(obj);
        return false;
    }
    // EMSCRIPTEN: WebGL needs both vertex *and* fragment shaders
    const GLcharARB *source2 =
      "attribute vec4 vPosition;    \n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = vPosition;  \n"
      "}                            \n";
    GLhandleARB obj2 = glCreateShaderObject_(GL_VERTEX_SHADER_ARB);
    if(!obj2) return false;
    glShaderSource_(obj2, 1, &source2, NULL);
    glCompileShader_(obj2);
    glGetObjectParameteriv_(obj2, GL_OBJECT_COMPILE_STATUS_ARB, &success);
    if(!success)
    {
        conoutf(CON_WARN, "Cannot compile vertex shader");
        glDeleteObject_(obj);
        glDeleteObject_(obj2);
        return false;
    }
    // EMSCRIPTEN end (also lines dealing with obj2 below)
    GLhandleARB program = glCreateProgramObject_();
    if(!program)
    {
        conoutf(CON_WARN, "Cannot create program");
        glDeleteObject_(obj);
        glDeleteObject_(obj2);
        return false;
    } 
    glAttachObject_(program, obj);
    glAttachObject_(program, obj2);
    glLinkProgram_(program); 
    glGetObjectParameteriv_(program, GL_OBJECT_LINK_STATUS_ARB, &success);
    if (!success) conoutf(CON_WARN, "Cannot link program");

    glDeleteObject_(obj);
    glDeleteObject_(program);
    return success!=0;
}

#define ALLOCEXTPARAM 0xFF
#define UNUSEDEXTPARAM 0xFE
            
static int addextparam(Shader &s, const char *name, int type, int index, int loc)
{
    if(!(s.numextparams%4))
    {
        LocalShaderParamState *extparams = new LocalShaderParamState[s.numextparams+4];
        if(s.extparams)
        {
            memcpy(extparams, s.extparams, s.numextparams*sizeof(LocalShaderParamState));
            delete[] s.extparams;
        }
        s.extparams = extparams;
    }
    int extindex = s.numextparams;
    LocalShaderParamState &ext = s.extparams[extindex];
    ext.name = name;
    ext.type = type;
    ext.index = index;
    ext.loc = loc;
    s.numextparams++;
    return extindex;
}

static void allocglsluniformparam(Shader &s, int type, int index, bool local = false)
{
    ShaderParamState &val = (type==SHPARAM_VERTEX ? vertexparamstate[index] : pixelparamstate[index]);
    int loc = val.name ? glGetUniformLocation_(s.program, val.name) : -1;
    if(loc == -1)
    {
        defformatstring(altname)("%s%d", type==SHPARAM_VERTEX ? "v" : "p", index);
        loc = glGetUniformLocation_(s.program, altname);
    }
    if(loc >= 0) loopi(s.numextparams)
    {
        LocalShaderParamState &ext = s.extparams[i];
        if(ext.loc != loc) continue;
        if(ext.type==SHPARAM_LOOKUP) 
        {
            ext.name = val.name;
            ext.type = type;
            ext.index = local ? -1 : index;
        }
        if(type==SHPARAM_VERTEX) s.extvertparams[index] = i;
        else s.extpixparams[index] = i;
        return;
    }
    if(loc == -1)
    {
        if(type==SHPARAM_VERTEX) s.extvertparams[index] = local ? UNUSEDEXTPARAM : ALLOCEXTPARAM;
        else s.extpixparams[index] = local ? UNUSEDEXTPARAM : ALLOCEXTPARAM;
        return;
    }
    int extindex = addextparam(s, val.name, type, local ? -1 : index, loc);
    if(type==SHPARAM_VERTEX) s.extvertparams[index] = extindex;
    else s.extpixparams[index] = extindex;
}

static void setglsluniformformat(Shader &s, const char *name, GLenum format, int size)
{
    switch(format)
    {
        case GL_FLOAT:
        case GL_FLOAT_VEC2_ARB:
        case GL_FLOAT_VEC3_ARB:
            break;
        case GL_FLOAT_VEC4_ARB:
        default:
            return;
    }
    if(size > 1 || !strncmp(name, "gl_", 3)) return;
    int loc = glGetUniformLocation_(s.program, name);
    if(loc < 0) return;
    loopvj(s.defaultparams) if(s.defaultparams[j].loc == loc)
    {
        s.defaultparams[j].format = format;
        return;
    }
    loopj(s.numextparams) if(s.extparams[j].loc == loc)
    {
        s.extparams[j].format = format;
        return;
    }
    int extindex = addextparam(s, NULL, SHPARAM_LOOKUP, -1, loc);
    if(extindex >= 0) s.extparams[extindex].format = format;
}

static void allocglslactiveuniforms(Shader &s)
{
    GLint numactive = 0;
    glGetObjectParameteriv_(s.program, GL_OBJECT_ACTIVE_UNIFORMS_ARB, &numactive);
    string name;
    loopi(numactive)
    {
        GLsizei namelen = 0;
        GLint size = 0;
        GLenum format = GL_FLOAT_VEC4_ARB;
        name[0] = '\0';
        glGetActiveUniform_(s.program, i, sizeof(name)-1, &namelen, &size, &format, name);
        if(namelen <= 0) continue;
        name[clamp(int(namelen), 0, (int)sizeof(name)-2)] = '\0'; 
        setglsluniformformat(s, name, format, size);
    } 
}

static inline bool duplicateenvparam(GlobalShaderParamState &param)
{
    loopj(RESERVEDSHADERPARAMS) 
    {
        GlobalShaderParamState &vp = vertexparamstate[j];
        if(vp.name && !vp.local && vp.version > param.version && !strcmp(vp.name, param.name))
            return true;
        GlobalShaderParamState &pp = pixelparamstate[j];
        if(pp.name && !pp.local && pp.version > param.version && !strcmp(pp.name, param.name))
            return true;
    }
    return false;
}

void Shader::allocenvparams(Slot *slot)
{
    if(!(type & SHADER_GLSLANG)) return;

    if(slot)
    {
#define UNIFORMTEX(name, tmu) \
        { \
            loc = glGetUniformLocation_(program, name); \
            int val = tmu; \
            if(loc != -1) glUniform1i_(loc, val); \
        }
        int loc, tmu = 2;
        if(type & SHADER_NORMALSLMS)
        {
            UNIFORMTEX("lmcolor", 1);
            UNIFORMTEX("lmdir", 2);
            tmu++;
        }
        else UNIFORMTEX("lightmap", 1);
        if(type & SHADER_ENVMAP) UNIFORMTEX("envmap", tmu++);
        UNIFORMTEX("shadowmap", 7);
        int stex = 0;
        loopv(slot->sts)
        {
            Slot::Tex &t = slot->sts[i];
            switch(t.type)
            {
                case TEX_DIFFUSE: UNIFORMTEX("diffusemap", 0); break;
                case TEX_NORMAL: UNIFORMTEX("normalmap", tmu++); break;
                case TEX_GLOW: UNIFORMTEX("glowmap", tmu++); break;
                case TEX_DECAL: UNIFORMTEX("decal", tmu++); break;
                case TEX_SPEC: if(t.combined<0) UNIFORMTEX("specmap", tmu++); break;
                case TEX_DEPTH: if(t.combined<0) UNIFORMTEX("depthmap", tmu++); break;
                case TEX_UNKNOWN: 
                {
                    defformatstring(sname)("stex%d", stex++); 
                    UNIFORMTEX(sname, tmu++);
                    break;
                }
            }
        }
    }
    if(!extvertparams) 
    {
        extvertparams = new uchar[2*RESERVEDSHADERPARAMS];
        extpixparams = extvertparams + RESERVEDSHADERPARAMS;
    }
    memset(extvertparams, ALLOCEXTPARAM, 2*RESERVEDSHADERPARAMS);
    loopi(RESERVEDSHADERPARAMS) if(vertexparamstate[i].name && !vertexparamstate[i].local && !duplicateenvparam(vertexparamstate[i]))
        allocglsluniformparam(*this, SHPARAM_VERTEX, i);
    loopi(RESERVEDSHADERPARAMS) if(pixelparamstate[i].name && !pixelparamstate[i].local && !duplicateenvparam(pixelparamstate[i]))
        allocglsluniformparam(*this, SHPARAM_PIXEL, i);
    allocglslactiveuniforms(*this);
}

static inline void setuniformval(LocalShaderParamState &l, const float *val)
{
    if(memcmp(l.curval, val, sizeof(l.curval)))
    {
        memcpy(l.curval, val, sizeof(l.curval));
        switch(l.format)
        {
            case GL_FLOAT:          glUniform1fv_(l.loc, 1, l.curval); break;
            case GL_FLOAT_VEC2_ARB: glUniform2fv_(l.loc, 1, l.curval); break;
            case GL_FLOAT_VEC3_ARB: glUniform3fv_(l.loc, 1, l.curval); break;
            case GL_FLOAT_VEC4_ARB: glUniform4fv_(l.loc, 1, l.curval); break;
        }
    }
}

static inline void flushparam(int type, int index)
{
    ShaderParamState &val = (type==SHPARAM_VERTEX ? vertexparamstate[index] : pixelparamstate[index]);
    if(Shader::lastshader && Shader::lastshader->type&SHADER_GLSLANG)
    {
        uchar &extindex = (type==SHPARAM_VERTEX ? Shader::lastshader->extvertparams[index] : Shader::lastshader->extpixparams[index]);
        if(extindex == ALLOCEXTPARAM) allocglsluniformparam(*Shader::lastshader, type, index, val.local);
        if(extindex < Shader::lastshader->numextparams)
            setuniformval(Shader::lastshader->extparams[extindex], val.val);
    }
    else if(val.dirty==ShaderParamState::DIRTY)
    {
        glProgramEnvParameter4fv_(type==SHPARAM_VERTEX ? GL_VERTEX_PROGRAM_ARB : GL_FRAGMENT_PROGRAM_ARB, index, val.val);
        val.dirty = ShaderParamState::CLEAN;
    }
}

static inline bool sortparamversions(const GlobalShaderParamState *x, const GlobalShaderParamState *y)
{
    return x->version < y->version;
}

static uint resetparamversions()
{
    GlobalShaderParamState *params[2*(RESERVEDSHADERPARAMS + MAXSHADERPARAMS)];
    loopi(RESERVEDSHADERPARAMS + MAXSHADERPARAMS)
    {
        params[2*i+0] = &vertexparamstate[i];  
        params[2*i+1] = &pixelparamstate[i];
    }
    quicksort(params, 2*(RESERVEDSHADERPARAMS + MAXSHADERPARAMS), sortparamversions);
    paramversion = 0;
    loopi(2*(RESERVEDSHADERPARAMS + MAXSHADERPARAMS)) params[i]->version = ++paramversion;
    return paramversion;
}
 
static inline ShaderParamState &setparamf(const char *name, int type, int index, float x, float y, float z, float w)
{
    GlobalShaderParamState &val = (type==SHPARAM_VERTEX ? vertexparamstate[index] : pixelparamstate[index]);
    val.name = name;
    val.version = ++paramversion > 0 ? paramversion : resetparamversions();
    if(val.dirty==ShaderParamState::INVALID || val.val[0]!=x || val.val[1]!=y || val.val[2]!=z || val.val[3]!=w)
    {
        val.val[0] = x;
        val.val[1] = y;
        val.val[2] = z;
        val.val[3] = w;
        val.dirty = ShaderParamState::DIRTY;
    }
    return val;
}

static inline ShaderParamState &setparamfv(const char *name, int type, int index, const float *v)
{
    GlobalShaderParamState &val = (type==SHPARAM_VERTEX ? vertexparamstate[index] : pixelparamstate[index]);
    val.name = name;
    val.version = ++paramversion > 0 ? paramversion : resetparamversions();
    if(val.dirty==ShaderParamState::INVALID || memcmp(val.val, v, sizeof(val.val)))
    {
        memcpy(val.val, v, sizeof(val.val));
        val.dirty = ShaderParamState::DIRTY;
    }
    return val;
}

void setenvparamf(const char *name, int type, int index, float x, float y, float z, float w)
{
    ShaderParamState &val = setparamf(name, type, index, x, y, z, w);
    val.local = false;
    if(val.dirty==ShaderParamState::DIRTY) dirtyenvparams = true;
}

void setenvparamfv(const char *name, int type, int index, const float *v)
{
    ShaderParamState &val = setparamfv(name, type, index, v);
    val.local = false;
    if(val.dirty==ShaderParamState::DIRTY) dirtyenvparams = true;
}

void flushenvparamf(const char *name, int type, int index, float x, float y, float z, float w)
{
    ShaderParamState &val = setparamf(name, type, index, x, y, z, w);
    val.local = false;
    flushparam(type, index);
}

void flushenvparamfv(const char *name, int type, int index, const float *v)
{
    ShaderParamState &val = setparamfv(name, type, index, v);
    val.local = false;
    flushparam(type, index);
}

void setlocalparamf(const char *name, int type, int index, float x, float y, float z, float w)
{
    ShaderParamState &val = setparamf(name, type, index, x, y, z, w);
    val.local = true;
    flushparam(type, index);
}

void setlocalparamfv(const char *name, int type, int index, const float *v)
{
    ShaderParamState &val = setparamfv(name, type, index, v);
    val.local = true;
    flushparam(type, index);
}

void invalidateenvparams(int type, int start, int count)
{
    GlobalShaderParamState *paramstate = type==SHPARAM_VERTEX ? vertexparamstate : pixelparamstate;
    int end = min(start + count, RESERVEDSHADERPARAMS + MAXSHADERPARAMS);
    while(start < end)
    {
        paramstate[start].dirty = ShaderParamState::INVALID;
        start++;
    }
}

void Shader::flushenvparams(Slot *slot)
{
    if(type & SHADER_GLSLANG)
    {
        if(!used) allocenvparams(slot);
            
        loopi(numextparams)
        {
            LocalShaderParamState &ext = extparams[i];
            if(ext.index >= 0)
                setuniformval(ext, ext.type==SHPARAM_VERTEX ? vertexparamstate[ext.index].val : pixelparamstate[ext.index].val);
        }
    }
    else if(dirtyenvparams)
    {
        loopi(RESERVEDSHADERPARAMS)
        {
            ShaderParamState &val = vertexparamstate[i];
            if(val.local || val.dirty!=ShaderParamState::DIRTY) continue;
            glProgramEnvParameter4fv_(GL_VERTEX_PROGRAM_ARB, i, val.val);
            val.dirty = ShaderParamState::CLEAN;
        }
        loopi(RESERVEDSHADERPARAMS)
        {
            ShaderParamState &val = pixelparamstate[i];
            if(val.local || val.dirty!=ShaderParamState::DIRTY) continue;
            glProgramEnvParameter4fv_(GL_FRAGMENT_PROGRAM_ARB, i, val.val);
            val.dirty = ShaderParamState::CLEAN;
        }
        dirtyenvparams = false;
    }
    used = true;
}

static inline void setglslslotparam(const ShaderParam &p, LocalShaderParamState &l, uint &mask, int i)
{
    if(!(mask&(1<<i)))
    {
        mask |= 1<<i;
        setuniformval(l, p.val);
    }
}

static inline void setglslslotparams(vector<LocalShaderParamState> &defaultparams, Slot &slot, VSlot &vslot)
{
    uint unimask = 0;
    loopv(vslot.params)
    {
        ShaderParam &p = vslot.params[i];
        if(!defaultparams.inrange(p.loc)) continue;
        LocalShaderParamState &l = defaultparams[p.loc];
        setglslslotparam(p, l, unimask, p.loc);
    }
    loopv(slot.params)
    {
        ShaderParam &p = slot.params[i];
        if(!defaultparams.inrange(p.loc)) continue;
        LocalShaderParamState &l = defaultparams[p.loc];
        setglslslotparam(p, l, unimask, p.loc);
    }
    loopv(defaultparams)
    {
        LocalShaderParamState &l = defaultparams[i];
        setglslslotparam(l, l, unimask, i);
    }
}

static inline void setasmslotparam(const ShaderParam &p, LocalShaderParamState &l, uint &mask)
{
    if(!(mask&(1<<l.index)))
    {
        mask |= 1<<l.index;
        ShaderParamState &val = (l.type==SHPARAM_VERTEX ? vertexparamstate[RESERVEDSHADERPARAMS+l.index] : pixelparamstate[RESERVEDSHADERPARAMS+l.index]);
        if(memcmp(val.val, p.val, sizeof(val.val))) memcpy(val.val, p.val, sizeof(val.val));
        else if(val.dirty==ShaderParamState::CLEAN) return;
        glProgramEnvParameter4fv_(l.type==SHPARAM_VERTEX ? GL_VERTEX_PROGRAM_ARB : GL_FRAGMENT_PROGRAM_ARB, RESERVEDSHADERPARAMS+l.index, val.val);
        val.local = true;
        val.dirty = ShaderParamState::CLEAN;
    }
}

static inline void setasmslotparams(vector<LocalShaderParamState> &defaultparams, Slot &slot, VSlot &vslot)
{
    uint vertmask = 0, pixmask = 0;
    loopv(vslot.params)
    {
        ShaderParam &p = vslot.params[i];
        if(!defaultparams.inrange(p.loc) || p.type==SHPARAM_UNIFORM) continue;
        LocalShaderParamState &l = defaultparams[p.loc];
        setasmslotparam(p, l, l.type==SHPARAM_VERTEX ? vertmask : pixmask);
    }
    loopv(slot.params)
    {
        ShaderParam &p = slot.params[i];
        if(!defaultparams.inrange(p.loc) || p.type==SHPARAM_UNIFORM) continue;
        LocalShaderParamState &l = defaultparams[p.loc];
        setasmslotparam(p, l, l.type==SHPARAM_VERTEX ? vertmask : pixmask);
    }
    loopv(defaultparams)
    {
        LocalShaderParamState &l = defaultparams[i];
        if(l.type!=SHPARAM_UNIFORM) setasmslotparam(l, l, l.type==SHPARAM_VERTEX ? vertmask : pixmask);
    }
}

void Shader::setslotparams(Slot &slot, VSlot &vslot)
{
    if(type & SHADER_GLSLANG) setglslslotparams(defaultparams, slot, vslot);
    else setasmslotparams(defaultparams, slot, vslot);
}

void Shader::bindprograms()
{
    if(this == lastshader || type&(SHADER_DEFERRED|SHADER_INVALID)) return;
    if(type & SHADER_GLSLANG)
    {
        glUseProgramObject_(program);
    }
    else
    {
        if(lastshader && lastshader->type & SHADER_GLSLANG) glUseProgramObject_(0);

        glBindProgram_(GL_VERTEX_PROGRAM_ARB,   vs);
        glBindProgram_(GL_FRAGMENT_PROGRAM_ARB, ps);
    }
    lastshader = this;
}

VARFN(shaders, useshaders, -1, 1, 1, initwarning("shaders"));
VARF(shaderprecision, 0, 0, 2, initwarning("shader quality"));
VARF(forceglsl, 0, 1, 1, initwarning("shaders"));

bool Shader::compile()
{
    if(type & SHADER_GLSLANG)
    {
        if(!vsstr) vsobj = !reusevs || reusevs->type&SHADER_INVALID ? 0 : reusevs->vsobj;
        else compileglslshader(GL_VERTEX_SHADER_ARB,   vsobj, vsstr, "VS", name, dbgshader || !variantshader);
        if(!psstr) psobj = !reuseps || reuseps->type&SHADER_INVALID ? 0 : reuseps->psobj;
        else compileglslshader(GL_FRAGMENT_SHADER_ARB, psobj, psstr, "PS", name, dbgshader || !variantshader);
        linkglslprogram(*this, !variantshader);
        return program!=0;
    }
    else
    {
        if(renderpath!=R_ASMSHADER && renderpath!=R_ASMGLSLANG) return false;
        if(!vsstr) vs = !reusevs || reusevs->type&SHADER_INVALID ? 0 : reusevs->vs;
        else if(!compileasmshader(GL_VERTEX_PROGRAM_ARB, vs, vsstr, "VS", name, dbgshader || !variantshader, variantshader!=NULL))
            native = false;
        if(!psstr) ps = !reuseps || reuseps->type&SHADER_INVALID ? 0 : reuseps->ps;
        else if(!compileasmshader(GL_FRAGMENT_PROGRAM_ARB, ps, psstr, "PS", name, dbgshader || !variantshader, variantshader!=NULL))
            native = false;
        return vs && ps && (!variantshader || native);
    }
}

void Shader::cleanup(bool invalid)
{
    detailshader = NULL;
    used = false;
    native = true;
    if(vs) { if(reusevs) glDeletePrograms_(1, &vs); vs = 0; }
    if(ps) { if(reuseps) glDeletePrograms_(1, &ps); ps = 0; }
    if(vsobj) { if(reusevs) glDeleteObject_(vsobj); vsobj = 0; }
    if(psobj) { if(reuseps) glDeleteObject_(psobj); psobj = 0; }
    if(program) { glDeleteObject_(program); program = 0; }
    numextparams = 0;
    DELETEA(extparams);
    DELETEA(extvertparams);
    extpixparams = NULL;
    loopv(defaultparams) memset(defaultparams[i].curval, -1, sizeof(defaultparams[i].curval));
    if(standard || invalid)
    {
        type = SHADER_INVALID;
        loopi(MAXVARIANTROWS) variants[i].setsize(0);
        DELETEA(vsstr);
        DELETEA(psstr);
        DELETEA(defer);
        defaultparams.setsize(0);
        attriblocs.setsize(0);
        uniformlocs.setsize(0);
        altshader = NULL;
        loopi(MAXSHADERDETAIL) fastshader[i] = this;
        reusevs = reuseps = NULL;
    }
}

static void genattriblocs(Shader &s, const char *vs, const char *ps)
{
    static int len = strlen("#pragma CUBE2_attrib");
    string name;
    int loc;
    while((vs = strstr(vs, "#pragma CUBE2_attrib")))
    {
        if(sscanf(vs, "#pragma CUBE2_attrib %s %d", name, &loc) == 2)
            s.attriblocs.add(AttribLoc(getshaderparamname(name), loc));
        vs += len;
    }
}

static void genuniformlocs(Shader &s, const char *vs, const char *ps)
{
    static int len = strlen("#pragma CUBE2_uniform");
    string name, blockname;
    int binding, stride;
    while((vs = strstr(vs, "#pragma CUBE2_uniform")))
    {
        int numargs = sscanf(vs, "#pragma CUBE2_uniform %s %s %d %d", name, blockname, &binding, &stride);
        if(numargs >= 3) s.uniformlocs.add(UniformLoc(getshaderparamname(name), getshaderparamname(blockname), binding, numargs >= 4 ? stride : 0));
        else if(numargs >= 1) s.uniformlocs.add(UniformLoc(getshaderparamname(name)));
        vs += len;
    }
}

Shader *newshader(int type, const char *name, const char *vs, const char *ps, Shader *variant = NULL, int row = 0)
{
    if(Shader::lastshader)
    {
        if(renderpath==R_ASMSHADER || renderpath==R_ASMGLSLANG)
        {
            glBindProgram_(GL_VERTEX_PROGRAM_ARB, 0);
            glBindProgram_(GL_FRAGMENT_PROGRAM_ARB, 0);
        }
        if(renderpath==R_GLSLANG || renderpath==R_ASMGLSLANG) glUseProgramObject_(0);
        Shader::lastshader = NULL;
    }

    Shader *exists = shaders.access(name); 
    char *rname = exists ? exists->name : newstring(name);
    Shader &s = shaders[rname];
    s.name = rname;
    s.vsstr = newstring(vs);
    s.psstr = newstring(ps);
    DELETEA(s.defer);
    s.type = type;
    s.variantshader = variant;
    s.standard = standardshader;
    if(forceshaders) s.forced = true;
    s.reusevs = s.reuseps = NULL;
    if(variant)
    {
        int row = 0, col = 0;
        if(!vs[0] || sscanf(vs, "%d , %d", &row, &col) >= 1) 
        {
            DELETEA(s.vsstr);
            s.reusevs = !vs[0] ? variant : (variant->variants[row].inrange(col) ? variant->variants[row][col] : NULL);
        }
        row = col = 0;
        if(!ps[0] || sscanf(ps, "%d , %d", &row, &col) >= 1)
        {
            DELETEA(s.psstr);
            s.reuseps = !ps[0] ? variant : (variant->variants[row].inrange(col) ? variant->variants[row][col] : NULL);
        }
    }
    if(variant) loopv(variant->defaultparams) s.defaultparams.add(variant->defaultparams[i]);
    else loopv(curparams) s.defaultparams.add(curparams[i]);
    s.attriblocs.setsize(0);
    s.uniformlocs.setsize(0);
    if(type & SHADER_GLSLANG)
    {
        genattriblocs(s, vs, ps);
        genuniformlocs(s, vs, ps);
    }
    if(renderpath!=R_FIXEDFUNCTION && !s.compile())
    {
        s.cleanup(true);
        if(variant) shaders.remove(rname);
        return NULL;
    }
    if(variant) variant->variants[row].add(&s);
    s.fixdetailshader();
    return &s;
}

static const char *findglslmain(const char *s)
{
    const char *main = strstr(s, "main");
    if(!main) return NULL;
    for(; main >= s; main--) switch(*main) { case '\r': case '\n': case ';': return main + 1; }
    return s;
}

static uint findusedtexcoords(const char *str)
{
    uint used = 0;
    for(;;)
    {
        const char *tc = strstr(str, "result.texcoord[");
        if(!tc) break;
        tc += strlen("result.texcoord[");
        int n = strtol(tc, (char **)&str, 10);
        if(n<0 || n>=16) continue;
        used |= 1<<n;
    }
    return used;
}

static bool findunusedtexcoordcomponent(const char *str, int &texcoord, int &component)
{
    uchar texcoords[16];
    memset(texcoords, 0, sizeof(texcoords));
    for(;;)
    {
        const char *tc = strstr(str, "result.texcoord[");
        if(!tc) break;
        tc += strlen("result.texcoord[");
        int n = strtol(tc, (char **)&str, 10);
        if(n<0 || n>=(int)sizeof(texcoords)) continue;
        while(*str && *str!=']') str++;
        if(*str==']')
        {
            if(*++str!='.') { texcoords[n] = 0xF; continue; }
            for(;;) 
            {
                switch(*++str)
                {
                    case 'r': case 'x': texcoords[n] |= 1; continue;
                    case 'g': case 'y': texcoords[n] |= 2; continue;
                    case 'b': case 'z': texcoords[n] |= 4; continue;
                    case 'a': case 'w': texcoords[n] |= 8; continue;
                }
                break;
            }
        }
    }
    loopi(sizeof(texcoords)) if(texcoords[i]>0 && texcoords[i]<0xF)
    {
        loopk(4) if(!(texcoords[i]&(1<<k))) { texcoord = i; component = k; return true; }
    }
    return false;
}

void genemufog(vector<char> &vsbuf, vector<char> &psbuf, int fogtc, int fogcomp)
{
    char *fogcoord = strstr(vsbuf.getbuf(), "result.fogcoord");
    if(!fogcoord) return;
    static const int fogcoordlen = strlen("result.fogcoord");
    char *afterfogcoord = fogcoord + fogcoordlen;
    if(*afterfogcoord=='.') afterfogcoord += 2;
    defformatstring(repfogcoord)("result.texcoord[%d].%c", fogtc, fogcomp==3 ? 'w' : 'x'+fogcomp);
    memcpy(fogcoord, repfogcoord, afterfogcoord - fogcoord);
    vsbuf.insert(afterfogcoord - vsbuf.getbuf(), repfogcoord + (afterfogcoord - fogcoord), strlen(repfogcoord) - (afterfogcoord - fogcoord));

    char *fogoption = strstr(psbuf.getbuf(), "OPTION ARB_fog_linear;");
    if(!fogoption) return;
    static const int fogoptionlen = strlen("OPTION ARB_fog_linear;");
    memcpy(fogoption, "TEMP emufogcolor;     ", fogoptionlen);
    char *str = psbuf.getbuf();
    for(;;)
    {
        static const int colorlen = strlen("result.color");
        str = strstr(str, "result.color");
        if(!str) break;
        if(str[colorlen]!='.' || (str[colorlen+1]!='a' && str[colorlen+1]!='w')) 
            memcpy(str, " emufogcolor", colorlen);
        str += colorlen;
    }

    defformatstring(fogtcstr)("fragment.texcoord[%d].%c", fogtc, fogcomp==3 ? 'w' : 'x'+fogcomp);
    static const int fragfogcoordlen = strlen("fragment.fogcoord.x");
    str = strstr(psbuf.getbuf(), "fragment.fogcoord.x");
    if(str)
    {
        memcpy(str, fogtcstr, fragfogcoordlen);
        psbuf.insert(&str[fragfogcoordlen] - psbuf.getbuf(), &fogtcstr[fragfogcoordlen], strlen(fogtcstr) - fragfogcoordlen);
    } 
    char *end = strstr(psbuf.getbuf(), "END");
    if(end) psbuf.setsize(end - psbuf.getbuf());
    defformatstring(calcfog)(
        "TEMP emufog;\n"
        "SUB emufog.x, state.fog.params.z, %s;\n"
        "MUL_SAT emufog.x, emufog.x, state.fog.params.w;\n"
        "LRP result.color.rgb, emufog.x, emufogcolor, state.fog.color;\n"
        "END\n",
        fogtcstr);
    psbuf.put(calcfog, strlen(calcfog)+1);
}

VAR(reserveshadowmaptc, 1, 0, 0);
VAR(reservedynlighttc, 1, 0, 0);
VAR(minimizedynlighttcusage, 1, 0, 0);

static void gengenericvariant(Shader &s, const char *sname, const char *vs, const char *ps, int row)
{
    bool vschanged = false, pschanged = false;
    vector<char> vsv, psv;
    vsv.put(vs, strlen(vs)+1);
    psv.put(ps, strlen(ps)+1);

    static const int len = strlen("#pragma CUBE2_variant"), olen = strlen("override");
    for(char *vspragma = vsv.getbuf();; vschanged = true)
    {
        vspragma = strstr(vspragma, "#pragma CUBE2_variant");
        if(!vspragma) break;
        memset(vspragma, ' ', len);
        vspragma += len;
        if(!strncmp(vspragma, "override", olen))
        { 
            memset(vspragma, ' ', olen);
            vspragma += olen;
            char *end = vspragma + strcspn(vspragma, "\n\r");
            int endlen = strspn(end, "\n\r");
            memset(end, ' ', endlen);
        }
    }
    for(char *pspragma = psv.getbuf();; pschanged = true)
    {
        pspragma = strstr(pspragma, "#pragma CUBE2_variant");
        if(!pspragma) break;
        memset(pspragma, ' ', len);
        pspragma += len;
        if(!strncmp(pspragma, "override", olen))
        { 
            memset(pspragma, ' ', olen);
            pspragma += olen;
            char *end = pspragma + strcspn(pspragma, "\n\r");
            int endlen = strspn(end, "\n\r");
            memset(end, ' ', endlen);
        }
    }
    defformatstring(varname)("<variant:%d,%d>%s", s.variants[row].length(), row, sname);
    defformatstring(reuse)("%d", row);
    newshader(s.type, varname, vschanged ? vsv.getbuf() : reuse, pschanged ? psv.getbuf() : reuse, &s, row);
}

static bool genwatervariant(Shader &s, const char *sname, vector<char> &vs, vector<char> &ps, int row)
{
    char *vspragma = strstr(vs.getbuf(), "#pragma CUBE2_water");
    if(!vspragma) return false;
    char *pspragma = strstr(ps.getbuf(), "#pragma CUBE2_water");
    if(!pspragma) return false;
    vspragma += strcspn(vspragma, "\n");
    if(*vspragma) vspragma++;
    pspragma += strcspn(pspragma, "\n");
    if(*pspragma) pspragma++;
    if(s.type & SHADER_GLSLANG)
    {
        const char *fadedef = "waterfade = gl_Vertex.z*waterfadeparams.x + waterfadeparams.y;\n";
        vs.insert(vspragma-vs.getbuf(), fadedef, strlen(fadedef));
        const char *fadeuse = "gl_FragColor.a = waterfade;\n";
        ps.insert(pspragma-ps.getbuf(), fadeuse, strlen(fadeuse));
        const char *fadedecl = "uniform vec4 waterfadeparams; varying float waterfade;\n";
        const char *vsmain = findglslmain(vs.getbuf()), *psmain = findglslmain(ps.getbuf());
        vs.insert(vsmain ? vsmain - vs.getbuf() : 0, fadedecl, strlen(fadedecl));
        ps.insert(psmain ? psmain - ps.getbuf() : 0, fadedecl, strlen(fadedecl));
    }
    else
    {
        int fadetc = -1, fadecomp = -1;
        if(!findunusedtexcoordcomponent(vs.getbuf(), fadetc, fadecomp))
        {
            uint usedtc = findusedtexcoords(vs.getbuf());
            int reservetc = row%2 ? reserveshadowmaptc : reservedynlighttc;
            loopi(maxtexcoords-reservetc) if(!(usedtc&(1<<i))) { fadetc = i; fadecomp = 3; break; }
        }
        if(fadetc>=0)
        {
            defformatstring(fadedef)("MAD result.texcoord[%d].%c, vertex.position.z, program.env[8].x, program.env[8].y;\n", 
                                fadetc, fadecomp==3 ? 'w' : 'x'+fadecomp);
            vs.insert(vspragma-vs.getbuf(), fadedef, strlen(fadedef));
            defformatstring(fadeuse)("MOV result.color.a, fragment.texcoord[%d].%c;\n",
                                fadetc, fadecomp==3 ? 'w' : 'x'+fadecomp);
            ps.insert(pspragma-ps.getbuf(), fadeuse, strlen(fadeuse));
        }
        else // fallback - use fog value, works under water but not above
        {
            const char *fogfade = "MAD result.color.a, fragment.fogcoord.x, program.env[8].z, program.env[8].w;\n";
            ps.insert(pspragma-ps.getbuf(), fogfade, strlen(fogfade));
        }
    }
    defformatstring(name)("<water>%s", sname);
    Shader *variant = newshader(s.type, name, vs.getbuf(), ps.getbuf(), &s, row);
    return variant!=NULL;
}
       
static void genwatervariant(Shader &s, const char *sname, const char *vs, const char *ps, int row = 2)
{
    vector<char> vsw, psw;
    vsw.put(vs, strlen(vs)+1);
    psw.put(ps, strlen(ps)+1);
    genwatervariant(s, sname, vsw, psw, row);
}

static void gendynlightvariant(Shader &s, const char *sname, const char *vs, const char *ps, int row = 0)
{
    int numlights = 0, lights[MAXDYNLIGHTS];
    int emufogtc = -1, emufogcomp = -1;
    if(s.type & SHADER_GLSLANG) numlights = maxvaryings < 48 || minimizedynlighttcusage ? 1 : MAXDYNLIGHTS;
    else
    {
        uint usedtc = findusedtexcoords(vs);
        int reservetc = row%2 ? reserveshadowmaptc : reservedynlighttc;
        if(maxtexcoords-reservetc<0) return;
        int limit = minimizedynlighttcusage ? 1 : MAXDYNLIGHTS;
        loopi(maxtexcoords-reservetc) if(!(usedtc&(1<<i))) 
        {
            lights[numlights++] = i;    
            if(numlights>=limit) break;
        }
        extern int emulatefog;
        if(emulatefog && reservetc>0 && numlights+1<limit && !(usedtc&(1<<(maxtexcoords-reservetc))) && strstr(ps, "OPTION ARB_fog_linear;") && strstr(vs, "result.fogcoord"))
        {
            if(!findunusedtexcoordcomponent(vs, emufogtc, emufogcomp))
            {
                emufogtc = maxtexcoords-reservetc;
                emufogcomp = 3;
            }
            lights[numlights++] = maxtexcoords-reservetc;
        }
        if(!numlights) return;
    }

    const char *vspragma = strstr(vs, "#pragma CUBE2_dynlight"), *pspragma = strstr(ps, "#pragma CUBE2_dynlight");
    string pslight;
    vspragma += strcspn(vspragma, "\n");
    if(*vspragma) vspragma++;
    
    if(sscanf(pspragma, "#pragma CUBE2_dynlight %s", pslight)!=1) return;

    pspragma += strcspn(pspragma, "\n"); 
    if(*pspragma) pspragma++;

    const char *vsmain = vs, *psmain = ps;
    if(s.type & SHADER_GLSLANG)
    {
        vsmain = findglslmain(vs); if(vsmain > vspragma) vsmain = vs;
        psmain = findglslmain(ps); if(psmain > pspragma) psmain = ps;
    }

    vector<char> vsdl, psdl;
    loopi(MAXDYNLIGHTS)
    {
        vsdl.setsize(0);
        psdl.setsize(0);
        if(vsmain >= vs) vsdl.put(vs, vsmain - vs);
        if(psmain >= ps) psdl.put(ps, psmain - ps);
        if(s.type & SHADER_GLSLANG)
        {
            loopk(i+1)
            {
                defformatstring(pos)("%sdynlight%d%s%s", 
                    !k || k==numlights ? "uniform vec4 " : " ", 
                    k, 
                    k < numlights ? "pos" : "offset",
                    k==i || k+1==numlights ? ";\n" : ",");
                if(k<numlights) vsdl.put(pos, strlen(pos));
                else psdl.put(pos, strlen(pos));
            }
            loopk(i+1)
            {
                defformatstring(color)("%sdynlight%dcolor%s", !k ? "uniform vec4 " : " ", k, k==i ? ";\n" : ",");
                psdl.put(color, strlen(color));
            }
            loopk(min(i+1, numlights))
            {
                defformatstring(dir)("%sdynlight%ddir%s", !k ? "varying vec3 " : " ", k, k==i || k+1==numlights ? ";\n" : ",");
                vsdl.put(dir, strlen(dir));
                psdl.put(dir, strlen(dir));
            }
        }
            
        vsdl.put(vsmain, vspragma-vsmain);
        psdl.put(psmain, pspragma-psmain);

        loopk(i+1)
        {
            extern int ati_dph_bug;
            string tc, dl;
            if(s.type & SHADER_GLSLANG) formatstring(tc)(
                k<numlights ? 
                    "dynlight%ddir = gl_Vertex.xyz*dynlight%dpos.w + dynlight%dpos.xyz;\n" :
                    "vec3 dynlight%ddir = dynlight0dir*dynlight%doffset.w + dynlight%doffset.xyz;\n",   
                k, k, k);
            else if(k>=numlights) formatstring(tc)(
                "%s"
                "MAD dynlightdir.xyz, fragment.texcoord[%d], program.env[%d].w, program.env[%d];\n",
                k==numlights ? "TEMP dynlightdir;\n" : "",
                lights[0], k-1, k-1);
            else if(ati_dph_bug || lights[k]==emufogtc) formatstring(tc)(
                "MAD result.texcoord[%d].xyz, vertex.position, program.env[%d].w, program.env[%d];\n",
                lights[k], 10+k, 10+k);
            else formatstring(tc)(
                "MAD result.texcoord[%d].xyz, vertex.position, program.env[%d].w, program.env[%d];\n" 
                "MOV result.texcoord[%d].w, 1;\n",
                lights[k], 10+k, 10+k, lights[k]);
            if(k < numlights) vsdl.put(tc, strlen(tc));
            else psdl.put(tc, strlen(tc));

            if(s.type & SHADER_GLSLANG) formatstring(dl)(
                "%s.rgb += dynlight%dcolor.rgb * (1.0 - clamp(dot(dynlight%ddir, dynlight%ddir), 0.0, 1.0));\n",
                pslight, k, k, k);
            else if(k>=numlights) formatstring(dl)(
                "DP3_SAT dynlight.x, dynlightdir, dynlightdir;\n"
                "SUB dynlight.x, 1, dynlight.x;\n"
                "MAD %s.rgb, program.env[%d], dynlight.x, %s;\n",
                pslight, 10+k, pslight);
            else if(ati_dph_bug || lights[k]==emufogtc) formatstring(dl)(
                "%s"
                "DP3_SAT dynlight.x, fragment.texcoord[%d], fragment.texcoord[%d];\n"
                "SUB dynlight.x, 1, dynlight.x;\n"
                "MAD %s.rgb, program.env[%d], dynlight.x, %s;\n",
                !k ? "TEMP dynlight;\n" : "",
                lights[k], lights[k],
                pslight, 10+k, pslight);
            else formatstring(dl)(
                "%s"
                "DPH_SAT dynlight.x, -fragment.texcoord[%d], fragment.texcoord[%d];\n"
                "MAD %s.rgb, program.env[%d], dynlight.x, %s;\n",
                !k ? "TEMP dynlight;\n" : "",
                lights[k], lights[k],
                pslight, 10+k, pslight);
            psdl.put(dl, strlen(dl));
        }

        vsdl.put(vspragma, strlen(vspragma)+1);
        psdl.put(pspragma, strlen(pspragma)+1);

        if(emufogtc >= 0 && i+1 == numlights) genemufog(vsdl, psdl, emufogtc, emufogcomp);

        defformatstring(name)("<dynlight %d>%s", i+1, sname);
        Shader *variant = newshader(s.type, name, vsdl.getbuf(), psdl.getbuf(), &s, row); 
        if(!variant) return;
        if(row < 4) genwatervariant(s, name, vsdl, psdl, row+2);
    }
}

static void genshadowmapvariant(Shader &s, const char *sname, const char *vs, const char *ps, int row = 1)
{
    return; // XXX EMSCRIPTEN

    int smtc = -1, emufogtc = -1, emufogcomp = -1;
    if(!(s.type & SHADER_GLSLANG))
    {
        uint usedtc = findusedtexcoords(vs);
        if(maxtexcoords-reserveshadowmaptc<0) return;
        loopi(maxtexcoords-reserveshadowmaptc) if(!(usedtc&(1<<i))) { smtc = i; break; }
        extern int emulatefog;
        if(smtc<0 && emulatefog && reserveshadowmaptc>0 && !(usedtc&(1<<(maxtexcoords-reserveshadowmaptc))) && strstr(ps, "OPTION ARB_fog_linear;"))
        {
            if(!strstr(vs, "result.fogcoord") || !findunusedtexcoordcomponent(vs, emufogtc, emufogcomp)) return;
            smtc = maxtexcoords-reserveshadowmaptc;
        }
        if(smtc<0) return;
    }

    const char *vspragma = strstr(vs, "#pragma CUBE2_shadowmap"), *pspragma = strstr(ps, "#pragma CUBE2_shadowmap");
    string pslight;
    vspragma += strcspn(vspragma, "\n");
    if(*vspragma) vspragma++;

    if(sscanf(pspragma, "#pragma CUBE2_shadowmap %s", pslight)!=1) return;

    pspragma += strcspn(pspragma, "\n");
    if(*pspragma) pspragma++;

    const char *vsmain = vs, *psmain = ps;
    if(s.type & SHADER_GLSLANG)
    {
        vsmain = findglslmain(vs); if(vsmain > vspragma) vsmain = vs;
        psmain = findglslmain(ps); if(psmain > pspragma) psmain = ps;
    }

    vector<char> vssm, pssm;
    if(vsmain >= vs) vssm.put(vs, vsmain - vs);
    if(psmain >= ps) pssm.put(ps, psmain - ps);

    if(s.type & SHADER_GLSLANG)
    {
        const char *tc = "varying vec3 shadowmaptc;\n";
        vssm.put(tc, strlen(tc));
        pssm.put(tc, strlen(tc));
        const char *smtex = 
            "uniform sampler2D shadowmap;\n"
            "uniform vec4 shadowmapambient;\n";
        pssm.put(smtex, strlen(smtex));
    }

    vssm.put(vsmain, vspragma-vsmain);
    pssm.put(psmain, pspragma-psmain);

    extern int smoothshadowmappeel;
    if(s.type & SHADER_GLSLANG)
    {
        const char *tc =
            "shadowmaptc = vec3(gl_TextureMatrix[2] * gl_Vertex);\n";
        vssm.put(tc, strlen(tc));
        const char *sm =
            smoothshadowmappeel ? 
                "vec4 smvals = texture2D(shadowmap, shadowmaptc.xy);\n"
                "vec2 smdiff = clamp(smvals.xz - shadowmaptc.zz*smvals.y, 0.0, 1.0);\n"
                "float shadowed = clamp((smdiff.x > 0.0 ? smvals.w : 0.0) - 8.0*smdiff.y, 0.0, 1.0);\n" :

                "vec4 smvals = texture2D(shadowmap, shadowmaptc.xy);\n"
                "float smtest = shadowmaptc.z*smvals.y;\n"
                "float shadowed = smtest < smvals.x && smtest > smvals.z ? smvals.w : 0.0;\n";
        pssm.put(sm, strlen(sm));
        defformatstring(smlight)(
            "%s.rgb -= shadowed*clamp(%s.rgb - shadowmapambient.rgb, 0.0, 1.0);\n",
            pslight, pslight, pslight);
        pssm.put(smlight, strlen(smlight));
    }
    else
    {
        defformatstring(tc)(
            "DP4 result.texcoord[%d].x, state.matrix.texture[2].row[0], vertex.position;\n"
            "DP4 result.texcoord[%d].y, state.matrix.texture[2].row[1], vertex.position;\n"
            "DP4 result.texcoord[%d].z, state.matrix.texture[2].row[2], vertex.position;\n",
            smtc, smtc, smtc);
        vssm.put(tc, strlen(tc));

        defformatstring(sm)(
            smoothshadowmappeel ? 
                "TEMP smvals, smdiff, smambient;\n"
                "TEX smvals, fragment.texcoord[%d], texture[7], 2D;\n"
                "MAD_SAT smdiff.xy, -fragment.texcoord[%d].z, smvals.y, smvals.xzzz;\n"
                "CMP smvals.w, -smdiff.x, smvals.w, 0;\n"
                "MAD_SAT smvals.w, -8, smdiff.y, smvals.w;\n" :

                "TEMP smvals, smtest, smambient;\n"
                "TEX smvals, fragment.texcoord[%d], texture[7], 2D;\n"
                "MUL smtest.y, fragment.texcoord[%d].z, smvals.y;\n"
                "SLT smtest.xy, smtest.y, smvals.xzzz;\n"
                "MAD_SAT smvals.w, smvals.w, smtest.x, -smtest.y;\n",
            smtc, smtc);
        pssm.put(sm, strlen(sm));
        formatstring(sm)(
            "SUB_SAT smambient.rgb, %s, program.env[7];\n"
            "MAD %s.rgb, smvals.w, -smambient, %s;\n",
            pslight, pslight, pslight);
        pssm.put(sm, strlen(sm));
    }

    if(!hasFBO) for(char *s = pssm.getbuf();;)
    {
        s = strstr(s, "smvals.w");
        if(!s) break;
        s[7] = 'y';
        s += 8;
    }

    vssm.put(vspragma, strlen(vspragma)+1);
    pssm.put(pspragma, strlen(pspragma)+1);

    if(emufogtc >= 0) genemufog(vssm, pssm, emufogtc, emufogcomp);

    defformatstring(name)("<shadowmap>%s", sname);
    Shader *variant = newshader(s.type, name, vssm.getbuf(), pssm.getbuf(), &s, row);
    if(!variant) return;
    genwatervariant(s, name, vssm.getbuf(), pssm.getbuf(), row+2);

    if(strstr(vs, "#pragma CUBE2_dynlight")) gendynlightvariant(s, name, vssm.getbuf(), pssm.getbuf(), row);
}

static void genfogshader(vector<char> &vsbuf, vector<char> &psbuf, const char *vs, const char *ps)
{
    const char *vspragma = strstr(vs, "#pragma CUBE2_fog"), *pspragma = strstr(ps, "#pragma CUBE2_fog");
    if(!vspragma && !pspragma) return;
    static const int pragmalen = strlen("#pragma CUBE2_fog");
    const char *vsend = strrchr(vs, '}');
    if(vsend)
    { 
        vsbuf.put(vs, vsend - vs);
        const char *vsdef = "\n#define FOG_COORD ";
        const char *vsfog = "\ngl_FogFragCoord = -dot((FOG_COORD), gl_ModelViewMatrixTranspose[2]);\n";
        int clen = 0;
        if(vspragma)
        {
            vspragma += pragmalen;
            while(*vspragma && !iscubespace(*vspragma)) vspragma++;
            vspragma += strspn(vspragma, " \t\v\f");
            clen = strcspn(vspragma, "\r\n");
        }
        if(clen <= 0) { vspragma = "gl_Vertex"; clen = strlen(vspragma); }
        vsbuf.put(vsdef, strlen(vsdef));
        vsbuf.put(vspragma, clen);
        vsbuf.put(vsfog, strlen(vsfog));
        vsbuf.put(vsend, strlen(vsend)+1);
    }
    const char *psend = strrchr(ps, '}');
    if(psend)
    {
        psbuf.put(ps, psend - ps);
        const char *psdef = "\n#define FOG_COLOR ";
        const char *psfog = 
            pspragma && !strncmp(pspragma+pragmalen, "rgba", 4) ? 
                "\ngl_FragColor = mix((FOG_COLOR), gl_FragColor, clamp((gl_Fog.end - gl_FogFragCoord) * gl_Fog.scale, 0.0, 1.0));\n" :
                "\ngl_FragColor.rgb = mix((FOG_COLOR).rgb, gl_FragColor.rgb, clamp((gl_Fog.end - gl_FogFragCoord) * gl_Fog.scale, 0.0, 1.0));\n";
        int clen = 0;
        if(pspragma)
        {
            pspragma += pragmalen;
            while(iscubealpha(*pspragma)) pspragma++;
            while(*pspragma && !iscubespace(*pspragma)) pspragma++;
            pspragma += strspn(pspragma, " \t\v\f");
            clen = strcspn(pspragma, "\r\n");
        }
        if(clen <= 0) { pspragma = "gl_Fog.color"; clen = strlen(pspragma); }
        psbuf.put(psdef, strlen(psdef));
        psbuf.put(pspragma, clen);
        psbuf.put(psfog, strlen(psfog));
        psbuf.put(psend, strlen(psend)+1);
    }
}

static void genuniformdefs(vector<char> &vsbuf, vector<char> &psbuf, const char *vs, const char *ps, Shader *variant = NULL)
{
    if(variant ? variant->defaultparams.empty() : curparams.empty()) return;
    const char *vsmain = findglslmain(vs), *psmain = findglslmain(ps);
    if(!vsmain || !psmain) return;
    vsbuf.put(vs, vsmain - vs);
    psbuf.put(ps, psmain - ps);
    if(variant) loopv(variant->defaultparams)
    {
        defformatstring(uni)("\nuniform vec4 %s;\n", variant->defaultparams[i].name);
        vsbuf.put(uni, strlen(uni));
        psbuf.put(uni, strlen(uni));
    }
    else loopv(curparams)
    {
        defformatstring(uni)("\nuniform vec4 %s;\n", curparams[i].name);
        vsbuf.put(uni, strlen(uni));
        psbuf.put(uni, strlen(uni));
    }
    vsbuf.put(vsmain, strlen(vsmain)+1);
    psbuf.put(psmain, strlen(psmain)+1);
}

VAR(defershaders, 0, 1, 1);

void defershader(int *type, const char *name, const char *contents)
{
    Shader *exists = shaders.access(name);
    if(exists && !(exists->type&SHADER_INVALID)) return;
    if(!defershaders) { execute(contents); return; }
    char *rname = exists ? exists->name : newstring(name);
    Shader &s = shaders[rname];
    s.name = rname;
    DELETEA(s.defer);
    s.defer = newstring(contents);
    s.type = SHADER_DEFERRED | *type;
    s.standard = standardshader;
}

void useshader(Shader *s)
{
    if(!(s->type&SHADER_DEFERRED) || !s->defer) return;
        
    char *defer = s->defer;
    s->defer = NULL;
    bool wasstandard = standardshader, wasforcing = forceshaders;
    int oldflags = identflags;
    standardshader = s->standard;
    forceshaders = false;
    identflags &= ~IDF_PERSIST;
    curparams.shrink(0);
    execute(defer);
    identflags = oldflags;
    forceshaders = wasforcing;
    standardshader = wasstandard;
    delete[] defer;

    if(s->type&SHADER_DEFERRED)
    {
        DELETEA(s->defer);
        s->type = SHADER_INVALID;
    }
}

void fixshaderdetail()
{
    // must null out separately because fixdetailshader can recursively set it
    enumerate(shaders, Shader, s, { if(!s.forced) s.detailshader = NULL; });
    enumerate(shaders, Shader, s, { if(s.forced) s.fixdetailshader(); }); 
    linkslotshaders();
}

int Shader::uniformlocversion()
{
    static int version = 0;
    if(++version >= 0) return version;
    version = 0;
    enumerate(shaders, Shader, s, { loopvj(s.uniformlocs) s.uniformlocs[j].version = -1; });
    return version;
}

VARF(nativeshaders, 0, 1, 1, fixshaderdetail());
VARFP(shaderdetail, 0, MAXSHADERDETAIL, MAXSHADERDETAIL, fixshaderdetail());

void Shader::fixdetailshader(bool force, bool recurse)
{
    Shader *alt = this;
    detailshader = NULL;
    do
    {
        Shader *cur = shaderdetail < MAXSHADERDETAIL ? alt->fastshader[shaderdetail] : alt;
        if(cur->type&SHADER_DEFERRED && force) useshader(cur);
        if(!(cur->type&SHADER_INVALID))
        {
            if(cur->type&SHADER_DEFERRED) break;
            detailshader = cur;
            if(cur->native || !nativeshaders) break;
        }
        alt = alt->altshader;
    } while(alt && alt!=this);

    if(recurse && detailshader) loopi(MAXVARIANTROWS) loopvj(detailshader->variants[i]) detailshader->variants[i][j]->fixdetailshader(force, false);
}

Shader *useshaderbyname(const char *name)
{
    Shader *s = shaders.access(name);
    if(!s) return NULL;
    if(!s->detailshader) s->fixdetailshader(); 
    s->forced = true;
    return s;
}

void shader(int *type, char *name, char *vs, char *ps)
{
    if(lookupshaderbyname(name)) return;
   
    if((*type & SHADER_GLSLANG ? renderpath!=R_GLSLANG && renderpath!=R_ASMGLSLANG : renderpath==R_GLSLANG) ||
       (!hasCM && strstr(ps, *type & SHADER_GLSLANG ? "textureCube" : "CUBE;")) ||
       (!hasTR && strstr(ps, *type & SHADER_GLSLANG ? "texture2DRect" : "RECT;")))
    {
        curparams.shrink(0);
        return;
    }
 
    extern int mesa_program_bug;
    if(renderpath!=R_FIXEDFUNCTION)
    {
        defformatstring(info)("shader %s", name);
        renderprogress(loadprogress, info);
    }
    if((renderpath==R_ASMSHADER || renderpath==R_ASMGLSLANG) && mesa_program_bug && initshaders && !(*type & SHADER_GLSLANG))
    {
        glEnable(GL_VERTEX_PROGRAM_ARB);
        glEnable(GL_FRAGMENT_PROGRAM_ARB);
    }
    vector<char> vsbuf, psbuf, vsbak, psbak;
#define GENSHADER(cond, body) \
    if(cond) \
    { \
        if(vsbuf.length()) { vsbak.setsize(0); vsbak.put(vs, strlen(vs)+1); vs = vsbak.getbuf(); vsbuf.setsize(0); } \
        if(psbuf.length()) { psbak.setsize(0); psbak.put(ps, strlen(ps)+1); ps = psbak.getbuf(); psbuf.setsize(0); } \
        body; \
        if(vsbuf.length()) vs = vsbuf.getbuf(); \
        if(psbuf.length()) ps = psbuf.getbuf(); \
    }
    if(renderpath!=R_FIXEDFUNCTION)
    {
        if(*type & SHADER_GLSLANG)
        {
            GENSHADER(curparams.length(), genuniformdefs(vsbuf, psbuf, vs, ps));
            GENSHADER(strstr(vs, "#pragma CUBE2_fog") || strstr(ps, "#pragma CUBE2_fog"), genfogshader(vsbuf, psbuf, vs, ps)); 
        }
    }
    Shader *s = newshader(*type, name, vs, ps);
    if(s && renderpath!=R_FIXEDFUNCTION)
    {
        // '#' is a comment in vertex/fragment programs, while '#pragma' allows an escape for GLSL, so can handle both at once
        if(strstr(vs, "#pragma CUBE2_water")) genwatervariant(*s, s->name, vs, ps);
        if(strstr(vs, "#pragma CUBE2_shadowmap")) genshadowmapvariant(*s, s->name, vs, ps);
        if(strstr(vs, "#pragma CUBE2_dynlight")) gendynlightvariant(*s, s->name, vs, ps);
    }
    if((renderpath==R_ASMSHADER || renderpath==R_ASMGLSLANG) && mesa_program_bug && initshaders && !(*type & SHADER_GLSLANG))
    {
        glDisable(GL_VERTEX_PROGRAM_ARB);
        glDisable(GL_FRAGMENT_PROGRAM_ARB);
    }
    curparams.shrink(0);
}

void variantshader(int *type, char *name, int *row, char *vs, char *ps)
{
    if(*row < 0)
    {
        shader(type, name, vs, ps);
        return;
    }

    if(renderpath==R_FIXEDFUNCTION && standardshader) return;

    Shader *s = lookupshaderbyname(name);
    if(!s) return;

    defformatstring(varname)("<variant:%d,%d>%s", s->variants[*row].length(), *row, name);
    //defformatstring(info)("shader %s", varname);
    //renderprogress(loadprogress, info);
    extern int mesa_program_bug;
    if((renderpath==R_ASMSHADER || renderpath==R_ASMGLSLANG) && mesa_program_bug && initshaders && !(*type & SHADER_GLSLANG))
    {
        glEnable(GL_VERTEX_PROGRAM_ARB);
        glEnable(GL_FRAGMENT_PROGRAM_ARB);
    }
    vector<char> vsbuf, psbuf, vsbak, psbak;
    if(renderpath!=R_FIXEDFUNCTION)
    {
        if(*type & SHADER_GLSLANG)
        {
            GENSHADER(s->defaultparams.length(), genuniformdefs(vsbuf, psbuf, vs, ps, s));
            GENSHADER(strstr(vs, "#pragma CUBE2_fog") || strstr(ps, "#pragma CUBE2_fog"), genfogshader(vsbuf, psbuf, vs, ps));
        }
    }
    Shader *v = newshader(*type, varname, vs, ps, s, *row);
    if(v && renderpath!=R_FIXEDFUNCTION)
    {
        // '#' is a comment in vertex/fragment programs, while '#pragma' allows an escape for GLSL, so can handle both at once
        if(strstr(vs, "#pragma CUBE2_dynlight")) gendynlightvariant(*s, varname, vs, ps, *row);
        if(strstr(ps, "#pragma CUBE2_variant") || strstr(vs, "#pragma CUBE2_variant")) gengenericvariant(*s, varname, vs, ps, *row);
    }
    if((renderpath==R_ASMSHADER || renderpath==R_ASMGLSLANG) && mesa_program_bug && initshaders && !(*type & SHADER_GLSLANG))
    {
        glDisable(GL_VERTEX_PROGRAM_ARB);
        glDisable(GL_FRAGMENT_PROGRAM_ARB);
    }
}

void setshader(char *name)
{
    curparams.shrink(0);
    Shader *s = shaders.access(name);
    if(!s)
    {
        if(renderpath!=R_FIXEDFUNCTION) conoutf(CON_ERROR, "no such shader: %s", name);
    }
    else curshader = s;
}

ShaderParam *findshaderparam(Slot &s, const char *name, int type = -1, int index = -1)
{
    loopv(s.params)
    {
        ShaderParam &param = s.params[i];
        if((name && param.name && !strcmp(name, param.name)) || (param.type==type && param.index==index)) return &param;
    }
    if(!s.shader->detailshader) return NULL;
    loopv(s.shader->detailshader->defaultparams)
    {
        ShaderParam &param = s.shader->detailshader->defaultparams[i];
        if((name && param.name && !strcmp(name, param.name)) || (param.type==type && param.index==index)) return &param;
    }
    return NULL;
}

ShaderParam *findshaderparam(VSlot &s, const char *name, int type = -1, int index = -1)
{
    loopv(s.params)
    {
        ShaderParam &param = s.params[i];
        if((name && param.name && !strcmp(name, param.name)) || (param.type==type && param.index==index)) return &param;
    }
    return findshaderparam(*s.slot, name, type, index);
}

void resetslotshader()
{
    curshader = NULL;
    curparams.shrink(0);
}

void setslotshader(Slot &s)
{
    s.shader = curshader;
    if(!s.shader)
    {
        s.shader = stdworldshader;
        return;
    }
    loopv(curparams) s.params.add(curparams[i]);
}

static void linkslotshaderparams(vector<ShaderParam> &params, Shader *sh, bool load)
{
    if(sh) loopv(params)
    {
        int loc = -1;
        ShaderParam &param = params[i];
        loopv(sh->defaultparams)
        {
            ShaderParam &dparam = sh->defaultparams[i];
            if(param.name ? dparam.name==param.name : dparam.type==param.type && dparam.index==param.index)
            {
                if(memcmp(param.val, dparam.val, sizeof(param.val))) loc = i;
                break;
            }
        }
        param.loc = loc;
    }
    else if(load) loopv(params) params[i].loc = -1;
}

void linkslotshader(Slot &s, bool load)
{
    if(!s.shader) return;

    if(load && !s.shader->detailshader) s.shader->fixdetailshader();

    Shader *sh = s.shader->detailshader;
    linkslotshaderparams(s.params, sh, load);
}

void linkvslotshader(VSlot &s, bool load)
{
    if(!s.slot->shader) return;

    Shader *sh = s.slot->shader->detailshader;
    linkslotshaderparams(s.params, sh, load);

    if(!sh) return;

    if(s.slot->texmask&(1<<TEX_GLOW))
    {
        ShaderParam *cparam = findshaderparam(s, "glowcolor");
        if(cparam) loopk(3) s.glowcolor[k] = clamp(cparam->val[k], 0.0f, 1.0f);
        ShaderParam *pulseparam = findshaderparam(s, "pulseglowcolor"), 
                    *speedparam = findshaderparam(s, "pulseglowspeed");
        if(pulseparam) loopk(3) s.pulseglowcolor[k] = clamp(pulseparam->val[k], 0.0f, 1.0f);
        if(speedparam) s.pulseglowspeed = speedparam->val[0]/1000.0f;
    }
    if(sh->type&SHADER_ENVMAP)
    {
        ShaderParam *envparam = findshaderparam(s, "envscale");
        if(envparam) loopk(3) s.envscale[k] = clamp(envparam->val[k], 0.0f, 1.0f);
    }
}

void altshader(char *origname, char *altname)
{
    Shader *orig = shaders.access(origname), *alt = shaders.access(altname);
    if(!orig || !alt) return;
    orig->altshader = alt;
    orig->fixdetailshader(false);
}

void fastshader(char *nice, char *fast, int *detail)
{
    Shader *ns = shaders.access(nice), *fs = shaders.access(fast);
    if(!ns || !fs) return;
    loopi(min(*detail+1, MAXSHADERDETAIL)) ns->fastshader[i] = fs;
    ns->fixdetailshader(false);
}

COMMAND(shader, "isss");
COMMAND(variantshader, "isiss");
COMMAND(setshader, "s");
COMMAND(altshader, "ss");
COMMAND(fastshader, "ssi");
COMMAND(defershader, "iss");
ICOMMAND(forceshader, "s", (const char *name), useshaderbyname(name));

void isshaderdefined(char *name)
{
    Shader *s = lookupshaderbyname(name);
    intret(s ? 1 : 0);
}

void isshadernative(char *name)
{
    Shader *s = lookupshaderbyname(name);
    intret(s && s->native ? 1 : 0);
}

COMMAND(isshaderdefined, "s");
COMMAND(isshadernative, "s");

static hashset<const char *> shaderparamnames(256);

const char *getshaderparamname(const char *name)
{
    const char **exists = shaderparamnames.access(name);
    if(exists) return *exists;
    name = newstring(name);
    shaderparamnames[name] = name;
    return name;
}

void addshaderparam(const char *name, int type, int n, float x, float y, float z, float w)
{
    if((type==SHPARAM_VERTEX || type==SHPARAM_PIXEL) && (n<0 || n>=MAXSHADERPARAMS))
    {
        conoutf(CON_ERROR, "shader param index must be 0..%d\n", MAXSHADERPARAMS-1);
        return;
    }
    if(name) name = getshaderparamname(name);
    loopv(curparams)
    {
        ShaderParam &param = curparams[i];
        if(param.type == type && (name ? param.name==name : param.index == n))
        {
            param.val[0] = x;
            param.val[1] = y;
            param.val[2] = z;
            param.val[3] = w;
            return;
        }
    }
    ShaderParam param = {name, type, n, -1, {x, y, z, w}};
    curparams.add(param);
}

ICOMMAND(setvertexparam, "iffff", (int *n, float *x, float *y, float *z, float *w), addshaderparam(NULL, SHPARAM_VERTEX, *n, *x, *y, *z, *w));
ICOMMAND(setpixelparam, "iffff", (int *n, float *x, float *y, float *z, float *w), addshaderparam(NULL, SHPARAM_PIXEL, *n, *x, *y, *z, *w));
ICOMMAND(setuniformparam, "sffff", (char *name, float *x, float *y, float *z, float *w), addshaderparam(name, SHPARAM_UNIFORM, -1, *x, *y, *z, *w));
ICOMMAND(setshaderparam, "sffff", (char *name, float *x, float *y, float *z, float *w), addshaderparam(name, SHPARAM_LOOKUP, -1, *x, *y, *z, *w));
ICOMMAND(defvertexparam, "siffff", (char *name, int *n, float *x, float *y, float *z, float *w), addshaderparam(name[0] ? name : NULL, SHPARAM_VERTEX, *n, *x, *y, *z, *w));
ICOMMAND(defpixelparam, "siffff", (char *name, int *n, float *x, float *y, float *z, float *w), addshaderparam(name[0] ? name : NULL, SHPARAM_PIXEL, *n, *x, *y, *z, *w));
ICOMMAND(defuniformparam, "sffff", (char *name, float *x, float *y, float *z, float *w), addshaderparam(name, SHPARAM_UNIFORM, -1, *x, *y, *z, *w));

#define NUMPOSTFXBINDS 10

struct postfxtex
{
    GLuint id;
    int scale, used;

    postfxtex() : id(0), scale(0), used(-1) {}
};
vector<postfxtex> postfxtexs;
int postfxbinds[NUMPOSTFXBINDS];
GLuint postfxfb = 0;
int postfxw = 0, postfxh = 0;

struct postfxpass
{
    Shader *shader;
    vec4 params;
    uint inputs, freeinputs;
    int outputbind, outputscale;

    postfxpass() : shader(NULL), inputs(1), freeinputs(1), outputbind(0), outputscale(0) {}
};
vector<postfxpass> postfxpasses;

static int allocatepostfxtex(int scale)
{
    loopv(postfxtexs)
    {
        postfxtex &t = postfxtexs[i];
        if(t.scale==scale && t.used < 0) return i; 
    }
    postfxtex &t = postfxtexs.add();
    t.scale = scale;
    glGenTextures(1, &t.id);
    createtexture(t.id, max(screen->w>>scale, 1), max(screen->h>>scale, 1), NULL, 3, 1, GL_RGB, GL_TEXTURE_RECTANGLE_ARB);
    return postfxtexs.length()-1;
}

void cleanuppostfx(bool fullclean)
{
    if(fullclean && postfxfb)
    {
        glDeleteFramebuffers_(1, &postfxfb);
        postfxfb = 0;
    }

    loopv(postfxtexs) glDeleteTextures(1, &postfxtexs[i].id);
    postfxtexs.shrink(0);

    postfxw = 0;
    postfxh = 0;
}

void renderpostfx()
{
    if(postfxpasses.empty() || renderpath==R_FIXEDFUNCTION) return;

    if(postfxw != screen->w || postfxh != screen->h) 
    {
        cleanuppostfx(false);
        postfxw = screen->w;
        postfxh = screen->h;
    }

    int binds[NUMPOSTFXBINDS];
    loopi(NUMPOSTFXBINDS) binds[i] = -1;
    loopv(postfxtexs) postfxtexs[i].used = -1;

    binds[0] = allocatepostfxtex(0);
    postfxtexs[binds[0]].used = 0;
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, postfxtexs[binds[0]].id);
    glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, screen->w, screen->h);

    if(hasFBO && postfxpasses.length() > 1)
    {
        if(!postfxfb) glGenFramebuffers_(1, &postfxfb);
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, postfxfb);
    }

    setenvparamf("millis", SHPARAM_VERTEX, 1, lastmillis/1000.0f, lastmillis/1000.0f, lastmillis/1000.0f);

    loopv(postfxpasses)
    {
        postfxpass &p = postfxpasses[i];

        int tex = -1;
        if(!postfxpasses.inrange(i+1))
        {
            if(hasFBO && postfxpasses.length() > 1) glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);
        }
        else
        {
            tex = allocatepostfxtex(p.outputscale);
            if(hasFBO) glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, postfxtexs[tex].id, 0);
        }

        int w = tex >= 0 ? max(screen->w>>postfxtexs[tex].scale, 1) : screen->w, 
            h = tex >= 0 ? max(screen->h>>postfxtexs[tex].scale, 1) : screen->h;
        glViewport(0, 0, w, h);
        p.shader->set();
        setlocalparamfv("params", SHPARAM_VERTEX, 0, p.params.v);
        setlocalparamfv("params", SHPARAM_PIXEL, 0, p.params.v);
        int tw = w, th = h, tmu = 0;
        loopj(NUMPOSTFXBINDS) if(p.inputs&(1<<j) && binds[j] >= 0)
        {
            if(!tmu)
            {
                tw = max(screen->w>>postfxtexs[binds[j]].scale, 1);
                th = max(screen->h>>postfxtexs[binds[j]].scale, 1);
            }
            else glActiveTexture_(GL_TEXTURE0_ARB + tmu);
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, postfxtexs[binds[j]].id);
            ++tmu;
        }
        if(tmu) glActiveTexture_(GL_TEXTURE0_ARB);
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0,  0);  glVertex2f(-1, -1);
        glTexCoord2f(tw, 0);  glVertex2f( 1, -1);
        glTexCoord2f(0,  th); glVertex2f(-1,  1);
        glTexCoord2f(tw, th); glVertex2f( 1,  1);
        glEnd();

        loopj(NUMPOSTFXBINDS) if(p.freeinputs&(1<<j) && binds[j] >= 0)
        {
            postfxtexs[binds[j]].used = -1;
            binds[j] = -1;
        }
        if(tex >= 0)
        {
            if(binds[p.outputbind] >= 0) postfxtexs[binds[p.outputbind]].used = -1;
            binds[p.outputbind] = tex;
            postfxtexs[tex].used = p.outputbind;
            if(!hasFBO)
            {
                glBindTexture(GL_TEXTURE_RECTANGLE_ARB, postfxtexs[tex].id);
                glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, w, h);
            }
        }
    }
}

static bool addpostfx(const char *name, int outputbind, int outputscale, uint inputs, uint freeinputs, const vec4 &params)
{
    if(!hasTR || !*name) return false;
    Shader *s = useshaderbyname(name);
    if(!s)
    {
        conoutf(CON_ERROR, "no such postfx shader: %s", name);
        return false;
    }
    postfxpass &p = postfxpasses.add();
    p.shader = s;
    p.outputbind = outputbind;
    p.outputscale = outputscale;
    p.inputs = inputs;
    p.freeinputs = freeinputs;
    p.params = params;
    return true;
}

void clearpostfx()
{
    postfxpasses.shrink(0);
    cleanuppostfx(false);
}

COMMAND(clearpostfx, "");

ICOMMAND(addpostfx, "siisffff", (char *name, int *bind, int *scale, char *inputs, float *x, float *y, float *z, float *w),
{
    int inputmask = inputs[0] ? 0 : 1;
    int freemask = inputs[0] ? 0 : 1;
    bool freeinputs = true;
    for(; *inputs; inputs++) if(isdigit(*inputs)) 
    {
        inputmask |= 1<<(*inputs-'0');
        if(freeinputs) freemask |= 1<<(*inputs-'0');
    }
    else if(*inputs=='+') freeinputs = false;
    else if(*inputs=='-') freeinputs = true;
    inputmask &= (1<<NUMPOSTFXBINDS)-1;
    freemask &= (1<<NUMPOSTFXBINDS)-1;
    addpostfx(name, clamp(*bind, 0, NUMPOSTFXBINDS-1), max(*scale, 0), inputmask, freemask, vec4(*x, *y, *z, *w));
});

ICOMMAND(setpostfx, "sffff", (char *name, float *x, float *y, float *z, float *w),
{
    clearpostfx();
    if(name[0]) addpostfx(name, 0, 0, 1, 1, vec4(*x, *y, *z, *w));
});

struct tmufunc
{
    GLenum combine, sources[4], ops[4];
    int scale;
};

struct tmu
{
    GLenum mode;
    GLfloat color[4];
    tmufunc rgb, alpha;
};

#define INVALIDTMU \
{ \
    0, \
    { -1, -1, -1, -1 }, \
    { 0, { 0, 0, 0, ~0 }, { 0, 0, 0, 0 }, 0 }, \
    { 0, { 0, 0, 0, ~0 }, { 0, 0, 0, 0 }, 0 } \
}

#define INITTMU \
{ \
    GL_MODULATE, \
    { 0, 0, 0, 0 }, \
    { GL_MODULATE, { GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB, GL_ZERO }, { GL_SRC_COLOR, GL_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR }, 1 }, \
    { GL_MODULATE, { GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB, GL_ZERO }, { GL_SRC_ALPHA, GL_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA }, 1 } \
}

#define MAXTMUS 8

tmu tmus[MAXTMUS] =
{
    INVALIDTMU,
    INVALIDTMU,
    INVALIDTMU,
    INVALIDTMU,
    INVALIDTMU,
    INVALIDTMU,
    INVALIDTMU,
    INVALIDTMU
};

VAR(maxtmus, 1, 0, 0);

void parsetmufunc(tmu &t, tmufunc &f, const char *s)
{
    int arg = -1;
    while(*s) switch(*s++)
    {
        case 'T':
        case 't': f.sources[++arg] = GL_TEXTURE; f.ops[arg] = GL_SRC_COLOR; break;
        case 'P':
        case 'p': f.sources[++arg] = GL_PREVIOUS_ARB; f.ops[arg] = GL_SRC_COLOR; break;
        case 'K':
        case 'k': f.sources[++arg] = GL_CONSTANT_ARB; f.ops[arg] = GL_SRC_COLOR; break;
        case 'C':
        case 'c': f.sources[++arg] = GL_PRIMARY_COLOR_ARB; f.ops[arg] = GL_SRC_COLOR; break;
        case '~': f.ops[arg] = GL_ONE_MINUS_SRC_COLOR; break;
        case 'A':
        case 'a': f.ops[arg] = f.ops[arg]==GL_ONE_MINUS_SRC_COLOR ? GL_ONE_MINUS_SRC_ALPHA : GL_SRC_ALPHA; break;
        case '=': f.combine = GL_REPLACE; break;
        case '*': f.combine = GL_MODULATE; break;
        case '+': f.combine = GL_ADD; break;
        case '-': f.combine = GL_SUBTRACT_ARB; break;
        case ',': 
        case '@': f.combine = GL_INTERPOLATE_ARB; break;
        case 'X':
        case 'x': while(!isdigit(*s)) s++; f.scale = *s++-'0'; break;
        // ARB_texture_env_crossbar, NV_texture_env_combine4
        case '$': f.sources[++arg] = GL_TEXTURE0_ARB + (*s++-'0'); f.ops[arg] = GL_SRC_COLOR; break;
        // EXT_texture_env_dot3
        case '.': f.combine = GL_DOT3_RGB_ARB; break;
        // ATI_texture_env_combine3
        case '3': f.combine = GL_MODULATE_ADD_ATI; break;
        // NV_texture_env_combine4
        case '4': t.mode = GL_COMBINE4_NV; f.combine = GL_ADD; break;
        case '0': f.sources[++arg] = GL_ZERO; f.ops[arg] = GL_SRC_COLOR; break;
        case '1': f.sources[++arg] = GL_ZERO; f.ops[arg] = GL_ONE_MINUS_SRC_COLOR; break;
    }
}

void resettmu(int n)
{
    if(renderpath!=R_FIXEDFUNCTION || n>=maxtmus) return;
    tmu &t = tmus[n];
    if(t.mode!=GL_MODULATE) { t.mode = GL_MODULATE; glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, t.mode); }
    if(t.rgb.scale != 1)  { t.rgb.scale = 1; glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, t.rgb.scale); }
    if(t.alpha.scale != 1)  { t.alpha.scale = 1; glTexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, t.alpha.scale); }
}

void scaletmu(int n, int rgbscale, int alphascale)
{
    if(renderpath!=R_FIXEDFUNCTION || n>=maxtmus) return;
    tmu &t = tmus[n];
    if(rgbscale && t.rgb.scale != rgbscale)  { t.rgb.scale = rgbscale; glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, t.rgb.scale); }
    if(alphascale && t.alpha.scale != alphascale)  { t.alpha.scale = alphascale; glTexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, t.alpha.scale); }
}

void colortmu(int n, float r, float g, float b, float a)
{
    if(renderpath!=R_FIXEDFUNCTION || n>=maxtmus) return;
    tmu &t = tmus[n];
    if(t.color[0] != r || t.color[1] != g || t.color[2] != b || t.color[3] != a)
    {
        t.color[0] = r;
        t.color[1] = g;
        t.color[2] = b;
        t.color[3] = a;
        glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, t.color);
    }
}

void committmufunc(GLenum mode, bool rgb, tmufunc &dst, tmufunc &src)
{
    if(dst.combine!=src.combine) glTexEnvi(GL_TEXTURE_ENV, rgb ? GL_COMBINE_RGB_ARB : GL_COMBINE_ALPHA_ARB, src.combine);
    loopi(3)
    {
        if(dst.sources[i]!=src.sources[i]) glTexEnvi(GL_TEXTURE_ENV, (rgb ? GL_SOURCE0_RGB_ARB : GL_SOURCE0_ALPHA_ARB)+i, src.sources[i]);
        if(dst.ops[i]!=src.ops[i]) glTexEnvi(GL_TEXTURE_ENV, (rgb ? GL_OPERAND0_RGB_ARB : GL_OPERAND0_ALPHA_ARB)+i, src.ops[i]);
    }
    if(mode==GL_COMBINE4_NV)
    {
        if(dst.sources[3]!=src.sources[3]) glTexEnvi(GL_TEXTURE_ENV, rgb ? GL_SOURCE3_RGB_NV : GL_SOURCE3_ALPHA_NV, src.sources[3]);
        if(dst.ops[3]!=src.ops[3]) glTexEnvi(GL_TEXTURE_ENV, rgb ? GL_OPERAND3_RGB_NV : GL_OPERAND3_ALPHA_NV, src.ops[3]);
    }
    if(dst.scale!=src.scale) glTexEnvi(GL_TEXTURE_ENV, rgb ? GL_RGB_SCALE_ARB : GL_ALPHA_SCALE, src.scale);
    dst = src;
}

void setuptmu(int n, const char *rgbfunc, const char *alphafunc)
{
    if(renderpath!=R_FIXEDFUNCTION || n>=maxtmus) return;

    static tmu init = INITTMU;
    tmu f = tmus[n];

    f.mode = GL_COMBINE_ARB;
    if(rgbfunc) parsetmufunc(f, f.rgb, rgbfunc);
    else f.rgb = init.rgb;
    if(alphafunc) parsetmufunc(f, f.alpha, alphafunc);
    else f.alpha = init.alpha;

    tmu &t = tmus[n];
    if(t.mode!=f.mode) { t.mode = f.mode; glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, t.mode); }
    committmufunc(f.mode, true, t.rgb, f.rgb);
    committmufunc(f.mode, false, t.alpha, f.alpha);
}

VAR(nolights, 1, 0, 0);
VAR(nowater, 1, 0, 0);
VAR(nomasks, 1, 0, 0);

void inittmus()
{
    if(hasTE && hasMT)
    {
        GLint val;
        glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &val);
        maxtmus = max(1, min(MAXTMUS, int(val)));
        loopi(maxtmus)
        {
            glActiveTexture_(GL_TEXTURE0_ARB+i);
            resettmu(i);
        }
        glActiveTexture_(GL_TEXTURE0_ARB);
    }
    else if(hasTE) { maxtmus = 1; resettmu(0); }
    if(renderpath==R_FIXEDFUNCTION)
    {
        if(maxtmus<4) caustics = 0;
        if(maxtmus<2)
        {
            nolights = nowater = nomasks = 1;
            extern int lightmodels;
            lightmodels = 0;
        }
    }
}

void cleanupshaders()
{
    cleanuppostfx(true);

    defaultshader = notextureshader = nocolorshader = foggedshader = foggednotextureshader = NULL;
    enumerate(shaders, Shader, s, s.cleanup());
    Shader::lastshader = NULL;
    if(renderpath==R_ASMSHADER || renderpath==R_ASMGLSLANG)
    {
        glBindProgram_(GL_VERTEX_PROGRAM_ARB, 0);
        glBindProgram_(GL_FRAGMENT_PROGRAM_ARB, 0);
        glDisable(GL_VERTEX_PROGRAM_ARB);
        glDisable(GL_FRAGMENT_PROGRAM_ARB);
    }
    if(renderpath==R_GLSLANG || renderpath==R_ASMGLSLANG) glUseProgramObject_(0);
    loopi(RESERVEDSHADERPARAMS + MAXSHADERPARAMS)
    {
        vertexparamstate[i].dirty = ShaderParamState::INVALID;
        pixelparamstate[i].dirty = ShaderParamState::INVALID;
    }

    tmu invalidtmu = INVALIDTMU;
    loopi(MAXTMUS) tmus[i] = invalidtmu;
}

void reloadshaders()
{
    identflags &= ~IDF_PERSIST;
    loadshaders();
    identflags |= IDF_PERSIST;
    if(renderpath==R_FIXEDFUNCTION) return;
    linkslotshaders();
    enumerate(shaders, Shader, s, 
    {
        if(!s.standard && !(s.type&(SHADER_DEFERRED|SHADER_INVALID)) && !s.variantshader) 
        {
            defformatstring(info)("shader %s", s.name);
            renderprogress(0.0, info);
            if(!s.compile()) s.cleanup(true);
            loopi(MAXVARIANTROWS) loopvj(s.variants[i])
            {
                Shader *v = s.variants[i][j];
                if((v->reusevs && v->reusevs->type&SHADER_INVALID) || 
                   (v->reuseps && v->reuseps->type&SHADER_INVALID) ||
                   !v->compile())
                    v->cleanup(true);
            }
        }
        if(s.forced && !s.detailshader) s.fixdetailshader();
    });
}

void setupblurkernel(int radius, float sigma, float *weights, float *offsets)
{
    if(radius<1 || radius>MAXBLURRADIUS) return;
    sigma *= 2*radius;
    float total = 1.0f/sigma;
    weights[0] = total;
    offsets[0] = 0;
    // rely on bilinear filtering to sample 2 pixels at once
    // transforms a*X + b*Y into (u+v)*[X*u/(u+v) + Y*(1 - u/(u+v))]
    loopi(radius)
    {
        float weight1 = exp(-((2*i)*(2*i)) / (2*sigma*sigma)) / sigma,
              weight2 = exp(-((2*i+1)*(2*i+1)) / (2*sigma*sigma)) / sigma,
              scale = weight1 + weight2,
              offset = 2*i+1 + weight2 / scale;
        weights[i+1] = scale;
        offsets[i+1] = offset;
        total += 2*scale;
    }
    loopi(radius+1) weights[i] /= total;
    for(int i = radius+1; i <= MAXBLURRADIUS; i++) weights[i] = offsets[i] = 0;
}

void setblurshader(int pass, int size, int radius, float *weights, float *offsets, GLenum target)
{
    if(radius<1 || radius>MAXBLURRADIUS) return; 
    static Shader *blurshader[7][2] = { { NULL, NULL }, { NULL, NULL }, { NULL, NULL }, { NULL, NULL }, { NULL, NULL }, { NULL, NULL }, { NULL, NULL } },
                  *blurrectshader[7][2] = { { NULL, NULL }, { NULL, NULL }, { NULL, NULL }, { NULL, NULL }, { NULL, NULL }, { NULL, NULL }, { NULL, NULL } };
    Shader *&s = (target == GL_TEXTURE_RECTANGLE_ARB ? blurrectshader : blurshader)[radius-1][pass];
    if(!s)
    {
        defformatstring(name)("blur%c%d%s", 'x'+pass, radius, target == GL_TEXTURE_RECTANGLE_ARB ? "rect" : "");
        s = lookupshaderbyname(name);
    }
    s->set();
    setlocalparamfv("weights", SHPARAM_PIXEL, 0, weights);
    setlocalparamfv("weights2", SHPARAM_PIXEL, 2, &weights[4]);
    setlocalparamf("offsets", SHPARAM_VERTEX, 1,
        pass==0 ? offsets[1]/size : offsets[0]/size,
        pass==1 ? offsets[1]/size : offsets[0]/size,
        (offsets[2] - offsets[1])/size,
        (offsets[3] - offsets[2])/size);
    loopk(4)
    {
        static const char *names[4] = { "offset4", "offset5", "offset6", "offset7" };
        setlocalparamf(names[k], SHPARAM_PIXEL, 3+k,
            pass==0 ? offsets[4+k]/size : offsets[0]/size,
            pass==1 ? offsets[4+k]/size : offsets[0]/size,
            0, 0);
    }
}

