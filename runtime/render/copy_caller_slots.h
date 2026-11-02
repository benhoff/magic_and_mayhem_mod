#pragma once
#include "copy_caller_layouts.h"
typedef int (*CopyCallerRead)(void*,u32,void*,u32);
/* Resolve a reviewed fixed return slot, never a guessed EBP chain. All reads
 * are bounded and supplied by the caller; output has no rendering consumer.
 * Cache only the code lock, not stack/object values or process addresses. */
static u32 copy_caller_resolve(u32 base,u32 extent,u32 api,u32 slot,u32 fast,u32 target,
 CopyCallerRead read,void* context,u8* code_state,u32* out){
 for(unsigned i=0;i<16;++i)out[i]=0;
 out[1]=api;out[2]=api>=base && api-base<extent?api-base:UINT32_MAX;out[8]=target;
 const struct CopyCallerSite* site=0;
 for(unsigned i=0;i<sizeof(copy_caller_sites)/sizeof(copy_caller_sites[0]);++i)
  if(out[2]==copy_caller_sites[i].api){site=copy_caller_sites+i;break;}
 u32 state=2;
 if(!site || !base || base>UINT32_MAX-extent)goto done;
 const struct CopyCallerLayout* layout=copy_caller_layouts+site->layout;
 out[3]=layout->entry;out[4]=layout->distance;state=3;
 if(layout->end>extent || site->api<3 || fast!=(site->method==0x1c))goto done;
 if(!code_state[site->layout]){
  u8 body[600];const u32 length=layout->end-layout->entry;u32 hash=2166136261u;
  if(length>sizeof(body) || !read(context,base+layout->entry,body,length))code_state[site->layout]=2;
  else{for(u32 i=0;i<length;++i)hash=(hash^body[i])*16777619u;code_state[site->layout]=hash==layout->checksum?1:2;}
 }
 u8 instruction[3];
 if(code_state[site->layout]!=1 || !read(context,api-3,instruction,3) ||
    instruction[0]!=0xff || instruction[1]!=site->modrm || instruction[2]!=site->method)goto done;
 state=4;
 if(!slot || slot>UINT32_MAX-layout->distance-8)goto done;
 u32 original[2];if(!read(context,slot+layout->distance,original,8))goto done;
 out[5]=original[0];out[6]=original[0]>=base && original[0]-base<extent?original[0]-base:UINT32_MAX;out[7]=original[1];
 state=5;
 u32 surface=0;if(!original[1] || original[1]>UINT32_MAX-8 || !read(context,original[1]+8,&surface,4) || surface!=target)goto done;
 state=6;
 if(original[0]<base+5 || original[0]-base>=extent)goto done;
 u8 call[5];if(!read(context,original[0]-5,call,5) || call[0]!=0xe8)goto done;
 const u32 displacement=(u32)call[1]|(u32)call[2]<<8|(u32)call[3]<<16|(u32)call[4]<<24;
 if(original[0]+displacement!=base+layout->entry)goto done;
 state=1;
done:out[9]=state;return state;
}
