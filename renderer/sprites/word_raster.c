#include "word_raster.h"

static uint32_t word(const uint8_t *p) {
    return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;
}
static int walk(const uint8_t *f, size_t bytes, const MnmWordCanvas *c,
                MnmWordDraw *d, int write) {
    size_t delta=word(f+40), pixel=word(f+44);
    for (uint32_t y=0; y<d->height; ++y) {
        /* Original word backends advance contiguous streams. Refuse padded
         * or reordered rows rather than silently applying another decoder. */
        if (word(f+40+y*8)!=delta || word(f+44+y*8)!=pixel) return MNM_WORD_INVALID;
        uint32_t x=0, opaque=0;
        while (x<d->width) {
            if (delta>=bytes) return MNM_WORD_INVALID;
            uint32_t n=f[delta++];
            if (n>d->width-x || (opaque && !n)) return MNM_WORD_INVALID;
            if (opaque) {
                if (pixel>bytes || n>(bytes-pixel)/2) return MNM_WORD_INVALID;
                d->opaque_pixels+=n; d->last_run=n;
                d->last_run_x=x; d->last_run_y=y;
                if (write) {
                    size_t at=((size_t)(d->top+(int32_t)y)*c->stride_words+
                               (uint32_t)d->left+x)*2;
                    for (uint32_t i=0; i<n*2; ++i) c->pixels[at+i]=f[pixel+i];
                }
                pixel+=n*2;
            }
            x+=n; opaque=!opaque;
        }
    }
    return MNM_WORD_OK;
}
int mnm_word_sprite_admit(const uint8_t *f, size_t bytes,
                         const MnmWordCanvas *c, int32_t ax, int32_t ay,
                         MnmWordDraw *out) {
    MnmWordDraw d={0};
    if (!f || !c || !out || bytes<48 || bytes>4*1024*1024 ||
        word(f)!=bytes || word(f+28)!=UINT32_MAX ||
        !c->pixels || !c->width || !c->height || c->width>2048 || c->height>2048 ||
        c->stride_words<c->width || c->stride_words>4096 ||
        (size_t)c->stride_words*c->height*2>c->bytes) return MNM_WORD_INVALID;
    uintptr_t fp=(uintptr_t)f,cp=(uintptr_t)c->pixels;
    if (fp>UINTPTR_MAX-bytes || cp>UINTPTR_MAX-c->bytes ||
        (fp<cp+c->bytes && cp<fp+bytes)) return MNM_WORD_INVALID;
    d.width=word(f+4); d.height=word(f+8);
    if (!d.width || !d.height || d.width>2048 || d.height>2048 ||
        40+(size_t)d.height*8>bytes) return MNM_WORD_INVALID;
    /* Widen before subtracting signed origins. Never form out-of-bounds pointers. */
    int64_t left=(int64_t)ax-(int32_t)word(f+12);
    int64_t top=(int64_t)ay-(int32_t)word(f+16);
    if (left<0 || top<0 || left+d.width>=c->width || top+d.height>=c->height)
        return MNM_WORD_CLIPPED;
    d.left=(int32_t)left; d.top=(int32_t)top;
    size_t table=40+(size_t)d.height*8;
    size_t delta=word(f+40), pixel=word(f+44);
    if (delta<table || pixel<delta || pixel>bytes) return MNM_WORD_INVALID;
    /* Auxiliary planes are left to the caller's original shared dispatch. */
    size_t plane_end=bytes;
    for (unsigned i=0; i<2; ++i) {
        uint32_t a=word(f+32+i*4);
        if (a && (a<table || a>bytes)) return MNM_WORD_INVALID;
        if (a && a<plane_end) plane_end=a;
    }
    if (pixel>plane_end) return MNM_WORD_INVALID;
    /* Control bytes cannot consume the pixel plane. Pixel data cannot consume
     * auxiliary planes. Inspect both streams independently before drawing. */
    size_t controls=delta, colours=pixel;
    for (uint32_t y=0; y<d.height; ++y) {
        if (word(f+40+y*8)!=controls || word(f+44+y*8)!=colours)
            return MNM_WORD_INVALID;
        uint32_t x=0, opaque=0;
        while (x<d.width) {
            if (controls>=pixel) return MNM_WORD_INVALID;
            uint32_t n=f[controls++];
            if (n>d.width-x || (opaque && !n)) return MNM_WORD_INVALID;
            if (opaque) {
                if (colours>plane_end || n>(plane_end-colours)/2) return MNM_WORD_INVALID;
                colours+=n*2;
            }
            x+=n; opaque=!opaque;
        }
    }
    int result=walk(f,plane_end,c,&d,0);
    if (!result) *out=d;
    return result;
}
int mnm_word_sprite_draw(const uint8_t *f, size_t bytes,
                        const MnmWordCanvas *c, int32_t ax, int32_t ay,
                        MnmWordDraw *out) {
    MnmWordDraw d;
    int result=mnm_word_sprite_admit(f,bytes,c,ax,ay,&d);
    if (result) return result;
    d.opaque_pixels=0;
    result=walk(f,bytes,c,&d,1);
    if (!result) *out=d;
    return result;
}
