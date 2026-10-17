#ifndef MNM_NATIVE_GPU_COPY_PLAN_V1_H
#define MNM_NATIVE_GPU_COPY_PLAN_V1_H
#include <stdint.h>
/* Process-local PE32 scalar output, never shared pointers or a channel opcode.
 * Function success means a complete plan; result is the application's HRESULT.
 * A failing result may retain an ordered prefix of successful clip pieces. */
#define MNM_GPU_COPY_PLAN_VERSION 1u
#define MNM_GPU_COPY_PLAN_CAPACITY 256u
#define MNM_GPU_COPY_PLAN_BYTES (48u+24u*MNM_GPU_COPY_PLAN_CAPACITY)
struct MnmGpuCopyPiece {uint32_t sx,sy,width,height,dx,dy;};
struct MnmGpuCopyPlan {
 uint32_t version,bytes,count,result,source_width,source_height,target_width,target_height;
 uint32_t self,keyed,key,reserved;
 struct MnmGpuCopyPiece pieces[MNM_GPU_COPY_PLAN_CAPACITY];
};
#ifdef __cplusplus
static_assert(sizeof(MnmGpuCopyPlan)==MNM_GPU_COPY_PLAN_BYTES,"GPU copy plan ABI");
#else
_Static_assert(sizeof(struct MnmGpuCopyPlan)==MNM_GPU_COPY_PLAN_BYTES,"GPU copy plan ABI");
#endif
#endif
