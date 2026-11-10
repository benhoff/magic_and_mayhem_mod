// Exercise the production assembly, including DF, x87, SSE, return value,
// registers and callee stack cleanup. Metadata callbacks intentionally clobber FX.
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
using U=uint32_t;
extern "C" {
U boundary_trampolines[23];
alignas(16) unsigned char boundary_expected_fx[512],boundary_actual_fx[512];
void boundary_probe(U*,void(*)(),U);
void boundary_original_0(); void boundary_original_4(); void boundary_trace_leave();
#define E(n) void boundary_enter_##n();
E(0) E(1) E(2) E(3) E(4) E(5) E(6) E(7) E(8) E(9) E(10) E(11)
E(12) E(13) E(14) E(15) E(16) E(17) E(18) E(19) E(20) E(21) E(22)
#undef E
U continuation,expected_sp,entered,returned;
void boundary_trace_enter(U*r,U tag){
 entered=tag+1;
 if(tag!=18&&tag!=19){continuation=r[9];expected_sp=U(r+9)+4+((tag==3||tag==11)?4:0);r[9]=U(boundary_trace_leave);}
 asm volatile("fninit; pxor %%xmm0,%%xmm0":::"xmm0");
}
void boundary_trace_return(U*r){
 if(U(r)+40!=expected_sp)std::abort();r[9]=continuation;++returned;
 asm volatile("fninit; pxor %%xmm0,%%xmm0":::"xmm0");
}
}
int main(){
#define E(n) boundary_enter_##n,
 void(*entries[])()={E(0) E(1) E(2) E(3) E(4) E(5) E(6) E(7) E(8) E(9) E(10) E(11)
 E(12) E(13) E(14) E(15) E(16) E(17) E(18) E(19) E(20) E(21) E(22)};
#undef E
 for(U tag=0;tag<23;++tag){
  U pop=(tag==3||tag==11)?4:0,a[9],b[9];auto original=pop?boundary_original_4:boundary_original_0;
  boundary_trampolines[tag]=U(original);boundary_probe(a,original,pop);
  entered=returned=0;boundary_probe(b,entries[tag],pop);
  for(U j=0;j<9;++j)if((j==8?((a[j]^b[j])&0xcd5):a[j]!=b[j])){std::fprintf(stderr,"tag%u register%u %x %x\n",tag,j,a[j],b[j]);return 1;}
  if(entered!=tag+1||returned!=U(tag!=18&&tag!=19))return 2;
  // FXSAVE reserved bytes are undefined. Compare defined x87 fields, register
  // payloads, MXCSR and all eight architectural i386 XMM registers.
  for(U i=0;i<288;++i)if(i!=5&&i!=14&&i!=15&&i!=22&&i!=23&&!(i>=32&&i<160&&((i-32)%16)>=10))
   if(boundary_expected_fx[i]!=boundary_actual_fx[i]){std::fprintf(stderr,"tag%u FX byte%u\n",tag,i);return 3;}
 }
 std::puts("23 production entry stubs: GPR/flags/DF/FX, 21 return stubs and RET0/RET4 passed");
}
