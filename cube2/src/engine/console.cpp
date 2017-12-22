// console.cpp: the console buffer, its display, and command line control

#include "engine.h"

struct cline { char *line; int type, outtime; };
vector<cline> conlines;

int commandmillis = -1;
string commandbuf;
char *commandaction = NULL, *commandprompt = NULL;
enum { CF_COMPLETE = 1<<0, CF_EXECUTE = 1<<1 };
int commandflags = 0, commandpos = -1;

VARFP(maxcon, 10, 200, 1000, { while(conlines.length() > maxcon) delete[] conlines.pop().line; });

#define CONSTRLEN 512

void conline(int type, const char *sf)        // add a line to the console buffer
{
    cline cl;
    cl.line = conlines.length()>maxcon ? conlines.pop().line : newstring("", CONSTRLEN-1);   // constrain the buffer size
    cl.type = type;
    cl.outtime = totalmillis;                       // for how long to keep line on screen
    conlines.insert(0, cl);
    copystring(cl.line, sf, CONSTRLEN);
}

void conoutfv(int type, const char *fmt, va_list args)
{
    static char buf[CONSTRLEN];
    vformatstring(buf, fmt, args, sizeof(buf));
    conline(type, buf);
    filtertext(buf, buf);
    logoutf("%s", buf);
}

void conoutf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    conoutfv(CON_INFO, fmt, args);
    va_end(args); 
}

void conoutf(int type, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    conoutfv(type, fmt, args);
    va_end(args);
}

VAR(fullconsole, 0, 0, 1);
ICOMMAND(toggleconsole, "", (), { fullconsole ^= 1; });

int rendercommand(int x, int y, int w)
{
    if(commandmillis < 0) return 0;

    defformatstring(s)("%s %s", commandprompt ? commandprompt : ">", commandbuf);
    int width, height;
    text_bounds(s, width, height, w);
    y -= height;
    draw_text(s, x, y, 0xFF, 0xFF, 0xFF, 0xFF, (commandpos>=0) ? (commandpos+1+(commandprompt?strlen(commandprompt):1)) : strlen(s), w);
    return height;
}

VARP(consize, 0, 5, 100);
VARP(miniconsize, 0, 5, 100);
VARP(miniconwidth, 0, 40, 100);
VARP(confade, 0, 30, 60);
VARP(miniconfade, 0, 30, 60);
VARP(fullconsize, 0, 75, 100);
HVARP(confilter, 0, 0x7FFFFFF, 0x7FFFFFF);
HVARP(fullconfilter, 0, 0x7FFFFFF, 0x7FFFFFF);
HVARP(miniconfilter, 0, 0, 0x7FFFFFF);

int conskip = 0, miniconskip = 0;

void setconskip(int &skip, int filter, int n)
{
    int offset = abs(n), dir = n < 0 ? -1 : 1;
    skip = clamp(skip, 0, conlines.length()-1);
    while(offset)
    {
        skip += dir;
        if(!conlines.inrange(skip))
        {
            skip = clamp(skip, 0, conlines.length()-1);
            return;
        }
        if(conlines[skip].type&filter) --offset;
    }
}

ICOMMAND(conskip, "i", (int *n), setconskip(conskip, fullconsole ? fullconfilter : confilter, *n));
ICOMMAND(miniconskip, "i", (int *n), setconskip(miniconskip, miniconfilter, *n));

ICOMMAND(clearconsole, "", (), { while(conlines.length()) delete[] conlines.pop().line; });

