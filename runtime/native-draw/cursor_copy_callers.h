#include "../../protocols/include/mnm/copy_caller_handoff_v1.h"
#include "../../protocols/include/mnm/cursor_copy_caller_v1.h"
#include "cursor_copy_caller_thunks.h"
struct NdCursorCaller {struct MnmCursorCallerFrame frame;struct NdCursorCaller* previous;};
static struct NdCursorCaller* nd_cursor_caller_active;
static u32 nd_cursor_caller_enabled;
static void* nd_cursor_caller_detour(u32 index);
struct NdCallerProfile {u32 address,length,pop;u8 prefix[8];};
static const struct NdCallerProfile nd_caller_profiles[5]={
 {0x58c140,5,12,{0x83,0xec,0x30,0x53,0x55}},
 {0x58c360,6,16,{0x83,0xec,0x18,0x33,0xd2,0x53}},
 {0x58c4a0,6,16,{0x83,0xec,0x20,0x33,0xd2,0x53}},
 {0x58c8a0,7,16,{0x83,0xec,0x20,0x8b,0x44,0x24,0x2c}},
 {0x58ca90,8,16,{0x83,0xec,0x18,0x53,0x8b,0x5c,0x24,0x28}}};
static u32 nd_cursor_caller_query(u32 address,u32 slot,u32* output,u32 words){
 if(!nd_cursor_caller_enabled || !nd_cursor_installed || words!=32 || !output || !nd_cursor_readable(output,128))return 0;
 u32 n=0;for(;n<5;++n)if(nd_caller_profiles[n].address==address)break;if(n==5)return 0;
 const struct NdCallerProfile* p=nd_caller_profiles+n;u32 index=n+13;
 u8* entry=(u8*)address,*trampoline=nd_cursor_original[index];
 if(!nd_cursor_readable(entry,p->length) || !nd_cursor_readable(trampoline,p->length+5) || entry[0]!=0xe9 ||
    address+5+*(u32*)(entry+1)!=(u32)nd_cursor_caller_detour(index))return 0;
 /* Scene saved six bytes for the two rect hooks; the native installer alone
  * saves five. Verify either exact continuation and normalize the full pinned
  * profile only inside the diagnostic reader, never in executable memory. */
 u32 displaced=p->length;
 if(n==1 || n==2){if(trampoline[5]==0xe9)displaced=5;}
 for(u32 k=0;k<displaced;++k)if(trampoline[k]!=p->prefix[k])return 0;
 if(trampoline[displaced]!=0xe9 || (u32)trampoline+displaced+5+*(u32*)(trampoline+displaced+1)!=address+displaced)return 0;
 for(u32 k=5;k<p->length;++k)if(entry[k]!=(k<displaced?0x90:p->prefix[k]))return 0;
 u32 response[32]={MNM_COPY_CALLER_HANDOFF_MAGIC,1,128};
 response[3]=address;response[4]=p->length;response[5]=(u32)trampoline;response[6]=(u32)nd_cursor_caller_detour(index);
 response[7]=(u32)(index==13?nd_copy_caller_whole_return:nd_copy_caller_rect_return);response[8]=p->pop;
 nd_cursor_copy((u8*)response+64,p->prefix,p->length);
 if(slot){
  u32 thread=ns_lifetime_thread();
  if(__atomic_load_n(&ns_lifetime_owner,__ATOMIC_ACQUIRE)!=thread || !ns_lifetime_depth ||
     slot>0xffffffffu-4-p->pop || !nd_cursor_readable((void*)slot,8) || !nd_cursor_caller_active)return 0;
  const struct MnmCursorCallerFrame* f=&nd_cursor_caller_active->frame;
  if(!nd_cursor_readable((void*)f->source,48) || !nd_cursor_readable((void*)f->destination,48))return 0;
  u32 source=((u32*)f->source)[2],destination=((u32*)f->destination)[2];
  struct NsState *s=ns_lifetime_find((void*)source),*d=ns_lifetime_find((void*)destination);
  if(!s || !d || s!=ns_lifetime_find((void*)((u32*)f->source)[1]) || d!=ns_lifetime_find((void*)((u32*)f->destination)[1]) ||
     !mnm_cursor_caller_frame_matches(f,index,thread,slot,response[7],*(u32*)slot,*(u32*)(slot+4),source,destination,s->lifetime_identity,d->lifetime_identity))return 0;
  response[9]=1;response[10]=f->caller;response[11]=f->destination;response[12]=f->source;response[13]=slot+4+p->pop;response[14]=thread;
 }
 nd_cursor_copy(output,response,sizeof(response));return 1;
}
__declspec(dllexport) u32 WIN NativeDrawCopyCallerV1(u32 address,u32 slot,u32* output,u32 words){
 u32 error=GetLastError(),result=nd_cursor_caller_query(address,slot,output,words);SetLastError(error);return result;
}
