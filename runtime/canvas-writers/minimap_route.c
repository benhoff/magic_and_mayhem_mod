#include "minimap_route.h"
MnmMapResolve mnm_map_resolve;
MnmMapRelease mnm_map_release;
uintptr_t mnm_map_original[MNM_MAP_SITES],mnm_map_resume[MNM_MAP_SITES];
uint32_t mnm_map_native[MNM_MAP_SITES],mnm_map_forwarded[MNM_MAP_SITES];
static uint32_t add_flags(uint32_t a,uint32_t n,uint32_t flags){
 uint32_t r=a+n,p=r&255,out=0;p^=p>>4;p^=p>>2;p^=p>>1;
 if(!(p&1))out|=4;if(r<a)out|=1;if((a^n^r)&16)out|=16;
 if(!r)out|=64;if(r&0x80000000u)out|=128;if((~(a^n)&(a^r))&0x80000000u)out|=0x800;
 return (flags&~0x8d5u)|out;
}
void (*mnm_map_notice)(unsigned,int);
int mnm_map_route(uint32_t *r,unsigned kind){
 if(kind>=MNM_MAP_SITES)return 0;
 struct MnmPixelPlane p={0},history={0};struct MnmMinimapTerrain t={0};struct MnmMinimapView v={0};struct MnmMinimapCamera c={0};uint32_t a[8]={0},returned=0;int ok=0;
 if(!mnm_map_resolve || !mnm_map_resolve(r,kind,&p,&history,&t,&v,&c,a))goto done;
 if(kind==MNM_MAP_TERRAIN)ok=mnm_minimap_terrain(&p,&history,&t);
 else if(kind==MNM_MAP_CELL){
  ok=mnm_minimap_cell_at(&p,(int32_t)a[0],(int32_t)a[1],(uint16_t)a[2],a[3],&returned);
  if(ok){r[7]=(uint32_t)(uintptr_t)p.pixels+returned*2;r[6]=p.stride;r[5]=(r[5]&0xffff0000u)|a[2];
   const uint32_t before=(uint32_t)(uintptr_t)p.pixels+((a[1]+(a[3]?1:0))*p.stride+a[0])*2;
   r[8]=add_flags(before,2,r[8]);}
 }else if(kind==MNM_MAP_CREATURE)ok=mnm_minimap_creature(&p,(int32_t)a[0],(int32_t)a[1],(uint16_t)r[1]);
 else {
  ok=mnm_minimap_camera(&p,&v,&c);
  if(ok){const int swapped=v.orientation==1 || v.orientation==2;
   const int32_t first=(swapped?v.realm_height:v.realm_width)/2,second=(swapped?v.realm_width:v.realm_height)/2,dy=c.directions[7];
   r[6]=(uint32_t)((first+second)/2-1-dy*(c.viewport_height/64)+(dy<0));
   r[5]=r[1]+0x6141;}
 }
done:
 if(mnm_map_release)mnm_map_release(&t);
 if(ok)++mnm_map_native[kind];else ++mnm_map_forwarded[kind];if(mnm_map_notice)mnm_map_notice(kind,ok);return ok;
}