int drawconlines(int conskip, int confade, int conwidth, int conheight, int conoff, int filter, int y = 0, int dir = 1)
{
    int numl = conlines.length(), offset = min(conskip, numl);

    if(confade)
    {
        if(!conskip)
        {
            numl = 0;
            loopvrev(conlines) if(totalmillis-conlines[i].outtime < confade*1000) { numl = i+1; break; }
        }
        else offset--;
    }

    int totalheight = 0;
    loopi(numl) //determine visible height
    {
        // shuffle backwards to fill if necessary
        int idx = offset+i < numl ? offset+i : --offset;
        if(!(conlines[idx].type&filter)) continue;
        char *line = conlines[idx].line;
        int width, height;
        text_bounds(line, width, height, conwidth);
        if(totalheight + height > conheight) { numl = i; if(offset == idx) ++offset; break; }
        totalheight += height;
    }
    if(dir > 0) y = conoff;
    loopi(numl)
    {
        int idx = offset + (dir > 0 ? numl-i-1 : i);
        if(!(conlines[idx].type&filter)) continue;
        char *line = conlines[idx].line;
        int width, height;
        text_bounds(line, width, height, conwidth);
        if(dir <= 0) y -= height; 
        draw_text(line, conoff, y, 0xFF, 0xFF, 0xFF, 0xFF, -1, conwidth);
        if(dir > 0) y += height;
    }
    return y+conoff;
}

int renderconsole(int w, int h, int abovehud)                   // render buffer taking into account time & scrolling
{
    int conpad = fullconsole ? 0 : FONTH/4,
        conoff = fullconsole ? FONTH : FONTH/3,
        conheight = min(fullconsole ? ((h*fullconsize/100)/FONTH)*FONTH : FONTH*consize, h - 2*(conpad + conoff)),
        conwidth = w - 2*(conpad + conoff) - (fullconsole ? 0 : game::clipconsole(w, h));
    
    extern void consolebox(int x1, int y1, int x2, int y2);
    if(fullconsole) consolebox(conpad, conpad, conwidth+conpad+2*conoff, conheight+conpad+2*conoff);
    
    int y = drawconlines(conskip, fullconsole ? 0 : confade, conwidth, conheight, conpad+conoff, fullconsole ? fullconfilter : confilter);
    if(!fullconsole && (miniconsize && miniconwidth))
        drawconlines(miniconskip, miniconfade, (miniconwidth*(w - 2*(conpad + conoff)))/100, min(FONTH*miniconsize, abovehud - y), conpad+conoff, miniconfilter, abovehud, -1);
    return fullconsole ? conheight + 2*(conpad + conoff) : y;
}

// keymap is defined externally in keymap.cfg

struct keym
{
    enum
    {
        ACTION_DEFAULT = 0,
        ACTION_SPECTATOR,
        ACTION_EDITING,
        NUMACTIONS
    };
    
    int code;
    char *name;
    char *actions[NUMACTIONS];
    bool pressed;

    keym() : code(-1), name(NULL), pressed(false) { loopi(NUMACTIONS) actions[i] = newstring(""); }
    ~keym() { DELETEA(name); loopi(NUMACTIONS) DELETEA(actions[i]); }
};

hashtable<int, keym> keyms(128);

void keymap(int *code, char *key)
{
    if(identflags&IDF_OVERRIDDEN) { conoutf(CON_ERROR, "cannot override keymap %s", code); return; }
    keym &km = keyms[*code];
    km.code = *code;
    DELETEA(km.name);
    km.name = newstring(key);
}
    
COMMAND(keymap, "is");

keym *keypressed = NULL;
char *keyaction = NULL;

const char *getkeyname(int code)
{
    keym *km = keyms.access(code);
    return km ? km->name : NULL;
}

void searchbinds(char *action, int type)
{
    vector<char> names;
    enumerate(keyms, keym, km,
    {
        if(!strcmp(km.actions[type], action))
        {
            if(names.length()) names.add(' ');
            names.put(km.name, strlen(km.name));
        }
    });
    names.add('\0');
    result(names.getbuf());
}

keym *findbind(char *key)
{
    enumerate(keyms, keym, km,
    {
        if(!strcasecmp(km.name, key)) return &km;
    });
    return NULL;
}   
    
void getbind(char *key, int type)
{
    keym *km = findbind(key);
    result(km ? km->actions[type] : "");
}   

