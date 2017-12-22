#include "cube.h"

///////////////////////// character conversion ///////////////

#define CUBECTYPE(s, p, d, a, A, u, U) \
    0, U, U, U, U, U, U, U, U, s, s, s, s, s, U, U, \
    U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, \
    s, p, p, p, p, p, p, p, p, p, p, p, p, p, p, p, \
    d, d, d, d, d, d, d, d, d, d, p, p, p, p, p, p, \
    p, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, \
    A, A, A, A, A, A, A, A, A, A, A, p, p, p, p, p, \
    p, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, \
    a, a, a, a, a, a, a, a, a, a, a, p, p, p, p, U, \
    U, u, u, u, u, u, u, u, u, u, u, u, u, u, u, u, \
    u, u, u, u, u, u, u, u, u, u, u, u, u, u, u, U, \
    u, U, u, U, u, U, u, U, u, U, u, U, u, U, u, U, \
    u, U, u, U, u, U, u, U, u, U, u, U, u, U, u, U, \
    u, U, u, U, u, U, u, U, U, u, U, u, U, u, U, U, \
    U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, \
    U, U, U, U, u, u, u, u, u, u, u, u, u, u, u, u, \
    u, u, u, u, u, u, u, u, u, u, u, u, u, u, U, u

extern const uchar cubectype[256] =
{
    CUBECTYPE(CT_SPACE,
              CT_PRINT,
              CT_PRINT|CT_DIGIT,
              CT_PRINT|CT_ALPHA|CT_LOWER,
              CT_PRINT|CT_ALPHA|CT_UPPER,
              CT_PRINT|CT_UNICODE|CT_ALPHA|CT_LOWER,
              CT_PRINT|CT_UNICODE|CT_ALPHA|CT_UPPER)
};
extern const int cube2unichars[256] =
{
    0, 192, 193, 194, 195, 196, 197, 198, 199, 9, 10, 11, 12, 13, 200, 201,
    202, 203, 204, 205, 206, 207, 209, 210, 211, 212, 213, 214, 216, 217, 218, 219,
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
    96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 220,
    221, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237,
    238, 239, 241, 242, 243, 244, 245, 246, 248, 249, 250, 251, 252, 253, 255, 0x104,
    0x105, 0x106, 0x107, 0x10C, 0x10D, 0x10E, 0x10F, 0x118, 0x119, 0x11A, 0x11B, 0x11E, 0x11F, 0x130, 0x131, 0x141,
    0x142, 0x143, 0x144, 0x147, 0x148, 0x150, 0x151, 0x152, 0x153, 0x158, 0x159, 0x15A, 0x15B, 0x15E, 0x15F, 0x160,
    0x161, 0x164, 0x165, 0x16E, 0x16F, 0x170, 0x171, 0x178, 0x179, 0x17A, 0x17B, 0x17C, 0x17D, 0x17E, 0x404, 0x411,
    0x413, 0x414, 0x416, 0x417, 0x418, 0x419, 0x41B, 0x41F, 0x423, 0x424, 0x426, 0x427, 0x428, 0x429, 0x42A, 0x42B,
    0x42C, 0x42D, 0x42E, 0x42F, 0x431, 0x432, 0x433, 0x434, 0x436, 0x437, 0x438, 0x439, 0x43A, 0x43B, 0x43C, 0x43D,
    0x43F, 0x442, 0x444, 0x446, 0x447, 0x448, 0x449, 0x44A, 0x44B, 0x44C, 0x44D, 0x44E, 0x44F, 0x454, 0x490, 0x491
};
extern const int uni2cubeoffsets[8] =
{
    0, 256, 658, 658, 512, 658, 658, 658
};
extern const uchar uni2cubechars[878] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 10, 11, 12, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
    96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 3, 4, 5, 6, 7, 8, 14, 15, 16, 17, 18, 19, 20, 21, 0, 22, 23, 24, 25, 26, 27, 0, 28, 29, 30, 31, 127, 128, 0, 129,
    130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 0, 146, 147, 148, 149, 150, 151, 0, 152, 153, 154, 155, 156, 157, 0, 158,
    0, 0, 0, 0, 159, 160, 161, 162, 0, 0, 0, 0, 163, 164, 165, 166, 0, 0, 0, 0, 0, 0, 0, 0, 167, 168, 169, 170, 0, 0, 171, 172,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 173, 174, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 175, 176, 177, 178, 0, 0, 179, 180, 0, 0, 0, 0, 0, 0, 0, 181, 182, 183, 184, 0, 0, 0, 0, 185, 186, 187, 188, 0, 0, 189, 190,
    191, 192, 0, 0, 193, 194, 0, 0, 0, 0, 0, 0, 0, 0, 195, 196, 197, 198, 0, 0, 0, 0, 0, 0, 199, 200, 201, 202, 203, 204, 205, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 17, 0, 0, 206, 83, 73, 21, 74, 0, 0, 0, 0, 0, 0, 0, 65, 207, 66, 208, 209, 69, 210, 211, 212, 213, 75, 214, 77, 72, 79, 215,
    80, 67, 84, 216, 217, 88, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 97, 228, 229, 230, 231, 101, 232, 233, 234, 235, 236, 237, 238, 239, 111, 240,
    112, 99, 241, 121, 242, 120, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 0, 141, 0, 0, 253, 115, 105, 145, 106, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 254, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

