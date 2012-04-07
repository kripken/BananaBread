// Feedback on playing videos:
//   quicktime - ok
//   vlc - ok
//   xine - ok
//   mplayer - ok
//   totem - ok
//   avidemux - ok - 3Apr09-RockKeyman:had to swap UV channels as it showed up blue
//   kino - ok

#include "engine.h"
#include "SDL_mixer.h"

VAR(dbgmovie, 0, 0, 1);

struct aviindexentry
{
    int frame, type, size;
    uint offset;

    aviindexentry() {}
    aviindexentry(int frame, int type, int size, uint offset) : frame(frame), type(type), size(size), offset(offset) {}
};

struct avisegmentinfo
{
    stream::offset offset, videoindexoffset, soundindexoffset;
    int firstindex;
    uint videoindexsize, soundindexsize, indexframes, videoframes, soundframes;
    
    avisegmentinfo() {}
    avisegmentinfo(stream::offset offset, int firstindex) : offset(offset), videoindexoffset(0), soundindexoffset(0), firstindex(firstindex), videoindexsize(0), soundindexsize(0), indexframes(0), videoframes(0), soundframes(0) {}
};

struct aviwriter
{
    stream *f;
    uchar *yuv;
    uint videoframes;
    stream::offset totalsize;
    const uint videow, videoh, videofps;
    string filename;
 
    int soundfrequency, soundchannels;
    Uint16 soundformat;
    
    vector<aviindexentry> index;
    vector<avisegmentinfo> segments;
    
    stream::offset fileframesoffset, fileextframesoffset, filevideooffset, filesoundoffset, superindexvideooffset, superindexsoundoffset;
    
    enum { MAX_CHUNK_DEPTH = 16, MAX_SUPER_INDEX = 1024 };
    stream::offset chunkoffsets[MAX_CHUNK_DEPTH];
    int chunkdepth;
    
    aviindexentry &addindex(int frame, int type, int size)
    {
        avisegmentinfo &seg = segments.last();
        int i = index.length();
        while(--i >= seg.firstindex)
        {
            aviindexentry &e = index[i];
            if(frame > e.frame || (frame == e.frame && type <= e.type)) break;
        }
        return index.insert(i + 1, aviindexentry(frame, type, size, uint(totalsize - chunkoffsets[chunkdepth])));
    }
    
    double filespaceguess() 
    {
        return double(totalsize);
    }
       
    void startchunk(const char *fcc, uint size = 0)
    {
        f->write(fcc, 4);
        f->putlil<uint>(size);
        totalsize += 4 + 4;
        chunkoffsets[++chunkdepth] = totalsize;
        totalsize += size;
    }
    
    void listchunk(const char *fcc, const char *lfcc)
    {
        startchunk(fcc);
        f->write(lfcc, 4);
        totalsize += 4;
    }
    
    void endchunk()
    {
        assert(chunkdepth >= 0);
        --chunkdepth;
    }

    void endlistchunk()
    {
        assert(chunkdepth >= 0);
        int size = int(totalsize - chunkoffsets[chunkdepth]);
        f->seek(-4 - size, SEEK_CUR);
        f->putlil(size);
        f->seek(0, SEEK_END);
        if(size & 1) { f->putchar(0x00); totalsize++; }
        endchunk();
    }
        
    void writechunk(const char *fcc, const void *data, uint len) // simplify startchunk()/endchunk() to avoid f->seek()
    {
        f->write(fcc, 4);
        f->putlil(len);
        f->write(data, len);
        totalsize += 4 + 4 + len;
        if(len & 1) { f->putchar(0x00); totalsize++; }
    }
    
    void close()
    {
        if(!f) return;
        flushsegment();

        uint soundindexes = 0, videoindexes = 0, soundframes = 0, videoframes = 0, indexframes = 0;
        loopv(segments)
        {
            avisegmentinfo &seg = segments[i];
            if(seg.soundindexsize) soundindexes++;
            videoindexes++;
            soundframes += seg.soundframes;
            videoframes += seg.videoframes;
            indexframes += seg.indexframes;
        }
        if(dbgmovie) conoutf(CON_DEBUG, "fileframes: sound=%d, video=%d+%d(dups)\n", soundframes, videoframes, indexframes-videoframes);
        f->seek(fileframesoffset, SEEK_SET);
        f->putlil<uint>(segments[0].indexframes);
        f->seek(filevideooffset, SEEK_SET);
        f->putlil<uint>(segments[0].videoframes);
        if(segments[0].soundframes > 0)
        {
            f->seek(filesoundoffset, SEEK_SET);
            f->putlil<uint>(segments[0].soundframes);
        }
        f->seek(fileextframesoffset, SEEK_SET);
        f->putlil<uint>(indexframes); // total video frames

        f->seek(superindexvideooffset + 2 + 2, SEEK_SET);
        f->putlil<uint>(videoindexes);
        f->seek(superindexvideooffset + 2 + 2 + 4 + 4 + 4 + 4 + 4, SEEK_SET);
        loopv(segments)
        {
            avisegmentinfo &seg = segments[i];
            f->putlil<uint>(seg.videoindexoffset&stream::offset(0xFFFFFFFFU));
            f->putlil<uint>(seg.videoindexoffset>>32);
            f->putlil<uint>(seg.videoindexsize);
            f->putlil<uint>(seg.indexframes);
        }

        if(soundindexes > 0)
        {
            f->seek(superindexsoundoffset + 2 + 2, SEEK_SET);
            f->putlil<uint>(soundindexes);
            f->seek(superindexsoundoffset + 2 + 2 + 4 + 4 + 4 + 4 + 4, SEEK_SET);
            loopv(segments)
            {
                avisegmentinfo &seg = segments[i];
                if(!seg.soundindexsize) continue;
                f->putlil<uint>(seg.soundindexoffset&stream::offset(0xFFFFFFFFU));
                f->putlil<uint>(seg.soundindexoffset>>32);
                f->putlil<uint>(seg.soundindexsize);
                f->putlil<uint>(seg.soundframes);
            }
        }

        f->seek(0, SEEK_END);
        
        DELETEP(f);
    }
    
