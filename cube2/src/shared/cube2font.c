#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>
#include <zlib.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
#include FT_GLYPH_H

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

int imin(int a, int b) { return a < b ? a : b; }
int imax(int a, int b) { return a > b ? a : b; }

void fatal(const char *fmt, ...)
{
    va_list v;
    va_start(v, fmt);
    vfprintf(stderr, fmt, v);
    va_end(v);
    fputc('\n', stderr);

    exit(EXIT_FAILURE);
}

uint bigswap(uint n)
{
    const int islittleendian = 1;
    return *(const uchar *)&islittleendian ? (n<<24) | (n>>24) | ((n>>8)&0xFF00) | ((n<<8)&0xFF0000) : n;
}

size_t writebig(FILE *f, uint n)
{
    n = bigswap(n);
    return fwrite(&n, 1, sizeof(n), f);
}

void writepngchunk(FILE *f, const char *type, uchar *data, uint len)
{
    uint crc;
    writebig(f, len);
    fwrite(type, 1, 4, f);
    fwrite(data, 1, len, f);

    crc = crc32(0, Z_NULL, 0);
    crc = crc32(crc, (const Bytef *)type, 4);
    if(data) crc = crc32(crc, data, len);
    writebig(f, crc);
}

struct pngihdr
{
    uint width, height;
    uchar bitdepth, colortype, compress, filter, interlace;
};

void savepng(const char *filename, uchar *data, int w, int h, int bpp, int flip)
{
    const uchar signature[] = { 137, 80, 78, 71, 13, 10, 26, 10 };
    struct pngihdr ihdr;
    FILE *f;
    long idat;
    uint len, crc;
    z_stream z;
    uchar buf[1<<12];
    int i, j;

    memset(&ihdr, 0, sizeof(ihdr));
    ihdr.width = bigswap(w);
    ihdr.height = bigswap(h);
    ihdr.bitdepth = 8;
    switch(bpp)
    {
        case 1: ihdr.colortype = 0; break;
        case 2: ihdr.colortype = 4; break;
        case 3: ihdr.colortype = 2; break;
        case 4: ihdr.colortype = 6; break;
        default: fatal("cube2font: invalid PNG bpp"); return;
    }
    f = fopen(filename, "wb");
    if(!f) { fatal("cube2font: could not write to %s", filename); return; }

    fwrite(signature, 1, sizeof(signature), f);

    writepngchunk(f, "IHDR", (uchar *)&ihdr, 13);

    idat = ftell(f);
    len = 0;
    fwrite("\0\0\0\0IDAT", 1, 8, f);
    crc = crc32(0, Z_NULL, 0);
    crc = crc32(crc, (const Bytef *)"IDAT", 4);

    z.zalloc = NULL;
    z.zfree = NULL;
    z.opaque = NULL;

    if(deflateInit(&z, Z_BEST_COMPRESSION) != Z_OK)
        goto error;

    z.next_out = (Bytef *)buf;
    z.avail_out = sizeof(buf);

    for(i = 0; i < h; i++)
    {
        uchar filter = 0;
        for(j = 0; j < 2; j++)
        {
            z.next_in = j ? (Bytef *)data + (flip ? h-i-1 : i)*w*bpp : (Bytef *)&filter;
            z.avail_in = j ? w*bpp : 1;
            while(z.avail_in > 0)
            {
                if(deflate(&z, Z_NO_FLUSH) != Z_OK) goto cleanuperror;
                #define FLUSHZ do { \
                    int flush = sizeof(buf) - z.avail_out; \
                    crc = crc32(crc, buf, flush); \
                    len += flush; \
                    fwrite(buf, 1, flush, f); \
                    z.next_out = (Bytef *)buf; \
                    z.avail_out = sizeof(buf); \
                } while(0)
                FLUSHZ;
            }
        }
    }

    for(;;)
    {
        int err = deflate(&z, Z_FINISH);
        if(err != Z_OK && err != Z_STREAM_END) goto cleanuperror;
        FLUSHZ;
        if(err == Z_STREAM_END) break;
    }

    deflateEnd(&z);

    fseek(f, idat, SEEK_SET);
    writebig(f, len);
    fseek(f, 0, SEEK_END);
    writebig(f, crc);

    writepngchunk(f, "IEND", NULL, 0);

    fclose(f);
    return;

