#include "raster_pixels.h"
static uint32_t u32(const uint8_t *p){return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static uint32_t u16(const uint8_t *p){return p[0]|(uint32_t)p[1]<<8;}
static int alias(uintptr_t a,uint32_t n,uintptr_t b,uint32_t m){return a>UINTPTR_MAX-n||b>UINTPTR_MAX-m||(a<b+m&&b<a+n);}
static int visit(const struct MnmPixelPlane *p,const struct MnmRasterPixels *r,int write,
                 MnmRasterOutsideSample outside,void *context,MnmPixelStore16 store){
 const uint8_t *f=r->frame;const uint32_t w=u32(f+4),h=u32(f+8),table=40+h*8,step=r->indexed?1:2;
 uint32_t end=r->bytes,previous_control=0,previous_pixel=0;
 for(uint32_t i=32;i<=36;i+=4){const uint32_t a=u32(f+i);if(a&&(a<table||a>r->bytes))return 0;if(a&&a<end)end=a;}
 const uint32_t colour_start=u32(f+44);
 if(colour_start<table||colour_start>end)return 0;
 const int64_t shadow_top=(int64_t)r->top+h/2;
 int64_t first=(int64_t)r->clip_top-shadow_top,last=(int64_t)r->clip_bottom-shadow_top;
 if(first<0)first=0;if(last>h)last=h;
 const uint32_t mask=p->format?0x7def:0x7bef;
 for(uint32_t y=0;y<h;++y){
  uint32_t control=u32(f+40+y*8),pixel=u32(f+44+y*8),x=0,on=0;
  if(control<table||control>=colour_start||pixel<colour_start||pixel>end||
     (r->contiguous&&y&&(control!=previous_control||pixel!=previous_pixel)))return 0;
  while(x<w){
   if(control>=colour_start)return 0;const uint32_t n=f[control++];
   if(n>w-x||(on&&!n))return 0;
   if(on){
    if(pixel>end||n>(end-pixel)/step)return 0;
    for(uint32_t i=0;i<n;++i){
     if(r->legacy_wave_clip){const int64_t a=(int64_t)r->left+x,b=a+n;
      if((a>=r->clip_left&&b>=r->clip_right)||(a<r->clip_left&&b>r->clip_right))continue;}
     int64_t dx=(int64_t)r->left+x+i,dy=(int64_t)r->top+y;
     if(r->mode==5){
      if((int64_t)r->left<r->clip_left||(int64_t)r->left+w+h/2>r->clip_right||
         shadow_top>=r->clip_bottom||shadow_top+h<=r->clip_top||y<(uint32_t)first||y>=(uint32_t)last||((y-(uint32_t)first)&1))continue;
      const uint32_t row=(y-(uint32_t)first)/2;dx+=last/2-row;dy=shadow_top+first+row;
     }else if(dx<r->clip_left||dx>=r->clip_right||dy<r->clip_top||dy>=r->clip_bottom)continue;
     if(dx<0||dy<0||dx>=p->width||dy>=p->height)return 0;
     const uint32_t at=(uint32_t)dy*p->stride+(uint32_t)dx;int32_t offset=0;
     if(r->mode==3){offset=r->offsets[(uint32_t)dy%r->period];
      if(offset<0||offset>16)return 0;
      if((uint32_t)offset>=p->stride*p->height-at){
       if(!outside||!outside(context,at,(uint32_t)offset,write))return 0;
       continue;
      }}
     if(!write)continue;
     uint32_t colour=(r->mode==3||r->mode==5||r->mode==7)?0:r->indexed?u16(r->palette+f[pixel+i]*r->palette_step):u16(f+pixel+i*2);
     if(r->paired_palette){
      const int64_t start=(int64_t)r->left+x<r->clip_left?r->clip_left:(int64_t)r->left+x;
      const int64_t finish=(int64_t)r->left+x+n>r->clip_right?r->clip_right:(int64_t)r->left+x+n;
      const uint32_t leading=r->paired_palette==2?(uint32_t)(finish-start)&1:
          (((uintptr_t)p->pixels+((uint32_t)dy*p->stride+(uint32_t)start)*2)&2)!=0;
      const uint32_t relative=(uint32_t)(dx-start);
      if(relative>=leading&&((relative-leading)&1))colour|=u16(r->palette+f[pixel+i-1]*4+2);
     }
     const uint32_t dest=(r->mode==1||r->mode==2||r->mode==4||r->mode==5)?p->pixels[at]:0;
     if(r->mode==1)colour=((colour>>1)&mask)+((dest>>1)&mask);
     else if(r->mode==2){const uint32_t half=(dest>>1)&mask;colour=half+((half>>1)&mask)+((colour>>2)&(mask>>1)&mask);}
     else if(r->mode==4){const uint32_t half=(colour>>1)&mask;colour=half+((half>>1)&mask)+((dest>>2)&(mask>>1)&mask);}
     else if(r->mode==3)colour=p->pixels[at+(uint32_t)offset];
     else if(r->mode==5)colour=(dest>>1)&mask;
     else if(r->mode==6){const uint32_t red=(colour>>(p->format?10:11))&31,green=(colour>>(p->format?5:6))&31;
      const uint32_t grey=(red+green+(colour&31))/3;colour=grey+(grey<<(p->format?10:11))+(grey<<(p->format?5:6));}
     mnm_pixel_store16(store,p->pixels+at,(uint16_t)colour);
    }
    pixel+=n*step;
   }
   x+=n;on=!on;
  }
  previous_control=control;previous_pixel=pixel;
 }
 return 1;
}
int mnm_raster_pixels_with_storage(const struct MnmPixelPlane *p,const struct MnmRasterPixels *r,
                                         MnmRasterOutsideSample outside,void *context,MnmPixelStore16 store){
 if(!mnm_pixel_plane(p)||!r||!r->frame||r->bytes<48||r->bytes>4*1024*1024||u32(r->frame)!=r->bytes||
    r->indexed>1||r->mode>7||!r->period||r->period>16||r->contiguous>1||r->legacy_wave_clip>1||r->paired_palette>2||
    (r->paired_palette&&(!r->indexed||r->mode||r->palette_step!=4))||
    (r->indexed&&r->mode!=3&&r->mode!=5&&r->mode!=7&&(!r->palette||(r->palette_step!=2&&r->palette_step!=4)))||
    r->clip_left<0||r->clip_top<0||r->clip_right<r->clip_left||r->clip_bottom<r->clip_top||
    alias((uintptr_t)r->frame,r->bytes,(uintptr_t)p->pixels,p->stride*p->height*2))return 0;
 /* Original callers can retain a logical clip beyond the physical canvas.
  * The atomic first visit checks every actual write against the allocation;
  * an oversized clip alone is not an out-of-bounds write. */
 const uint32_t w=u32(r->frame+4),h=u32(r->frame+8);
 if(!w||w>2048||!h||h>2048||40+h*8>r->bytes)return 0;
 if(r->indexed&&r->palette&&alias((uintptr_t)r->palette,256*r->palette_step,(uintptr_t)p->pixels,p->stride*p->height*2))return 0;
 if(r->strict_horizontal&&((int64_t)r->left<r->clip_left||(int64_t)r->left+w>=r->clip_right))return 0;
 if(!visit(p,r,0,outside,context,store))return 0;
 return visit(p,r,1,outside,context,store);
}
int mnm_raster_pixels(const struct MnmPixelPlane *p,const struct MnmRasterPixels *r){
 return mnm_raster_pixels_with_sample_boundary(p,r,0,0);
}

int mnm_raster_pixels_with_sample_boundary(const struct MnmPixelPlane* p,const struct MnmRasterPixels* r,MnmRasterOutsideSample outside,void* context){return mnm_raster_pixels_with_storage(p,r,outside,context,0);}
