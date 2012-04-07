
struct editline
{
    enum { CHUNKSIZE = 256 };

    char *text;
    int len, maxlen;

    editline() : text(NULL), len(0), maxlen(0) {}
    editline(const char *init) : text(NULL), len(0), maxlen(0)
    {
        set(init);
    }

    bool empty() { return len <= 0; }

    void clear()
    {
        DELETEA(text);
        len = maxlen = 0;
    }

    bool grow(int total, const char *fmt = "", ...)
    {
        if(total + 1 <= maxlen) return false;
        maxlen = (total + CHUNKSIZE) - total%CHUNKSIZE;
        char *newtext = new char[maxlen];
        if(fmt)
        {
            va_list args;
            va_start(args, fmt);
            _vsnprintf(newtext, maxlen, fmt, args);
            va_end(args);
        }
        DELETEA(text);
        text = newtext;
        return true;
    }

    void set(const char *str, int slen = -1)
    {
        if(slen < 0)
        {
            slen = strlen(str);
            if(!grow(slen, "%s", str)) memcpy(text, str, slen + 1);
        }
        else
        {
            grow(slen);
            memcpy(text, str, slen);
            text[slen] = '\0';
        }
        len = slen;
    }

    void prepend(const char *str)
    {
        int slen = strlen(str);
        if(!grow(slen + len, "%s%s", str, text ? text : ""))
        {
            memmove(&text[slen], text, len + 1);
            memcpy(text, str, slen + 1);
        }
        len += slen;
    }

    void append(const char *str)
    {
        int slen = strlen(str);
        if(!grow(len + slen, "%s%s", text ? text : "", str)) memcpy(&text[len], str, slen + 1);
        len += slen;
    }

    bool read(stream *f, int chop = -1)
    {
        if(chop < 0) chop = INT_MAX; else chop++;
        set("");
        while(len + 1 < chop && f->getline(&text[len], min(maxlen, chop) - len))
        {
            len += strlen(&text[len]);
            if(len > 0 && text[len-1] == '\n')
            {
                text[--len] = '\0';
                return true;
            }
            if(len + 1 >= maxlen && len + 1 < chop) grow(len + CHUNKSIZE, "%s", text);
        }
        if(len + 1 >= chop)
        {
            char buf[CHUNKSIZE];
            while(f->getline(buf, sizeof(buf)))
            {
                int blen = strlen(buf);
                if(blen > 0 && buf[blen-1] == '\n') return true;
            }
        }
        return len > 0;
    }

    void del(int start, int count)
    {
        if(!text) return;
        if(start < 0) { count += start; start = 0; } 
        if(count <= 0 || start >= len) return;
        if(start + count > len) count = len - start - 1;
        memmove(&text[start], &text[start+count], len + 1 - (start + count));
        len -= count;
    }

    void chop(int newlen)
    {
        if(!text) return;
        len = clamp(newlen, 0, len);
        text[len] = '\0';
    }

    void insert(char *str, int start, int count = 0)
    {
        if(count <= 0) count = strlen(str);
        start = clamp(start, 0, len);
        grow(len + count, "%s", text ? text : "");
        memmove(&text[start + count], &text[start], len - start + 1);
        memcpy(&text[start], str, count);
        len += count;
    }

    void combinelines(vector<editline> &src)
    {
        if(src.empty()) set("");
        else loopv(src)
        {
            if(i) append("\n");
            if(!i) set(src[i].text, src[i].len);
            else insert(src[i].text, len, src[i].len);
        }
    }
};
        
struct editor 
{
    int mode; //editor mode - 1= keep while focused, 2= keep while used in gui, 3= keep forever (i.e. until mode changes)
    bool active, rendered;
    const char *name;
    const char *filename;

    int cx, cy; // cursor position - ensured to be valid after a region() or currentline()
    int mx, my; // selection mark, mx=-1 if following cursor - avoid direct access, instead use region()
    int maxx, maxy; // maxy=-1 if unlimited lines, 1 if single line editor
    
    int scrolly; // vertical scroll offset
    
    bool linewrap;
    int pixelwidth; // required for up/down/hit/draw/bounds
    int pixelheight; // -1 for variable sized, i.e. from bounds()
    
    vector<editline> lines; // MUST always contain at least one line!
        
    editor(const char *name, int mode, const char *initval) : 
        mode(mode), active(true), rendered(false), name(newstring(name)), filename(NULL),
        cx(0), cy(0), mx(-1), maxx(-1), maxy(-1), scrolly(0), linewrap(false), pixelwidth(-1), pixelheight(-1)
    {
        //printf("editor %08x '%s'\n", this, name);
        lines.add().set(initval ? initval : "");
    }
    