    aviwriter(const char *name, uint w, uint h, uint fps, bool sound) : f(NULL), yuv(NULL), videoframes(0), totalsize(0), videow(w&~1), videoh(h&~1), videofps(fps), soundfrequency(0),soundchannels(0),soundformat(0)
    {
        copystring(filename, name);
        path(filename);
        if(!strrchr(filename, '.')) concatstring(filename, ".avi");
        
        extern bool nosound; // sound.cpp
        if(sound && !nosound) 
        {
            Mix_QuerySpec(&soundfrequency, &soundformat, &soundchannels);
            const char *desc;
            switch(soundformat)
            {
                case AUDIO_U8:     desc = "u8"; break;
                case AUDIO_S8:     desc = "s8"; break;
                case AUDIO_U16LSB: desc = "u16l"; break;
                case AUDIO_U16MSB: desc = "u16b"; break;
                case AUDIO_S16LSB: desc = "s16l"; break;
                case AUDIO_S16MSB: desc = "s16b"; break;
                default:           desc = "unkn";
            }
            if(dbgmovie) conoutf(CON_DEBUG, "soundspec: %dhz %s x %d", soundfrequency, desc, soundchannels);
        }
    }
    
    ~aviwriter()
    {
        close();
        if(yuv) delete [] yuv;
    }
    
    bool open()
    {
        f = openfile(filename, "wb");
        if(!f) return false;
        
        chunkdepth = -1;
        
        listchunk("RIFF", "AVI ");
        
        listchunk("LIST", "hdrl");
        
        startchunk("avih", 56);
        f->putlil<uint>(1000000 / videofps); // microsecsperframe
        f->putlil<uint>(0); // maxbytespersec
        f->putlil<uint>(0); // reserved
        f->putlil<uint>(0x10 | 0x20); // flags - hasindex|mustuseindex
        fileframesoffset = f->tell();
        f->putlil<uint>(0); // totalvideoframes
        f->putlil<uint>(0); // initialframes
        f->putlil<uint>(soundfrequency > 0 ? 2 : 1); // streams
        f->putlil<uint>(0); // buffersize
        f->putlil<uint>(videow); // video width
        f->putlil<uint>(videoh); // video height
        loopi(4) f->putlil<uint>(0); // reserved
        endchunk(); // avih
        
        listchunk("LIST", "strl");
        
        startchunk("strh", 56);
        f->write("vids", 4); // fcctype
        f->write("I420", 4); // fcchandler
        f->putlil<uint>(0); // flags
        f->putlil<uint>(0); // priority
        f->putlil<uint>(0); // initialframes
        f->putlil<uint>(1); // scale
        f->putlil<uint>(videofps); // rate
        f->putlil<uint>(0); // start
        filevideooffset = f->tell();
        f->putlil<uint>(0); // length
        f->putlil<uint>(videow*videoh*3/2); // suggested buffersize
        f->putlil<uint>(0); // quality
        f->putlil<uint>(0); // samplesize
        f->putlil<ushort>(0); // frame left
        f->putlil<ushort>(0); // frame top
        f->putlil<ushort>(videow); // frame right
        f->putlil<ushort>(videoh); // frame bottom
        endchunk(); // strh
        
        startchunk("strf", 40);
        f->putlil<uint>(40); //headersize
        f->putlil<uint>(videow); // width
        f->putlil<uint>(videoh); // height
        f->putlil<ushort>(3); // planes
        f->putlil<ushort>(12); // bitcount
        f->write("I420", 4); // compression
        f->putlil<uint>(videow*videoh*3/2); // imagesize
        f->putlil<uint>(0); // xres
        f->putlil<uint>(0); // yres;
        f->putlil<uint>(0); // colorsused
        f->putlil<uint>(0); // colorsrequired
        endchunk(); // strf
       
        startchunk("indx", 24 + 16*MAX_SUPER_INDEX);
        superindexvideooffset = f->tell();
        f->putlil<ushort>(4); // longs per entry
        f->putlil<ushort>(0); // index of indexes
        f->putlil<uint>(0); // entries in use
        f->write("00dc", 4); // chunk id
        f->putlil<uint>(0); // reserved 1
        f->putlil<uint>(0); // reserved 2
        f->putlil<uint>(0); // reserved 3
        loopi(MAX_SUPER_INDEX)
        {
            f->putlil<uint>(0); // offset low
            f->putlil<uint>(0); // offset high
            f->putlil<uint>(0); // size
            f->putlil<uint>(0); // duration
        }
        endchunk(); // indx

        startchunk("vprp", 68);
        f->putlil<uint>(0); // video format token
        f->putlil<uint>(0); // video standard
        f->putlil<uint>(videofps); // vertical refresh rate
        f->putlil<uint>(videow); // horizontal total
        f->putlil<uint>(videoh); // vertical total
        int gcd = screen->w, rem = screen->h;
        while(rem > 0) { gcd %= rem; swap(gcd, rem); }
        f->putlil<ushort>(screen->h/gcd); // aspect denominator
        f->putlil<ushort>(screen->w/gcd); // aspect numerator
        f->putlil<uint>(videow); // frame width
        f->putlil<uint>(videoh); // frame height
        f->putlil<uint>(1); // fields per frame
        f->putlil<uint>(videoh); // compressed bitmap height
        f->putlil<uint>(videow); // compressed bitmap width
        f->putlil<uint>(videoh); // valid bitmap height
        f->putlil<uint>(videow); // valid bitmap width
        f->putlil<uint>(0); // valid bitmap x offset
        f->putlil<uint>(0); // valid bitmap y offset
        f->putlil<uint>(0); // video x offset
        f->putlil<uint>(0); // video y start
        endchunk(); // vprp

        endlistchunk(); // LIST strl
                
        if(soundfrequency > 0)
        {
            const int bps = (soundformat==AUDIO_U8 || soundformat == AUDIO_S8) ? 1 : 2;
            
            listchunk("LIST", "strl");
            
            startchunk("strh", 56);
            f->write("auds", 4); // fcctype
            f->putlil<uint>(1); // fcchandler - normally 4cc, but audio is a special case
            f->putlil<uint>(0); // flags
            f->putlil<uint>(0); // priority
            f->putlil<uint>(0); // initialframes
            f->putlil<uint>(1); // scale
            f->putlil<uint>(soundfrequency); // rate
            f->putlil<uint>(0); // start
            filesoundoffset = f->tell();
            f->putlil<uint>(0); // length
            f->putlil<uint>(soundfrequency*bps*soundchannels/2); // suggested buffer size (this is a half second)
            f->putlil<uint>(0); // quality
            f->putlil<uint>(bps*soundchannels); // samplesize
            f->putlil<ushort>(0); // frame left
            f->putlil<ushort>(0); // frame top
            f->putlil<ushort>(0); // frame right
            f->putlil<ushort>(0); // frame bottom
            endchunk(); // strh
            
            startchunk("strf", 18);
            f->putlil<ushort>(1); // format (uncompressed PCM)
            f->putlil<ushort>(soundchannels); // channels
            f->putlil<uint>(soundfrequency); // sampleframes per second
            f->putlil<uint>(soundfrequency*bps*soundchannels); // average bytes per second
            f->putlil<ushort>(bps*soundchannels); // block align <-- guess
            f->putlil<ushort>(bps*8); // bits per sample
            f->putlil<ushort>(0); // size
            endchunk(); //strf

            startchunk("indx", 24 + 16*MAX_SUPER_INDEX);
            superindexsoundoffset = f->tell();
            f->putlil<ushort>(4); // longs per entry
            f->putlil<ushort>(0); // index of indexes
            f->putlil<uint>(0); // entries in use
            f->write("01wb", 4); // chunk id
            f->putlil<uint>(0); // reserved 1
            f->putlil<uint>(0); // reserved 2
            f->putlil<uint>(0); // reserved 3
            loopi(MAX_SUPER_INDEX)
            {
                f->putlil<uint>(0); // offset low
                f->putlil<uint>(0); // offset high
                f->putlil<uint>(0); // size
                f->putlil<uint>(0); // duration
            }
            endchunk(); // indx

            endlistchunk(); // LIST strl
        }
       
        listchunk("LIST", "odml");
        startchunk("dmlh", 4);
        fileextframesoffset = f->tell();
        f->putlil<uint>(0);
        endchunk(); // dmlh
        endlistchunk(); // LIST odml

        listchunk("LIST", "INFO");
        const char *software = "Cube 2: Sauerbraten";
        writechunk("ISFT", software, strlen(software)+1);
        endlistchunk(); // LIST INFO
        
        endlistchunk(); // LIST hdrl
        
        nextsegment();
 
        return true;
    }
  
