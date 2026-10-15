#include "minimap_pixels.h"
#include <stddef.h>
static int slot(const struct MnmPixelPlane *p,int32_t x,int32_t y,uint32_t *at){
 if(x<0 || y<0 || (uint32_t)x>=p->width || (uint32_t)y>=p->height)return 0;
 *at=(uint32_t)y*p->stride+(uint32_t)x;return 1;
}
static int point(const struct MnmPixelPlane *p,int32_t x,int32_t y,uint16_t colour,int commit){
 uint32_t at;if(!slot(p,x,y,&at))return 0;if(commit)p->pixels[at]=colour;return 1;
}
static int view(const struct MnmMinimapView *v){
 return v && v->grid_width>0 && v->grid_width<=256 && v->grid_height>0 && v->grid_height<=256 &&
  v->center_x>=0 && v->center_x<v->grid_width && v->center_y>=0 && v->center_y<v->grid_height &&
  v->realm_width>=0 && v->realm_width<=256 && v->realm_height>=0 && v->realm_height<=256 &&
  v->origin_x>=-2048 && v->origin_x<=2048 && v->origin_y>=-2048 && v->origin_y<=2048 && v->orientation<=3;
}
static int wrap(int32_t n,int32_t size){int32_t r=n%size;return r<0?r+size:r;}
int mnm_minimap_terrain(const struct MnmPixelPlane *p,const struct MnmPixelPlane *previous,const struct MnmMinimapTerrain *t){
 if(!mnm_pixel_plane(p) || !t || !t->cells || !t->width || t->width>256 || (t->width&1) ||
  !t->height || t->height>256 || t->center_x>=t->width || t->center_y>=t->height || t->orientation>3 ||
  t->root_x<-2048 || t->root_x>2048 || t->root_y<-2048 || t->root_y>2048 ||
  (previous && !mnm_pixel_plane(previous)))return 0;
 const uintptr_t data=(uintptr_t)t->cells,dst=(uintptr_t)p->pixels;const uint32_t bytes=t->width*t->height*3,pbytes=p->stride*p->height*2;
 if(data>UINTPTR_MAX-bytes || (dst<data+bytes && data<dst+pbytes))return 0;
 if(previous){const uintptr_t old=(uintptr_t)previous->pixels;const uint32_t oldbytes=previous->stride*previous->height*2;
  if(dst<old+oldbytes && old<dst+pbytes)return 0;}
 const int32_t w=(int32_t)t->width,h=(int32_t)t->height;
 const int32_t sx=wrap((int32_t)t->center_x-w/2+(int32_t)(t->orientation&1),w),sy=wrap((int32_t)t->center_y-h/2,h);
 for(int commit=0;commit<2;++commit)for(int32_t row=0;row<h;++row)for(int32_t col=0;col<w;++col){
  int32_t x,y;switch(t->orientation){
   case 0:x=-2*(row/2)+col;y=(row+1)/2+col/2;break;
   case 1:x=-w+2+2*(row/2)+col;y=w/2-1+(row+1)/2-col/2;break;
   case 2:x=1+2*(row/2)-col;y=w-1-(row+1)/2-col/2;break;
   default:x=w-1-2*(row/2)-col;y=w/2-(row+1)/2+col/2;break;}
  const uint32_t cell=(uint32_t)((sy+row)%h*w+(sx+col)%w);const uint8_t* source=t->cells+cell*3;
  uint32_t at,old;if(!slot(p,t->root_x+x,t->root_y+y,&at) || source[2]>1)return 0;
  uint16_t colour=(uint16_t)(source[0]|(uint32_t)source[1]<<8);
  if(source[2]){if(!previous || !slot(previous,w-1+x,y,&old))return 0;colour=previous->pixels[old];}
  if(commit)p->pixels[at]=colour;
 }
 return 1;
}
int mnm_minimap_cell(const struct MnmPixelPlane *p,const struct MnmMinimapView *v,int32_t x,int32_t y,uint16_t colour,uint32_t ring,uint32_t *returned){
 if(!mnm_pixel_plane(p) || !view(v) || x<-4096 || x>4096 || y<-4096 || y>4096 || ring>1 || !returned)return 0;
 int32_t u=wrap(x+v->grid_width/2-v->center_x,v->grid_width),z=wrap(y+v->grid_height/2-v->center_y,v->grid_height),old=u;
 if(v->orientation==1){u=z;z=v->realm_width-old;}else if(v->orientation==2){u=v->realm_width-u;z=v->realm_height-z;}
 else if(v->orientation==3){u=v->realm_height-z;z=old;}
 const int32_t cx=v->origin_x+u-z,cy=v->origin_y+(u+z)/2;
 return mnm_minimap_cell_at(p,cx,cy,colour,ring,returned);
}
int mnm_minimap_cell_at(const struct MnmPixelPlane *p,int32_t cx,int32_t cy,uint16_t colour,uint32_t ring,uint32_t *returned){
 if(!mnm_pixel_plane(p) || cx<-2048 || cx>2048 || cy<-2048 || cy>2048 || ring>1 || !returned)return 0;
 const int32_t dx[8]={-1,2,1,0,0,1,2,-1},dy[8]={-1,-1,0,0,1,1,2,2};
 const int32_t sx[4]={0,1,0,1},sy[4]={0,0,1,1};uint32_t out;
 if(!slot(p,cx+(ring?2:0),cy+(ring?2:1),&out))return 0;
 for(int commit=0;commit<2;++commit)for(uint32_t i=0;i<(ring?8u:4u);++i)
  if(!point(p,cx+(ring?dx[i]:sx[i]),cy+(ring?dy[i]:sy[i]),colour,commit))return 0;
 *returned=out;return 1;
}
int mnm_minimap_creature(const struct MnmPixelPlane *p,int32_t x,int32_t y,uint16_t colour){
 if(!mnm_pixel_plane(p) || x<-2048 || x>2048 || y<-2048 || y>2048)return 0;
 const int32_t dx[13]={-1,0,1,2,-2,-2,0,2,2,-2,-1,0,1},dy[13]={-2,-2,-2,-1,-1,0,0,0,1,1,2,2,2};
 for(int commit=0;commit<2;++commit)for(uint32_t i=0;i<13;++i)if(!point(p,x+dx[i],y+dy[i],colour,commit))return 0;
 return 1;
}
int mnm_minimap_camera(const struct MnmPixelPlane *p,const struct MnmMinimapView *v,const struct MnmMinimapCamera *c){
 if(!mnm_pixel_plane(p) || !view(v) || !c || c->viewport_width<0 || c->viewport_width>8192 || c->viewport_height<0 || c->viewport_height>8192 || c->border>1)return 0;
 const int swapped=v->orientation==1 || v->orientation==2;
 const int32_t a=(swapped?v->realm_height:v->realm_width)/2,b=(swapped?v->realm_width:v->realm_height)/2;
 const int32_t sw=c->viewport_width/128+1,sh=c->viewport_height/64;
 for(uint32_t i=0;i<8;++i)if(c->directions[i]!=-1 && c->directions[i]!=1)return 0;
 for(int commit=0;commit<2;++commit){
  if(c->border){
   for(int32_t j=0;j<=a;++j)for(int32_t k=0;k<2;++k)if(!point(p,v->origin_x+2*j+k,v->origin_y-1+j,c->border_colour,commit))return 0;
   for(int32_t j=0;j<=b;++j)for(int32_t k=0;k<2;++k)if(!point(p,v->origin_x+2*(a-j)+k,v->origin_y+a+j,c->border_colour,commit))return 0;
   for(int32_t j=0;j<a;++j)for(int32_t k=0;k<2;++k)if(!point(p,v->origin_x+2*(a-b-1-j)+k,v->origin_y+a+b-1-j,c->border_colour,commit))return 0;
   for(int32_t j=0;j<b;++j)for(int32_t k=0;k<2;++k)if(!point(p,v->origin_x-2*b+2*j+k,v->origin_y+b-1-j,c->border_colour,commit))return 0;
  }
  for(uint32_t j=0;j<4;++j){const int32_t dx=c->directions[j*2],dy=c->directions[j*2+1];
   const int32_t x=(a+b)/2-dx*sw*2-b,y=(a+b)/2-1-dy*sh+(dy<0);
   for(int32_t i=0;i<4;++i)if(!point(p,v->origin_x+x+i*dx,v->origin_y+y,p->format?0x200:0x400,commit) ||
    !point(p,v->origin_x+x,v->origin_y+y+i*dy,p->format?0x200:0x400,commit))return 0;
  }
 }
 return 1;
}