    ~editor()
    {
        //printf("~editor %08x '%s'\n", this, name);
        DELETEA(name);
        DELETEA(filename);
        clear(NULL);
    }
        
    void clear(const char *init = "")
    {
        cx = cy = 0;
        mark(false);
        loopv(lines) lines[i].clear();
        lines.shrink(0);
        if(init) lines.add().set(init);
    }
    
    void setfile(const char *fname)
    {
        DELETEA(filename); 
        if(fname) filename = newstring(fname);
    }
    
    void load()
    {
        if(!filename) return;
        clear(NULL);
        stream *file = openutf8file(filename, "r");
        if(file) 
        {
            while(lines.add().read(file, maxx) && (maxy < 0 || lines.length() <= maxy));
            lines.pop().clear();
            delete file;
        }
        if(lines.empty()) lines.add().set("");
    }
    
    void save()
    {
        if(!filename) return;
        stream *file = openutf8file(filename, "w");
        if(!file) return;
        loopv(lines) file->putline(lines[i].text);
        delete file;
    }
   
    void mark(bool enable) 
    {
        mx = (enable) ? cx : -1;
        my = cy;
    }
        
    void selectall()
    {
        mx = my = INT_MAX;
        cx = cy = 0;
    }
    
    // constrain results to within buffer - s=start, e=end, return true if a selection range
    // also ensures that cy is always within lines[] and cx is valid
    bool region(int &sx, int &sy, int &ex, int &ey)
    {
        int n = lines.length(); 
        assert(n != 0);
        if(cy < 0) cy = 0; else if(cy >= n) cy = n-1;
        int len = lines[cy].len;
        if(cx < 0) cx = 0; else if(cx > len) cx = len;
        if(mx >= 0) 
        {
            if(my < 0) my = 0; else if(my >= n) my = n-1;
            len = lines[my].len;
            if(mx > len) mx = len;
        }
        sx = (mx >= 0) ? mx : cx;
        sy = (mx >= 0) ? my : cy;
        ex = cx;
        ey = cy;
        if(sy > ey) { swap(sy, ey); swap(sx, ex); }
        else if(sy==ey && sx > ex) swap(sx, ex);        
        return (sx != ex) || (sy != ey);
    }
   
    bool region() { int sx, sy, ex, ey; return region(sx, sy, ex, ey); }

    // also ensures that cy is always within lines[] and cx is valid
    editline &currentline()
    {
        int n = lines.length(); 
        assert(n != 0);
        if(cy < 0) cy = 0; else if(cy >= n) cy = n-1;
        if(cx < 0) cx = 0; else if(cx > lines[cy].len) cx = lines[cy].len;
        return lines[cy];
    }

    void copyselectionto(editor *b)
    {
        if(b==this) return;

        b->clear(NULL);
        int sx, sy, ex, ey;
        region(sx, sy, ex, ey);
        loopi(1+ey-sy)
        {
            if(b->maxy != -1 && b->lines.length() >= b->maxy) break;
            int y = sy+i;
            char *line = lines[y].text;
            int len = lines[y].len;
            if(y == sy && y == ey)
            {
                line += sx;
                len = ex - sx;
            }
            else if(y == sy) line += sx;
            else if(y == ey) len = ex;
            b->lines.add().set(line, len);
        }
        if(b->lines.empty()) b->lines.add().set("");
    }

    char *tostring()
    {
        int len = 0;
        loopv(lines) len += lines[i].len + 1;
        char *str = newstring(len);
        int offset = 0;
        loopv(lines)
        {
            editline &l = lines[i];
            memcpy(&str[offset], l.text, l.len);
            offset += l.len;
            str[offset++] = '\n';
        }
        str[offset] = '\0';
        return str;
    }

    char *selectiontostring()
    {
        vector<char> buf;
        int sx, sy, ex, ey;
        region(sx, sy, ex, ey);
        loopi(1+ey-sy)
        {
            int y = sy+i;
            char *line = lines[y].text;
            int len = lines[y].len;
            if(y == sy && y == ey)
            {
                line += sx;
                len = ex - sx;
            }
            else if(y == sy) line += sx;
            else if(y == ey) len = ex;
            buf.put(line, len);
            buf.add('\n');
        }
        buf.add('\0');
        return newstring(buf.getbuf(), buf.length()-1);
    }

    void removelines(int start, int count)
    {
        loopi(count) lines[start+i].clear();
        lines.remove(start, count);
    }
            
