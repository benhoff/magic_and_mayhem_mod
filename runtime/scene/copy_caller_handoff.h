#include "../../protocols/include/mnm/copy_caller_handoff_v1.h"
/* Read only published hook/frame state; original calls, stack, arguments and
 * producer lifecycles are never modified by this diagnostic query. */
static u32 scene_copy_caller_query(u32 address,u32 slot,u32* output,u32 words){
 char mode[2];
 if(!enabled || stopped || words!=32 || !output || !readable((u32)output,128) ||
    GetEnvironmentVariableA("MNM_NATIVE_COPY_CALLERS",mode,2)!=1 || mode[0]!='1')return 0;
 const u32 indices[7]={7,8,32,33,38,39,41};
 for(unsigned n=0;n<7;++n){
  u32 index=indices[n];const struct Hook* h=hooks+index;if(h->address!=address)continue;
  u8* entry=(u8*)address;u8* original=(u8*)producer_trampolines[index];
  if(!readable(address,h->length) || !readable((u32)original,h->length+5) || entry[0]!=0xe9 ||
     address+5+get(entry+1)!=(u32)h->entry)return 0;
  for(u32 k=5;k<h->length;++k)if(entry[k]!=0x90)return 0;
  for(u32 k=0;k<h->length;++k)if(original[k]!=h->bytes[k])return 0;
  if(original[h->length]!=0xe9 || (u32)original+h->length+5+get(original+h->length+1)!=address+h->length)return 0;
  u32 response[32]={MNM_COPY_CALLER_HANDOFF_MAGIC,1,128};
  response[3]=address;response[4]=h->length;response[5]=(u32)original;response[6]=(u32)h->entry;response[7]=(u32)producer_leave;response[8]=h->pop;
  copy((u8*)response+64,h->bytes,h->length);
  if(slot){
   if(slot>0xffffffffu-4-h->pop || !readable(slot,8))return 0;
   const u32 expected=slot+4+h->pop;
   for(unsigned i=0;i<MAX_FRAMES;++i){
    if(__atomic_load_n(&frames[i].sp,__ATOMIC_ACQUIRE)!=expected)continue;
    const struct Frame frame=frames[i];
    if(__atomic_load_n(&frames[i].sp,__ATOMIC_ACQUIRE)!=expected || frame.tag!=index ||
       (frame.owner && frame.owner!=GetCurrentThreadId()) || get((void*)slot)!=(u32)producer_leave ||
       get((void*)(slot+4))!=frame.args[0])return 0;
    if(response[9])return 0;
    response[9]=1;response[10]=frame.caller;response[11]=frame.args[0];response[12]=frame.ecx;response[13]=expected;response[14]=GetCurrentThreadId();
   }
  }
  copy(output,response,sizeof(response));return 1;
 }
 return 0;
}
__declspec(dllexport) u32 WIN SceneCopyCallerV1(u32 address,u32 slot,u32* result,u32 words){
 u32 error=GetLastError(),value=scene_copy_caller_query(address,slot,result,words);SetLastError(error);return value;
}
