#include "../../protocols/include/mnm/drawing_storage_point_v1.h"
/* Compare cached addresses only; do not read storage or driver object layouts. */
u32 WIN SceneDrawingStoragePointV1(void* storage,u32 row_pitch,u32 slice_pitch,u32* out,u32 size){
 u32 error=GetLastError();
 if(size!=64||!producer_trace_mode||!bypass_writable((u32)out,64)){SetLastError(error);return 0;}
 PRODUCER_TRACE_SCOPE;
 u32 r[16]={64,1};r[2]=1|(enabled?2:0)|(stopped?4:0);r[3]=sequence;r[4]=queue;r[5]=failures;r[14]=GetCurrentThreadId();
 for(u32 i=0;i<MAX_FRAMES;++i)if(frames[i].sp)++r[12];
 for(u32 i=0;i<32;++i)if(producer_trace_threads[i].id==r[14]){r[13]=producer_trace_threads[i].depth;break;}
 struct Surface* match=0;u32 matches=0;
 if(storage&&enabled&&!stopped&&!failures)for(u32 i=0;i<MAX_SURFACES;++i){struct Surface* s=surfaces+i;
  if(s->object&&s->pixels==(u32)storage){match=s;++matches;}
 }
 if(matches>1)r[2]|=8;
 else if(matches==1){
  if(!match->width||!match->height||match->width>2048||match->height>2048||match->stride<match->width||match->stride>4096||row_pitch!=match->stride*2||slice_pitch<row_pitch*match->height)r[2]|=32;
  else{r[2]|=16;r[6]=match->id;r[7]=match->width;r[8]=match->height;r[9]=match->stride;r[10]=match->dirty;r[11]=match->sampled;}
 }
 copy(out,r,64);SetLastError(error);return 1;
}