    void del() // removes the current selection (if any)
    {
        int sx, sy, ex, ey;
        if(!region(sx, sy, ex, ey)) 
        { 
            mark(false); 
            return; 
        }
        if(sy == ey) 
        {
            if(sx == 0 || ex == lines[ey].len) removelines(sy, 1);
            else lines[sy].del(sx, ex - sx);
        }
        else
        {
            if(ey > sy+2) { removelines(sy+1, ey-(sy+2)); ey = sy+1; }
            if(ex == lines[ey].len) removelines(ey, 1); else lines[ey].del(0, ex);
            if(sx == 0) removelines(sy, 1); else lines[sy].del(sx, lines[sy].len - sx);
        }
        if(lines.empty()) lines.add().set("");
        mark(false);
        cx = sx;
        cy = sy;
    }
        
    void insert(char ch) 
    {
        del();
        editline &current = currentline();
        if(ch == '\n') 
        {
            if(maxy == -1 || cy < maxy-1)
            {
                editline newline(&current.text[cx]);
                current.chop(cx);
                cy = min(lines.length(), cy+1);
                lines.insert(cy, newline);
            }
            else current.chop(cx);
            cx = 0;
        } 
        else
        {
            int len = current.len;
            if(maxx >= 0 && len > maxx-1) len = maxx-1;
            if(cx <= len) current.insert(&ch, cx++, 1);
        }
    }

    void insert(const char *s)
    {
        while(*s) insert(*s++);
    }

    void insertallfrom(editor *b) 
    {   
        if(b==this) return;

        del();
        
        if(b->lines.length() == 1 || maxy == 1) 
        {
            editline &current = currentline();
            char *str = b->lines[0].text;            
            int slen = b->lines[0].len;
            if(maxx >= 0 && b->lines[0].len + cx > maxx) slen = maxx-cx;
            if(slen > 0) 
            {
                int len = current.len;
                if(maxx >= 0 && slen + cx + len > maxx) len = max(0, maxx-(cx+slen));
                current.insert(str, cx, slen); 
                cx += slen;
            }
        } 
        else 
        {
            loopv(b->lines) 
            {   
                if(!i) 
                {
                    lines[cy++].append(b->lines[i].text);
                }
                else if(i >= b->lines.length())
                {
                    cx = b->lines[i].len;
                    lines[cy].prepend(b->lines[i].text);
                }
                else if(maxy < 0 || lines.length() < maxy) lines.insert(cy++, editline(b->lines[i].text));
            }
        }
    }

    void key(int code, int cooked)
    {
        switch(code) 
        {
            case SDLK_UP:
                if(linewrap) 
                {
                    int x, y; 
                    char *str = currentline().text;
                    text_pos(str, cx+1, x, y, pixelwidth);
                    if(y > 0) { cx = text_visible(str, x, y-FONTH, pixelwidth); break; }
                }
                cy--;
                break;
            case SDLK_DOWN:
                if(linewrap) 
                {
                    int x, y, width, height;
                    char *str = currentline().text;
                    text_pos(str, cx, x, y, pixelwidth);
                    text_bounds(str, width, height, pixelwidth);
                    y += FONTH;
                    if(y < height) { cx = text_visible(str, x, y, pixelwidth); break; }
                }
                cy++;
                break;
            case -4:
                cy--;
                break;
            case -5:
                cy++;
                break;
            case SDLK_PAGEUP:
                cy-=pixelheight/FONTH;
                break;
            case SDLK_PAGEDOWN:
                cy+=pixelheight/FONTH;
                break;
            case SDLK_HOME:
                cx = cy = 0;
                break;
            case SDLK_END:
                cx = cy = INT_MAX;
                break;
            case SDLK_LEFT:
                cx--;
                break;
            case SDLK_RIGHT:
                cx++;
                break;
            case SDLK_DELETE:
            {
                del();
                editline &current = currentline();
                if(cx < current.len) current.del(cx, 1);
                else if(cy < lines.length()-1)
                {   //combine with next line
                    current.append(lines[cy+1].text);
                    removelines(cy+1, 1);
                }
                break;
            }
            case SDLK_BACKSPACE:
            {
                del();
                editline &current = currentline();
                if(cx > 0) current.del(--cx, 1); 
                else if(cy > 0)
                {   //combine with previous line
                    cx = lines[cy-1].len;
                    lines[cy-1].append(current.text);
                    removelines(cy--, 1);
                }
                break;
            }
            case SDLK_LSHIFT:
            case SDLK_RSHIFT:
                break;
            case SDLK_RETURN:    
                cooked = '\n';
                // fall through
            default:
                insert(cooked);
                break;
        }
    }

