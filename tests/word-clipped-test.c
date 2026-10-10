#include "word_clipped.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

static unsigned placements, refusals;
static void put(uint8_t *p,uint32_t n) {
    p[0]=n; p[1]=n>>8; p[2]=n>>16; p[3]=n>>24;
}
static void fixture(uint8_t f[200]) {
    memset(f,0,200);
    put(f,73); put(f+4,4); put(f+8,2); put(f+12,UINT32_MAX);
    put(f+16,1); put(f+28,UINT32_MAX);
    put(f+40,56); put(f+44,61); put(f+48,59); put(f+52,65);
    const uint8_t runs[]={1,2,1,0,4}; memcpy(f+56,runs,5);
    const uint16_t colours[]={0,0xf800,0x7e0,0x1f,0xffff,0};
    for (unsigned i=0;i<6;++i) { f[61+i*2]=colours[i]; f[62+i*2]=colours[i]>>8; }
}
static int rejection(const uint8_t *f,size_t bytes,const MnmWordCanvas *c,
                     const MnmWordClip *clip,int32_t x,int32_t y) {
    uint8_t before[144]; MnmWordClippedDraw d,previous;
    memcpy(before,c->pixels,144); memset(&d,0xc7,sizeof(d)); previous=d;
    if (mnm_word_sprite_clip_admit(f,bytes,c,clip,x,y,&d)!=MNM_WORD_INVALID ||
        memcmp(&d,&previous,sizeof(d)) || memcmp(before,c->pixels,144)) return 1;
    if (mnm_word_sprite_clip_draw(f,bytes,c,clip,x,y,&d)!=MNM_WORD_INVALID ||
        memcmp(&d,&previous,sizeof(d)) || memcmp(before,c->pixels,144)) return 1;
    ++refusals; return 0;
}
int main(void) {
    uint8_t f[200], storage[211], expected[211]; fixture(f);
    const int xs[]={-9,-4,-1,0,1,4,7,8,12}, ys[]={-4,-1,0,1,6,7,8,10};
    const MnmWordClip clips[]={{0,0,8,8},{2,2,6,6}};
    const int sx[]={1,2,0,1,2,3}, sy[]={0,0,1,1,1,1};
    const uint16_t colours[]={0,0xf800,0x7e0,0x1f,0xffff,0};
    for (unsigned plane=0;plane<5;++plane) for (unsigned a=0;a<2;++a) for (unsigned k=0;k<2;++k)
        for (unsigned ix=0;ix<9;++ix) for (unsigned iy=0;iy<8;++iy) {
            fixture(f);
            unsigned frame_bytes=73;
            if (plane) {
                memset(f+73,0xde,27); frame_bytes=100; put(f,frame_bytes);
                if (plane==1) put(f+32,76);
                if (plane==2) put(f+36,76);
                if (plane==3) { put(f+32,76); put(f+36,88); }
                if (plane==4) { put(f+32,88); put(f+36,76); }
            }
            uint8_t frame_before[200]; memcpy(frame_before,f,200);
            memset(storage,0xa5,sizeof(storage));
            uint8_t *pixels=storage+32+a;
            for (unsigned i=0;i<72;++i) { pixels[i*2]=0x34; pixels[i*2+1]=0x12; }
            memcpy(expected,storage,sizeof(storage));
            unsigned visible=0;
            for (unsigned i=0;i<6;++i) {
                int x=xs[ix]+sx[i],y=ys[iy]+sy[i];
                if (x>=clips[k].left && x<clips[k].right && y>=clips[k].top && y<clips[k].bottom) {
                    size_t at=32+a+(y*9+x)*2;
                    expected[at]=colours[i]; expected[at+1]=colours[i]>>8; ++visible;
                }
            }
            MnmWordCanvas c={pixels,144,8,8,9}; MnmWordClippedDraw d,admitted;
            if (mnm_word_sprite_clip_admit(f,frame_bytes,&c,&clips[k],xs[ix]-1,ys[iy]+1,&admitted)) return 1;
            /* Admission must not draw, including visible opaque zero. */
            for (unsigned i=0;i<144;++i) if (pixels[i]!=(i%2?0x12:0x34)) return 2;
            if (mnm_word_sprite_clip_draw(f,frame_bytes,&c,&clips[k],xs[ix]-1,ys[iy]+1,&d) ||
                memcmp(storage,expected,sizeof(storage)) || memcmp(&d,&admitted,sizeof(d)) ||
                d.visible_pixels!=visible || d.opaque_pixels!=6 || d.left!=xs[ix] || d.top!=ys[iy] || memcmp(f,frame_before,200)) return 3;
            ++placements;
        }
    MnmWordCanvas c={storage+32,144,8,8,9}; MnmWordClip clip={0,0,8,8};
    const unsigned offsets[]={0,28,40,44,48,52,32,36,4,8};
    const uint32_t values[]={0,0,0,58,58,64,60,60,2049,2049};
    for (unsigned i=0;i<10;++i) {
        fixture(f); put(f+offsets[i],values[i]);
        if (rejection(f,73,&c,&clip,0,1) || rejection(f,73,&c,&clip,-100,1)) return 4;
    }
    /* Plane bounds are checked even for hidden sprites and late rows. */
    const uint32_t plane_bad[]={1,39,55,56,60,61,72,74,UINT32_MAX};
    for (unsigned field=32;field<=36;field+=4) for (unsigned i=0;i<9;++i) {
        fixture(f); put(f+field,plane_bad[i]);
        if (rejection(f,73,&c,&clip,0,1) || rejection(f,73,&c,&clip,-100,1)) return 13;
    }
    fixture(f); f[60]=5; /* Late bad row must not draw the valid first row. */
    if (rejection(f,73,&c,&clip,0,1) || rejection(f,73,&c,&clip,-100,1)) return 5;
    fixture(f); f[59]=0; f[60]=0; /* Zero opaque run. */
    if (rejection(f,73,&c,&clip,0,1)) return 6;
    fixture(f);
    if (rejection(f,39,&c,&clip,0,1) || rejection(f,4*1024*1024+1,&c,&clip,0,1) ||
        rejection(f,73,&c,&clip,INT32_MAX,1) || rejection(f,73,&c,&clip,0,INT32_MIN)) return 7;
    MnmWordCanvas bad=c; bad.bytes=143;
    if (rejection(f,73,&bad,&clip,0,1)) return 8;
    bad=c; bad.stride_words=7; if (rejection(f,73,&bad,&clip,0,1)) return 8;
    bad=c; bad.stride_words=4097; if (rejection(f,73,&bad,&clip,0,1)) return 8;
    bad=c; bad.width=0; if (rejection(f,73,&bad,&clip,0,1)) return 8;
    bad=c; bad.height=2049; if (rejection(f,73,&bad,&clip,0,1)) return 8;
    bad=c; bad.pixels=f; if (rejection(f,73,&bad,&clip,0,1)) return 9;
    const MnmWordClip invalid[]={{-1,0,8,8},{0,-1,8,8},{0,0,9,8},{0,0,8,9},{2,0,2,8},{0,3,8,2}};
    for (unsigned i=0;i<6;++i) if (rejection(f,73,&c,&invalid[i],0,1)) return 10;
    uint8_t nullBefore[144]; memcpy(nullBefore,c.pixels,144);
    MnmWordClippedDraw nullOut,nullSeed; memset(&nullOut,0xc7,sizeof(nullOut)); nullSeed=nullOut;
    if (mnm_word_sprite_clip_draw(0,73,&c,&clip,0,1,&nullOut)!=MNM_WORD_INVALID ||
        mnm_word_sprite_clip_draw(f,73,0,&clip,0,1,&nullOut)!=MNM_WORD_INVALID ||
        mnm_word_sprite_clip_draw(f,73,&c,0,0,1,&nullOut)!=MNM_WORD_INVALID ||
        mnm_word_sprite_clip_draw(f,73,&c,&clip,0,1,0)!=MNM_WORD_INVALID ||
        memcmp(&nullOut,&nullSeed,sizeof(nullOut)) || memcmp(nullBefore,c.pixels,144)) return 12;
    refusals+=4;
    /* Empty frames need no row table, retain input anchors, and write no pixels. */
    uint8_t before[211]; memcpy(before,storage,sizeof(storage));
    for (unsigned i=0;i<3;++i) {
        fixture(f); put(f,40); put(f+4,i==2?3:0); put(f+8,i==1?2:0);
        MnmWordClippedDraw d;
        if (mnm_word_sprite_clip_draw(f,40,&c,&clip,INT32_MIN,INT32_MAX,&d) ||
            memcmp(before,storage,sizeof(storage)) || d.visible_pixels || d.opaque_pixels ||
            d.left!=INT32_MIN || d.top!=INT32_MAX) return 11;
    }
    printf("%u clipped placements, %u atomic refusals, 3 empty no-ops passed\n",placements,refusals);
    return 0;
}
