/* Native No-CD40209ca7 image-row body, including its caller-visible registers
 * and defined flags. The original twenty-byte CPU copy body is never entered. */
API i32 WIN FlushInstructionCache(HANDLE,const void*,u32);
API i32 WIN IntersectRect(i32*,const i32*,const i32*);
#include "image_copy_original.h"
static u32 ns_image_copy_installed;
__attribute__((used,noinline)) static void ns_image_copy_prepare(u32* r){
 struct NsLifeGuard lifetime __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 u32 error=GetLastError(),source=r[7],destination=r[5],bytes=r[2];
 struct NsState* target=0;
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
  /* Only the source is read. A protected destination may accept value-only
   * stores without materializing the unrelated rest of its GPU history. */
  if(touches_source)ns_cpu_require_at(s,0,0,0x533aec);
  if(touches_destination){if(target && target!=s)ExitProcess(124);target=s;}
 }
 const u32 private_write=target && target->history_stale && ns_guard_enabled==3 &&
  target->pixels_alias && target->cpu_dirty && target->cpu_guard && target->locked>0 &&
  target->lock_owner==GetCurrentThreadId() && !target->dc && !target->storage_dc && !target->cpu_guard_world;
 if(target && !private_write)ns_cpu_require_at(target,0,0,0x533aec);
 /* Preserve forward DWORD/residual order, including overlap. A source in this
  * same owned allocation required current history above and takes the ordinary
  * path. Every private store keeps its exact dirty bytes and stale authority. */
 for(u32 i=0;i<(bytes&~3u);i+=4){u32 value=*(const u32*)(source+i);
  if(private_write){if(NativeDrawCpuStoreV1((void*)(destination+i),value,4))ExitProcess(124);}
  else *(u32*)(destination+i)=value;
 }
 for(u32 i=bytes&~3u;i<bytes;++i){u32 value=*(const u8*)(source+i);
  if(private_write){if(NativeDrawCpuStoreV1((void*)(destination+i),value,1))ExitProcess(124);}
  else *(u8*)(destination+i)=(u8)value;
 }
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
/* Complete recovered thiscall entry: destination, destination byte pitch,
 * unused third argument, x, y and clip. Horizontal clipping is in bytes.
 * Preserve the original non-WORD preparation quirk (initial source stride=x),
 * rather than silently changing an unobserved branch to a different policy. */
