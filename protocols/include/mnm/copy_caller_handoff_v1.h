#ifndef MNM_COPY_CALLER_HANDOFF_V1_H
#define MNM_COPY_CALLER_HANDOFF_V1_H
#include <stdint.h>
#define MNM_COPY_CALLER_HANDOFF_WORDS 32u
#define MNM_COPY_CALLER_HANDOFF_MAGIC 0x3148434du
/* Guest PE32 scalar call contract. Fields3..8: entry,length,trampoline,detour,
 * return marker,RET pop. Fields9..14: matched,caller,destination,source,
 * after-return SP,thread. Bytes64..75 hold the saved prefix; rest reserved. */
static inline int mnm_copy_caller_handoff_valid(const uint32_t* h,uint32_t entry,uint32_t length,uint32_t pop){
 if(h[0]!=MNM_COPY_CALLER_HANDOFF_MAGIC || h[1]!=1 || h[2]!=128 || h[3]!=entry ||
    h[4]!=length || !length || length>10 || !h[5] || !h[6] || !h[7] || h[8]!=pop || h[9]>1 || h[15])return 0;
 for(unsigned i=19;i<32;++i)if(h[i])return 0;
 if(!h[9]){for(unsigned i=10;i<=14;++i)if(h[i])return 0;}
 else{for(unsigned i=10;i<=14;++i)if(!h[i])return 0;}
 const uint8_t* bytes=(const uint8_t*)h;
 for(unsigned i=length;i<12;++i)if(bytes[64+i])return 0;
 return 1;
}
#endif
