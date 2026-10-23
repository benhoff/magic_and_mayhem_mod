#define _GNU_SOURCE
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef uint8_t u8;typedef uint32_t u32;
#include "../runtime/native-draw/dirty_bytes.h"
static void check(int ok){if(!ok)abort();}
static void oracle(u8* dirty,u32 width,u32 height,u32 pitch,u32 offset,u32 bytes,u32* count,u32* partial){
 for(u32 i=0;i<bytes;++i){u32 at=offset+i,row=at/pitch,col=at%pitch;
  if(row>=height||col>=width*2)continue;
  u32 bit=row*width*2+col,index=bit>>3,flag=1u<<(bit&7);
  if(dirty[index]&flag)continue;
  if(dirty[index]&(1u<<((bit^1u)&7)))--*partial;else ++*partial;
  dirty[index]|=(u8)flag;++*count;
 }
}
static void one(u32 width,u32 height,u32 pitch,u32 offset,u32 bytes,u32 pattern){
 u8 a[64],b[64];memset(a,pattern,sizeof(a));memcpy(b,a,sizeof(a));u32 count=0,partial=0;
 for(u32 i=0;i<width*height*2;i+=2){u32 f=(a[i>>3]>>(i&7))&3;count+=(f&1)+((f>>1)&1);partial+=f==1||f==2;}
 u32 expected_count=count,expected_partial=partial;
 oracle(a,width,height,pitch,offset,bytes,&expected_count,&expected_partial);
 ns_dirty_bytes(b,width,height,pitch,offset,bytes,&count,&partial);
 check(!memcmp(a,b,sizeof(a))&&count==expected_count&&partial==expected_partial);
}
static u32 expand_cases(void){u32 cases=0;
 for(u32 pattern=0;pattern<256;++pattern)for(u32 bytes=0;bytes<=64;++bytes)for(u32 offset=0;offset<8;++offset){
  u8 bits[16],out[128];memset(bits,pattern,sizeof(bits));memset(out,0xa5,sizeof(out));
  ns_expand_dirty_mask(out+offset,bits+(7-offset),bytes);
  for(u32 i=0;i<128;++i)check(out[i]==(i>=offset&&i-offset<bytes?(u8)((pattern>>((i-offset)&7))&1):0xa5));
  for(u32 i=0;i<16;++i)check(bits[i]==pattern);++cases;
 }return cases;}
static u32 expand_overlap(void){u32 cases=0;
 for(u32 src=24;src<=64;src+=4)for(u32 dst=16;dst<=96;dst+=4)for(u32 bytes=1;bytes<=32;++bytes){
  u8 expected[160],actual[160];for(u32 i=0;i<160;++i)expected[i]=(u8)(i*53+17);memcpy(actual,expected,sizeof(actual));
  for(u32 i=0;i<bytes;++i)expected[dst+i]=(expected[src+(i>>3)]>>(i&7))&1;
  ns_expand_dirty_mask(actual+dst,actual+src,bytes);check(!memcmp(expected,actual,sizeof(actual)));++cases;
 }return cases;}
static u32 expand_guarded(void){long page=sysconf(_SC_PAGESIZE);check(page>=4096);u32 cases=0;
 for(u32 bytes=0;bytes<=129;++bytes){
  u8* source=mmap(0,(size_t)page*3,PROT_NONE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
  u8* out=mmap(0,(size_t)page*3,PROT_NONE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
  check(source!=MAP_FAILED&&out!=MAP_FAILED);check(!mprotect(source+page,(size_t)page,PROT_READ|PROT_WRITE));check(!mprotect(out+page,(size_t)page,PROT_READ|PROT_WRITE));
  memset(source+page,0xa5,(size_t)page);memset(out+page,0x37,(size_t)page);check(!mprotect(source+page,(size_t)page,PROT_READ));
  ns_expand_dirty_mask(out+page*2-bytes,source+page*2-(bytes/8+(bytes%8!=0)),bytes);
  for(long i=0;i<page;++i)check(out[page+i]==(i<page-(long)bytes?0x37:(u8)((0xa5>>((i-(page-bytes))&7))&1)));
  check(!munmap(source,(size_t)page*3));check(!munmap(out,(size_t)page*3));++cases;
 }return cases;}
int main(void){u32 cases=0;
 for(u32 width=1;width<=23;++width)for(u32 height=1;height<=3;++height)for(u32 pad=0;pad<=4;++pad){
  u32 pitch=width*2+pad;
  for(u32 offset=0;offset<=pitch*height+3;++offset)for(u32 bytes=0;bytes<=6;++bytes)for(u32 pattern=0;pattern<3;++pattern){
   one(width,height,pitch,offset,bytes,pattern==0?0:pattern==1?0x55:0xff);++cases;
  }
 }
 one(3,2,9,UINT32_MAX-2,6,0);one(3,2,9,UINT32_MAX-2,6,0x55);cases+=2;
 u32 expanded=expand_cases(),overlap=expand_overlap(),guarded=expand_guarded();
 printf("{\"success\":true,\"dirty_byte_cases\":%u,\"mask_expansion_cases\":%u,\"mask_overlap_cases\":%u,\"mask_guard_page_cases\":%u}\n",cases,expanded,overlap,guarded);return 0;
}
