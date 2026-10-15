#include "../reconstruction/rendering/raster_pixels.h"
#include "../reconstruction/rendering/colour_rect.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void put(uint8_t *p,uint32_t n){for(unsigned i=0;i<4;++i)p[i]=(uint8_t)(n>>(i*8));}
int main(void){
 uint8_t frame[76]={0},palette[1024];uint16_t pixels[80],before[80];
 put(frame,76);put(frame+4,8);put(frame+8,2);put(frame+28,0);put(frame+40,56);put(frame+44,60);put(frame+48,58);put(frame+52,68);
 frame[56]=frame[58]=0;frame[57]=frame[59]=8;
 for(unsigned i=60;i<76;++i)frame[i]=(uint8_t)i;
 for(unsigned i=0;i<1024;++i)palette[i]=(uint8_t)(i*43+71);
 for(unsigned i=0;i<80;++i)pixels[i]=before[i]=(uint16_t)(i*7919+7);
 struct MnmPixelPlane p={pixels,16,4,20,0};struct MnmRasterPixels r={0};
 r.frame=frame;r.bytes=sizeof frame;r.indexed=1;r.palette_step=4;r.palette=palette;r.clip_right=16;r.clip_bottom=4;r.period=16;r.contiguous=1;
 unsigned refusals=0;
 frame[59]=9;assert(!mnm_raster_pixels(&p,&r)&&!memcmp(pixels,before,sizeof pixels));++refusals;frame[59]=8;
 put(frame+48,57);assert(!mnm_raster_pixels(&p,&r)&&!memcmp(pixels,before,sizeof pixels));++refusals;put(frame+48,58);
 r.palette=(const uint8_t*)pixels;assert(!mnm_raster_pixels(&p,&r)&&!memcmp(pixels,before,sizeof pixels));++refusals;r.palette=palette;
 r.mode=3;r.left=8;r.top=2;r.offsets[3]=16;
 assert(!mnm_raster_pixels(&p,&r)&&!memcmp(pixels,before,sizeof pixels));++refusals;r.offsets[3]=0;
 r.offsets[2]=-1;assert(!mnm_raster_pixels(&p,&r)&&!memcmp(pixels,before,sizeof pixels));++refusals;r.offsets[2]=0;
 r.mode=0;r.left=0;r.top=0;r.paired_palette=1;assert(mnm_raster_pixels(&p,&r));
 memcpy(pixels,before,sizeof pixels);r.bytes=75;assert(!mnm_raster_pixels(&p,&r)&&!memcmp(pixels,before,sizeof pixels));++refusals;
 uint8_t triples[12]={1,0,2,0,3,0,31,0,64,0,1,0};struct MnmColourRect c={0};c.width=2;c.height=1;c.visible_width=2;c.visible_height=1;c.triples=triples;uint32_t result=0x12345678;
 assert(!mnm_colour_rect(&p,&c,&result)&&!memcmp(pixels,before,sizeof pixels)&&result==0x12345678);++refusals;
 triples[8]=63;p.format=1;assert(!mnm_colour_rect(&p,&c,&result)&&!memcmp(pixels,before,sizeof pixels)&&result==0x12345678);++refusals;
 printf("%u atomic malformed/alias/last-row displacement/channel refusals pass\n",refusals);return 0;
}