    static inline void boxsample(const uchar *src, const uint stride, 
                                 const uint area, const uint w, uint h, 
                                 const uint xlow, const uint xhigh, const uint ylow, const uint yhigh,
                                 uint &bdst, uint &gdst, uint &rdst)
    {
        const uchar *end = &src[w<<2];
        uint bt = 0, gt = 0, rt = 0;
        for(const uchar *cur = &src[4]; cur < end; cur += 4)
        {
            bt += cur[0];
            gt += cur[1];
            rt += cur[2];
        }
        bt = ylow*(bt + ((src[0]*xlow + end[0]*xhigh)>>12));
        gt = ylow*(gt + ((src[1]*xlow + end[1]*xhigh)>>12));
        rt = ylow*(rt + ((src[2]*xlow + end[2]*xhigh)>>12));
        if(h) 
        {
            for(src += stride, end += stride; --h; src += stride, end += stride)  
            {
                uint b = 0, g = 0, r = 0;
                for(const uchar *cur = &src[4]; cur < end; cur += 4)
                {
                    b += cur[0];
                    g += cur[1];
                    r += cur[2];
                }
                bt += (b<<12) + src[0]*xlow + end[0]*xhigh;
                gt += (g<<12) + src[1]*xlow + end[1]*xhigh;
                rt += (r<<12) + src[2]*xlow + end[2]*xhigh;
            }
            uint b = 0, g = 0, r = 0;
            for(const uchar *cur = &src[4]; cur < end; cur += 4)
            {
                b += cur[0];
                g += cur[1];
                r += cur[2];
            }
            bt += yhigh*(b + ((src[0]*xlow + end[0]*xhigh)>>12));
            gt += yhigh*(g + ((src[1]*xlow + end[1]*xhigh)>>12));
            rt += yhigh*(r + ((src[2]*xlow + end[2]*xhigh)>>12));
        }
        bdst = (bt*area)>>24;
        gdst = (gt*area)>>24;
        rdst = (rt*area)>>24;
    }
 
