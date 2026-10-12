#ifndef MNM_WORLD_CONTINUOUS_V5_H
#define MNM_WORLD_CONTINUOUS_V5_H
#include "world_continuous_v4.h"
/* V5 explicitly accepts owned and validated snapshots before GPU execution.
 * Original actual return and native/GUI consumption are independent milestones.
 * V3 pixel and V4 GPU-submission peers are not reinterpreted. */
#define MNM_CONT_SNAPSHOT_MAGIC "MNMCON05"
#define MNM_CONT_SNAPSHOT_VERSION 5u
#define MNM_CONT_SNAPSHOT_MODE 2u
#define MNM_CONT_SNAPSHOT_ACK_MAGIC "MNMSACK1"
static inline void mnm_snapshot_ack_encode(unsigned char *p,
 const unsigned char *request,unsigned gl,unsigned gh,unsigned nl,unsigned nh) {
 unsigned i;mnm_ack_encode(p,request,gl,gh,nl,nh);
 for(i=0;i<8;++i)p[i]=(unsigned char)MNM_CONT_SNAPSHOT_ACK_MAGIC[i];
 mnm_ack_put(p,18,2);
}
static inline int mnm_snapshot_ack_valid(const unsigned char *p,unsigned size,
 const unsigned char *request,unsigned gl,unsigned gh,unsigned nl,unsigned nh) {
 unsigned char expected[MNM_CONT_ACK_BYTES];unsigned i;
 if(size!=MNM_CONT_ACK_BYTES || !(gl|gh) || !(nl|nh))return 0;
 mnm_snapshot_ack_encode(expected,request,gl,gh,nl,nh);
 for(i=0;i<MNM_CONT_ACK_BYTES;++i)if(p[i]!=expected[i])return 0;
 return 1;
}
#endif
