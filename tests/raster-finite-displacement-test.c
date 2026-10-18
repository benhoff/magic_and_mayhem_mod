#define _GNU_SOURCE
#include "../reconstruction/rendering/raster_pixels.h"
#include "../renderer/sprites/displacement_boundary.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
static void put(uint8_t *p,uint32_t n){for(unsigned i=0;i<4;++i)p[i]=(uint8_t)(n>>(i*8));}
int main(void){
 const size_t page=(size_t)sysconf(_SC_PAGESIZE);
 uint8_t *storage=mmap(0,page*2,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
 assert(storage!=MAP_FAILED);assert(!mprotect(storage+page,page,PROT_NONE));
 uint16_t *pixels=(uint16_t*)(storage+page-64),before[32];
 uint8_t f[64]={0};put(f,64);put(f+4,4);put(f+8,2);put(f+28,0);
 put(f+40,56);put(f+44,60);put(f+48,58);put(f+52,64);
 /* Displacement retains opaque shape but never reads colour/palette payload. */
 f[56]=f[58]=0;f[57]=f[59]=4;
 struct MnmPixelPlane p={pixels,8,4,8,0};struct MnmRasterPixels r={0};
 r.frame=f;r.bytes=64;r.indexed=1;r.mode=3;r.left=4;r.top=2;
 r.clip_right=8;r.clip_bottom=4;r.period=16;r.contiguous=1;
 /* Four indexed bytes per row are required even though not sampled. */
 uint8_t frame[68];memcpy(frame,f,64);memset(frame+64,0,4);put(frame,68);r.frame=frame;r.bytes=68;
 unsigned cases=0;
 for(unsigned format=0;format<2;++format)for(unsigned shift=0;shift<=16;++shift){
  p.format=format;r.offsets[2]=r.offsets[3]=(int32_t)shift;
  for(unsigned i=0;i<32;++i)pixels[i]=before[i]=(uint16_t)(i*771+19);
  const int strict=mnm_raster_pixels(&p,&r);
  assert(strict==(shift==0));if(!strict)assert(!memcmp(pixels,before,sizeof before));
  memcpy(pixels,before,sizeof before);uint32_t retained=0;
  assert(mnm_raster_pixels_with_sample_boundary(&p,&r,mnm_retain_outside_displacement,&retained));
  unsigned expected_retained=0;
  for(unsigned i=0;i<32;++i){
   const int covered=(i/8>=2&&i%8>=4);const unsigned sample=i+shift;
   uint16_t expected=before[i];
   if(covered){if(sample<32)expected=before[sample];else ++expected_retained;}
   assert(pixels[i]==expected);
  }
  assert(retained==expected_retained);++cases;
 }
 /* A late malformed row still rejects atomically after an earlier outside tap;
  * invalid offsets and out-of-plane destinations are never policy skips. */
 for(unsigned which=0;which<4;++which){
  for(unsigned i=0;i<32;++i)pixels[i]=before[i]=(uint16_t)(i+3);
  r.offsets[2]=r.offsets[3]=16;r.top=2;r.clip_bottom=4;frame[59]=4;
  if(which==0)frame[59]=5;
  if(which==1)r.offsets[3]=-1;
  if(which==2)r.offsets[3]=17;
  if(which==3){r.top=3;r.clip_bottom=5;}
  uint32_t retained=0;
  assert(!mnm_raster_pixels_with_sample_boundary(&p,&r,mnm_retain_outside_displacement,&retained));
  assert(!memcmp(pixels,before,sizeof before)&&!retained);++cases;
 }
 assert(!munmap(storage,page*2));printf("%u guarded finite/strict/atomic cases pass\n",cases);return 0;
}
