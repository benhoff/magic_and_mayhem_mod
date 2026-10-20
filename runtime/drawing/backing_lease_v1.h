#ifndef MNM_BACKING_LEASE_V1_H
#define MNM_BACKING_LEASE_V1_H
/* Process-local provider ABI, never a shared-memory or wire object. Every
 * provider must enforce the lease across ALL cooperating aliases, processes
 * and device writers, and retain its callback code until unregister succeeds.
 * A completed producer fence alone does not exclude subsequent writes. */
typedef unsigned int MnmBackingWord;
enum MnmBackingKind { MNM_BACKING_SHARED=1, MNM_BACKING_FILE=2,
 MNM_BACKING_IMPORTED=3, MNM_BACKING_GPU=4 };
enum MnmBackingGuarantees { MNM_BACKING_STABLE_READ=1,
 MNM_BACKING_ALL_WRITERS_EXCLUDED=2, MNM_BACKING_LIFETIME_PIN=4,
 MNM_BACKING_BINDING_TRANSFER=8 };
struct MnmBackingRequestV1 {
 MnmBackingWord version,bytes,submission,texture_id,texture_generation,subresource;
 void *texture,*resource;
 MnmBackingWord width,height,pitch;
 const void *mapped;
};
struct MnmBackingReadV1 {
 MnmBackingWord version,bytes,kind,guarantees;
 MnmBackingWord object_low,object_high,generation,token;
 MnmBackingWord fence_low,fence_high,completed_low,completed_high;
 const void *data;
 MnmBackingWord pitch,size;
};
struct MnmBackingProviderV1 {
 MnmBackingWord version,bytes,id,kind;
 void *context;
 /* 1 accepted, 0 does not own this exact resource, -1 owned but unavailable.
  * Success acquires read ownership and object/code lifetime through release.
  * A refused call must retain no lease. No callback may throw/unwind/reenter. */
 int (*acquire)(void*,const struct MnmBackingRequestV1*,struct MnmBackingReadV1*);
 /* Transfer THIS original binding to immutable private RGB565 storage. The
  * adapter checks Wine's actual postconditions; declaring success is insufficient.
  * Original foreign allocations and descriptor state stay owned until restore. */
 int (*bind_private)(void*,const struct MnmBackingRequestV1*,const struct MnmBackingReadV1*,const void*,MnmBackingWord);
 int (*restore)(void*,const struct MnmBackingRequestV1*,const struct MnmBackingReadV1*);
 /* Verify original storage/binding/generation before releasing read ownership. */
 int (*verify_restored)(void*,const struct MnmBackingRequestV1*,const struct MnmBackingReadV1*);
 void (*release)(void*,const struct MnmBackingReadV1*);
};
/* These helpers perform metadata validation only. A producer is admitted only
 * through registered enforcing callbacks; an arbitrary pointer/flag is no lease. */
static int mnm_backing_read_valid(const struct MnmBackingRequestV1* q,
 const struct MnmBackingProviderV1* p,const struct MnmBackingReadV1* r){
 if(!q||!p||!r||q->version!=1||q->bytes!=sizeof(*q)||r->version!=1||r->bytes!=sizeof(*r)||
    p->version!=1||p->bytes!=sizeof(*p)||!p->id||p->kind<1||p->kind>4||r->kind!=p->kind||
    r->guarantees!=15||(!r->object_low&&!r->object_high)||!r->generation||!r->token||!r->data||
    !q->submission||!q->texture_id||!q->texture_generation||!q->texture||!q->resource||q->subresource||
    !q->width||q->width>800||!q->height||q->height>600||r->pitch<q->width*2||r->pitch>32768||
    r->size<r->pitch*q->height||!p->acquire||!p->bind_private||!p->restore||!p->verify_restored||!p->release)return 0;
 if(r->completed_high<r->fence_high || (r->completed_high==r->fence_high&&r->completed_low<r->fence_low))return 0;
 if(p->kind==MNM_BACKING_GPU&&!r->fence_low&&!r->fence_high)return 0;
 return 1;
}
#endif
