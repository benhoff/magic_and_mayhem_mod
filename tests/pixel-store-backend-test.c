#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../reconstruction/rendering/minimap_pixels.h"
#include "../reconstruction/rendering/raster_pixels.h"
#define check(x) do{if(!(x)){fprintf(stderr,"failed line%d\n",__LINE__);exit(1);}}while(0)
static uint16_t visible[256],shadow[256],expected[256],history[256];static uint32_t calls;
static void sink(uint16_t* target,uint16_t value){check(target>=visible&&target<visible+256);shadow[target-visible]=value;++calls;}
static void reset(void){for(unsigned i=0;i<256;++i)visible[i]=shadow[i]=expected[i]=0xaaaa;calls=0;}
static void compare(void){check(calls>0&&!memcmp(shadow,expected,sizeof(shadow)));for(unsigned i=0;i<256;++i)check(visible[i]==0xaaaa);}
static void word(uint8_t* p,uint32_t v){for(unsigned i=0;i<4;++i)p[i]=(uint8_t)(v>>(8*i));}
int main(void){
 struct MnmPixelPlane p={visible,16,16,16,0},ref={expected,16,16,16,0},old={history,16,16,16,0};
 uint8_t cells[12]={0x11,0x11,0,0x22,0x22,0,0x33,0x33,0,0x44,0x44,1};
 for(unsigned i=0;i<256;++i)history[i]=(uint16_t)(i+0x1000);
 struct MnmMinimapTerrain t={2,2,0,0,0,8,8,cells};
 for(unsigned orientation=0;orientation<4;++orientation){reset();t.orientation=orientation;check(mnm_minimap_terrain(&ref,&old,&t));check(mnm_minimap_terrain_with_store(&p,&old,&t,sink));check(calls==4);compare();}
 reset();t.root_x=15;check(!mnm_minimap_terrain_with_store(&p,&old,&t,sink));check(!calls);t.root_x=8;
 struct MnmMinimapView v={4,4,0,0,4,4,8,8,0};uint32_t a,b;
 for(unsigned ring=0;ring<2;++ring){reset();check(mnm_minimap_cell(&ref,&v,0,0,0x5678,ring,&a));check(mnm_minimap_cell_with_store(&p,&v,0,0,0x5678,ring,&b,sink));check(a==b&&calls==(ring?8u:4u));compare();}
 reset();check(mnm_minimap_cell_at(&ref,8,8,0x4321,1,&a));check(mnm_minimap_cell_at_with_store(&p,8,8,0x4321,1,&b,sink));check(a==b);compare();
 reset();check(mnm_minimap_creature(&ref,8,8,0x7654));check(mnm_minimap_creature_with_store(&p,8,8,0x7654,sink));check(calls==13);compare();
 struct MnmMinimapCamera camera={128,64,{1,1,1,-1,-1,-1,-1,1},1,0x1f};
 for(unsigned orientation=0;orientation<4;++orientation){reset();v.orientation=orientation;check(mnm_minimap_camera(&ref,&v,&camera));check(mnm_minimap_camera_with_store(&p,&v,&camera,sink));compare();}
 uint8_t frame[54]={0};word(frame,54);word(frame+4,2);word(frame+8,1);word(frame+40,48);word(frame+44,50);frame[48]=0;frame[49]=2;frame[50]=0x34;frame[51]=0x12;frame[52]=0xcd;frame[53]=0xab;
 struct MnmRasterPixels r={0};r.frame=frame;r.bytes=sizeof(frame);r.contiguous=1;r.period=1;r.left=r.top=1;r.clip_right=r.clip_bottom=16;
 reset();check(mnm_raster_pixels(&ref,&r));check(mnm_raster_pixels_with_storage(&p,&r,0,0,sink));check(calls==2);compare();
 reset();r.bytes=53;check(!mnm_raster_pixels_with_storage(&p,&r,0,0,sink));check(!calls);
 puts("{\"success\":true,\"default_and_storage_outputs_equal\":true,\"direct_destination_writes\":0}");return 0;
}