void bindkey(char *key, char *action, int state, const char *cmd)
{
    if(identflags&IDF_OVERRIDDEN) { conoutf(CON_ERROR, "cannot override %s \"%s\"", cmd, key); return; }
    keym *km = findbind(key);
    if(!km) { conoutf(CON_ERROR, "unknown key \"%s\"", key); return; }
    char *&binding = km->actions[state];
    if(!keypressed || keyaction!=binding) delete[] binding;
    // trim white-space to make searchbinds more reliable
    while(iscubespace(*action)) action++;
    int len = strlen(action);
    while(len>0 && iscubespace(action[len-1])) len--;
    binding = newstring(action, len);
}

ICOMMAND(bind,     "ss", (char *key, char *action), bindkey(key, action, keym::ACTION_DEFAULT, "bind"));
ICOMMAND(specbind, "ss", (char *key, char *action), bindkey(key, action, keym::ACTION_SPECTATOR, "specbind"));
ICOMMAND(editbind, "ss", (char *key, char *action), bindkey(key, action, keym::ACTION_EDITING, "editbind"));
ICOMMAND(getbind,     "s", (char *key), getbind(key, keym::ACTION_DEFAULT));
ICOMMAND(getspecbind, "s", (char *key), getbind(key, keym::ACTION_SPECTATOR));
ICOMMAND(geteditbind, "s", (char *key), getbind(key, keym::ACTION_EDITING));
ICOMMAND(searchbinds,     "s", (char *action), searchbinds(action, keym::ACTION_DEFAULT));
ICOMMAND(searchspecbinds, "s", (char *action), searchbinds(action, keym::ACTION_SPECTATOR));
ICOMMAND(searcheditbinds, "s", (char *action), searchbinds(action, keym::ACTION_EDITING));

void inputcommand(char *init, char *action = NULL, char *prompt = NULL, char *flags = NULL) // turns input to the command line on or off
{
    commandmillis = init ? totalmillis : -1;
    SDL_EnableUNICODE(commandmillis >= 0 ? 1 : 0);
    if(!editmode) keyrepeat(commandmillis >= 0);
    copystring(commandbuf, init ? init : "");
    DELETEA(commandaction);
    DELETEA(commandprompt);
    commandpos = -1;
    if(action && action[0]) commandaction = newstring(action);
    if(prompt && prompt[0]) commandprompt = newstring(prompt);
    commandflags = 0;
    if(flags) while(*flags) switch(*flags++)
    {
        case 'c': commandflags |= CF_COMPLETE; break;
        case 'x': commandflags |= CF_EXECUTE; break;
        case 's': commandflags |= CF_COMPLETE|CF_EXECUTE; break;
    }
    else if(init) commandflags |= CF_COMPLETE|CF_EXECUTE;
}

ICOMMAND(saycommand, "C", (char *init), inputcommand(init));
COMMAND(inputcommand, "ssss");

#if !defined(WIN32) && !defined(__APPLE__)
#include <X11/Xlib.h>
#include <SDL_syswm.h>
#endif