    void scaleyuv(const uchar *pixels, uint srcw, uint srch)
    {
        const int flip = -1;
        const uint planesize = videow * videoh;
        if(!yuv) yuv = new uchar[(planesize*3)/2];
        uchar *yplane = yuv, *uplane = yuv + planesize, *vplane = yuv + planesize + planesize/4;
        const int ystride = flip*int(videow), uvstride = flip*int(videow)/2;
        if(flip < 0) { yplane -= int(videoh-1)*ystride; uplane -= int(videoh/2-1)*uvstride; vplane -= int(videoh/2-1)*uvstride; }

        const uint stride = srcw<<2;
        srcw &= ~1;
        srch &= ~1;
        const uint wfrac = (srcw<<12)/videow, hfrac = (srch<<12)/videoh, 
                   area = ((ullong)planesize<<12)/(srcw*srch + 1),
                   dw = videow*wfrac, dh = videoh*hfrac;
  
        for(uint y = 0; y < dh;)
        {
            uint yn = y + hfrac - 1, yi = y>>12, h = (yn>>12) - yi, ylow = ((yn|(-int(h)>>24))&0xFFFU) + 1 - (y&0xFFFU), yhigh = (yn&0xFFFU) + 1;
            y += hfrac;
            uint y2n = y + hfrac - 1, y2i = y>>12, h2 = (y2n>>12) - y2i, y2low = ((y2n|(-int(h2)>>24))&0xFFFU) + 1 - (y&0xFFFU), y2high = (y2n&0xFFFU) + 1;
            y += hfrac;

            const uchar *src = &pixels[yi*stride], *src2 = &pixels[y2i*stride];
            uchar *ydst = yplane, *ydst2 = yplane + ystride, *udst = uplane, *vdst = vplane;
            for(uint x = 0; x < dw;)
            {
                uint xn = x + wfrac - 1, xi = x>>12, w = (xn>>12) - xi, xlow = ((w+0xFFFU)&0x1000U) - (x&0xFFFU), xhigh = (xn&0xFFFU) + 1;
                x += wfrac;
                uint x2n = x + wfrac - 1, x2i = x>>12, w2 = (x2n>>12) - x2i, x2low = ((w2+0xFFFU)&0x1000U) - (x&0xFFFU), x2high = (x2n&0xFFFU) + 1;
                x += wfrac;

                uint b1, g1, r1, b2, g2, r2, b3, g3, r3, b4, g4, r4;
                boxsample(&src[xi<<2], stride, area, w, h, xlow, xhigh, ylow, yhigh, b1, g1, r1);
                boxsample(&src[x2i<<2], stride, area, w2, h, x2low, x2high, ylow, yhigh, b2, g2, r2);
                boxsample(&src2[xi<<2], stride, area, w, h2, xlow, xhigh, y2low, y2high, b3, g3, r3);
                boxsample(&src2[x2i<<2], stride, area, w2, h2, x2low, x2high, y2low, y2high, b4, g4, r4);


                // Y  = 16 + 65.481*R + 128.553*G + 24.966*B
                // Cb = 128 - 37.797*R - 74.203*G + 112.0*B
                // Cr = 128 + 112.0*R - 93.786*G - 18.214*B
                *ydst++ = ((16<<12) + 1052*r1 + 2065*g1 + 401*b1)>>12;
                *ydst++ = ((16<<12) + 1052*r2 + 2065*g2 + 401*b2)>>12;
                *ydst2++ = ((16<<12) + 1052*r3 + 2065*g3 + 401*b3)>>12;;
                *ydst2++ = ((16<<12) + 1052*r4 + 2065*g4 + 401*b4)>>12;;

                const uint b = b1 + b2 + b3 + b4,
                           g = g1 + g2 + g3 + g4,
                           r = r1 + r2 + r3 + r4;
                // note: weights here are scaled by 1<<10, as opposed to 1<<12, since r/g/b are already *4
                *udst++ = ((128<<12) - 152*r - 298*g + 450*b)>>12;
                *vdst++ = ((128<<12) + 450*r - 377*g - 73*b)>>12;
            }

            yplane += 2*ystride;
            uplane += uvstride;
            vplane += uvstride;
        }
    }

    void encodeyuv(const uchar *pixels)
    {
        const int flip = -1;
        const uint planesize = videow * videoh;
        if(!yuv) yuv = new uchar[(planesize*3)/2];
        uchar *yplane = yuv, *uplane = yuv + planesize, *vplane = yuv + planesize + planesize/4;
        const int ystride = flip*int(videow), uvstride = flip*int(videow)/2;
        if(flip < 0) { yplane -= int(videoh-1)*ystride; uplane -= int(videoh/2-1)*uvstride; vplane -= int(videoh/2-1)*uvstride; }

        const uint stride = videow<<2;
        const uchar *src = pixels, *yend = src + videoh*stride;
        while(src < yend)    
        {
            const uchar *src2 = src + stride, *xend = src2;
            uchar *ydst = yplane, *ydst2 = yplane + ystride, *udst = uplane, *vdst = vplane;
            while(src < xend)
            {
                const uint b1 = src[0], g1 = src[1], r1 = src[2],
                           b2 = src[4], g2 = src[5], r2 = src[6],
                           b3 = src2[0], g3 = src2[1], r3 = src2[2],
                           b4 = src2[4], g4 = src2[5], r4 = src2[6];

                // Y  = 16 + 65.481*R + 128.553*G + 24.966*B
                // Cb = 128 - 37.797*R - 74.203*G + 112.0*B
                // Cr = 128 + 112.0*R - 93.786*G - 18.214*B
                *ydst++ = ((16<<12) + 1052*r1 + 2065*g1 + 401*b1)>>12;
                *ydst++ = ((16<<12) + 1052*r2 + 2065*g2 + 401*b2)>>12;
                *ydst2++ = ((16<<12) + 1052*r3 + 2065*g3 + 401*b3)>>12;;
                *ydst2++ = ((16<<12) + 1052*r4 + 2065*g4 + 401*b4)>>12;;

                const uint b = b1 + b2 + b3 + b4,
                           g = g1 + g2 + g3 + g4,
                           r = r1 + r2 + r3 + r4;
                // note: weights here are scaled by 1<<10, as opposed to 1<<12, since r/g/b are already *4
                *udst++ = ((128<<12) - 152*r - 298*g + 450*b)>>12;
                *vdst++ = ((128<<12) + 450*r - 377*g - 73*b)>>12;

                src += 8;
                src2 += 8; 
            }
            src = src2;
            yplane += 2*ystride;
            uplane += uvstride;
            vplane += uvstride;
        }
    }