int decodeutf8(uchar *dstbuf, int dstlen, uchar *srcbuf, int srclen, int *carry)
{
    uchar *dst = dstbuf, *dstend = &dstbuf[dstlen], *src = srcbuf, *srcend = &srcbuf[srclen];
    if(dstbuf == srcbuf)
    {
        int len = min(dstlen, srclen);
        for(uchar *end4 = &srcbuf[len&~3]; src < end4; src += 4) if(*(int *)src & 0x80808080) goto decode;
        for(uchar *end = &srcbuf[len]; src < end; src++) if(*src & 0x80) goto decode;
        if(carry) *carry += len;
        return len;
    }

decode:
    dst += src - srcbuf;
    while(src < srcend && dst < dstend)
    {
        int c = *src++;
        if(c < 0x80) *dst++ = c;
        else if(c >= 0xC0)
        {
            int uni;
            if(c >= 0xE0)
            {
                if(c >= 0xF0)
                {
                    if(c >= 0xF8)
                    {
                        if(c >= 0xFC)
                        {
                            if(c >= 0xFE) continue;
                            uni = c&1; if(srcend - src < 5) break;
                            c = *src; if((c&0xC0) != 0x80) continue; src++; uni = (uni<<6) | (c&0x3F);
                        }
                        else { uni = c&3; if(srcend - src < 4) break; }
                        c = *src; if((c&0xC0) != 0x80) continue; src++; uni = (uni<<6) | (c&0x3F);
                    }
                    else { uni = c&7; if(srcend - src < 3) break; }
                    c = *src; if((c&0xC0) != 0x80) continue; src++; uni = (uni<<6) | (c&0x3F);
                }
                else { uni = c&0xF; if(srcend - src < 2) break; }
                c = *src; if((c&0xC0) != 0x80) continue; src++; uni = (uni<<6) | (c&0x3F);
            }
            else { uni = c&0x1F; if(srcend - src < 1) break; }
            c = *src; if((c&0xC0) != 0x80) continue; src++; uni = (uni<<6) | (c&0x3F);
            c = uni2cube(uni);
            if(!c) continue;
            *dst++ = c;
        }
    }
    if(carry) *carry += src - srcbuf;
    return dst - dstbuf;
}

int encodeutf8(uchar *dstbuf, int dstlen, uchar *srcbuf, int srclen, int *carry)
{
    uchar *dst = dstbuf, *dstend = &dstbuf[dstlen], *src = srcbuf, *srcend = &srcbuf[srclen];
    if(src < srcend && dst < dstend) do
    {
        int uni = cube2uni(*src);
        if(uni <= 0x7F)
        {
            if(dst >= dstend) goto done;
            uchar *end = min(srcend, &src[dstend-dst]);
            do 
            { 
                *dst++ = uni; 
                if(++src >= end) goto done; 
                uni = cube2uni(*src); 
            } 
            while(uni <= 0x7F);
        }
        if(uni <= 0x7FF) { if(dst + 2 > dstend) goto done; *dst++ = 0xC0 | (uni>>6); goto uni2; }
        else if(uni <= 0xFFFF) { if(dst + 3 > dstend) goto done; *dst++ = 0xE0 | (uni>>12); goto uni3; }
        else if(uni <= 0x1FFFFF) { if(dst + 4 > dstend) goto done; *dst++ = 0xF0 | (uni>>18); goto uni4; }
        else if(uni <= 0x3FFFFFF) { if(dst + 5 > dstend) goto done; *dst++ = 0xF8 | (uni>>24); goto uni5; }
        else if(uni <= 0x7FFFFFFF) { if(dst + 6 > dstend) goto done; *dst++ = 0xFC | (uni>>30); goto uni6; }
        else goto uni1;
    uni6: *dst++ = 0x80 | ((uni>>24)&0x3F);
    uni5: *dst++ = 0x80 | ((uni>>18)&0x3F);
    uni4: *dst++ = 0x80 | ((uni>>12)&0x3F);
    uni3: *dst++ = 0x80 | ((uni>>6)&0x3F);
    uni2: *dst++ = 0x80 | (uni&0x3F);
    uni1:;
    } 
    while(++src < srcend);

done:
    if(carry) *carry += src - srcbuf;
    return dst - dstbuf;
}