void pasteconsole()
{
#ifdef WIN32
    UINT fmt = CF_UNICODETEXT;
    if(!IsClipboardFormatAvailable(fmt)) 
    {
        fmt = CF_TEXT;
        if(!IsClipboardFormatAvailable(fmt)) return; 
    }
    if(!OpenClipboard(NULL)) return;
    HANDLE h = GetClipboardData(fmt);
    size_t commandlen = strlen(commandbuf);
    int cblen = int(GlobalSize(h)), decoded = 0;
    ushort *cb = (ushort *)GlobalLock(h);
    switch(fmt)
    {
        case CF_UNICODETEXT:
            decoded = min(int(sizeof(commandbuf)-1-commandlen), cblen/2);
            loopi(decoded) commandbuf[commandlen++] = uni2cube(cb[i]);
            break;
        case CF_TEXT:
            decoded = min(int(sizeof(commandbuf)-1-commandlen), cblen);
            memcpy(&commandbuf[commandlen], cb, decoded);
            break;
    }    
    commandbuf[commandlen + decoded] = '\0';
    GlobalUnlock(cb);
    CloseClipboard();
#elif defined(__APPLE__)
	extern char *mac_pasteconsole(int *cblen);
    int cblen = 0;
	uchar *cb = (uchar *)mac_pasteconsole(&cblen);
    if(!cb) return;
    size_t commandlen = strlen(commandbuf);
    int decoded = decodeutf8((uchar *)&commandbuf[commandlen], int(sizeof(commandbuf)-1-commandlen), cb, cblen);
    commandbuf[commandlen + decoded] = '\0';
    free(cb);
#elif !defined(__EMSCRIPTEN__)
    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version); 
    wminfo.subsystem = SDL_SYSWM_X11;
    if(!SDL_GetWMInfo(&wminfo)) return;
    int cbsize;
    uchar *cb = (uchar *)XFetchBytes(wminfo.info.x11.display, &cbsize);
    if(!cb || !cbsize) return;
    size_t commandlen = strlen(commandbuf);
    for(uchar *cbline = cb, *cbend; commandlen + 1 < sizeof(commandbuf) && cbline < &cb[cbsize]; cbline = cbend + 1)
    {
        cbend = (uchar *)memchr(cbline, '\0', &cb[cbsize] - cbline);
        if(!cbend) cbend = &cb[cbsize];
        int cblen = int(cbend-cbline), commandmax = int(sizeof(commandbuf)-1-commandlen); 
        loopi(cblen) if((cbline[i]&0xC0) == 0x80) 
        { 
            commandlen += decodeutf8((uchar *)&commandbuf[commandlen], commandmax, cbline, cblen);
            goto nextline;
        }
        cblen = min(cblen, commandmax);
        loopi(cblen) commandbuf[commandlen++] = uni2cube(*cbline++);
    nextline:
        commandbuf[commandlen] = '\n';
        if(commandlen + 1 < sizeof(commandbuf) && cbend < &cb[cbsize]) ++commandlen;
        commandbuf[commandlen] = '\0';
    }
    XFree(cb);
#endif
}

struct hline
{
    char *buf, *action, *prompt;
    int flags;

    hline() : buf(NULL), action(NULL), prompt(NULL), flags(0) {}
    ~hline()
    {
        DELETEA(buf);
        DELETEA(action);
        DELETEA(prompt);
    }

    void restore()
    {
        copystring(commandbuf, buf);
        if(commandpos >= (int)strlen(commandbuf)) commandpos = -1;
        DELETEA(commandaction);
        DELETEA(commandprompt);
        if(action) commandaction = newstring(action);
        if(prompt) commandprompt = newstring(prompt);
        commandflags = flags;
    }

    bool shouldsave()
    {
        return strcmp(commandbuf, buf) ||
               (commandaction ? !action || strcmp(commandaction, action) : action!=NULL) ||
               (commandprompt ? !prompt || strcmp(commandprompt, prompt) : prompt!=NULL) ||
               commandflags != flags;
    }
    
    void save()
    {
        buf = newstring(commandbuf);
        if(commandaction) action = newstring(commandaction);
        if(commandprompt) prompt = newstring(commandprompt);
        flags = commandflags;
    }

    void run()
    {
        if(flags&CF_EXECUTE && buf[0]=='/') execute(buf+1);
        else if(action)
        {
            alias("commandbuf", buf);
            execute(action);
        }
        else game::toserver(buf);
    }
};
vector<hline *> history;
int histpos = 0;

VARP(maxhistory, 0, 1000, 10000);

void history_(int *n)
{
    static bool inhistory = false;
    if(!inhistory && history.inrange(*n))
    {
        inhistory = true;
        history[history.length()-*n-1]->run();
        inhistory = false;
    }
}

COMMANDN(history, history_, "i");

struct releaseaction
{
    keym *key;
    char *action;
};
vector<releaseaction> releaseactions;

const char *addreleaseaction(char *s)
{
    if(!keypressed) return NULL;
    releaseaction &ra = releaseactions.add();
    ra.key = keypressed;
    ra.action = s;
    return keypressed->name;
}