    void compressyuv(const uchar *pixels)
    {
        const int flip = -1;
        const uint planesize = videow * videoh;
        if(!yuv) yuv = new uchar[(planesize*3)/2];
        uchar *yplane = yuv, *uplane = yuv + planesize, *vplane = yuv + planesize + planesize/4;
        const int ystride = flip*int(videow), uvstride = flip*int(videow)/2;
        if(flip < 0) { yplane -= int(videoh-1)*ystride; uplane -= int(videoh/2-1)*uvstride; vplane -= int(videoh/2-1)*uvstride; }

        const uint stride = videow<<2;
        const uchar *src = pixels, *yend = src + videoh*stride;
        while(src < yend)
        {
            const uchar *src2 = src + stride, *xend = src2;
            uchar *ydst = yplane, *ydst2 = yplane + ystride, *udst = uplane, *vdst = vplane;
            while(src < xend)
            {
                *ydst++ = src[0];
                *ydst++ = src[4];
                *ydst2++ = src2[0];
                *ydst2++ = src2[4];
 
                *udst++ = (uint(src[1]) + uint(src[5]) + uint(src2[1]) + uint(src2[5])) >> 2;
                *vdst++ = (uint(src[2]) + uint(src[6]) + uint(src2[2]) + uint(src2[6])) >> 2;

                src += 8;
                src2 += 8;
            }
            src = src2;
            yplane += 2*ystride;
            uplane += uvstride;
            vplane += uvstride;
        }
    }

    bool writesound(uchar *data, uint framesize, uint frame)
    {
        // do conversion in-place to little endian format
        // note that xoring by half the range yields the same bit pattern as subtracting the range regardless of signedness
        // ... so can toggle signedness just by xoring the high byte with 0x80
        switch(soundformat)
        {
            case AUDIO_U8:
                for(uchar *dst = data, *end = &data[framesize]; dst < end; dst++) *dst ^= 0x80;
                break;
            case AUDIO_S8:
                break;
            case AUDIO_U16LSB:
                for(uchar *dst = &data[1], *end = &data[framesize]; dst < end; dst += 2) *dst ^= 0x80;
                break;
            case AUDIO_U16MSB:
                for(ushort *dst = (ushort *)data, *end = (ushort *)&data[framesize]; dst < end; dst++)
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                    *dst = endianswap(*dst) ^ 0x0080;
#else
                    *dst = endianswap(*dst) ^ 0x8000;
#endif
                break;
            case AUDIO_S16LSB:
                break;
            case AUDIO_S16MSB:
                endianswap((short *)data, framesize/2);
                break;
        }
       
        if(totalsize - segments.last().offset + framesize > 1000*1000*1000 && !nextsegment()) return false;
 
        addindex(frame, 1, framesize);
    
        writechunk("01wb", data, framesize);

        return true;
    }
   

    enum
    {
        VID_RGB = 0,
        VID_YUV,
        VID_YUV420
    };

    void flushsegment()
    {
        endlistchunk(); // LIST movi

        avisegmentinfo &seg = segments.last();

        uint indexframes = 0, videoframes = 0, soundframes = 0;
        for(int i = seg.firstindex; i < index.length(); i++)
        {
            aviindexentry &e = index[i];
            if(e.type) soundframes++; 
            else 
            {
                if(i == seg.firstindex || e.offset != index[i-1].offset)
                    videoframes++;
                indexframes++;
            }
        }

        if(segments.length() == 1)
        {
            startchunk("idx1", index.length()*16);
            loopv(index)
            {
                aviindexentry &entry = index[i];
                // printf("%3d %s %08x\n", i, (entry.type==1)?"s":"v", entry.offset);
                f->write(entry.type ? "01wb" : "00dc", 4); // chunkid
                f->putlil<uint>(0x10); // flags - KEYFRAME
                f->putlil<uint>(entry.offset); // offset (relative to movi)
                f->putlil<uint>(entry.size); // size
            }
            endchunk();
        }

        seg.videoframes = videoframes;
        seg.videoindexoffset = totalsize;
        startchunk("ix00", 24 + indexframes*8);
        f->putlil<ushort>(2); // longs per entry
        f->putlil<ushort>(0x0100); // index of chunks
        f->putlil<uint>(indexframes); // entries in use
        f->write("00dc", 4); // chunk id
        f->putlil<uint>(seg.offset&stream::offset(0xFFFFFFFFU)); // offset low
        f->putlil<uint>(seg.offset>>32); // offset high
        f->putlil<uint>(0); // reserved 3
        for(int i = seg.firstindex; i < index.length(); i++)
        {
            aviindexentry &e = index[i];
            if(e.type) continue;
            f->putlil<uint>(e.offset + 4 + 4);
            f->putlil<uint>(e.size);
        }
        endchunk(); // ix00
        seg.videoindexsize = uint(totalsize - seg.videoindexoffset);

        if(soundframes)
        {
            seg.soundframes = soundframes;
            seg.soundindexoffset = totalsize;
            startchunk("ix01", 24 + soundframes*8);
            f->putlil<ushort>(2); // longs per entry
            f->putlil<ushort>(0x0100); // index of chunks
            f->putlil<uint>(soundframes); // entries in use
            f->write("01wb", 4); // chunk id
            f->putlil<uint>(seg.offset&stream::offset(0xFFFFFFFFU)); // offset low
            f->putlil<uint>(seg.offset>>32); // offset high
            f->putlil<uint>(0); // reserved 3
            for(int i = seg.firstindex; i < index.length(); i++)
            {
                aviindexentry &e = index[i];
                if(!e.type) continue;
                f->putlil<uint>(e.offset + 4 + 4);
                f->putlil<uint>(e.size);
            }
            endchunk(); // ix01
            seg.soundindexsize = uint(totalsize - seg.soundindexoffset);
        }

        endlistchunk(); // RIFF AVI/AVIX
    }

    bool nextsegment()
    {
        if(segments.length()) 
        {
            if(segments.length() >= MAX_SUPER_INDEX) return false;
            flushsegment();
            listchunk("RIFF", "AVIX");
        }
        listchunk("LIST", "movi");
        segments.add(avisegmentinfo(chunkoffsets[chunkdepth], index.length()));
        return true;
    }
  
