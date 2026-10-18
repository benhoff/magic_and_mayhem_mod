/* Native No-CD40209ca7 image-row body, including its caller-visible registers
 * and defined flags. The original twenty-byte CPU copy body is never entered. */
API i32 WIN FlushInstructionCache(HANDLE,const void*,u32);
static u32 ns_image_copy_installed;
__attribute__((used,noinline)) static void ns_image_copy_prepare(u32* r){
 u32 error=GetLastError(),source=r[7],destination=r[5],bytes=r[2];
 if((r[8]&0x400) || bytes>16u*1024u*1024u || source>0xffffffffu-bytes || destination>0xffffffffu-bytes)ExitProcess(124);
 if(bytes)for(struct NsState* s=ns_owned;s;s=s->next_owned){
  u32 base=(u32)s->pixels,extent=s->pitch*s->height;
  u32 alias=(u32)s->pixels_alias;
  if(alias && ((source<alias+extent && alias<source+bytes) || (destination<alias+extent && alias<destination+bytes)))ExitProcess(124);
  u32 touches_source=source<base+extent && base<source+bytes;
  u32 touches_destination=destination<base+extent && base<destination+bytes;
  if(!touches_source && !touches_destination)continue;
  if((touches_source && (bytes>extent || source<base || source-base>extent-bytes)) ||
     (touches_destination && (bytes>extent || destination<base || destination-base>extent-bytes)))ExitProcess(124);
  ns_cpu_require_at(s,0,0,0x533aec);
 }
 /* Preserve the original forward DWORD granularity for overlapping operands. */
 for(u32 i=0;i<(bytes&~3u);i+=4)*(u32*)(destination+i)=*(const u32*)(source+i);
 for(u32 i=bytes&~3u;i<bytes;++i)*(u8*)(destination+i)=*(const u8*)(source+i);
 r[7]=source+r[4];r[1]=source+bytes;r[0]=destination+bytes;r[6]=0;
 const u32 residual=bytes&3u;
 r[8]=(r[8]&~0x8c5u)|(residual==0?0x40u:0u)|((residual==0 || residual==3)?4u:0u);
 SetLastError(error);
}
__attribute__((naked)) static void ns_image_copy_entry(void){
 __asm__ volatile(
  "pushfl\n\tpushal\n\tcld\n\tmovl %esp,%eax\n\tsubl $528,%esp\n\tandl $-16,%esp\n\tfxsave (%esp)\n\t"
  "pushl %eax\n\tpushl %eax\n\tcalll _ns_image_copy_prepare\n\taddl $4,%esp\n\tpopl %eax\n\t"
  "fxrstor (%esp)\n\tmovl %eax,%esp\n\tpopal\n\tpopfl\n\t"
  "pushl $0x533b00\n\tretl\n\t");
}
static void ns_image_copy_install(void){
 if(!ns_ownership_required || ns_image_copy_installed)return;
 const u8 expected[20]={0x8b,0xcd,0x8b,0xf0,0x8b,0xfa,0x03,0xc3,0xc1,0xe9,2,0xf3,0xa5,0x8b,0xcd,0x83,0xe1,3,0xf3,0xa4};
 u8* site=(u8*)0x533aec;u32 memory[7],old,ignored;
 if((u32)GetModuleHandleA(0)!=0x400000 || VirtualQuery(site,memory,28)!=28 ||
    memory[4]!=0x1000 || !(memory[5]&0xee) || (memory[5]&0x101))ExitProcess(124);
 for(u32 i=0;i<sizeof(expected);++i)if(site[i]!=expected[i])ExitProcess(124);
 if(!VirtualProtect(site,8,0x40,&old))ExitProcess(124);
 site[0]=0xe9;*(u32*)(site+1)=(u32)ns_image_copy_entry-(u32)site-5;
 site[5]=site[6]=site[7]=0x90;
 if(!VirtualProtect(site,8,old,&ignored) || !FlushInstructionCache((HANDLE)-1,site,8))ExitProcess(124);
 ns_image_copy_installed=1;
}