    void hit(int hitx, int hity, bool dragged)
    {
        int maxwidth = linewrap?pixelwidth:-1;
        int h = 0;
        for(int i = scrolly; i < lines.length(); i++)
        {
            int width, height;
            text_bounds(lines[i].text, width, height, maxwidth);
            if(h + height > pixelheight) break;
            
            if(hity >= h && hity <= h+height) 
            {
                int x = text_visible(lines[i].text, hitx, hity-h, maxwidth); 
                if(dragged) { mx = x; my = i; } else { cx = x; cy = i; };
                break;
            }
           h+=height;
        }
    }

    int limitscrolly()
    {
        int maxwidth = linewrap?pixelwidth:-1;
        int slines = lines.length();
        for(int ph = pixelheight; slines > 0 && ph > 0;)
        {
            int width, height;
            text_bounds(lines[slines-1].text, width, height, maxwidth);
            if(height > ph) break;
            ph -= height;
            slines--;
        }
        return slines;
    }

    void draw(int x, int y, int color, bool hit)
    {
        int maxwidth = linewrap?pixelwidth:-1;
        
        int sx, sy, ex, ey;
        bool selection = region(sx, sy, ex, ey);
        
        // fix scrolly so that <cx, cy> is always on screen
        if(cy < scrolly) scrolly = cy;
        else 
        {
            if(scrolly < 0) scrolly = 0;
            int h = 0;
            for(int i = cy; i >= scrolly; i--)
            {
                int width, height;
                text_bounds(lines[i].text, width, height, maxwidth);
                if(h + height > pixelheight) { scrolly = i+1; break; }
                h += height;
            }
        }
        
        if(selection)
        {
            // convert from cursor coords into pixel coords
            int psx, psy, pex, pey;
            text_pos(lines[sy].text, sx, psx, psy, maxwidth);
            text_pos(lines[ey].text, ex, pex, pey, maxwidth);
            int maxy = lines.length();
            int h = 0;
            for(int i = scrolly; i < maxy; i++)
            {
                int width, height;
                text_bounds(lines[i].text, width, height, maxwidth);                
                if(h + height > pixelheight) { maxy = i; break; }
                if(i == sy) psy += h;
                if(i == ey) { pey += h; break; }
                h += height;
            }
            maxy--;
            
            if(ey >= scrolly && sy <= maxy) 
            {
                // crop top/bottom within window
                if(sy < scrolly) { sy = scrolly; psy = 0; psx = 0; }
                if(ey > maxy) { ey = maxy; pey = pixelheight - FONTH; pex = pixelwidth; }

                notextureshader->set();
                glDisable(GL_TEXTURE_2D);
                glColor3ub(0xA0, 0x80, 0x80);
                glBegin(GL_QUADS);
                if(psy == pey) 
                {
                    glVertex2f(x+psx, y+psy);
                    glVertex2f(x+pex, y+psy);
                    glVertex2f(x+pex, y+pey+FONTH);
                    glVertex2f(x+psx, y+pey+FONTH);
                } 
                else 
                {   glVertex2f(x+psx,        y+psy);
                    glVertex2f(x+psx,        y+psy+FONTH);
                    glVertex2f(x+pixelwidth, y+psy+FONTH);
                    glVertex2f(x+pixelwidth, y+psy);
                    if(pey-psy > FONTH) 
                    {
                        glVertex2f(x,            y+psy+FONTH);
                        glVertex2f(x+pixelwidth, y+psy+FONTH);
                        glVertex2f(x+pixelwidth, y+pey);
                        glVertex2f(x,            y+pey);
                    }
                    glVertex2f(x,     y+pey);
                    glVertex2f(x,     y+pey+FONTH);
                    glVertex2f(x+pex, y+pey+FONTH);
                    glVertex2f(x+pex, y+pey);
                }
                glEnd();
                glEnable(GL_TEXTURE_2D);
                defaultshader->set();
            }
        }
    
        int h = 0;
        for(int i = scrolly; i < lines.length(); i++)
        {
            int width, height;
            text_bounds(lines[i].text, width, height, maxwidth);
            if(h + height > pixelheight) break;
            
            draw_text(lines[i].text, x, y+h, color>>16, (color>>8)&0xFF, color&0xFF, 0xFF, hit&&(cy==i)?cx:-1, maxwidth);
            if(linewrap && height > FONTH) // line wrap indicator
            {   
                notextureshader->set();
                glDisable(GL_TEXTURE_2D);
                glColor3ub(0x80, 0xA0, 0x80);
                glBegin(GL_TRIANGLE_STRIP);
                glVertex2f(x,         y+h+FONTH);
                glVertex2f(x,         y+h+height);
                glVertex2f(x-FONTW/2, y+h+FONTH);
                glVertex2f(x-FONTW/2, y+h+height);
                glEnd();
                glEnable(GL_TEXTURE_2D);
                defaultshader->set();
            }
            h+=height;
        }
    }
};

