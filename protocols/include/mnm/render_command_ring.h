/* Version2 single-producer/single-consumer mapped-byte ring. Host-local cursors;
 * wire identity/counters are aligned32-bit little-endian atomics on x86.
 * Nonblocking: FULL writes nothing; callers must retain input and retry later.
 * ACK means a private copy owns the bytes, not that GPU execution is complete. */
#ifndef MNM_RENDER_COMMAND_RING_H
#define MNM_RENDER_COMMAND_RING_H
#include "render_commands_v2.h"
#include <stdint.h>
#if !defined(__BYTE_ORDER__) || __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error "Command ring requires a verified little-endian atomic32 host"
#endif
#if !defined(__i386__) && !defined(__x86_64__)
#error "Command ring currently supports x86 atomic32 hosts only"
#endif
#define MNM_RING_REJECTED (-1)
#define MNM_RING_FULL 0
#define MNM_RING_WRITTEN 1
struct mnm_ring_writer {uint32_t* map;uint32_t session,published,acknowledged;};
struct mnm_ring_reader {uint32_t* map;uint32_t session,consumed,published,state,failed;};
static inline uint32_t mnm_ring_load(const uint32_t* p){return __atomic_load_n(p,__ATOMIC_ACQUIRE);}
static inline void mnm_ring_store(uint32_t* p,uint32_t value){__atomic_store_n(p,value,__ATOMIC_RELEASE);}
static inline void mnm_ring_copy(unsigned char* dst,const unsigned char* src,uint32_t n){for(uint32_t i=0;i<n;++i)dst[i]=src[i];}
static inline int mnm_ring_identity(const uint32_t* p,uint32_t session){
    if(!p || ((uintptr_t)p&3) || !session)return 0;
    const unsigned char* b=(const unsigned char*)p;
    for(unsigned i=0;i<8;++i)if(b[i]!=(unsigned char)MNM_RENDER_COMMANDS_V2_MAGIC[i])return 0;
    if(p[2]!=2 || p[3]!=MNM_RENDER_COMMANDS_V2_SIZE || p[4]!=session)return 0;
    for(unsigned i=10;i<16;++i)if(p[i])return 0;
    return 1;
}
static inline void mnm_ring_fail(struct mnm_ring_writer* w,uint32_t reason){
    if(mnm_ring_identity(w->map,w->session) && mnm_ring_load(w->map+6)==1){
        w->map[7]=reason;mnm_ring_store(w->map+6,3);
    }
}
static inline int mnm_ring_writer_bind(struct mnm_ring_writer* w,void* mapping,uint32_t size){
    uint32_t* p=(uint32_t*)mapping;w->map=0;w->session=w->published=w->acknowledged=0;
    if(!p || ((uintptr_t)p&3) || size!=MNM_RENDER_COMMANDS_V2_SIZE || !mnm_ring_identity(p,p[4]))return 0;
    if(mnm_ring_load(p+5) || mnm_ring_load(p+9) || mnm_ring_load(p+7) || mnm_ring_load(p+8))return 0;
    uint32_t ready=0;if(!__atomic_compare_exchange_n(p+6,&ready,1,0,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE))return 0;
    w->map=p;w->session=p[4];w->published=w->acknowledged=0;return 1;
}
static inline int mnm_ring_writer_valid(struct mnm_ring_writer* w){
    if(!mnm_ring_identity(w->map,w->session) || mnm_ring_load(w->map+6)!=1)return 0;
    if(mnm_ring_load(w->map+8)){mnm_ring_fail(w,MNM_RENDER_COMMANDS_V2_REASON_CANCELLED);return 0;}
    const uint32_t ack=mnm_ring_load(w->map+9);
    if(mnm_ring_load(w->map+5)!=w->published || ack<w->acknowledged || ack>w->published ||
       w->published-ack>MNM_RENDER_COMMANDS_V2_CAPACITY){mnm_ring_fail(w,MNM_RENDER_COMMANDS_V2_REASON_INVALID);return 0;}
    w->acknowledged=ack;return 1;
}
static inline int mnm_ring_write(struct mnm_ring_writer* w,const void* input,uint32_t length){
    if(!mnm_ring_writer_valid(w))return MNM_RING_REJECTED;
    if(!input || !length || length>MNM_RENDER_COMMANDS_V2_CAPACITY){mnm_ring_fail(w,MNM_RENDER_COMMANDS_V2_REASON_INVALID);return MNM_RING_REJECTED;}
    if(length>UINT32_MAX-w->published){mnm_ring_fail(w,MNM_RENDER_COMMANDS_V2_REASON_OVERFLOW);return MNM_RING_REJECTED;}
    if(length>MNM_RENDER_COMMANDS_V2_CAPACITY-(w->published-w->acknowledged))return MNM_RING_FULL;
    unsigned char* data=(unsigned char*)w->map+64;uint32_t offset=w->published%MNM_RENDER_COMMANDS_V2_CAPACITY;
    uint32_t first=MNM_RENDER_COMMANDS_V2_CAPACITY-offset;if(first>length)first=length;
    mnm_ring_copy(data+offset,(const unsigned char*)input,first);
    mnm_ring_copy(data,(const unsigned char*)input+first,length-first);
    /* A cancellation/identity change during the private copy cannot publish. */
    if(!mnm_ring_writer_valid(w))return MNM_RING_REJECTED;
    w->published+=length;mnm_ring_store(w->map+5,w->published);return MNM_RING_WRITTEN;
}
static inline int mnm_ring_end(struct mnm_ring_writer* w){
    if(!mnm_ring_writer_valid(w))return 0;
    mnm_ring_store(w->map+6,2);return 1;
}
static inline int mnm_ring_reader_bind(struct mnm_ring_reader* r,void* mapping,uint32_t size){
    uint32_t* p=(uint32_t*)mapping;r->map=0;r->session=r->consumed=r->published=r->state=0;r->failed=1;
    if(!p || ((uintptr_t)p&3) || size!=MNM_RENDER_COMMANDS_V2_SIZE || !mnm_ring_identity(p,p[4]) || mnm_ring_load(p+9))return 0;
    r->map=p;r->session=p[4];r->consumed=r->published=r->state=r->failed=0;return 1;
}
static inline void mnm_ring_cancel(struct mnm_ring_reader* r){
    if(mnm_ring_identity(r->map,r->session))mnm_ring_store(r->map+8,1);
}
static inline int mnm_ring_reader_reject(struct mnm_ring_reader* r){r->failed=1;mnm_ring_cancel(r);return -1;}
/* Returns copied length,0 when temporarily empty, or -1 on refusal.
 * Caller allocates at least budget bytes. Never expose a borrowed ring pointer. */