void onrelease(const char *s)
{
    addreleaseaction(newstring(s));
}

COMMAND(onrelease, "s");

void execbind(keym &k, bool isdown)
{
    loopv(releaseactions)
    {
        releaseaction &ra = releaseactions[i];
        if(ra.key==&k)
        {
            if(!isdown) execute(ra.action);
            delete[] ra.action;
            releaseactions.remove(i--);
        }
    }
    if(isdown)
    {
        int state = keym::ACTION_DEFAULT;
        if(!mainmenu)
        {
            if(editmode) state = keym::ACTION_EDITING;
            else if(player->state==CS_SPECTATOR) state = keym::ACTION_SPECTATOR;
        }
        char *&action = k.actions[state][0] ? k.actions[state] : k.actions[keym::ACTION_DEFAULT];
        keyaction = action;
        keypressed = &k;
        execute(keyaction);
        keypressed = NULL;
        if(keyaction!=action) delete[] keyaction;
    }
    k.pressed = isdown;
}

void consolekey(int code, bool isdown, int cooked)
{
    #ifdef __APPLE__
        #define MOD_KEYS (KMOD_LMETA|KMOD_RMETA) 
    #else
        #define MOD_KEYS (KMOD_LCTRL|KMOD_RCTRL)
    #endif

    if(isdown)
    {
        switch(code)
        {
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                break;

            case SDLK_HOME:
                if(strlen(commandbuf)) commandpos = 0;
                break;

            case SDLK_END:
                commandpos = -1;
                break;

            case SDLK_DELETE:
            {
                int len = (int)strlen(commandbuf);
                if(commandpos<0) break;
                memmove(&commandbuf[commandpos], &commandbuf[commandpos+1], len - commandpos);
                resetcomplete();
                if(commandpos>=len-1) commandpos = -1;
                break;
            }

            case SDLK_BACKSPACE:
            {
                int len = (int)strlen(commandbuf), i = commandpos>=0 ? commandpos : len;
                if(i<1) break;
                memmove(&commandbuf[i-1], &commandbuf[i], len - i + 1);
                resetcomplete();
                if(commandpos>0) commandpos--;
                else if(!commandpos && len<=1) commandpos = -1;
                break;
            }

            case SDLK_LEFT:
                if(commandpos>0) commandpos--;
                else if(commandpos<0) commandpos = (int)strlen(commandbuf)-1;
                break;

            case SDLK_RIGHT:
                if(commandpos>=0 && ++commandpos>=(int)strlen(commandbuf)) commandpos = -1;
                break;

            case SDLK_UP:
                if(histpos > history.length()) histpos = history.length();
                if(histpos > 0) history[--histpos]->restore(); 
                break;

            case SDLK_DOWN:
                if(histpos + 1 < history.length()) history[++histpos]->restore();
                break;

            case SDLK_TAB:
                if(commandflags&CF_COMPLETE)
                {
                    complete(commandbuf, commandflags&CF_EXECUTE ? "/" : NULL);
                    if(commandpos>=0 && commandpos>=(int)strlen(commandbuf)) commandpos = -1;
                }
                break;

            case SDLK_v:
                if(SDL_GetModState()&MOD_KEYS) { pasteconsole(); return; }
                // fall through

            default:
                resetcomplete();
                if(cooked)
                {
                    size_t len = (int)strlen(commandbuf);
                    if(len+1<sizeof(commandbuf))
                    {
                        if(commandpos<0) commandbuf[len] = cooked;
                        else
                        {
                            memmove(&commandbuf[commandpos+1], &commandbuf[commandpos], len - commandpos);
                            commandbuf[commandpos++] = cooked;
                        }
                        commandbuf[len+1] = '\0';
                    }
                }
                break;
        }
    }
    else
    {
        if(code==SDLK_RETURN || code==SDLK_KP_ENTER)
        {
            hline *h = NULL;
            if(commandbuf[0])
            {
                if(history.empty() || history.last()->shouldsave())
                {
                    if(maxhistory && history.length() >= maxhistory)
                    {
                        loopi(history.length()-maxhistory+1) delete history[i];
                        history.remove(0, history.length()-maxhistory+1);
                    }
                    history.add(h = new hline)->save();
                }
                else h = history.last();
            }
            histpos = history.length();
            inputcommand(NULL);
            if(h) h->run();
        }
        else if(code==SDLK_ESCAPE)
        {
            histpos = history.length();
            inputcommand(NULL);
        }
    }
}

