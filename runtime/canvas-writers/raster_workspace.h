#pragma once
#include "../../reconstruction/rendering/raster_pixels.h"
/* Pixel-loop workspace writes after original x87/coordinate/palette setup.
 * This build-specific adapter never supplies pixels to the native kernel. */
static uint32_t mnm_raster_word(const uint8_t *p){return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static void mnm_raster_workspace(uint32_t s[16],const struct MnmRasterPixels *r,
 const struct MnmPixelPlane *p,uint32_t frame,unsigned family,int clipped){
 const uint8_t *f=r->frame;const uint32_t width=mnm_raster_word(f+4),height=mnm_raster_word(f+8);
 uint32_t first=r->top<r->clip_top?(uint32_t)(r->clip_top-r->top):0;
 int32_t last=(int64_t)r->top+height>=r->clip_bottom?r->clip_bottom-r->top:(int32_t)height;
 s[2]=(p->stride-width)*2;s[7]=frame+40+first*8;
 if(last<=(int32_t)first)return;
 s[8]=0;
 const int scalar=family==0||family==2||family==5;
 const int32_t begin=r->left<r->clip_left?r->clip_left-r->left:0;
 const int32_t end=(int64_t)r->left+width>=r->clip_right?r->clip_right-r->left:(int32_t)width;
 for(uint32_t y=first;y<(uint32_t)last;++y){
  uint32_t at=mnm_raster_word(f+40+y*8),x=0;s[9]=width;if(clipped)s[14]=0;
  while(x<width){
   const uint32_t skip=f[at++];x+=skip;
   if(!scalar){s[9]-=skip;if(clipped)s[14]+=skip;}
   if(x==width)break;
   const uint32_t n=f[at++];s[9]=width-x-n;
   if(clipped){s[15]=0;int32_t a=(int32_t)x;
    if(begin>a){if(begin-a>=(int32_t)n){x+=n;s[14]=x;continue;}a=begin;}
    if(a>=end){x+=n;s[14]=x;continue;}
    int32_t b=(int32_t)(x+n);if(b>end){s[15]=(uint32_t)(b-end);b=end;}
    if(scalar){const uint32_t dest=(uint32_t)(uintptr_t)p->pixels+((r->top+y)*p->stride+r->left+a)*2;s[11]=(uint32_t)(b-a)-((dest&2)!=0);}
    x+=n;s[14]=x;
   }else{if(scalar){const uint32_t dest=(uint32_t)(uintptr_t)p->pixels+((r->top+y)*p->stride+r->left+x)*2;s[11]=n-((dest&2)!=0);}x+=n;}
  }
 }
 if(family==4){s[7]+=((uint32_t)last-first)*8;s[9]=0;}
}