    bool writevideoframe(const uchar *pixels, uint srcw, uint srch, int format, uint frame)
    {
        if(frame < videoframes) return true;
        
        switch(format)
        {
            case VID_RGB: 
                if(srcw != videow || srch != videoh) scaleyuv(pixels, srcw, srch);
                else encodeyuv(pixels);
                break;
            case VID_YUV:
                compressyuv(pixels);
                break;
        }

        const uint framesize = (videow * videoh * 3) / 2;
        if(totalsize - segments.last().offset + framesize > 1000*1000*1000 && !nextsegment()) return false;

        while(videoframes <= frame) addindex(videoframes++, 0, framesize);

        writechunk("00dc", format == VID_YUV420 ? pixels : yuv, framesize);

        return true;
    }
    
};

VAR(movieaccelblit, 0, 0, 1);
VAR(movieaccelyuv, 0, 1, 1);
VARP(movieaccel, 0, 1, 1);
VARP(moviesync, 0, 0, 1);
FVARP(movieminquality, 0, 0, 1);

namespace recorder
{
    static enum { REC_OK = 0, REC_USERHALT, REC_TOOSLOW, REC_FILERROR } state = REC_OK;
    
    static aviwriter *file = NULL;
    static int starttime = 0;
    
    static int stats[1000];
    static int statsindex = 0;
    static uint dps = 0; // dropped frames per sample
    
    enum { MAXSOUNDBUFFERS = 128 }; // sounds queue up until there is a video frame, so at low fps you'll need a bigger queue
    struct soundbuffer
    {
        uchar *sound;
        uint size, maxsize;
        uint frame;
        
        soundbuffer() : sound(NULL), maxsize(0) {}
        ~soundbuffer() { cleanup(); }
        
        void load(uchar *stream, uint len, uint fnum)
        {
            if(len > maxsize)
            {
                DELETEA(sound);
                sound = new uchar[len];
                maxsize = len;
            }
            size = len;
            frame = fnum;
            memcpy(sound, stream, len);
        }
        
        void cleanup() { DELETEA(sound); maxsize = 0; }
    };
    static queue<soundbuffer, MAXSOUNDBUFFERS> soundbuffers;
    static SDL_mutex *soundlock = NULL;
    
    enum { MAXVIDEOBUFFERS = 2 }; // double buffer
    struct videobuffer 
    {
        uchar *video;
        uint w, h, bpp, frame;
        int format;

        videobuffer() : video(NULL){}
        ~videobuffer() { cleanup(); }

        void init(int nw, int nh, int nbpp)
        {
            DELETEA(video);
            w = nw;
            h = nh;
            bpp = nbpp;
            video = new uchar[w*h*bpp];
            format = -1;
        }
         
        void cleanup() { DELETEA(video); }
    };
    static queue<videobuffer, MAXVIDEOBUFFERS> videobuffers;
    static uint lastframe = ~0U;

    static GLuint scalefb = 0, scaletex[2] = { 0, 0 };
    static uint scalew = 0, scaleh = 0;
    static GLuint encodefb = 0, encoderb = 0;

    static SDL_Thread *thread = NULL;
    static SDL_mutex *videolock = NULL;
    static SDL_cond *shouldencode = NULL, *shouldread = NULL;

    bool isrecording() { return file != NULL; }
    
    float calcquality()
    {
        return 1.0f - float(dps)/float(dps+file->videofps); // strictly speaking should lock to read dps - 1.0=perfect, 0.5=half of frames are beingdropped
    }

    int gettime()
    {
        return inbetweenframes ? getclockmillis() : totalmillis;
    }

    int videoencoder(void *data) // runs on a separate thread
    {
        for(int numvid = 0, numsound = 0;;)
        {   
            SDL_LockMutex(videolock);
            for(; numvid > 0; numvid--) videobuffers.remove();
            SDL_CondSignal(shouldread);
            while(videobuffers.empty() && state == REC_OK) SDL_CondWait(shouldencode, videolock);
            if(state != REC_OK) { SDL_UnlockMutex(videolock); break; }
            videobuffer &m = videobuffers.removing();
            numvid++;
            SDL_UnlockMutex(videolock);
            
            if(file->soundfrequency > 0)
            {
                // chug data from lock protected buffer to avoid holding lock while writing to file
                SDL_LockMutex(soundlock);
                for(; numsound > 0; numsound--) soundbuffers.remove();
                for(; numsound < soundbuffers.length(); numsound++)
                {
                    soundbuffer &s = soundbuffers.removing(numsound);
                    if(s.frame > m.frame) break; // sync with video
                }
                SDL_UnlockMutex(soundlock);
                loopi(numsound)
                {
                    soundbuffer &s = soundbuffers.removing(i);
                    if(!file->writesound(s.sound, s.size, s.frame)) state = REC_FILERROR;
                }
            }
            
            int duplicates = m.frame - (int)file->videoframes + 1;
            if(duplicates > 0) // determine how many frames have been dropped over the sample window
            {
                dps -= stats[statsindex];
                stats[statsindex] = duplicates-1;
                dps += stats[statsindex];
                statsindex = (statsindex+1)%file->videofps;
            }
            //printf("frame %d->%d (%d dps): sound = %d bytes\n", file->videoframes, nextframenum, dps, m.soundlength);
            if(calcquality() < movieminquality) state = REC_TOOSLOW;
            else if(!file->writevideoframe(m.video, m.w, m.h, m.format, m.frame)) state = REC_FILERROR;
            
            m.frame = ~0U;
        }
        
        return 0;
    }
    
    void soundencoder(void *udata, Uint8 *stream, int len) // callback occurs on a separate thread
    {
        SDL_LockMutex(soundlock);
        if(soundbuffers.full()) 
        {
            if(movieminquality >= 1) state = REC_TOOSLOW;
        }
        else if(state == REC_OK)
        {
            uint nextframe = (max(gettime() - starttime, 0)*file->videofps)/1000;
            soundbuffer &s = soundbuffers.add();
            s.load((uchar *)stream, len, nextframe);
        }
        SDL_UnlockMutex(soundlock);
    }
    