__attribute__((used,noinline)) static void ns_image_copy_full_prepare(u32* saved,u8* outgoing_fx){
 const u32* image=(const u32*)saved[6];u32* arguments=saved+10;
 u8* destination=(u8*)arguments[0];i32 pitch=(i32)arguments[1];
 i32 x=(i32)arguments[3],y=(i32)arguments[4];const i32* clip=(const i32*)arguments[5];
 u32 error=GetLastError();
 if(saved[8]&0x400u)ExitProcess(124); /* Existing forward-copy direction policy. */
 /* The original last flag-writing instruction is ADD ESP,38h, after its
  * four callee-register pops. Derive its exact arithmetic flags from the
  * original entry ESP, independent of this helper's compiler stack frame.
  * PUSHAL saved ESP points at the preceding PUSHFL word. */
 u32 original_stack=saved[3]+4u-0x38u,return_flags;
 __asm__ volatile("addl $0x38,%0;pushfl;popl %1":"+r"(original_stack),"=r"(return_flags)::"cc");
 saved[8]=(saved[8]&~0x8d5u)|(return_flags&0x8d5u);
 i32 word=image[4]!=0,width=(i32)image[2],height=(i32)image[3];
 if(!clip || width<0 || height<0 || width>8*1024*1024 || height>1024*1024)ExitProcess(124);
 const u32 scale=word?2u:1u;
 i32 origin=(i32)((u32)x*scale);
 i32 bounds[4]={origin,y,(i32)((u32)origin+(u32)width*2u),(i32)((u32)y+(u32)height)};
 i32 clip_bounds[4]={(i32)((u32)clip[0]*scale),clip[1],(i32)((u32)clip[2]*scale),clip[3]},intersection[4];
 /* The original calls USER32.IntersectRect. Its provider may modify volatile
  * SIMD state (the real Wine empty case clears XMM0). Execute the same service
  * with the incoming FP state and retain its outgoing image through native
  * ownership/history work. Integer preparation/copying itself changes no FX. */
 __asm__ volatile("fxrstor (%0)"::"r"(outgoing_fx):"memory");
 IntersectRect(intersection,bounds,clip_bounds);
 __asm__ volatile("fxsave (%0)"::"r"(outgoing_fx):"memory");
 error=GetLastError();
 long long left=intersection[0],top=intersection[1],right=intersection[2],bottom=intersection[3];
 if(right<=left || bottom<=top){saved[7]=0;saved[6]=0;saved[5]=0;SetLastError(error);return;}
 const i32 stride=word?width*2:width;
 long long source=(long long)image[0]+(top-y)*(word?width*2:x)-origin+left;
 long long target=(u32)destination+top*pitch+left;
 if(right-left>16*1024*1024 || bottom-top>1024*1024)ExitProcess(124);
 for(long long row=top;row<bottom;++row){
  if(source<0 || source>0xffffffffu || target<0 || target>0xffffffffu)ExitProcess(124);
  u32 r[9]={0};r[7]=(u32)source;r[5]=(u32)target;r[2]=(u32)(right-left);
  ns_image_copy_prepare(r);source+=stride;target+=pitch;
 }
 saved[7]=(u32)source;saved[6]=0;saved[5]=(u32)target;
 /* Original533b0b reuses the incoming X argument as its row countdown. */
 arguments[3]=0;
 SetLastError(error);
}
__attribute__((naked)) static void ns_image_copy_full_entry(void){
 __asm__ volatile(
  "pushfl\n\tpushal\n\tcld\n\tmovl %esp,%eax\n\tsubl $528,%esp\n\tandl $-16,%esp\n\tfxsave (%esp)\n\t"
  "movl %esp,%edx\n\tpushl %eax\n\tpushl %edx\n\tpushl %eax\n\tcalll _ns_image_copy_full_prepare\n\taddl $8,%esp\n\tpopl %eax\n\t"
  "fxrstor (%esp)\n\tmovl %eax,%esp\n\tpopal\n\tpopfl\n\tretl $24\n\t");
}
static void ns_image_copy_install(void){
 if(!ns_ownership_required || ns_image_copy_installed)return;
 const u8 expected[20]={0x8b,0xcd,0x8b,0xf0,0x8b,0xfa,0x03,0xc3,0xc1,0xe9,2,0xf3,0xa5,0x8b,0xcd,0x83,0xe1,3,0xf3,0xa4};
 u8* site=(u8*)0x533aec;u8* entry=(u8*)0x533a00;u32 memory[7],old,ignored;
 if((u32)GetModuleHandleA(0)!=0x400000 || VirtualQuery(site,memory,28)!=28 ||
    memory[4]!=0x1000 || !(memory[5]&0xee) || (memory[5]&0x101))ExitProcess(124);
 for(u32 i=0;i<sizeof(expected);++i)if(site[i]!=expected[i])ExitProcess(124);
 for(u32 i=0;i<sizeof(ns_image_copy_original);++i)if(entry[i]!=ns_image_copy_original[i])ExitProcess(124);
 if(!VirtualProtect(entry,8,0x40,&old))ExitProcess(124);
 entry[0]=0xe9;*(u32*)(entry+1)=(u32)ns_image_copy_full_entry-(u32)entry-5;
 entry[5]=entry[6]=entry[7]=0x90;
 if(!VirtualProtect(entry,8,old,&ignored) || !FlushInstructionCache((HANDLE)-1,entry,8))ExitProcess(124);
 if(!VirtualProtect(site,8,0x40,&old))ExitProcess(124);
 site[0]=0xe9;*(u32*)(site+1)=(u32)ns_image_copy_entry-(u32)site-5;
 site[5]=site[6]=site[7]=0x90;
 if(!VirtualProtect(site,8,old,&ignored) || !FlushInstructionCache((HANDLE)-1,site,8))ExitProcess(124);
 ns_image_copy_installed=1;
}