static inline int mnm_ring_read(struct mnm_ring_reader* r,void* output,uint32_t budget){
    if(r->failed)return -1;
    if(!output || !budget || budget>MNM_RENDER_COMMANDS_V2_POLL_BYTES || !mnm_ring_identity(r->map,r->session))return mnm_ring_reader_reject(r);
    uint32_t state=mnm_ring_load(r->map+6),published=mnm_ring_load(r->map+5);
    if(state>3 || state<r->state || (r->state>=2 && state!=r->state) || state==3 || mnm_ring_load(r->map+8) ||
       published<r->published || published<r->consumed || published-r->consumed>MNM_RENDER_COMMANDS_V2_CAPACITY ||
       mnm_ring_load(r->map+9)!=r->consumed || (state==0 && published) ||
       (r->state>=2 && published!=r->published) || (state==2 && mnm_ring_load(r->map+7)))return mnm_ring_reader_reject(r);
    r->state=state;r->published=published;uint32_t count=published-r->consumed;if(count>budget)count=budget;
    if(!count)return 0;
    const unsigned char* data=(const unsigned char*)r->map+64;uint32_t offset=r->consumed%MNM_RENDER_COMMANDS_V2_CAPACITY;
    uint32_t first=MNM_RENDER_COMMANDS_V2_CAPACITY-offset;if(first>count)first=count;
    mnm_ring_copy((unsigned char*)output,data+offset,first);mnm_ring_copy((unsigned char*)output+first,data,count-first);
    r->consumed+=count;mnm_ring_store(r->map+9,r->consumed);return (int)count;
}
#endif
