#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include "../reconstruction/rendering/vertical_bar.h"
#include "vertical-bar-original.inc"
typedef void(__attribute__((thiscall))*Generator)(void*);
int main(void){
 void* global=mmap((void*)0x6e1000,4096,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0);
 if(global!=(void*)0x6e1000)return 1;
 void* code=mmap(0,sizeof(original_bar),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
 if(code==MAP_FAILED)return 2;
 memcpy(code,original_bar,sizeof(original_bar));
 if(mprotect(code,sizeof(original_bar),PROT_READ|PROT_EXEC))return 3;
 const unsigned widths[]={2,3,4,5,8,32,128},heights[]={2,3,4,40,75,128};
 uint16_t storage[128*128+32];unsigned cases=0,words=0;
 for(unsigned format=0;format<2;++format)for(unsigned wi=0;wi<7;++wi)for(unsigned hi=0;hi<6;++hi){
  unsigned w=widths[wi],h=heights[hi];
  for(unsigned fill=0;fill<=h-2;++fill){
   for(unsigned i=0;i<128*128+32;++i)storage[i]=0x1234;
   uint32_t object[12]={w,h,0,0,0,100,fill,(uint32_t)(storage+16),w*h*2,w,h,1};
   *(uint32_t*)0x6e1f88=format;
   ((Generator)code)(object);
   for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;++x){
    unsigned expected=mnm_vertical_bar_pixel(w,h,fill,format,x,y);
    if(storage[16+y*w+x]!=expected){fprintf(stderr,"Mismatch %u,%u,%u,%u at %u,%u: %04x != %04x\n",w,h,fill,format,x,y,storage[16+y*w+x],expected);return 4;}
   }
   for(unsigned i=0;i<16;++i)if(storage[i]!=0x1234)return 5;
   for(unsigned i=16+w*h;i<128*128+32;++i)if(storage[i]!=0x1234)return 6;
   ++cases;words+=w*h;
  }
 }
 if(mnm_vertical_bar_valid(1,75,0,0)||mnm_vertical_bar_valid(4,1,0,0)||mnm_vertical_bar_valid(129,75,0,0)||mnm_vertical_bar_valid(4,129,0,0)||mnm_vertical_bar_valid(4,75,74,0)||mnm_vertical_bar_valid(4,75,0,2))return 7;
 printf("{\"success\":true,\"cases\":%u,\"words_compared\":%u,\"surrounding_storage_preserved\":true,\"bounded_refusals\":6}\n",cases,words);
 return 0;
}