cleanuperror:
    deflateEnd(&z);

error:
    fclose(f);

    fatal("cube2font: failed saving PNG to %s", filename);
}

enum
{
    CT_PRINT   = 1<<0,
    CT_SPACE   = 1<<1,
    CT_DIGIT   = 1<<2,
    CT_ALPHA   = 1<<3,
    CT_LOWER   = 1<<4,
    CT_UPPER   = 1<<5,
    CT_UNICODE = 1<<6
};
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
const uchar cubectype[256] =
{
    CUBECTYPE(CT_SPACE,
              CT_PRINT,
              CT_PRINT|CT_DIGIT,
              CT_PRINT|CT_ALPHA|CT_LOWER,
              CT_PRINT|CT_ALPHA|CT_UPPER,
              CT_PRINT|CT_UNICODE|CT_ALPHA|CT_LOWER,
              CT_PRINT|CT_UNICODE|CT_ALPHA|CT_UPPER)
};
int iscubeprint(uchar c) { return cubectype[c]&CT_PRINT; }
int iscubespace(uchar c) { return cubectype[c]&CT_SPACE; }
int iscubealpha(uchar c) { return cubectype[c]&CT_ALPHA; }
int iscubealnum(uchar c) { return cubectype[c]&(CT_ALPHA|CT_DIGIT); }
int iscubelower(uchar c) { return cubectype[c]&CT_LOWER; }
int iscubeupper(uchar c) { return cubectype[c]&CT_UPPER; }
const int cube2unichars[256] =
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
int cube2uni(uchar c)
{
    return cube2unichars[c];
}

const char *encodeutf8(int uni)
{
    static char buf[7];
    char *dst = buf;
    if(uni <= 0x7F) { *dst++ = uni; goto uni1; }
    else if(uni <= 0x7FF) { *dst++ = 0xC0 | (uni>>6); goto uni2; }
    else if(uni <= 0xFFFF) { *dst++ = 0xE0 | (uni>>12); goto uni3; }
    else if(uni <= 0x1FFFFF) { *dst++ = 0xF0 | (uni>>18); goto uni4; }
    else if(uni <= 0x3FFFFFF) { *dst++ = 0xF8 | (uni>>24); goto uni5; }
    else if(uni <= 0x7FFFFFFF) { *dst++ = 0xFC | (uni>>30); goto uni6; }
    else goto uni1;
uni6: *dst++ = 0x80 | ((uni>>24)&0x3F);
uni5: *dst++ = 0x80 | ((uni>>18)&0x3F);
uni4: *dst++ = 0x80 | ((uni>>12)&0x3F);
uni3: *dst++ = 0x80 | ((uni>>6)&0x3F);
uni2: *dst++ = 0x80 | (uni&0x3F);
uni1: *dst++ = '\0';
    return buf;
}

struct fontchar { int code, uni, tex, x, y, w, h, offx, offy, offset, advance; FT_BitmapGlyph color, alpha; };

const char *texdir = "";

const char *texfilename(const char *name, int texnum)
{
    static char file[256];
    snprintf(file, sizeof(file), "%s%d.png", name, texnum);
    return file;
}

const char *texname(const char *name, int texnum)
{
    static char file[512];
    snprintf(file, sizeof(file), "<grey>%s%s", texdir, texfilename(name, texnum));
    return file;
}

