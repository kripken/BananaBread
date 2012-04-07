static void FUNCNAME(halvetexture)(uchar *src, uint sw, uint sh, uint stride, uchar *dst)
{
    for(uchar *yend = &src[sh*stride]; src < yend;)
    {
        for(uchar *xend = &src[stride]; src < xend; src += 2*BPP, dst += BPP)
        {
            #define OP(c, n) dst[n] = (uint(src[n]) + uint(src[n+BPP]) + uint(src[stride+n]) + uint(src[stride+n+BPP]))>>2
            PIXELOP
            #undef OP
        }
        src += stride;
    }
}

static void FUNCNAME(shifttexture)(uchar *src, uint sw, uint sh, uint stride, uchar *dst, uint dw, uint dh)
{
    uint wfrac = sw/dw, hfrac = sh/dh, wshift = 0, hshift = 0;
    while(dw<<wshift < sw) wshift++;
    while(dh<<hshift < sh) hshift++;
    uint tshift = wshift + hshift;
    for(uchar *yend = &src[sh*stride]; src < yend;)
    {
        for(uchar *xend = &src[stride]; src < xend; src += wfrac*BPP, dst += BPP)
        {        
            #define OP(c, n) c##t = 0
            DEFPIXEL
            #undef OP
            for(uchar *ycur = src, *xend = &ycur[wfrac*BPP], *yend = &src[hfrac*stride]; 
                ycur < yend; 
                ycur += stride, xend += stride)
            {
                for(uchar *xcur = ycur; xcur < xend; xcur += BPP)
                {
                    #define OP(c, n) c##t += xcur[n]    
                    PIXELOP
                    #undef OP
                }
            }
            #define OP(c, n) dst[n] = (c##t)>>tshift
            PIXELOP
            #undef OP
        }
        src += (hfrac-1)*stride; 
    }
}

static void FUNCNAME(scaletexture)(uchar *src, uint sw, uint sh, uint stride, uchar *dst, uint dw, uint dh)
{
    uint wfrac = (sw<<12)/dw, hfrac = (sh<<12)/dh, darea = dw*dh, sarea = sw*sh;
    int over, under;
    for(over = 0; (darea>>over) > sarea; over++);
    for(under = 0; (darea<<under) < sarea; under++);
    uint cscale = clamp(under, over - 12, 12),
         ascale = clamp(12 + under - over, 0, 24),
         dscale = ascale + 12 - cscale,
         area = ((ullong)darea<<ascale)/sarea;
    dw *= wfrac;
    dh *= hfrac;
    for(uint y = 0; y < dh; y += hfrac)
    {
        const uint yn = y + hfrac - 1, yi = y>>12, h = (yn>>12) - yi, ylow = ((yn|(-int(h)>>24))&0xFFFU) + 1 - (y&0xFFFU), yhigh = (yn&0xFFFU) + 1;
        const uchar *ysrc = &src[yi*stride]; 
        for(uint x = 0; x < dw; x += wfrac, dst += BPP)
        {
            const uint xn = x + wfrac - 1, xi = x>>12, w = (xn>>12) - xi, xlow = ((w+0xFFFU)&0x1000U) - (x&0xFFFU), xhigh = (xn&0xFFFU) + 1;
            const uchar *xsrc = &ysrc[xi*BPP], *xend = &xsrc[w*BPP];
            #define OP(c, n) c##t = 0
            DEFPIXEL
            #undef OP
            for(const uchar *xcur = &xsrc[BPP]; xcur < xend; xcur += BPP)
            {
                #define OP(c, n) c##t += xcur[n]    
                PIXELOP
                #undef OP
            }
            #define OP(c, n) c##t = (ylow*(c##t + ((xsrc[n]*xlow + xend[n]*xhigh)>>12)))>>cscale
            PIXELOP
            #undef OP
            if(h)
            {
                xsrc += stride;
                xend += stride;
                for(uint hcur = h; --hcur; xsrc += stride, xend += stride)
                {
                    #define OP(c, n) c = 0
                    DEFPIXEL
                    #undef OP
                    for(const uchar *xcur = &xsrc[BPP]; xcur < xend; xcur += BPP)
                    {
                        #define OP(c, n) c += xcur[n]    
                        PIXELOP
                        #undef OP
                    }
                    #define OP(c, n) c##t += ((c<<12) + xsrc[n]*xlow + xend[n]*xhigh)>>cscale;
                    PIXELOP
                    #undef OP
                }
                #define OP(c, n) c = 0
                DEFPIXEL
                #undef OP
                for(const uchar *xcur = &xsrc[BPP]; xcur < xend; xcur += BPP)
                {
                    #define OP(c, n) c += xcur[n]    
                    PIXELOP
                    #undef OP
                }
                #define OP(c, n) c##t += (yhigh*(c + ((xsrc[n]*xlow + xend[n]*xhigh)>>12)))>>cscale
                PIXELOP
                #undef OP
            }
            #define OP(c, n) dst[n] = (c##t * area)>>dscale
            PIXELOP
            #undef OP
        }
    }
}

#undef FUNCNAME
#undef DEFPIXEL
#undef PIXELOP
#undef BPP

