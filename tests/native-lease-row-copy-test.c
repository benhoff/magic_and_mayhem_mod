#define _GNU_SOURCE
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef uint16_t u16;typedef uint32_t u32;
#include "../runtime/native-draw/row_words.h"
static void check(int ok){if(!ok)abort();}
static u32 guarded_rows(void){
 const long page=sysconf(_SC_PAGESIZE);check(page>=4096);u32 cases=0;
 for(u32 width=0;width<=129;++width){
  unsigned char* source=mmap(0,(size_t)page*3,PROT_NONE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
  unsigned char* target=mmap(0,(size_t)page*3,PROT_NONE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
  check(source!=MAP_FAILED&&target!=MAP_FAILED);
  check(!mprotect(source+page,(size_t)page,PROT_READ|PROT_WRITE));check(!mprotect(target+page,(size_t)page,PROT_READ|PROT_WRITE));
  memset(source+page,0x37,(size_t)page);memset(target+page,0xa5,(size_t)page);
  ns_copy_word_rows((u16*)(target+page*2-width*2),width,(u16*)(source+page*2-width*2),width,width,1);
  for(long i=0;i<page;++i){check(source[page+i]==0x37);check(target[page+i]==(i<page-(long)width*2?0xa5:0x37));}
  check(!munmap(source,(size_t)page*3));check(!munmap(target,(size_t)page*3));++cases;
 }
 return cases;
}
int main(void){
 u16 in[2048],out[2048],original[2048];unsigned count=0;
 for(u32 width=0;width<=37;++width)for(u32 height=0;height<=17;++height)for(u32 pad=0;pad<6;++pad){
  u32 input_stride=width+pad,output_stride=width+pad+3;
  for(unsigned i=0;i<2048;++i){in[i]=(u16)(i*317+0x8000);out[i]=0xa55a;}memcpy(original,in,sizeof(in));
  ns_copy_word_rows(out+17,output_stride,in+9,input_stride,width,height);
  for(unsigned i=0;i<2048;++i){u16 expected=0xa55a;
   if(i>=17){u32 at=i-17,y=at/output_stride,x=at%output_stride;if(y<height&&x<width)expected=in[9+y*input_stride+x];}
   check(out[i]==expected);
  }
  check(!memcmp(in,original,sizeof(in)));++count;
 }
 u16 right[8]={1,2,3,4,5,6,7,8},right_expected[8]={1,1,1,1,1,1,7,8};
 ns_copy_word_rows(right+1,5,right,5,5,1);check(!memcmp(right,right_expected,sizeof(right)));
 u16 left[8]={1,2,3,4,5,6,7,8},left_expected[8]={2,3,4,5,6,6,7,8};
 ns_copy_word_rows(left,5,left+1,5,5,1);check(!memcmp(left,left_expected,sizeof(left)));
 u16 rows[9]={1,2,3,4,5,6,7,8,9},rows_expected[9]={1,2,1,2,5,2,5,8,9};
 ns_copy_word_rows(rows+2,3,rows,3,2,2);check(!memcmp(rows,rows_expected,sizeof(rows)));
 u32 guarded=guarded_rows();
 printf("{\"success\":true,\"padded_geometry_cases\":%u,\"forward_overlap_cases\":3,\"guard_page_cases\":%u}\n",count,guarded);return 0;
}