///////////////////////// file system ///////////////////////

#ifdef WIN32
#include <shlobj.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#endif

string homedir = "";
vector<char *> packagedirs;

char *makerelpath(const char *dir, const char *file, const char *prefix, const char *cmd)
{
    static string tmp;
    if(prefix) copystring(tmp, prefix);
    else tmp[0] = '\0';
    if(file[0]=='<')
    {
        const char *end = strrchr(file, '>');
        if(end)
        {
            size_t len = strlen(tmp);
            copystring(&tmp[len], file, min(sizeof(tmp)-len, size_t(end+2-file)));
            file = end+1;
        }
    }
    if(cmd) concatstring(tmp, cmd);
    if(dir)
    {
        defformatstring(pname)("%s/%s", dir, file);
        concatstring(tmp, pname);
    }
    else concatstring(tmp, file);
    return tmp;
}


char *path(char *s)
{
    for(char *curpart = s;;)
    {
        char *endpart = strchr(curpart, '&');
        if(endpart) *endpart = '\0';
        if(curpart[0]=='<')
        {
            char *file = strrchr(curpart, '>');
            if(!file) return s;
            curpart = file+1;
        }
        for(char *t = curpart; (t = strpbrk(t, "/\\")); *t++ = PATHDIV);
        for(char *prevdir = NULL, *curdir = curpart;;)
        {
            prevdir = curdir[0]==PATHDIV ? curdir+1 : curdir;
            curdir = strchr(prevdir, PATHDIV);
            if(!curdir) break;
            if(prevdir+1==curdir && prevdir[0]=='.')
            {
                memmove(prevdir, curdir+1, strlen(curdir+1)+1);
                curdir = prevdir;
            }
            else if(curdir[1]=='.' && curdir[2]=='.' && curdir[3]==PATHDIV)
            {
                if(prevdir+2==curdir && prevdir[0]=='.' && prevdir[1]=='.') continue;
                memmove(prevdir, curdir+4, strlen(curdir+4)+1);
                curdir = prevdir;
            }
        }
        if(endpart)
        {
            *endpart = '&';
            curpart = endpart+1;
        }
        else break;
    }
    return s;
}

char *path(const char *s, bool copy)
{
    static string tmp;
    copystring(tmp, s);
    path(tmp);
    return tmp;
}

const char *parentdir(const char *directory)
{
    const char *p = directory + strlen(directory);
    while(p > directory && *p != '/' && *p != '\\') p--;
    static string parent;
    size_t len = p-directory+1;
    copystring(parent, directory, len);
    return parent;
}

bool fileexists(const char *path, const char *mode)
{
    bool exists = true;
    if(mode[0]=='w' || mode[0]=='a') path = parentdir(path);
#ifdef WIN32
    if(GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES) exists = false;
#else
    if(access(path, R_OK | (mode[0]=='w' || mode[0]=='a' ? W_OK : 0)) == -1) exists = false;
#endif
    return exists;
}

bool createdir(const char *path)
{
    size_t len = strlen(path);
    if(path[len-1]==PATHDIV)
    {
        static string strip;
        path = copystring(strip, path, len);
    }
#ifdef WIN32
    return CreateDirectory(path, NULL)!=0;
#else
    return mkdir(path, 0777)==0;
#endif
}

size_t fixpackagedir(char *dir)
{
    path(dir);
    size_t len = strlen(dir);
    if(len > 0 && dir[len-1] != PATHDIV)
    {
        dir[len] = PATHDIV;
        dir[len+1] = '\0';
    }
    return len;
}

bool subhomedir(char *dst, int len, const char *src)
{
    const char *sub = strstr(src, "$HOME");
    if(!sub) sub = strchr(src, '~');
    if(sub && sub-src < len)
    {
#ifdef WIN32
        char home[MAX_PATH+1];
        home[0] = '\0';
        if(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, home) != S_OK || !home[0]) return false;
#else
        const char *home = getenv("HOME");
        if(!home || !home[0]) return false;
#endif
        dst[sub-src] = '\0';
        concatstring(dst, home);
        concatstring(dst, sub+(*sub == '~' ? 1 : strlen("$HOME")));
    }
    return true;
}

const char *sethomedir(const char *dir)
{
    string pdir;
    copystring(pdir, dir);
    if(!subhomedir(pdir, sizeof(pdir), dir) || !fixpackagedir(pdir)) return NULL;
    copystring(homedir, pdir);
    return homedir;
}