extern bool menukey(int code, bool isdown, int cooked);

void keypress(int code, bool isdown, int cooked)
{
    keym *haskey = keyms.access(code);
    if(haskey && haskey->pressed) execbind(*haskey, isdown); // allow pressed keys to release
    else if(!menukey(code, isdown, cooked)) // 3D GUI mouse button intercept   
    {
        if(commandmillis >= 0) consolekey(code, isdown, cooked);
        else if(haskey) execbind(*haskey, isdown);
    }
}

void clear_console()
{
    keyms.clear();
}

static inline bool sortbinds(keym *x, keym *y)
{
    return strcmp(x->name, y->name) < 0;
}

void writebinds(stream *f)
{
    static const char *cmds[3] = { "bind", "specbind", "editbind" };
    vector<keym *> binds;
    enumerate(keyms, keym, km, binds.add(&km));
    binds.sort(sortbinds);
    loopj(3)
    {
        loopv(binds)
        {
            keym &km = *binds[i];
            if(*km.actions[j]) 
            {
                if(validateblock(km.actions[j])) f->printf("%s %s [%s]\n", cmds[j], escapestring(km.name), km.actions[j]);
                else f->printf("%s %s %s\n", cmds[j], escapestring(km.name), escapestring(km.actions[j]));
            }
        }
    }
}

// tab-completion of all idents and base maps

enum { FILES_DIR = 0, FILES_LIST };

struct fileskey
{
    int type;
    const char *dir, *ext;

    fileskey() {}
    fileskey(int type, const char *dir, const char *ext) : type(type), dir(dir), ext(ext) {}
};

struct filesval
{
    int type;
    char *dir, *ext;
    vector<char *> files;
    int millis;
    
    filesval(int type, const char *dir, const char *ext) : type(type), dir(newstring(dir)), ext(ext && ext[0] ? newstring(ext) : NULL), millis(-1) {}
    ~filesval() { DELETEA(dir); DELETEA(ext); files.deletearrays(); }

    static bool comparefiles(const char *x, const char *y) { return strcmp(x, y) < 0; }

    void update()
    {
        if(type!=FILES_DIR || millis >= commandmillis) return;
        files.deletearrays();        
        listfiles(dir, ext, files);
        files.sort(comparefiles); 
        loopv(files) if(i && !strcmp(files[i], files[i-1])) delete[] files.remove(i--);
        millis = totalmillis;
    }
};

static inline bool htcmp(const fileskey &x, const fileskey &y)
{
    return x.type==y.type && !strcmp(x.dir, y.dir) && (x.ext == y.ext || (x.ext && y.ext && !strcmp(x.ext, y.ext)));
}

static inline uint hthash(const fileskey &k)
{
    return hthash(k.dir);
}

static hashtable<fileskey, filesval *> completefiles;
static hashtable<char *, filesval *> completions;

int completesize = 0;
string lastcomplete;

void resetcomplete() { completesize = 0; }

