#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
typedef uint32_t u32;typedef int32_t i32;typedef uint8_t u8;
#include "../runtime/native-draw/cpu_store.h"
static unsigned count;
static void check(int ok){++count;if(!ok){fprintf(stderr,"store decode check %u\n",count);exit(1);}}
static void put(u8* p,u32 v){for(unsigned i=0;i<4;++i)p[i]=(u8)(v>>(8*i));}
int main(void){
 u32 c[51]={0},regs[8]={0x12345678,0x21345678,0x32345678,0x41345678,0x52345678,0x61345678,0x72345678,0x81345678};
 const unsigned offsets[8]={44,43,42,41,49,45,40,39};
 for(unsigned i=0;i<8;++i)c[offsets[i]]=regs[i];
 for(unsigned width=1;width<=4;width*=2)for(unsigned mod=0;mod<3;++mod)for(unsigned rm=0;rm<8;++rm)for(unsigned reg=0;reg<8;++reg){
  for(unsigned sib=0;sib<(rm==4?256u:1u);++sib){
   u8 code[16]={0};unsigned n=0;if(width==2)code[n++]=0x66;
   code[n++]=width==1?0x88:0x89;code[n++]=(u8)(mod*64+reg*8+rm);
   u32 address=0;
   if(rm==4){code[n++]=(u8)sib;unsigned base=sib&7,index=(sib>>3)&7;
    if(index!=4)address=regs[index]<<(sib>>6);
    if(!mod && base==5){put(code+n,0x12345678);n+=4;address+=0x12345678;}
    else address+=regs[base];
   }else if(!mod && rm==5){put(code+n,0x12345678);n+=4;address=0x12345678;}
   else address=regs[rm];
   if(mod==1){code[n++]=0xfb;address-=5;}
   else if(mod==2){put(code+n,0xfffffe00);n+=4;address-=512;}
   struct NsCpuStore s;check(ns_cpu_decode_store(code,c,&s));
   u32 value=width==1?regs[reg&3]>>(reg>=4?8:0):regs[reg];
   check(s.address==address && s.value==value && s.width==width && s.length==n && !s.stos);
  }
 }
 for(unsigned width=1;width<=4;width*=2){
  u8 code[16]={0};unsigned n=0;if(width==2)code[n++]=0x66;
  code[n++]=width==1?0xc6:0xc7;code[n++]=0x45;code[n++]=0xf9;
  for(unsigned i=0;i<width;++i)code[n++]=(u8)(0x89abcdefu>>(8*i));
  struct NsCpuStore s;check(ns_cpu_decode_store(code,c,&s));
  check(s.address==regs[5]-7 && s.width==width && s.length==n && s.value==(0x89abcdefu&(width==4?0xffffffffu:(1u<<(width*8))-1)));
  /* Check immediate low bytes independently of unused upper bits. */
  for(unsigned i=0;i<width;++i)check((u8)(s.value>>(i*8))==(u8)(0x89abcdefu>>(i*8)));
  code[width==2?2:1]|=8;check(!ns_cpu_decode_store(code,c,&s));
 }
 const u8 rejected[]={0xf0,0xf2,0xf3,0x67,0x64,0x65,0x26,0x01,0x03,0x8a,0x8b,0x90,0x0f};
 for(unsigned i=0;i<sizeof(rejected);++i){u8 code[16]={0};code[0]=rejected[i];struct NsCpuStore s;check(!ns_cpu_decode_store(code,c,&s));}
 for(unsigned op=0;op<4;++op){u8 code[8]={op<2?(op?0xa3:0xa2):(op==2?0xaa:0xab)};put(code+1,0xaabbccdd);struct NsCpuStore s;
  check(ns_cpu_decode_store(code,c,&s));check(s.address==(op<2?0xaabbccddu:regs[7]) && s.value==regs[0] && s.length==(op<2?5u:1u) && s.stos==(op>=2));}
 printf("{\"success\":true,\"checks\":%u}\n",count);return 0;
}
