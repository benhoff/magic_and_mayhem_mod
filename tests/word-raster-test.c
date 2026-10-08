#include "word_raster.h"
#include <string.h>
#include <stdio.h>
static void put(uint8_t* p,uint32_t n){p[0]=n;p[1]=n>>8;p[2]=n>>16;p[3]=n>>24;}
int main(void){
    uint8_t frame[200]={0},pixels[144],expected[144];
    put(frame,73);put(frame+4,4);put(frame+8,2);put(frame+12,UINT32_MAX);put(frame+16,1);put(frame+28,UINT32_MAX);
    put(frame+40,56);put(frame+44,61);put(frame+48,59);put(frame+52,65);
    const uint8_t runs[]={1,2,1,0,4};memcpy(frame+56,runs,5);
    const uint16_t colours[]={0,0xf800,0x7e0,0x1f,0xffff,0};
    for(unsigned i=0;i<6;++i){frame[61+i*2]=colours[i];frame[62+i*2]=colours[i]>>8;}
    for(unsigned i=0;i<72;++i){pixels[i*2]=0x34;pixels[i*2+1]=0x12;}
    memcpy(expected,pixels,144);
    const unsigned positions[]={21,22,29,30,31,32};
    for(unsigned i=0;i<6;++i){expected[positions[i]*2]=colours[i];expected[positions[i]*2+1]=colours[i]>>8;}
    MnmWordCanvas canvas={pixels,sizeof(pixels),8,8,9};MnmWordDraw draw;
    if(mnm_word_sprite_draw(frame,73,&canvas,1,3,&draw)||memcmp(pixels,expected,144)||draw.opaque_pixels!=6)return 1;
    if(mnm_word_sprite_draw(frame,73,&canvas,3,3,&draw)!=MNM_WORD_CLIPPED||memcmp(pixels,expected,144))return 2;
    /* A late malformed row must not alter even the valid first row. */
    frame[60]=5;
    if(mnm_word_sprite_draw(frame,73,&canvas,1,3,&draw)!=MNM_WORD_INVALID||memcmp(pixels,expected,144))return 3;
    frame[60]=4;
    MnmWordCanvas alias={frame,144,8,8,9};
    if(mnm_word_sprite_draw(frame,73,&alias,1,3,&draw)!=MNM_WORD_INVALID)return 4;
    puts("Complete pixels, opaque zero, signed origin, stride, edge refusal, late admission and alias checks passed");
    return 0;
}
