#ifndef MNM_WORLD_DEFERRED_H
#define MNM_WORLD_DEFERRED_H
#include "../../protocols/include/mnm/canvas_producers_v1.h"
#define MNM_WORLD_DEFERRED_BYTES (4u*1024u*1024u)
#define MNM_WORLD_DEFERRED_RECORDS 512u
/* Native capture policy: these secondary operations are independent of World
 * traversal. Admission never permits a World source/destination or global bind. */
static int mnm_world_defer_admitted(const unsigned* r,unsigned n,unsigned world,
                                   unsigned count,unsigned used){
 unsigned op=r[2];
 if((op!=MNM_PRODUCER_RELEASE && op!=MNM_PRODUCER_CREATE && op!=MNM_PRODUCER_DIB && op!=MNM_PRODUCER_COPY && op!=MNM_PRODUCER_CHECKPOINT && op!=MNM_PRODUCER_FILL && op!=MNM_PRODUCER_JPEG && op!=MNM_PRODUCER_BMP && op!=MNM_PRODUCER_FADE && op!=MNM_PRODUCER_BEVEL && op!=MNM_PRODUCER_HALO && op!=MNM_PRODUCER_WASH && op!=MNM_PRODUCER_PCX) || !r[3] ||
    (world && (r[3]==world || (op==MNM_PRODUCER_COPY && r[4]==world))) ||
    count>=MNM_WORLD_DEFERRED_RECORDS || n>MNM_WORLD_DEFERRED_BYTES-96 || used>MNM_WORLD_DEFERRED_BYTES-96-n)return 0;
 return 1;
}
static int mnm_world_defer_append(unsigned char* storage,unsigned* used,unsigned* count,
                                 const unsigned* r,const void* payload,unsigned n,unsigned world){
 if(!storage || (n&&!payload) || !mnm_world_defer_admitted(r,n,world,*count,*used))return 0;
 unsigned char* out=storage+*used;const unsigned char* header=(const unsigned char*)r;
 for(unsigned i=0;i<96;++i)out[i]=header[i];
 for(unsigned i=0;i<4;++i){out[i]=(96+n)>>(i*8);out[72+i]=n>>(i*8);}
 for(unsigned i=0;i<n;++i)out[96+i]=((const unsigned char*)payload)[i];
 *used+=96+n;++*count;return 1;
}
#endif