void addcomplete(char *command, int type, char *dir, char *ext)
{
    if(identflags&IDF_OVERRIDDEN)
    {
        conoutf(CON_ERROR, "cannot override complete %s", command);
        return;
    }
    if(!dir[0])
    {
        filesval **hasfiles = completions.access(command);
        if(hasfiles) *hasfiles = NULL;
        return;
    }
    if(type==FILES_DIR)
    {
        int dirlen = (int)strlen(dir);
        while(dirlen > 0 && (dir[dirlen-1] == '/' || dir[dirlen-1] == '\\'))
            dir[--dirlen] = '\0';
        if(ext)
        {
            if(strchr(ext, '*')) ext[0] = '\0';
            if(!ext[0]) ext = NULL;
        }
    }
    fileskey key(type, dir, ext);
    filesval **val = completefiles.access(key);
    if(!val)
    {
        filesval *f = new filesval(type, dir, ext);
        if(type==FILES_LIST) explodelist(dir, f->files); 
        val = &completefiles[fileskey(type, f->dir, f->ext)];
        *val = f;
    }
    filesval **hasfiles = completions.access(command);
    if(hasfiles) *hasfiles = *val;
    else completions[newstring(command)] = *val;
}

void addfilecomplete(char *command, char *dir, char *ext)
{
    addcomplete(command, FILES_DIR, dir, ext);
}

void addlistcomplete(char *command, char *list)
{
    addcomplete(command, FILES_LIST, list, NULL);
}

COMMANDN(complete, addfilecomplete, "sss");
COMMANDN(listcomplete, addlistcomplete, "ss");

void complete(char *s, const char *cmdprefix)
{
    int cmdlen = 0;
    if(cmdprefix)
    {
        cmdlen = strlen(cmdprefix);
        if(strncmp(s, cmdprefix, cmdlen))
        {
            defformatstring(cmd)("%s%s", cmdprefix, s);
            copystring(s, cmd);
        }
    }
    if(!s[cmdlen]) return;
    if(!completesize) { completesize = (int)strlen(&s[cmdlen]); lastcomplete[0] = '\0'; }

    filesval *f = NULL;
    if(completesize)
    {
        char *end = strchr(&s[cmdlen], ' ');
        if(end)
        {
            string command;
            copystring(command, &s[cmdlen], min(size_t(end-&s[cmdlen]+1), sizeof(command)));
            filesval **hasfiles = completions.access(command);
            if(hasfiles) f = *hasfiles;
        }
    }

    const char *nextcomplete = NULL;
    string prefix;
    if(f) // complete using filenames
    {
        int commandsize = strchr(&s[cmdlen], ' ')+1-s;
        copystring(prefix, s, min(size_t(commandsize+1), sizeof(prefix)));
        f->update();
        loopv(f->files)
        {
            if(strncmp(f->files[i], &s[commandsize], completesize+cmdlen-commandsize)==0 &&
               strcmp(f->files[i], lastcomplete) > 0 && (!nextcomplete || strcmp(f->files[i], nextcomplete) < 0))
                nextcomplete = f->files[i];
        }
    }
    else // complete using command names
    {
        if(cmdprefix) copystring(prefix, cmdprefix); else prefix[0] = '\0';
        enumerate(idents, ident, id,
            if(strncmp(id.name, &s[cmdlen], completesize)==0 &&
               strcmp(id.name, lastcomplete) > 0 && (!nextcomplete || strcmp(id.name, nextcomplete) < 0))
                nextcomplete = id.name;
        );
    }
    if(nextcomplete)
    {
        formatstring(s)("%s%s", prefix, nextcomplete);
        copystring(lastcomplete, nextcomplete);
    }
    else lastcomplete[0] = '\0';
}

static inline bool sortcompletions(const char *x, const char *y)
{
    return strcmp(x, y) < 0;
}

void writecompletions(stream *f)
{
    vector<char *> cmds;
    enumeratekt(completions, char *, k, filesval *, v, { if(v) cmds.add(k); });
    cmds.sort(sortcompletions);
    loopv(cmds)
    {
        char *k = cmds[i];
        filesval *v = completions[k];
        if(v->type==FILES_LIST) 
        {
            if(validateblock(v->dir)) f->printf("listcomplete %s [%s]\n", escapeid(k), v->dir);
            else f->printf("listcomplete %s %s\n", escapeid(k), escapestring(v->dir));
        }
        else f->printf("complete %s %s %s\n", escapeid(k), escapestring(v->dir), escapestring(v->ext ? v->ext : "*"));
    }
}