    void start(const char *filename, int videofps, int videow, int videoh, bool sound) 
    {
        if(file) return;
       
        useshaderbyname("moviergb");
        useshaderbyname("movieyuv");
        useshaderbyname("moviey");
        useshaderbyname("movieu");
        useshaderbyname("moviev");
 
        int fps, bestdiff, worstdiff;
        getfps(fps, bestdiff, worstdiff);
        if(videofps > fps) conoutf(CON_WARN, "frame rate may be too low to capture at %d fps", videofps);
        
        if(videow%2) videow += 1;
        if(videoh%2) videoh += 1;

        file = new aviwriter(filename, videow, videoh, videofps, sound);
        if(!file->open()) 
        { 
            conoutf(CON_ERROR, "unable to create file %s", filename);
            DELETEP(file);
            return;
        }
        conoutf("movie recording to: %s %dx%d @ %dfps%s", file->filename, file->videow, file->videoh, file->videofps, (file->soundfrequency>0)?" + sound":"");
        
        starttime = gettime();
        loopi(file->videofps) stats[i] = 0;
        statsindex = 0;
        dps = 0;
        
        lastframe = ~0U;
        videobuffers.clear();
        loopi(MAXVIDEOBUFFERS)
        {
            uint w = screen->w, h = screen->w;
            videobuffers.data[i].init(w, h, 4);
            videobuffers.data[i].frame = ~0U;
        }
        
        soundbuffers.clear();
        
        soundlock = SDL_CreateMutex();
        videolock = SDL_CreateMutex();
        shouldencode = SDL_CreateCond();
        shouldread = SDL_CreateCond();
        thread = SDL_CreateThread(videoencoder, NULL); 
        if(file->soundfrequency > 0) Mix_SetPostMix(soundencoder, NULL);
    }
    
    void cleanup()
    {
        if(scalefb) { glDeleteFramebuffers_(1, &scalefb); scalefb = 0; }
        if(scaletex[0] || scaletex[1]) { glDeleteTextures(2, scaletex); memset(scaletex, 0, sizeof(scaletex)); }
        scalew = scaleh = 0;
        if(encodefb) { glDeleteFramebuffers_(1, &encodefb); encodefb = 0; }
        if(encoderb) { glDeleteRenderbuffers_(1, &encoderb); encoderb = 0; }
    }

    void stop()
    {
        if(!file) return;
        if(state == REC_OK) state = REC_USERHALT;
        if(file->soundfrequency > 0) Mix_SetPostMix(NULL, NULL);
        
        SDL_LockMutex(videolock); // wakeup thread enough to kill it
        SDL_CondSignal(shouldencode);
        SDL_UnlockMutex(videolock);
        
        SDL_WaitThread(thread, NULL); // block until thread is finished

        cleanup();

        loopi(MAXVIDEOBUFFERS) videobuffers.data[i].cleanup();
        loopi(MAXSOUNDBUFFERS) soundbuffers.data[i].cleanup();

        SDL_DestroyMutex(soundlock);
        SDL_DestroyMutex(videolock);
        SDL_DestroyCond(shouldencode);
        SDL_DestroyCond(shouldread);

        soundlock = videolock = NULL;
        shouldencode = shouldread = NULL;
        thread = NULL;
 
        static const char *mesgs[] = { "ok", "stopped", "computer too slow", "file error"};
        conoutf("movie recording halted: %s, %d frames", mesgs[state], file->videoframes);
        
        DELETEP(file);
        state = REC_OK;
    }
  