const char *addpackagedir(const char *dir)
{
    string pdir;
    copystring(pdir, dir);
    if(!subhomedir(pdir, sizeof(pdir), dir) || !fixpackagedir(pdir)) return NULL;
    return packagedirs.add(newstring(pdir));
}

const char *findfile(const char *filename, const char *mode)
{
    static string s;
    if(homedir[0])
    {
        formatstring(s)("%s%s", homedir, filename);
        if(fileexists(s, mode)) return s;
        if(mode[0]=='w' || mode[0]=='a')
        {
            string dirs;
            copystring(dirs, s);
            char *dir = strchr(dirs[0]==PATHDIV ? dirs+1 : dirs, PATHDIV);
            while(dir)
            {
                *dir = '\0';
                if(!fileexists(dirs, "r") && !createdir(dirs)) return s;
                *dir = PATHDIV;
                dir = strchr(dir+1, PATHDIV);
            }
            return s;
        }
    }
    if(mode[0]=='w' || mode[0]=='a') return filename;
    loopv(packagedirs)
    {
        formatstring(s)("%s%s", packagedirs[i], filename);
        if(fileexists(s, mode)) return s;
    }
    return filename;
}

bool listdir(const char *dir, bool rel, const char *ext, vector<char *> &files)
{
    int extsize = ext ? (int)strlen(ext)+1 : 0;
    string dirname;
    copystring(dirname, dir);
    path(dirname);
    #ifdef WIN32
	defformatstring(pathname)(rel ? ".\\%s\\*.%s" : "%s\\*.%s", dirname, ext ? ext : "*");
    WIN32_FIND_DATA FindFileData;
    HANDLE Find = FindFirstFile(pathname, &FindFileData);
    if(Find != INVALID_HANDLE_VALUE)
    {
        do {
            files.add(newstring(FindFileData.cFileName, (int)strlen(FindFileData.cFileName) - extsize));
        } while(FindNextFile(Find, &FindFileData));
        FindClose(Find);
        return true;
    }
    #else
    defformatstring(pathname)(rel ? "./%s" : "%s", dirname);
    DIR *d = opendir(pathname);
    if(d)
    {
        struct dirent *de;
        while((de = readdir(d)) != NULL)
        {
            if(!ext) files.add(newstring(de->d_name));
            else
            {
                int namelength = (int)strlen(de->d_name) - extsize;
                if(namelength > 0 && de->d_name[namelength] == '.' && strncmp(de->d_name+namelength+1, ext, extsize-1)==0)
                    files.add(newstring(de->d_name, namelength));
            }
        }
        closedir(d);
        return true;
    }
    #endif
    else return false;
}

int listfiles(const char *dir, const char *ext, vector<char *> &files)
{
    int dirs = 0;
    if(listdir(dir, true, ext, files)) dirs++;
    string s;
    if(homedir[0])
    {
        formatstring(s)("%s%s", homedir, dir);
        if(listdir(s, false, ext, files)) dirs++;
    }
    loopv(packagedirs)
    {
        formatstring(s)("%s%s", packagedirs[i], dir);
        if(listdir(s, false, ext, files)) dirs++;
    }
#ifndef STANDALONE
    dirs += listzipfiles(dir, ext, files);
#endif
    return dirs;
}

#ifndef STANDALONE
#ifndef __EMSCRIPTEN__
static int rwopsseek(SDL_RWops *rw, int pos, int whence)
#else
static long rwopsseek(SDL_RWops *rw, long pos, int whence)
#endif
{
    stream *f = (stream *)rw->hidden.unknown.data1;
    if((!pos && whence==SEEK_CUR) || f->seek(pos, whence)) return (int)f->tell();
    return -1;
}

#ifndef __EMSCRIPTEN__
static int rwopsread(SDL_RWops *rw, void *buf, int size, int nmemb)
#else
static size_t rwopsread(SDL_RWops *rw, void *buf, size_t size, size_t nmemb)
#endif
{
    stream *f = (stream *)rw->hidden.unknown.data1;
    return f->read(buf, size*nmemb)/size;
}

#ifndef __EMSCRIPTEN__
static int rwopswrite(SDL_RWops *rw, const void *buf, int size, int nmemb)
#else
static size_t rwopswrite(SDL_RWops *rw, const void *buf, size_t size, size_t nmemb)
#endif
{
    stream *f = (stream *)rw->hidden.unknown.data1;
    return f->write(buf, size*nmemb)/size;
}

static int rwopsclose(SDL_RWops *rw)
{
    return 0;
}

SDL_RWops *stream::rwops()
{
    SDL_RWops *rw = SDL_AllocRW();
    if(!rw) return NULL;
    rw->hidden.unknown.data1 = this;
    rw->seek = rwopsseek;
    rw->read = rwopsread;
    rw->write = rwopswrite;
    rw->close = rwopsclose;
    return rw;
}
#endif

