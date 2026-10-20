#ifndef MNM_BACKING_LEASE_V2_H
#define MNM_BACKING_LEASE_V2_H
#include "backing_lease_v1.h"
/* Process-local ABI. Version2 retains the original foreign binding under a
 * producer read lease. No Wine READ map or in-place binding transfer is needed.
 * The request has version2, mapped=NULL and pitch=0. The read has version2 and
 * guarantees exactly7, covering the ACTUAL resource and its same-value CPU
 * readback through the recorder's completed consumer fence and release.
 * Acquire is nonblocking: report unavailable until producer work is complete.
 * Callbacks must not reenter, block on Wine/recorder work, throw, or unwind.
 * This profile requires MNM_DRAW_WINE_SOURCE_OWNERSHIP=2; private-only profile1
 * never admits a lease-backed original binding. Version1 remains unchanged. */
typedef struct MnmBackingRequestV1 MnmBackingRequestV2;
typedef struct MnmBackingReadV1 MnmBackingReadV2;
struct MnmBackingProviderV2 {
 MnmBackingWord version,bytes,id,kind;
 void* context;
 int (*acquire)(void*,const MnmBackingRequestV2*,MnmBackingReadV2*);
 /* Called only after the exact consumer GPU/context fence has completed.
  * Verify resource/object generation and the still-held read lease. */
 int (*verify_unchanged)(void*,const MnmBackingRequestV2*,const MnmBackingReadV2*);
 void (*release)(void*,const MnmBackingReadV2*);
};
static int mnm_backing_read2_valid(const MnmBackingRequestV2* q,unsigned kind,const MnmBackingReadV2* r){
 if(!q||!r||q->version!=2||q->bytes!=sizeof(*q)||q->mapped||q->pitch||r->version!=2||r->bytes!=sizeof(*r)||
    kind<1||kind>4||r->kind!=kind||r->guarantees!=7||(!r->object_low&&!r->object_high)||!r->generation||!r->token||!r->data||
    !q->submission||!q->texture_id||!q->texture_generation||!q->texture||!q->resource||q->subresource||
    !q->width||q->width>800||!q->height||q->height>600||r->pitch<q->width*2||r->pitch>32768||r->size<r->pitch*q->height)return 0;
 if(r->completed_high<r->fence_high||(r->completed_high==r->fence_high&&r->completed_low<r->fence_low))return 0;
 if(kind==MNM_BACKING_GPU&&!r->fence_low&&!r->fence_high)return 0;
 return 1;
}
#endif