    void drawquad(float tw, float th, float x, float y, float w, float h)
    {
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0,  0);  glVertex2f(x,   y);
        glTexCoord2f(tw, 0);  glVertex2f(x+w, y);
        glTexCoord2f(0,  th); glVertex2f(x,   y+h);
        glTexCoord2f(tw, th); glVertex2f(x+w, y+h);
        glEnd();
    }

    void readbuffer(videobuffer &m, uint nextframe)
    {
        bool accelyuv = movieaccelyuv && renderpath!=R_FIXEDFUNCTION && !(m.w%8),
             usefbo = movieaccel && hasFBO && hasTR && file->videow <= (uint)screen->w && file->videoh <= (uint)screen->h && (accelyuv || file->videow < (uint)screen->w || file->videoh < (uint)screen->h);
        uint w = screen->w, h = screen->h;
        if(usefbo) { w = file->videow; h = file->videoh; }
        if(w != m.w || h != m.h) m.init(w, h, 4);
        m.format = aviwriter::VID_RGB;
        m.frame = nextframe;

        glPixelStorei(GL_PACK_ALIGNMENT, texalign(m.video, m.w, 4));
        if(usefbo)
        {
            uint tw = screen->w, th = screen->h;
            if(hasFBB && movieaccelblit) { tw = max(tw/2, m.w); th = max(th/2, m.h); }
            if(tw != scalew || th != scaleh)
            {
                if(!scalefb) glGenFramebuffers_(1, &scalefb);
                loopi(2)
                {
                    if(!scaletex[i]) glGenTextures(1, &scaletex[i]);
                    createtexture(scaletex[i], tw, th, NULL, 3, 1, GL_RGB, GL_TEXTURE_RECTANGLE_ARB);
                }
                scalew = tw;
                scaleh = th;
            }
            if(accelyuv && (!encodefb || !encoderb))
            {
                if(!encodefb) glGenFramebuffers_(1, &encodefb);
                glBindFramebuffer_(GL_FRAMEBUFFER_EXT, encodefb);
                if(!encoderb) glGenRenderbuffers_(1, &encoderb);
                glBindRenderbuffer_(GL_RENDERBUFFER_EXT, encoderb);
                glRenderbufferStorage_(GL_RENDERBUFFER_EXT, GL_RGBA, (m.w*3)/8, m.h);
                glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, encoderb);
                glBindRenderbuffer_(GL_RENDERBUFFER_EXT, 0);
                glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);
            }
                     
            if(tw < (uint)screen->w || th < (uint)screen->h)
            {
                glBindFramebuffer_(GL_READ_FRAMEBUFFER_EXT, 0);
                glBindFramebuffer_(GL_DRAW_FRAMEBUFFER_EXT, scalefb);
                glFramebufferTexture2D_(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, scaletex[0], 0);
                glBlitFramebuffer_(0, 0, screen->w, screen->h, 0, 0, tw, th, GL_COLOR_BUFFER_BIT, GL_LINEAR);
                glBindFramebuffer_(GL_DRAW_FRAMEBUFFER_EXT, 0);
            }
            else
            {
                glBindTexture(GL_TEXTURE_RECTANGLE_ARB, scaletex[0]);
                glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, screen->w, screen->h);
            }

            if(tw > m.w || th > m.h || (!accelyuv && renderpath != R_FIXEDFUNCTION && tw >= m.w && th >= m.h))
            {
                glBindFramebuffer_(GL_FRAMEBUFFER_EXT, scalefb);
                glViewport(0, 0, tw, th);
                glColor3f(1, 1, 1);
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                glOrtho(0, tw, 0, th, -1, 1);
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                glEnable(GL_TEXTURE_RECTANGLE_ARB);
                do
                {
                    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, scaletex[1], 0);
                    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, scaletex[0]);
                    uint dw = max(tw/2, m.w), dh = max(th/2, m.h);
                    if(dw == m.w && dh == m.h && !accelyuv && renderpath != R_FIXEDFUNCTION) { SETSHADER(movieyuv); m.format = aviwriter::VID_YUV; }
                    else SETSHADER(moviergb);
                    drawquad(tw, th, 0, 0, dw, dh);
                    tw = dw;
                    th = dh;
                    swap(scaletex[0], scaletex[1]);
                } while(tw > m.w || th > m.h);
                glDisable(GL_TEXTURE_RECTANGLE_ARB);
            }
            if(accelyuv)
            {
                glBindFramebuffer_(GL_FRAMEBUFFER_EXT, encodefb); 
                glViewport(0, 0, (m.w*3)/8, m.h);
                glColor3f(1, 1, 1);
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                glOrtho(0, (m.w*3)/8, m.h, 0, -1, 1);
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                glEnable(GL_TEXTURE_RECTANGLE_ARB);
                glBindTexture(GL_TEXTURE_RECTANGLE_ARB, scaletex[0]); 
                SETSHADER(moviey); drawquad(m.w, m.h, 0, 0, m.w/4, m.h);
                SETSHADER(moviev); drawquad(m.w, m.h, m.w/4, 0, m.w/8, m.h/2);
                SETSHADER(movieu); drawquad(m.w, m.h, m.w/4, m.h/2, m.w/8, m.h/2);
                glDisable(GL_TEXTURE_RECTANGLE_ARB);
                const uint planesize = m.w * m.h;
                glPixelStorei(GL_PACK_ALIGNMENT, texalign(m.video, m.w/4, 4)); 
                glReadPixels(0, 0, m.w/4, m.h, GL_BGRA, GL_UNSIGNED_BYTE, m.video);
                glPixelStorei(GL_PACK_ALIGNMENT, texalign(&m.video[planesize], m.w/8, 4));
                glReadPixels(m.w/4, 0, m.w/8, m.h/2, GL_BGRA, GL_UNSIGNED_BYTE, &m.video[planesize]);
                glPixelStorei(GL_PACK_ALIGNMENT, texalign(&m.video[planesize + planesize/4], m.w/8, 4));
                glReadPixels(m.w/4, m.h/2, m.w/8, m.h/2, GL_BGRA, GL_UNSIGNED_BYTE, &m.video[planesize + planesize/4]);
                m.format = aviwriter::VID_YUV420;
            }
            else
            {
                glBindFramebuffer_(GL_FRAMEBUFFER_EXT, scalefb);
                glReadPixels(0, 0, m.w, m.h, GL_BGRA, GL_UNSIGNED_BYTE, m.video);
            }
            glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);
            glViewport(0, 0, screen->w, screen->h);

        }
        else glReadPixels(0, 0, m.w, m.h, GL_BGRA, GL_UNSIGNED_BYTE, m.video);
    }
 
    bool readbuffer()
    {
        if(!file) return false;
        if(state != REC_OK)
        {
            stop();
            return false;
        }
        SDL_LockMutex(videolock);
        if(moviesync && videobuffers.full()) SDL_CondWait(shouldread, videolock);
        uint nextframe = (max(gettime() - starttime, 0)*file->videofps)/1000;
        if(!videobuffers.full() && (lastframe == ~0U || nextframe > lastframe))
        {
            videobuffer &m = videobuffers.adding();
            SDL_UnlockMutex(videolock);
            readbuffer(m, nextframe);
            SDL_LockMutex(videolock);
            lastframe = nextframe;
            videobuffers.add();
            SDL_CondSignal(shouldencode);
        }
        SDL_UnlockMutex(videolock);
        return true;
    }

    void drawhud()
    {
        int w = screen->w, h = screen->h;

        gettextres(w, h);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, w, h, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glEnable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);
        defaultshader->set();

        glPushMatrix();
        glScalef(1/3.0f, 1/3.0f, 1);
    
        double totalsize = file->filespaceguess();
        const char *unit = "KB";
        if(totalsize >= 1e9) { totalsize /= 1e9; unit = "GB"; }
        else if(totalsize >= 1e6) { totalsize /= 1e6; unit = "MB"; }
        else totalsize /= 1e3;

        draw_textf("recorded %.1f%s %d%%", w*3-10*FONTH, h*3-FONTH-FONTH*3/2, totalsize, unit, int(calcquality()*100)); 

        glPopMatrix();

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
    }

    void capture()
    {
        if(readbuffer()) drawhud();
    }
}

VARP(moview, 0, 320, 10000);
VARP(movieh, 0, 240, 10000);
VARP(moviefps, 1, 24, 1000);
VARP(moviesound, 0, 1, 1);

void movie(char *name)
{
    if(name[0] == '\0') recorder::stop();
    else if(!recorder::isrecording()) recorder::start(name, moviefps, moview ? moview : screen->w, movieh ? movieh : screen->h, moviesound!=0);
}

COMMAND(movie, "s");
ICOMMAND(movierecording, "", (), intret(recorder::isrecording() ? 1 : 0));

