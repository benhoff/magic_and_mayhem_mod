#include "jpeg_pixels.h"
#include <stddef.h>
static uint32_t word(const uint8_t* b){return b[0]|(uint32_t)b[1]<<8|(uint32_t)b[2]<<16|(uint32_t)b[3]<<24;}
static uint8_t normal(uint8_t c){return c=='\\'?'/':c>='A'&&c<='Z'?(uint8_t)(c+32):c;}
static int path(const uint8_t* b,uint32_t n){
 if(!n || n>239 || b[0]=='/' || b[0]=='\\')return 0;
 uint32_t start=0;
 for(uint32_t i=0;i<=n;++i){if(i<n && (b[i]<32 || b[i]>126 || b[i]==':'))return 0;
  if(i==n || normal(b[i])=='/'){const uint32_t size=i-start;if(!size || (size==1 && b[start]=='.') || (size==2 && b[start]=='.' && b[start+1]=='.'))return 0;start=i+1;}}
 return 1;
}
int mnm_jpeg_catalog_admit(const uint8_t* b,uint32_t bytes){
 static const uint8_t magic[8]={'M','N','M','J','P','I','X','1'};
 if(!b || bytes<32 || bytes>1024u*1024u*1024u || (uintptr_t)b>UINTPTR_MAX-bytes)return 0;
 for(uint32_t i=0;i<8;++i)if(b[i]!=magic[i])return 0;
 const uint32_t count=word(b+16);
 if(word(b+8)!=1 || word(b+12)!=32 || !count || count>4096 || word(b+20)!=bytes || word(b+24) || word(b+28))return 0;
 uint32_t at=32;
 for(uint32_t i=0;i<count;++i){
  if(bytes-at<20)return 0;
  const uint32_t size=word(b+at),n=word(b+at+4),w=word(b+at+8),h=word(b+at+12),rgb=word(b+at+16);
  if(size<20 || size>bytes-at || !w || w>2048 || !h || h>2048 || rgb!=w*h*3 || n>size-20 || rgb!=size-20-n || !path(b+at+20,n))return 0;
  /* Canonical unique paths: an ambiguous cache is refused before painting. */
  uint32_t previous=32;
  for(uint32_t j=0;j<i;++j){const uint32_t m=word(b+previous+4);int equal=n==m;
   for(uint32_t k=0;equal && k<n;++k)if(normal(b[previous+20+k])!=normal(b[at+20+k]))equal=0;
   if(equal)return 0;previous+=word(b+previous);}
  at+=size;
 }
 return at==bytes;
}
int mnm_jpeg_record_words(const uint8_t* record,uint32_t bytes,const struct MnmPixelPlane* p){
 if(!mnm_pixel_plane(p) || !record || bytes<20 || (uintptr_t)record>UINTPTR_MAX-bytes)return 0;
 const uint32_t n=word(record+4),w=word(record+8),h=word(record+12),rgb_bytes=word(record+16);
 if(word(record)!=bytes || !w || w>2048 || !h || h>2048 || rgb_bytes!=w*h*3 || n>bytes-20 ||
  rgb_bytes!=bytes-20-n || !path(record+20,n))return 0;
 const uintptr_t dst=(uintptr_t)p->pixels,begin=(uintptr_t)record;
 if(dst<begin+bytes && begin<dst+p->stride*p->height*2)return 0;
 const uint8_t* rgb=record+20+n;
 const uint32_t width=w<p->width?w:p->width,height=h<p->height?h:p->height;
 for(uint32_t y=0;y<height;++y)for(uint32_t x=0;x<width;++x){const uint8_t* c=rgb+(y*w+x)*3;
  p->pixels[y*p->stride+x]=(uint16_t)((uint32_t)(c[0]>>3)<<(p->format?10:11)|(uint32_t)(c[1]>>(p->format?3:2))<<5|c[2]>>3);}
 return 1;
}
int mnm_jpeg_catalog_words(const uint8_t* b,uint32_t bytes,const char* name,const struct MnmPixelPlane* p){
 if(!mnm_pixel_plane(p) || !name || !mnm_jpeg_catalog_admit(b,bytes))return 0;
 uint32_t n=0;while(n<240 && name[n])++n;if(n==240 || !path((const uint8_t*)name,n))return 0;
 const uintptr_t dst=(uintptr_t)p->pixels,begin=(uintptr_t)b;
 if(dst<begin+bytes && begin<dst+p->stride*p->height*2)return 0;
 uint32_t at=32;
 for(uint32_t i=0;i<word(b+16);++i){const uint32_t m=word(b+at+4);int equal=n==m;
  for(uint32_t k=0;equal && k<n;++k)if(normal((uint8_t)name[k])!=normal(b[at+20+k]))equal=0;
  if(equal)return mnm_jpeg_record_words(b+at,word(b+at),p);
  at+=word(b+at);
 }
 return 0;
}
