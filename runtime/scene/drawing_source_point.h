#include "../../protocols/include/mnm/drawing_source_point_v1.h"
/* Read-only association with known wrapper interface fields+4/+8. The guard
 * owns recorder state only. Pending frames and unknown/ambiguous associations
 * remain explicit; this metadata cannot assert immutable producer inputs. */
u32 WIN SceneDrawingSourcePointV1(void* com,u32* out,u32 size){
 u32 error=GetLastError();
 if(size!=MNM_DRAW_SOURCE_POINT_BYTES||!out||!producer_trace_mode||!bypass_writable((u32)out,64)){SetLastError(error);return 0;}
 PRODUCER_TRACE_SCOPE;
 u32 r[16]={0};r[0]=MNM_DRAW_SOURCE_POINT_BYTES;r[1]=MNM_DRAW_SOURCE_POINT_VERSION;
 r[2]=MNM_DRAW_SOURCE_POINT_TRACE|(enabled?MNM_DRAW_SOURCE_POINT_LIVE:0)|(stopped?MNM_DRAW_SOURCE_POINT_STOPPED:0);
 r[3]=sequence;r[4]=queue;r[5]=failures;r[14]=GetCurrentThreadId();
 for(u32 i=0;i<MAX_FRAMES;++i)if(frames[i].sp)++r[12];
 for(u32 i=0;i<32;++i)if(producer_trace_threads[i].id==r[14]){r[13]=producer_trace_threads[i].depth;break;}
 struct Surface* match=0;u32 matches=0;
 if(com)for(u32 i=0;i<MAX_SURFACES;++i){struct Surface* s=surfaces+i;
  if(s->object&&readable(s->object+4,8)&&((u32)com==get((void*)(s->object+4))||(u32)com==get((void*)(s->object+8)))){match=s;++matches;}
 }
 if(matches==1){r[2]|=MNM_DRAW_SOURCE_POINT_MATCH;r[6]=match->id;r[7]=match->width;r[8]=match->height;r[9]=match->stride;r[10]=match->dirty;r[11]=match->sampled;}
 else if(matches>1)r[2]|=MNM_DRAW_SOURCE_POINT_AMBIGUOUS;
 copy(out,r,64);SetLastError(error);return 1;
}
/* One recorder scope for both associations. Original bytes remain outside
 * this scope; identical sequence fields are metadata consistency only. */
u32 WIN SceneDrawingSourcePairV1(void* destination,void* source,u32* out,u32 size){
 u32 error=GetLastError();
 if(size!=MNM_DRAW_SOURCE_PAIR_BYTES||!producer_trace_mode||!bypass_writable((u32)out,128)){SetLastError(error);return 0;}
 PRODUCER_TRACE_SCOPE;
 u32 okay=SceneDrawingSourcePointV1(destination,out,64)&&SceneDrawingSourcePointV1(source,out+16,64);
 SetLastError(error);return okay;
}
