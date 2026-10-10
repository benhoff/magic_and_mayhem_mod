#include "word_clipped.h"

static uint32_t word(const uint8_t *p) {
    return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;
}
static uint32_t limit(int64_t value, uint32_t extent) {
    return value<0?0:value>extent?extent:(uint32_t)value;
}
int mnm_word_sprite_clip_admit(const uint8_t *f, size_t bytes,
                             const MnmWordCanvas *c, const MnmWordClip *clip,
                             int32_t ax, int32_t ay, MnmWordClippedDraw *out) {
    if (!f || !c || !clip || !out || bytes<40 || bytes>4*1024*1024 ||
        word(f)!=bytes || word(f+28)!=UINT32_MAX ||
        !c->pixels || !c->width || !c->height || c->width>2048 || c->height>2048 ||
        c->stride_words<c->width || c->stride_words>4096 ||
        (size_t)c->stride_words*c->height*2>c->bytes ||
        clip->left<0 || clip->top<0 || clip->right<=clip->left ||
        clip->bottom<=clip->top || clip->right>(int32_t)c->width ||
        clip->bottom>(int32_t)c->height) return MNM_WORD_INVALID;
    uintptr_t fp=(uintptr_t)f, cp=(uintptr_t)c->pixels;
    if (fp>UINTPTR_MAX-bytes || cp>UINTPTR_MAX-c->bytes ||
        (fp<cp+c->bytes && cp<fp+bytes)) return MNM_WORD_INVALID;
    MnmWordClippedDraw d={0};
    d.width=word(f+4); d.height=word(f+8);
    if (d.width>2048 || d.height>2048) return MNM_WORD_INVALID;
    d.left=ax; d.top=ay;
    if (!d.width || !d.height) { *out=d; return MNM_WORD_OK; }
    int64_t left=(int64_t)ax-(int32_t)word(f+12);
    int64_t top=(int64_t)ay-(int32_t)word(f+16);
    if (left<-4096 || left>4096 || top<-4096 || top>4096)
        return MNM_WORD_INVALID;
    d.left=(int32_t)left; d.top=(int32_t)top;
    d.source_left=limit(clip->left-left,d.width);
    d.source_right=limit(clip->right-left,d.width);
    d.source_top=limit(clip->top-top,d.height);
    d.source_bottom=limit(clip->bottom-top,d.height);
    size_t table=40+(size_t)d.height*8;
    if (table>bytes) return MNM_WORD_INVALID;
    /* Only the main word plane is drawn. Opaque auxiliary payloads stay owned
     * by the original caller; their earliest offset bounds our colour reads. */
    size_t plane_end=bytes;
    for (unsigned i=0;i<2;++i) {
        uint32_t auxiliary=word(f+32+i*4);
        if (auxiliary && (auxiliary<table || auxiliary>bytes)) return MNM_WORD_INVALID;
        if (auxiliary && auxiliary<plane_end) plane_end=auxiliary;
    }
    size_t control=word(f+40), colour=word(f+44), pixel_base=colour;
    if (control<table || colour<control || colour>plane_end) return MNM_WORD_INVALID;
    for (uint32_t y=0; y<d.height; ++y) {
        if (word(f+40+y*8)!=control || word(f+44+y*8)!=colour)
            return MNM_WORD_INVALID;
        uint32_t x=0, opaque=0;
        while (x<d.width) {
            if (control>=pixel_base) return MNM_WORD_INVALID;
            uint32_t n=f[control++];
            if (n>d.width-x || (opaque && !n)) return MNM_WORD_INVALID;
            if (opaque) {
                if (colour>plane_end || n>(plane_end-colour)/2) return MNM_WORD_INVALID;
                d.opaque_pixels+=n;
                uint32_t begin=x>d.source_left?x:d.source_left;
                uint32_t end=x+n<d.source_right?x+n:d.source_right;
                if (y>=d.source_top && y<d.source_bottom && end>begin)
                    d.visible_pixels+=end-begin;
                colour+=n*2;
            }
            x+=n; opaque=!opaque;
        }
    }
    *out=d;
    return MNM_WORD_OK;
}
int mnm_word_sprite_clip_draw(const uint8_t *f, size_t bytes,
                            const MnmWordCanvas *c, const MnmWordClip *clip,
                            int32_t ax, int32_t ay, MnmWordClippedDraw *out) {
    if (!out) return MNM_WORD_INVALID;
    MnmWordClippedDraw d;
    int result=mnm_word_sprite_clip_admit(f,bytes,c,clip,ax,ay,&d);
    if (result) return result;
    if (d.visible_pixels) {
        for (uint32_t y=d.source_top; y<d.source_bottom; ++y) {
            size_t control=word(f+40+y*8), colour=word(f+44+y*8);
            uint32_t x=0, opaque=0;
            while (x<d.width) {
                uint32_t n=f[control++];
                if (opaque) {
                    uint32_t begin=x>d.source_left?x:d.source_left;
                    uint32_t end=x+n<d.source_right?x+n:d.source_right;
                    if (end>begin) {
                        size_t at=((size_t)(d.top+(int32_t)y)*c->stride_words+
                                   (uint32_t)(d.left+(int32_t)begin))*2;
                        size_t source=colour+(begin-x)*2;
                        for (uint32_t i=0; i<(end-begin)*2; ++i)
                            c->pixels[at+i]=f[source+i];
                    }
                    colour+=n*2;
                }
                x+=n; opaque=!opaque;
            }
        }
    }
    *out=d;
    return MNM_WORD_OK;
}