stream::offset stream::size()
{
    offset pos = tell(), endpos;
    if(pos < 0 || !seek(0, SEEK_END)) return -1;
    endpos = tell();
    return pos == endpos || seek(pos, SEEK_SET) ? endpos : -1;
}

bool stream::getline(char *str, int len)
{
    loopi(len-1)
    {
        if(read(&str[i], 1) != 1) { str[i] = '\0'; return i > 0; }
        else if(str[i] == '\n') { str[i+1] = '\0'; return true; }
    }
    if(len > 0) str[len-1] = '\0';
    return true;
}

int stream::printf(const char *fmt, ...)
{
    char buf[512];
    char *str = buf;
    va_list args;
#if defined(WIN32) && !defined(__GNUC__)
    va_start(args, fmt);
    size_t len = _vscprintf(fmt, args);
    if(len >= sizeof(buf)) str = new char[len+1];
    _vsnprintf(str, len+1, fmt, args);
    va_end(args);
#else
    va_start(args, fmt);
    size_t len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if(len >= sizeof(buf))
    {
        str = new char[len+1];
        va_start(args, fmt);
        vsnprintf(str, len+1, fmt, args);
        va_end(args);
    }
#endif
    int n = write(str, len);
    if(str != buf) delete[] str;
    return n;
}

struct filestream : stream
{
    FILE *file;

    filestream() : file(NULL) {}
    ~filestream() { close(); }

    bool open(const char *name, const char *mode)
    {
        if(file) return false;
        file = fopen(name, mode);
        return file!=NULL;
    }

    bool opentemp(const char *name, const char *mode)
    {
        if(file) return false;
#ifdef WIN32
        file = fopen(name, mode);
#else
        file = tmpfile();
#endif
        return file!=NULL;
    }

    void close()
    {
        if(file) { fclose(file); file = NULL; }
    }

    bool end() { return feof(file)!=0; }
    offset tell() 
    { 
#ifdef WIN32
#ifdef __GNUC__
        return ftello64(file);
#else
        return _ftelli64(file);       
#endif
#else
        return ftello(file); 
#endif
    }
    bool seek(offset pos, int whence) 
    { 
#ifdef WIN32
#ifdef __GNUC__
        return fseeko64(file, pos, whence) >= 0;
#else
        return _fseeki64(file, pos, whence) >= 0;
#endif
#else
        return fseeko(file, pos, whence) >= 0;
#endif
    }

    int read(void *buf, int len) { return (int)fread(buf, 1, len, file); }
    int write(const void *buf, int len) { return (int)fwrite(buf, 1, len, file); }
    int getchar() { return fgetc(file); }
    bool putchar(int c) { return fputc(c, file)!=EOF; }
    bool getline(char *str, int len) { return fgets(str, len, file)!=NULL; }
    bool putstring(const char *str) { return fputs(str, file)!=EOF; }

    int printf(const char *fmt, ...)
    {
        va_list v;
        va_start(v, fmt);
        int result = vfprintf(file, fmt, v);
        va_end(v);
        return result;
    }
};

#ifndef STANDALONE
VAR(dbggz, 0, 0, 1);
#endif

struct gzstream : stream
{
    enum
    {
        MAGIC1   = 0x1F,
        MAGIC2   = 0x8B,
        BUFSIZE  = 16384,
        OS_UNIX  = 0x03
    };

    enum
    {
        F_ASCII    = 0x01,
        F_CRC      = 0x02,
        F_EXTRA    = 0x04,
        F_NAME     = 0x08,
        F_COMMENT  = 0x10,
        F_RESERVED = 0xE0
    };

    stream *file;
    z_stream zfile;
    uchar *buf;
    bool reading, writing, autoclose;
    uint crc;
    int headersize;

    gzstream() : file(NULL), buf(NULL), reading(false), writing(false), autoclose(false), crc(0), headersize(0)
    {
        zfile.zalloc = NULL;
        zfile.zfree = NULL;
        zfile.opaque = NULL;
        zfile.next_in = zfile.next_out = NULL;
        zfile.avail_in = zfile.avail_out = 0;
    }

    ~gzstream()
    {
        close();
    }

    void writeheader()
    {
        uchar header[] = { MAGIC1, MAGIC2, Z_DEFLATED, 0, 0, 0, 0, 0, 0, OS_UNIX };
        file->write(header, sizeof(header));
    }

    void readbuf(int size = BUFSIZE)
    {
        if(!zfile.avail_in) zfile.next_in = (Bytef *)buf;
        size = min(size, int(&buf[BUFSIZE] - &zfile.next_in[zfile.avail_in]));
        int n = file->read(zfile.next_in + zfile.avail_in, size);
        if(n > 0) zfile.avail_in += n;
    }