// a 'stack' where the last is the current focused editor
static vector <editor*> editors;

static editor *currentfocus() { return editors.length() ? editors.last() : NULL; }

static void readyeditors() 
{
    loopv(editors) editors[i]->active = (editors[i]->mode==EDITORFOREVER);
}

static void flusheditors() 
{
    loopvrev(editors) if(!editors[i]->active) 
    {
        editor *e = editors.remove(i);
        DELETEP(e);
    }
}

static editor *useeditor(const char *name, int mode, bool focus, const char *initval = NULL) 
{
    loopv(editors) if(strcmp(editors[i]->name, name) == 0) 
    {
        editor *e = editors[i];
        if(focus) { editors.add(e); editors.remove(i); } // re-position as last
        e->active = true;
        return e;
    }
    editor *e = new editor(name, mode, initval);
    if(focus) editors.add(e); else editors.insert(0, e); 
    return e;
}


#define TEXTCOMMAND(f, s, d, body) ICOMMAND(f, s, d,\
    editor *top = currentfocus();\
    if(!top) return;\
    body\
)

ICOMMAND(textlist, "", (), // @DEBUG return list of all the editors
    vector<char> s;
    loopv(editors)
    {   
        if(i > 0) s.put(", ", 2);
        s.put(editors[i]->name, strlen(editors[i]->name));
    }
    s.add('\0');
    result(s.getbuf());
);
TEXTCOMMAND(textshow, "", (), // @DEBUG return the start of the buffer
    editline line;
    line.combinelines(top->lines);
    result(line.text);
    line.clear();
);
ICOMMAND(textfocus, "si", (char *name, int *mode), // focus on a (or create a persistent) specific editor, else returns current name
    if(*name) useeditor(name, *mode<=0 ? EDITORFOREVER : *mode, true);
    else if(editors.length() > 0) result(editors.last()->name);
);
TEXTCOMMAND(textprev, "", (), editors.insert(0, top); editors.pop();); // return to the previous editor
TEXTCOMMAND(textmode, "i", (int *m), // (1= keep while focused, 2= keep while used in gui, 3= keep forever (i.e. until mode changes)) topmost editor, return current setting if no args
    if(*m) top->mode = *m;
    else intret(top->mode);
);
TEXTCOMMAND(textsave, "s", (char *file),  // saves the topmost (filename is optional)
    if(*file) top->setfile(path(file, true)); 
    top->save();
);  
TEXTCOMMAND(textload, "s", (char *file), // loads into the topmost editor, returns filename if no args
    if(*file)
    {
        top->setfile(path(file, true));
        top->load();
    }
    else if(top->filename) result(top->filename);
);
TEXTCOMMAND(textinit, "sss", (char *name, char *file, char *initval), // loads into named editor if no file assigned and editor has been rendered
{
    editor *e = NULL;
    loopv(editors) if(!strcmp(editors[i]->name, name)) { e = editors[i]; break; }
    if(e && e->rendered && !e->filename && *file && (e->lines.empty() || (e->lines.length() == 1 && !strcmp(e->lines[0].text, initval))))
    {
        e->setfile(path(file, true));
        e->load();
    }
});
 
#define PASTEBUFFER "#pastebuffer"

TEXTCOMMAND(textcopy, "", (), editor *b = useeditor(PASTEBUFFER, EDITORFOREVER, false); top->copyselectionto(b););
TEXTCOMMAND(textpaste, "", (), editor *b = useeditor(PASTEBUFFER, EDITORFOREVER, false); top->insertallfrom(b););
TEXTCOMMAND(textmark, "i", (int *m),  // (1=mark, 2=unmark), return current mark setting if no args
    if(*m) top->mark(*m==1);
    else intret(top->region() ? 1 : 2);
);
TEXTCOMMAND(textselectall, "", (), top->selectall(););
TEXTCOMMAND(textclear, "", (), top->clear(););
TEXTCOMMAND(textcurrentline, "",  (), result(top->currentline().text););

TEXTCOMMAND(textexec, "i", (int *selected), // execute script commands from the buffer (0=all, 1=selected region only)
    char *script = *selected ? top->selectiontostring() : top->tostring();
    execute(script);
    delete[] script;
);

