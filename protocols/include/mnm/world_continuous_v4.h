#ifndef MNM_WORLD_CONTINUOUS_V4_H
#define MNM_WORLD_CONTINUOUS_V4_H
#include "world_continuous_v3.h"
/* V4 keeps V3 slot capacities but has an explicit immutable resident mode.
 * Its reply is an acknowledgement, never a framebuffer. All fields are LE. */
#define MNM_CONT_RESIDENT_MAGIC "MNMCON04"
#define MNM_CONT_RESIDENT_VERSION 4u
#define MNM_CONT_RESIDENT_MODE 1u
#define MNM_CONT_ACK_MAGIC "MNMGACK1"
#define MNM_CONT_ACK_BYTES 80u
static inline unsigned mnm_ack_word(const unsigned char *p, unsigned n) {
  p += n * 4;
  return (unsigned)p[0] | (unsigned)p[1] << 8 |
         (unsigned)p[2] << 16 | (unsigned)p[3] << 24;
}
static inline void mnm_ack_put(unsigned char *p, unsigned n, unsigned v) {
  unsigned i;
  for(i=0;i<4;++i)p[n*4+i]=(unsigned char)(v>>(i*8));
}
static inline void mnm_ack_encode(unsigned char *p, const unsigned char *request,
                                 unsigned gl, unsigned gh, unsigned nl, unsigned nh) {
  unsigned i;
  for(i=0;i<8;++i)p[i]=(unsigned char)MNM_CONT_ACK_MAGIC[i];
  mnm_ack_put(p,2,1);mnm_ack_put(p,3,MNM_CONT_ACK_BYTES);
  for(i=4;i<=10;++i)mnm_ack_put(p,i,mnm_ack_word(request,i));
  mnm_ack_put(p,11,mnm_ack_word(request,12));
  mnm_ack_put(p,12,mnm_ack_word(request,14));
  mnm_ack_put(p,13,mnm_ack_word(request,15));
  mnm_ack_put(p,14,gl);mnm_ack_put(p,15,gh);
  mnm_ack_put(p,16,nl);mnm_ack_put(p,17,nh);
  mnm_ack_put(p,18,1);mnm_ack_put(p,19,0);
}
static inline int mnm_ack_valid(const unsigned char *p, unsigned size,
                               const unsigned char *request,
                               unsigned gl, unsigned gh, unsigned nl, unsigned nh) {
  unsigned char expected[MNM_CONT_ACK_BYTES];unsigned i;
  if(size!=MNM_CONT_ACK_BYTES || !(gl|gh) || !(nl|nh))return 0;
  mnm_ack_encode(expected,request,gl,gh,nl,nh);
  for(i=0;i<MNM_CONT_ACK_BYTES;++i)if(p[i]!=expected[i])return 0;
  return 1;
}
#endif