    int readbyte(int size = BUFSIZE)
    {
        if(!zfile.avail_in) readbuf(size);
        if(!zfile.avail_in) return 0;
        zfile.avail_in--;
        return *(uchar *)zfile.next_in++;
    }

    void skipbytes(int n)
    {
        while(n > 0 && zfile.avail_in > 0)
        {
            int skipped = min(n, (int)zfile.avail_in);
            zfile.avail_in -= skipped;
            zfile.next_in += skipped;
            n -= skipped;
        }
        if(n <= 0) return;
        file->seek(n, SEEK_CUR);
    }

    bool checkheader()
    {
        readbuf(10);
        if(readbyte() != MAGIC1 || readbyte() != MAGIC2 || readbyte() != Z_DEFLATED) return false;
        int flags = readbyte();
        if(flags & F_RESERVED) return false;
        skipbytes(6);
        if(flags & F_EXTRA)
        {
            int len = readbyte(512);
            len |= readbyte(512)<<8;
            skipbytes(len);
        }
        if(flags & F_NAME) while(readbyte(512));
        if(flags & F_COMMENT) while(readbyte(512));
        if(flags & F_CRC) skipbytes(2);
        headersize = int(file->tell() - zfile.avail_in);
        return zfile.avail_in > 0 || !file->end();
    }

    bool open(stream *f, const char *mode, bool needclose, int level)
    {
        if(file) return false;
        for(; *mode; mode++)
        {
            if(*mode=='r') { reading = true; break; }
            else if(*mode=='w') { writing = true; break; }
        }
        if(reading)
        {
            if(inflateInit2(&zfile, -MAX_WBITS) != Z_OK) reading = false;
        }
        else if(writing && deflateInit2(&zfile, level, Z_DEFLATED, -MAX_WBITS, min(MAX_MEM_LEVEL, 8), Z_DEFAULT_STRATEGY) != Z_OK) writing = false;
        if(!reading && !writing) return false;

        autoclose = needclose;
        file = f;
        crc = crc32(0, NULL, 0);
        buf = new uchar[BUFSIZE];

        if(reading)
        {
            if(!checkheader()) { stopreading(); return false; }
        }
        else if(writing) writeheader();
        return true;
    }

    uint getcrc() { return crc; }

    void finishreading()
    {
        if(!reading) return;
#ifndef STANDALONE
        if(dbggz)
        {
            uint checkcrc = 0, checksize = 0;
            loopi(4) checkcrc |= uint(readbyte()) << (i*8);
            loopi(4) checksize |= uint(readbyte()) << (i*8);
            if(checkcrc != crc)
                conoutf(CON_DEBUG, "gzip crc check failed: read %X, calculated %X", checkcrc, crc);
            if(checksize != zfile.total_out)
                conoutf(CON_DEBUG, "gzip size check failed: read %d, calculated %d", checksize, zfile.total_out);
        }
#endif
    }

    void stopreading()
    {
        if(!reading) return;
        inflateEnd(&zfile);
        reading = false;
    }

    void finishwriting()
    {
        if(!writing) return;
        for(;;)
        {
            int err = zfile.avail_out > 0 ? deflate(&zfile, Z_FINISH) : Z_OK;
            if(err != Z_OK && err != Z_STREAM_END) break;
            flush();
            if(err == Z_STREAM_END) break;
        }
        uchar trailer[8] =
        {
            crc&0xFF, (crc>>8)&0xFF, (crc>>16)&0xFF, (crc>>24)&0xFF,
            zfile.total_in&0xFF, (zfile.total_in>>8)&0xFF, (zfile.total_in>>16)&0xFF, (zfile.total_in>>24)&0xFF
        };
        file->write(trailer, sizeof(trailer));
    }

    void stopwriting()
    {
        if(!writing) return;
        deflateEnd(&zfile);
        writing = false;
    }

    void close()
    {
        if(reading) finishreading();
        stopreading();
        if(writing) finishwriting();
        stopwriting();
        DELETEA(buf);
        if(autoclose) DELETEP(file);
    }

    bool end() { return !reading && !writing; }
    offset tell() { return reading ? zfile.total_out : (writing ? zfile.total_in : -1); }
    offset rawtell() { return file ? file->tell() : -1; }

