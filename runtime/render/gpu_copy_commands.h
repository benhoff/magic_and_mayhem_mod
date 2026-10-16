#ifndef MNM_GPU_COPY_COMMANDS_H
#define MNM_GPU_COPY_COMMANDS_H
#include "../../protocols/include/mnm/native_gpu_copy_plan_v1.h"
/* No emission before complete scalar validation. Aliased operations consume one
 * immutable GPU source for every clip piece, matching owned facade policy. */
static int mnm_gpu_copy_plan_valid(const struct MnmGpuCopyPlan* p,uint32_t source,uint32_t target,uint32_t scratch){
 if(!p || p->version!=1 || p->bytes!=sizeof(*p) || p->count>256 || p->reserved || p->self>1 || p->keyed>1 ||
    (!p->keyed && p->key) || p->key>65535 || !source || !target || p->self!=(source==target) ||
    !p->source_width || !p->source_height || !p->target_width || !p->target_height ||
    p->source_width>2048 || p->source_height>2048 || p->target_width>2048 || p->target_height>2048 ||
    (p->result && p->result!=0x88760096u))return 0;
 if(p->self && (p->source_width!=p->target_width || p->source_height!=p->target_height))return 0;
 if(p->self && p->count){if(!scratch || scratch==source || scratch==target)return 0;}else if(scratch)return 0;
 for(uint32_t i=0;i<p->count;++i){const struct MnmGpuCopyPiece* r=p->pieces+i;
  if(!r->width || !r->height || r->width>p->source_width || r->height>p->source_height ||
     r->width>p->target_width || r->height>p->target_height || r->sx>p->source_width-r->width ||
     r->sy>p->source_height-r->height || r->dx>p->target_width-r->width || r->dy>p->target_height-r->height)return 0;}
 for(uint32_t i=p->count;i<MNM_GPU_COPY_PLAN_CAPACITY;++i){const struct MnmGpuCopyPiece* r=p->pieces+i;
  if(r->sx || r->sy || r->width || r->height || r->dx || r->dy)return 0;}
 return 1;
}
typedef int (*MnmGpuCopyEmit)(void*,const uint32_t*);
static int mnm_gpu_copy_commands(const struct MnmGpuCopyPlan* p,uint32_t source,uint32_t target,uint32_t scratch,MnmGpuCopyEmit emit,void* context){
 if(!emit || !mnm_gpu_copy_plan_valid(p,source,target,scratch))return 0;
 if(p->self && p->count){uint32_t f[10]={source,scratch,0,0,p->source_width,p->source_height,0,0,0,0};if(!emit(context,f))return 0;source=scratch;}
 for(uint32_t i=0;i<p->count;++i){const struct MnmGpuCopyPiece* r=p->pieces+i;
  uint32_t f[10]={source,target,r->sx,r->sy,r->sx+r->width,r->sy+r->height,r->dx,r->dy,p->keyed,p->key};
  if(!emit(context,f))return 0;}
 return 1;
}
#endif
