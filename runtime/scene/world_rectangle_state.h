#pragma once
#include <stdint.h>

typedef struct {
    int32_t width,height,x,y,clip_left,clip_top,right,bottom;
    uint32_t stride;
} MnmWorldAddInput;
typedef struct {uint32_t eax,ecx,edx,flags;} MnmWorldAddState;

static inline uint32_t mnm_world_add_flags(uint32_t left,uint32_t right){
    const uint32_t sum=left+right;
    uint32_t parity=sum&255u;parity^=parity>>4;parity^=parity>>2;parity^=parity>>1;
    return(sum<left?1u:0u)|(!(parity&1)?4u:0u)|((left^right^sum)&16u)|(!sum?64u:0u)|
        (sum&0x80000000u?128u:0u)|((~(left^right)&(left^sum)&0x80000000u)?0x800u:0u);
}
/* Whole545c10 prefix, pixel-loop register termination and final ADD ESP,1ch.
 * The original returns AX0; upper EAX, ECX and EDX are branch-specific even
 * when no pixel changes. No destination value or floating instruction supplies
 * these outputs. Prepare atomically before the command is accepted. */
static inline int mnm_world_add_prepare(const MnmWorldAddInput* in,uint32_t entry_sp,MnmWorldAddState* output){
    if(!in||!output||!entry_sp||in->width< -2048||in->width>2048||in->height< -2048||in->height>2048||
       in->x< -4096||in->x>4096||in->y< -4096||in->y>4096||
       in->clip_left<0||in->clip_top<0||in->right<=in->clip_left||in->right>2048||
       in->bottom<=in->clip_top||in->bottom>2048||in->stride<(uint32_t)in->right||in->stride>4096)return 1;
    const int32_t skip_x=in->x<in->clip_left?in->clip_left-in->x:0;
    const int32_t skip_y=in->y<in->clip_top?in->clip_top-in->y:0;
    const int32_t right=in->width<in->right-in->x?in->width:in->right-in->x;
    const int32_t bottom=in->height<in->bottom-in->y?in->height:in->bottom-in->y;
    MnmWorldAddState state;
    state.eax=(uint32_t)(skip_y<bottom?right:bottom)&0xffff0000u;
    state.ecx=skip_y<bottom?0:((uint32_t)(in->y+skip_y)*in->stride+(uint32_t)(in->x+skip_x));
    state.edx=(uint32_t)skip_x;state.flags=mnm_world_add_flags(entry_sp-28u,28u);
    *output=state;return 0;
}
static inline void mnm_world_add_commit(const MnmWorldAddState* state,uint32_t saved[9]){
    saved[7]=state->eax;saved[6]=state->ecx;saved[5]=state->edx;
    saved[8]=(saved[8]&~0x8d5u)|state->flags;
}