    bool seek(offset pos, int whence)
    {
        if(writing || !reading) return false;

        if(whence == SEEK_END)
        {
            uchar skip[512];
            while(read(skip, sizeof(skip)) == sizeof(skip));
            return !pos;
        }
        else if(whence == SEEK_CUR) pos += zfile.total_out;

        if(pos >= (offset)zfile.total_out) pos -= zfile.total_out;
        else if(pos < 0 || !file->seek(headersize, SEEK_SET)) return false;
        else
        {
            if(zfile.next_in && zfile.total_in <= uint(zfile.next_in - buf))
            {
                zfile.avail_in += zfile.total_in;
                zfile.next_in -= zfile.total_in;
            }
            else
            {
                zfile.avail_in = 0;
                zfile.next_in = NULL;
            }
            inflateReset(&zfile);
            crc = crc32(0, NULL, 0);
        }

        uchar skip[512];
        while(pos > 0)
        {
            int skipped = (int)min(pos, (offset)sizeof(skip));
            if(read(skip, skipped) != skipped) { stopreading(); return false; }
            pos -= skipped;
        }

        return true;
    }

    int read(void *buf, int len)
    {
        if(!reading || !buf || !len) return 0;
        zfile.next_out = (Bytef *)buf;
        zfile.avail_out = len;
        while(zfile.avail_out > 0)
        {
            if(!zfile.avail_in)
            {
                readbuf(BUFSIZE);
                if(!zfile.avail_in) { stopreading(); break; }
            }
            int err = inflate(&zfile, Z_NO_FLUSH);
            if(err == Z_STREAM_END) { crc = crc32(crc, (Bytef *)buf, len - zfile.avail_out); finishreading(); stopreading(); return len - zfile.avail_out; }
            else if(err != Z_OK) { stopreading(); break; }
        }
        crc = crc32(crc, (Bytef *)buf, len - zfile.avail_out);
        return len - zfile.avail_out;
    }

    bool flush()
    {
        if(zfile.next_out && zfile.avail_out < BUFSIZE)
        {
            if(file->write(buf, BUFSIZE - zfile.avail_out) != int(BUFSIZE - zfile.avail_out))
                return false;
        }
        zfile.next_out = buf;
        zfile.avail_out = BUFSIZE;
        return true;
    }

    int write(const void *buf, int len)
    {
        if(!writing || !buf || !len) return 0;
        zfile.next_in = (Bytef *)buf;
        zfile.avail_in = len;
        while(zfile.avail_in > 0)
        {
            if(!zfile.avail_out && !flush()) { stopwriting(); break; }
            int err = deflate(&zfile, Z_NO_FLUSH);
            if(err != Z_OK) { stopwriting(); break; }
        }
        crc = crc32(crc, (Bytef *)buf, len - zfile.avail_in);
        return len - zfile.avail_in;
    }
};

struct utf8stream : stream
{
    enum
    {
        BUFSIZE = 4096
    };
    stream *file;
    offset pos;
    int bufread, bufcarry, buflen;
    bool reading, writing, autoclose;
    uchar buf[BUFSIZE]; 

    utf8stream() : file(NULL), pos(0), bufread(0), bufcarry(0), buflen(0), reading(false), writing(false), autoclose(false)
    {
    }

    ~utf8stream()
    {
        close();
    }

    bool readbuf(int size = BUFSIZE)
    {
        if(bufread >= bufcarry) { if(bufcarry > 0 && bufcarry < buflen) memmove(buf, &buf[bufcarry], buflen - bufcarry); buflen -= bufcarry; bufread = bufcarry = 0; }
        int n = file->read(&buf[buflen], min(size, BUFSIZE - buflen));
        if(n <= 0) return false;
        buflen += n;
        int carry = bufcarry;
        bufcarry += decodeutf8(&buf[bufcarry], BUFSIZE-bufcarry, &buf[bufcarry], buflen-bufcarry, &carry);
        if(carry > bufcarry && carry < buflen) { memmove(&buf[bufcarry], &buf[carry], buflen - carry); buflen -= carry - bufcarry; }
        return true;
    }

    bool checkheader()
    {
        int n = file->read(buf, 3);
        if(n == 3 && buf[0] == 0xEF && buf[1] == 0xBB && buf[2] == 0xBF) return true;
        buflen = n; 
        return false;
    }
            
    bool open(stream *f, const char *mode, bool needclose)
    {
        if(file) return false;
        for(; *mode; mode++)
        {
            if(*mode=='r') { reading = true; break; }
            else if(*mode=='w') { writing = true; break; }
        }
        if(!reading && !writing) return false;
       
        autoclose = needclose;
        file = f;
       
        if(reading) checkheader();
 
        return true;
    } 

    void finishreading() 
    {
        if(!reading) return;
    }

    void stopreading()
    {
        if(!reading) return;
        reading = false;
    }

    void stopwriting()
    {
        if(!writing) return;
        writing = false;
    }

    void close()
    {
        stopreading();
        stopwriting();
        if(autoclose) DELETEP(file);
    }

    bool end() { return !reading && !writing; }
    offset tell() { return reading || writing ? pos : -1; }