void writetexs(const char *name, struct fontchar *chars, int numchars, int numtexs, int tw, int th)
{
    int tex;
    uchar *pixels = (uchar *)malloc(tw*th*2);
    if(!pixels) fatal("cube2font: failed allocating textures");
    for(tex = 0; tex < numtexs; tex++)
    {
        const char *file = texfilename(name, tex);
        int texchars = 0, i;
        uchar *dst, *src;
        memset(pixels, 0, tw*th*2);
        for(i = 0; i < numchars; i++)
        {
            struct fontchar *c = &chars[i];
            int x, y;
            if(c->tex != tex) continue;
            texchars++;
            dst = &pixels[2*((c->y + c->offy - c->color->top)*tw + c->x + c->color->left - c->offx)];
            src = (uchar *)c->color->bitmap.buffer;
            for(y = 0; y < c->color->bitmap.rows; y++)
            {
                for(x = 0; x < c->color->bitmap.width; x++)
                    dst[2*x] = src[x];
                src += c->color->bitmap.pitch;
                dst += 2*tw;
            }
            dst = &pixels[2*((c->y + c->offy - c->alpha->top)*tw + c->x + c->alpha->left - c->offx)];
            src = (uchar *)c->alpha->bitmap.buffer;
            for(y = 0; y < c->alpha->bitmap.rows; y++)
            {
                for(x = 0; x < c->alpha->bitmap.width; x++)
                    dst[2*x+1] = src[x];
                src += c->alpha->bitmap.pitch;
                dst += 2*tw;
            }
        }
        printf("cube2font: writing %d chars to %s\n", texchars, file);
        savepng(file, pixels, tw, th, 2, 0);
   }
   free(pixels);
}

void writecfg(const char *name, struct fontchar *chars, int numchars, int x1, int y1, int x2, int y2, int sw, int sh, int argc, char **argv)
{
    FILE *f;
    char file[256];
    int i, lastcode = 0, lasttex = 0;
    snprintf(file, sizeof(file), "%s.cfg", name);
    f = fopen(file, "w");
    if(!f) fatal("cube2font: failed writing %s", file);
    printf("cube2font: writing %d chars to %s\n", numchars, file);
    fprintf(f, "//");
    for(i = 1; i < argc; i++)
        fprintf(f, " %s", argv[i]);
    fprintf(f, "\n");
    fprintf(f, "font \"%s\" \"%s\" %d %d\n", name, texname(name, 0), sw, sh);
    for(i = 0; i < numchars; i++)
    {
        struct fontchar *c = &chars[i];
        if(!lastcode && lastcode < c->code)
        {
            fprintf(f, "fontoffset \"%s\"\n", encodeutf8(c->uni));
            lastcode = c->code;
        }
        else if(lastcode < c->code)
        {
            if(lastcode + 1 == c->code)
                fprintf(f, "fontskip // %d\n", lastcode);
            else
                fprintf(f, "fontskip %d // %d .. %d\n", c->code - lastcode, lastcode, c->code-1);
            lastcode = c->code;
        }
        if(lasttex != c->tex)
        {
            fprintf(f, "\nfonttex \"%s\"\n", texname(name, c->tex));
            lasttex = c->tex;
        }
        if(c->code != c->uni)
            fprintf(f, "fontchar %d %d %d %d %d %d %d // %s (%d -> 0x%X)\n", c->x, c->y, c->w, c->h, c->offx+c->offset, y2-c->offy, c->advance, encodeutf8(c->uni), c->code, c->uni);
        else
            fprintf(f, "fontchar %d %d %d %d %d %d %d // %s (%d)\n", c->x, c->y, c->w, c->h, c->offx+c->offset, y2-c->offy, c->advance, encodeutf8(c->uni), c->code);
        lastcode++;
    }
    fclose(f);
}

int groupchar(int c)
{
    switch(c)
    {
    case 0x152: case 0x153: case 0x178: return 1;
    }
    if(c < 127 || c >= 0x2000) return 0;
    if(c < 0x100) return 1;
    if(c < 0x400) return 2;
    return 3;
}

int sortchars(const void *x, const void *y)
{
    const struct fontchar *xc = *(const struct fontchar **)x, *yc = *(const struct fontchar **)y;
    int xg = groupchar(xc->uni), yg = groupchar(yc->uni);
    if(xg < yg) return -1;
    if(xg > yg) return 1;
    if(xc->h != yc->h) return yc->h - xc->h;
    if(xc->w != yc->w) return yc->w - xc->w;
    return yc->uni - xc->uni;
}

