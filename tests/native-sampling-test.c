#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
typedef uint32_t u32;typedef int32_t i32;
struct Palette {u32 entries[256],count;};
struct NsState {u32 bits,masks[3];struct Palette* palette;};
#include "../runtime/native-draw/pixel_conversion.h"
static void check(int ok){if(!ok)abort();}
int main(void){
 const u32 limits[]={0,1,2,3,7,255,65535,65536,0x7fffffffu,0xfffffffeu,0xffffffffu};
 unsigned count=0;
 for(unsigned i=0;i<11;++i)for(unsigned j=1;j<11;++j)for(unsigned k=1;k<11;++k){
  u32 at=limits[i],source=limits[j],target=limits[k];
  check(ns_sample(at,source,target)==(u32)((uint64_t)at*source/target));++count;
  struct NsSampleCursor cursor=ns_sample_begin(at,source,target);
  for(u32 n=0;n<32 && n<=UINT32_MAX-at;++n){check(cursor.value==(u32)((uint64_t)(at+n)*source/target));ns_sample_next(&cursor);++count;}
 }
 u32 seed=0x72f183;
 for(unsigned i=0;i<100000;++i){seed=seed*1664525u+1013904223u;u32 at=seed;
  seed=seed*1664525u+1013904223u;u32 source=seed;
  seed=seed*1664525u+1013904223u;u32 target=seed|1;
  check(ns_sample(at,source,target)==(u32)((uint64_t)at*source/target));
  check(ns_sample(at,target,target)==at);count+=2;
  struct NsSampleCursor cursor=ns_sample_begin(at,source,target);
  for(u32 n=0;n<32 && n<=UINT32_MAX-at;++n){check(cursor.value==(u32)((uint64_t)(at+n)*source/target));ns_sample_next(&cursor);++count;}
 }
 printf("{\"success\":true,\"independent_sampling_cases\":%u}\n",count);return 0;
}