    bool seek(offset off, int whence)
    {
        if(writing || !reading) return false;

        if(whence == SEEK_END)
        {
            uchar skip[512];
            while(read(skip, sizeof(skip)) == sizeof(skip));
            return !off;
        }
        else if(whence == SEEK_CUR) off += pos;
       
        if(off >= pos) off -= pos;
        else if(off < 0 || !file->seek(0, SEEK_SET)) return false;
        else
        {
            bufread = bufcarry = buflen = 0;
            pos = 0;
            checkheader(); 
        }

        uchar skip[512];
        while(off > 0)
        {
            int skipped = (int)min(off, (offset)sizeof(skip));
            if(read(skip, skipped) != skipped) { stopreading(); return false; }
            off -= skipped;
        }
        
        return true;
    }

    int read(void *dst, int len)
    {
        if(!reading || !dst || !len) return 0;
        int next = 0;
        while(next < len)
        {
            if(bufread >= bufcarry) { if(readbuf(BUFSIZE)) continue; stopreading(); break; }
            int n = min(len - next, bufcarry - bufread);
            memcpy(&((uchar *)dst)[next], &buf[bufread], n);
            next += n;
            bufread += n;
        }
        pos += next;
        return next;
    }

    bool getline(char *dst, int len)
    {
        if(!reading || !dst || !len) return false;
        --len;
        int next = 0;
        while(next < len)
        {
            if(bufread >= bufcarry) { if(readbuf(BUFSIZE)) continue; stopreading(); if(!next) return false; break; }
            int n = min(len - next, bufcarry - bufread);
            uchar *endline = (uchar *)memchr(&buf[bufread], '\n', n);
            if(endline) { n = endline+1 - &buf[bufread]; len = next + n; } 
            memcpy(&((uchar *)dst)[next], &buf[bufread], n);
            next += n;
            bufread += n;
        }
        dst[next] = '\0';
        pos += next;
        return true;
    }

    int write(const void *src, int len)
    {
        if(!writing || !src || !len) return 0;
        uchar dst[512];
        int next = 0;
        while(next < len)
        {
            int carry = 0, n = encodeutf8(dst, sizeof(dst), &((uchar *)src)[next], len - next, &carry);
            if(n > 0 && file->write(dst, n) != n) { stopwriting(); break; }
            next += carry;
        }
        pos += next;
        return next;
    }
};

stream *openrawfile(const char *filename, const char *mode)
{
    const char *found = findfile(filename, mode);
    if(!found) return NULL;
    filestream *file = new filestream;
    if(!file->open(found, mode)) { delete file; return NULL; }
    return file;
}

stream *openfile(const char *filename, const char *mode)
{
#ifndef STANDALONE
    stream *s = openzipfile(filename, mode);
    if(s) return s;
#endif
    return openrawfile(filename, mode);
}

stream *opentempfile(const char *name, const char *mode)
{
    const char *found = findfile(name, mode);
    filestream *file = new filestream;
    if(!file->opentemp(found ? found : name, mode)) { delete file; return NULL; }
    return file;
}

stream *opengzfile(const char *filename, const char *mode, stream *file, int level)
{
    stream *source = file ? file : openfile(filename, mode);
    if(!source) return NULL;
    gzstream *gz = new gzstream;
    if(!gz->open(source, mode, !file, level)) { if(!file) delete source; return NULL; }
    return gz;
}

stream *openutf8file(const char *filename, const char *mode, stream *file)
{
    stream *source = file ? file : openfile(filename, mode);
    if(!source) return NULL;
    utf8stream *utf8 = new utf8stream;
    if(!utf8->open(source, mode, !file)) { if(!file) delete source; return NULL; }
    return utf8;
}

char *loadfile(const char *fn, int *size, bool utf8)
{
    stream *f = openfile(fn, "rb");
    if(!f) return NULL;
    int len = (int)f->size();
    if(len <= 0) { delete f; return NULL; }
    char *buf = new char[len+1];
    if(!buf) { delete f; return NULL; }
    int offset = 0;
    if(utf8 && len >= 3)
    {
        if(f->read(buf, 3) != 3) { delete f; delete[] buf; return NULL; }
        if(((uchar *)buf)[0] == 0xEF && ((uchar *)buf)[1] == 0xBB && ((uchar *)buf)[2] == 0xBF) len -= 3;
        else offset += 3;
    } 
    int rlen = f->read(&buf[offset], len-offset);
    delete f;
    if(rlen != len-offset) { delete[] buf; return NULL; }
    if(utf8) len = decodeutf8((uchar *)buf, len, (uchar *)buf, len);
    buf[len] = '\0';
    if(size!=NULL) *size = len;
    return buf;
}

