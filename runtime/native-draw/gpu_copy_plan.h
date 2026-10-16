#include "../../protocols/include/mnm/native_gpu_copy_plan_v1.h"
/* Called after the facade's ordinary geometry/clip/lease/format/key checks.
 * Match owned CPU clipping order and retain partial writes on source failure. */
static void ns_gpu_plan(struct MnmGpuCopyPlan* p,const struct NsState* d,const struct NsState* s,
 const i32* rd,const i32* rs,const u32* clips,u32 keyed){
 ns_zero((u32*)p,sizeof(*p)/4);p->version=1;p->bytes=sizeof(*p);
 p->source_width=s->width;p->source_height=s->height;p->target_width=d->width;p->target_height=d->height;
 p->self=d==s;p->keyed=keyed;p->key=keyed?s->key_low:0;
 for(u32 region=0;region<(clips?clips[2]:1u);++region){
  const i32* clip=clips?(const i32*)(clips+8+4*region):0;
  i32 left=rd[0],top=rd[1],right=rd[2],bottom=rd[3];
  if(left<0)left=0;if(top<0)top=0;if(right>(i32)d->width)right=d->width;if(bottom>(i32)d->height)bottom=d->height;
  if(clip){if(left<clip[0])left=clip[0];if(top<clip[1])top=clip[1];if(right>clip[2])right=clip[2];if(bottom>clip[3])bottom=clip[3];}
  if(left>=right || top>=bottom)continue;
  long long sx=(long long)rs[0]+left-rd[0],sy=(long long)rs[1]+top-rd[1];
  if(sx<0 || sy<0 || sx+right-left>s->width || sy+bottom-top>s->height){p->result=(u32)NS_RECT;break;}
  p->pieces[p->count++]=(struct MnmGpuCopyPiece){(u32)sx,(u32)sy,(u32)(right-left),(u32)(bottom-top),(u32)left,(u32)top};
 }
}
/* The ordered bridge must commit this exact plan before returning to callers.
 * No CPU image bytes are copied, read, updated or declared current here. */
__declspec(dllexport) i32 WIN NativeDrawGpuCopyPlanV1(void* dst,const i32* dr,void* src,const i32* sr,u32 flags,struct MnmGpuCopyPlan* out,u32 bytes){
 struct NsState *d=0,*s=0;
 for(struct NsState* state=ns_owned;state;state=state->next_owned)for(u32 i=0;i<5;++i){
  if(dst==&state->aliases[i])d=state;if(src==&state->aliases[i])s=state;}
 if(!d || !s || !dr || !sr || !out || bytes!=sizeof(*out))return ND_INVALID;
 long long width=(long long)dr[2]-dr[0],height=(long long)dr[3]-dr[1];
 if(width<=0 || height<=0 || sr[2]<=sr[0] || sr[3]<=sr[1])return NS_RECT;
 i32 rd[4],rs[4];for(u32 i=0;i<4;++i){rd[i]=dr[i];rs[i]=sr[i];}
 if(!d->clipper && (!ns_rectangle(d,rd,rd) || !ns_rectangle(s,rs,rs)))return NS_RECT;
 if((long long)sr[2]-sr[0]!=width || (long long)sr[3]-sr[1]!=height)return NS_UNSUPPORTED;
 u32* clips=0;i32 result=0;
 if(d->clipper){result=nc_snapshot(d->clipper,&clips);if(result)return result;}
 if(ns_borrowed(d) || ns_borrowed(s)){result=NS_BUSY;goto done;}
 if(flags&~0x8000u){result=NS_UNSUPPORTED;goto done;}
 if(s->bits!=16 || d->bits!=16 || s->masks[0]!=0xf800 || s->masks[1]!=0x7e0 || s->masks[2]!=31 ||
    d->masks[0]!=s->masks[0] || d->masks[1]!=s->masks[1] || d->masks[2]!=s->masks[2]){result=NS_UNSUPPORTED;goto done;}
 if((flags&0x8000) && !s->key_set){result=ND_INVALID;goto done;}
 if(!s->history_stale || ((flags&0x8000) && s->key_low!=s->key_high)){result=NS_UNSUPPORTED;goto done;}
 ns_gpu_plan(out,d,s,rd,rs,clips,(flags&0x8000)!=0);
 if(out->count){d->history_stale=1;for(u32 i=0;i<2;++i){d->history_generation[i]=s->history_generation[i];d->history_nonce[i]=s->history_nonce[i];}++ns_copies;}
 done:if(clips)HeapFree(GetProcessHeap(),0,clips);return result;
}
