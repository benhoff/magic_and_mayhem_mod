/* Actual protected-page instructions; compare architectural state against an
 * authored instruction oracle. Reserved FXSAVE bytes and empty x87 payloads
 * are intentionally excluded, never treated as deterministic processor state. */
__attribute__((used,aligned(16))) static u8 t1_caller_fx[512],t1_before_fx[512],t1_after_fx[512];
__attribute__((used,aligned(16))) static u32 t1_seed[4]={0x12345678,0xabcdef01,0x87654321,0x10fedcba};
__attribute__((used)) static u32 t1_before[9],t1_after[9],t1_target,t1_source,t1_mxcsr=0x1f80;
static u32 t1_fx_checks;
#define T1_CAPTURE(name) \
 "movl %eax,_" name ";movl %ecx,_" name "+4;movl %edx,_" name "+8;movl %ebx,_" name "+12;" \
 "movl %esp,_" name "+16;movl %ebp,_" name "+20;movl %esi,_" name "+24;movl %edi,_" name "+28;pushfl;popl _" name "+32;"
#define T1_INSTRUCTION(name, instruction) \
 __attribute__((naked)) static void name(void){__asm__ volatile( \
 "pushfl;pushal;fxsave _t1_caller_fx;fninit;fld1;fldz;ldmxcsr _t1_mxcsr;" \
 "movdqu _t1_seed,%xmm0;movdqu _t1_seed,%xmm1;movdqu _t1_seed,%xmm2;movdqu _t1_seed,%xmm3;" \
 "movdqu _t1_seed,%xmm4;movdqu _t1_seed,%xmm5;movdqu _t1_seed,%xmm6;movdqu _t1_seed,%xmm7;" \
 "movl $0x11223344,%eax;movl $7,%ecx;movl $0x33445566,%edx;movl $0x44556677,%ebx;movl $0x55667788,%ebp;" \
 "movl _t1_source,%esi;movl _t1_target,%edi;pushl $0x246;popfl;fxsave _t1_before_fx;" \
 T1_CAPTURE("t1_before") instruction ";" T1_CAPTURE("t1_after") \
 "fxsave _t1_after_fx;fxrstor _t1_caller_fx;popal;popfl;retl;");}
T1_INSTRUCTION(t1_load,"movzwl (%edi),%eax")
T1_INSTRUCTION(t1_xor,"xorw $0x1234,(%edi)")
T1_INSTRUCTION(t1_rep,"rep movsb")
T1_INSTRUCTION(t1_simd,"movdqu (%edi),%xmm0")
T1_INSTRUCTION(t1_locked,"lock addw $3,(%edi)")
static u32 t1_even_parity(u32 value){u32 bits=0;for(u32 i=0;i<8;++i)bits+=(value>>i)&1;return !(bits&1);}
static void t1_state_check(u32 test,volatile u16* pixels,const u8* source){
 t1_target=(u32)((const u8*)pixels+(test==2?32:6));t1_source=(u32)source;
 void(*call[5])(void)={t1_load,t1_xor,t1_rep,t1_simd,t1_locked};call[test]();
 u32 want[9];for(u32 i=0;i<9;++i)want[i]=t1_before[i];
 if(test==0)want[0]=0x4589;
 if(test==2){want[1]=0;want[6]+=7;want[7]+=7;}
 u32 flag_mask=0xffffffffu;
 if(test==1 || test==4){
  u32 result=test==1?(0x4589u^0x1234u):0x458cu;
  u32 changed=0x8d5u,flags=(t1_even_parity(result)?4u:0u)|(result==0?0x40u:0u)|(result&0x8000u?0x80u:0u);
  if(test==1){changed&=~0x10u;flag_mask&=~0x10u;} /* XOR AF is undefined. */
  want[8]=(want[8]&~changed)|flags;
 }
 for(u32 i=0;i<9;++i)require((t1_after[i]&(i==8?flag_mask:0xffffffffu))==(want[i]&(i==8?flag_mask:0xffffffffu)),210+i);
 u8 expected[512];for(u32 i=0;i<512;++i)expected[i]=t1_before_fx[i];
 if(test==3)for(u32 i=0;i<16;++i)expected[160+i]=(i==0?0x89u:((i+6)&1)?0x45u:0x67u);
 for(u32 i=0;i<288;++i){
  if(i==5 || i==14 || i==15 || i==22 || i==23)continue;
  if(i>=32 && i<160){u32 reg=(i-32)/16,byte=(i-32)%16,top=(t1_before_fx[3]>>3)&7;
   if(byte>=10 || !(t1_before_fx[4]&(1u<<((reg+top)&7))))continue;}
  require(t1_after_fx[i]==expected[i],220);++t1_fx_checks;
 }
}
#undef T1_CAPTURE
#undef T1_INSTRUCTION