int scorechar(struct fontchar *f, int pad, int tw, int th, int rw, int rh, int ry)
{
    int score = 0;
    if(rw + f->w > tw) { ry += rh + pad; score = 1; }
    if(ry + f->h > th) score = 2;
    return score;
}

int main(int argc, char **argv)
{
    FT_Library l;
    FT_Face f;
    FT_Stroker s, s2;
    int i, pad, offset, advance, w, h, tw, th, c, trial = -2, rw = 0, rh = 0, ry = 0, x1 = INT_MAX, x2 = INT_MIN, y1 = INT_MAX, y2 = INT_MIN, w2 = 0, h2 = 0, sw = 0, sh = 0;
    float outborder = 0, inborder = 0;
    struct fontchar chars[256];
    struct fontchar *order[256];
    int numchars = 0, numtex = 0;
    if(argc < 11)
        fatal("Usage: cube2font infile outfile outborder[:inborder] pad offset advance charwidth charheight texwidth texheight [spacewidth spaceheight texdir]");
    sscanf(argv[3], "%f:%f", &outborder, &inborder);
    pad = atoi(argv[4]);
    offset = atoi(argv[5]);
    advance = atoi(argv[6]);
    w = atoi(argv[7]);
    h = atoi(argv[8]);
    tw = atoi(argv[9]);
    th = atoi(argv[10]);
    if(argc > 11) sw = atoi(argv[11]);
    if(argc > 12) sh = atoi(argv[12]);
    if(argc > 13) texdir = argv[13];
    if(FT_Init_FreeType(&l))
        fatal("cube2font: failed initing freetype");
    if(FT_New_Face(l, argv[1], 0, &f) ||
       FT_Set_Charmap(f, f->charmaps[0]) ||
       FT_Set_Pixel_Sizes(f, w, h) ||
       FT_Stroker_New(l, &s) ||
       FT_Stroker_New(l, &s2))
        fatal("cube2font: failed loading font %s", argv[1]);
    if(outborder > 0) FT_Stroker_Set(s, (FT_Fixed)(outborder * 64), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
    if(inborder > 0) FT_Stroker_Set(s2, (FT_Fixed)(inborder * 64), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
    for(c = 0; c < 256; c++) if(iscubeprint(c))
    {
        FT_Glyph p, p2;
        FT_BitmapGlyph b, b2;
        struct fontchar *dst = &chars[numchars];
        dst->code = c;
        dst->uni = cube2uni(c);
        if(FT_Load_Char(f, dst->uni, FT_LOAD_DEFAULT))
            fatal("cube2font: failed loading character %s", encodeutf8(dst->uni));
        FT_Get_Glyph(f->glyph, &p);
        p2 = p;
        if(outborder > 0) FT_Glyph_StrokeBorder(&p, s, 0, 0);
        if(inborder > 0) FT_Glyph_StrokeBorder(&p2, s2, 1, 0);
        FT_Glyph_To_Bitmap(&p, FT_RENDER_MODE_NORMAL, 0, 1);
        FT_Glyph_To_Bitmap(&p2, FT_RENDER_MODE_NORMAL, 0, 1);
        b = (FT_BitmapGlyph)p;
        b2 = (FT_BitmapGlyph)p2;
        dst->tex = -1;
        dst->x = INT_MIN;
        dst->y = INT_MIN;
        dst->offx = imin(b->left, b2->left);
        dst->offy = imax(b->top, b2->top);
        dst->offset = offset;
        dst->advance = offset + ((p->advance.x+0xFFFF)>>16) + advance;
        dst->w = imax(b->left + b->bitmap.width, b2->left + b2->bitmap.width) - dst->offx;
        dst->h = dst->offy - imin(b->top - b->bitmap.rows, b2->top - b2->bitmap.rows);
        dst->alpha = b;
        dst->color = b2;
        order[numchars++] = dst;
    }
    qsort(order, numchars, sizeof(order[0]), sortchars);
    for(i = 0; i < numchars;)
    {
        struct fontchar *dst;
        int j, k, trial0, prevscore, dstscore, fitscore;
        for(trial0 = trial, prevscore = -1; (trial -= 2) >= trial0-512;)
        {
            int g, fw = rw, fh = rh, fy = ry, curscore = 0, reused = 0;
            for(j = i; j < numchars; j++)
            {
                dst = order[j];
                if(dst->tex >= 0 || dst->tex <= trial) continue;
                g = groupchar(dst->uni);
                dstscore = scorechar(dst, pad, tw, th, fw, fh, fy);
                for(k = j; k < numchars; k++)
                {
                    struct fontchar *fit = order[k];
                    if(fit->tex >= 0 || fit->tex <= trial) continue;
                    if(fit->tex >= trial0 && groupchar(fit->uni) != g) break;
                    fitscore = scorechar(fit, pad, tw, th, fw, fh, fy);
                    if(fitscore < dstscore || (fitscore == dstscore && fit->h > dst->h))
                    {
                        dst = fit;
                        dstscore = fitscore;
                    }
                }
                if(fw + dst->w > tw)
                {
                    fy += fh + pad;
                    fw = fh = 0;
                }
                if(fy + dst->h > th)
                {
                    fy = fw = fh = 0;
                    if(curscore > 0) break;
                }
                if(dst->tex >= trial+1 && dst->tex <= trial+2) { dst->tex = trial; reused++; }
                else dst->tex = trial;
                fw += dst->w + pad;
                fh = imax(fh, dst->h);
                if(dst != order[j]) --j;
                curscore++;
            }
            if(reused < prevscore || curscore <= prevscore) break;
            prevscore = curscore;
        }
        for(; i < numchars; i++)
        {
            dst = order[i];
            if(dst->tex >= 0) continue;
            dstscore = scorechar(dst, pad, tw, th, rw, rh, ry);
            for(j = i; j < numchars; j++)
            {
                struct fontchar *fit = order[j];
                if(fit->tex < trial || fit->tex > trial+2) continue;
                fitscore = scorechar(fit, pad, tw, th, rw, rh, ry);
                if(fitscore < dstscore || (fitscore == dstscore && fit->h > dst->h))
                {
                    dst = fit;
                    dstscore = fitscore;
                }
            }
            if(dst->tex < trial || dst->tex > trial+2) break;
            if(rw + dst->w > tw)
            {
                ry += rh + pad;
                rw = rh = 0;
            }
            if(ry + dst->h > th)
            {
                ry = rw = rh = 0;
                numtex++;
            }
            dst->tex = numtex;
            dst->x = rw;
            dst->y = ry;
            rw += dst->w + pad;
            rh = imax(rh, dst->h);
            y1 = imin(y1, dst->offy - dst->h);
            y2 = imax(y2, dst->offy);
            x1 = imin(x1, dst->offx);
            x2 = imax(x2, dst->offx + dst->w);
            w2 = imax(w2, dst->w);
            h2 = imax(h2, dst->h);
            if(dst != order[i]) --i;
        }
    }
    if(rh > 0) numtex++;
#if 0
    if(sw <= 0)
    {
        if(FT_Load_Char(f, ' ', FT_LOAD_DEFAULT))
            fatal("cube2font: failed loading space character");
        sw = (f->glyph->advance.x+0x3F)>>6;
    }
#endif
    if(sh <= 0) sh = y2 - y1; 
    if(sw <= 0) sw = sh/3;
    writetexs(argv[2], chars, numchars, numtex, tw, th);
    writecfg(argv[2], chars, numchars, x1, y1, x2, y2, sw, sh, argc, argv);
    for(i = 0; i < numchars; i++)
    {
        FT_Done_Glyph((FT_Glyph)chars[i].alpha);
        FT_Done_Glyph((FT_Glyph)chars[i].color);
    }
    FT_Stroker_Done(s);
    FT_Stroker_Done(s2);
    FT_Done_FreeType(l);
    printf("cube2font: (%d, %d) .. (%d, %d) = (%d, %d) / (%d, %d), %d texs\n", x1, y1, x2, y2, x2 - x1, y2 - y1, w2, h2, numtex);
    return EXIT_SUCCESS;
}

