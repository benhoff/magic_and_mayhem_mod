#ifndef MNM_NATIVE_COPY_CALLERS_V2_H
#define MNM_NATIVE_COPY_CALLERS_V2_H
#include "native_copy_callers_v1.h"
#define MNM_COPY_CALLERS_V2_BYTES (128u+80u*MNM_COPY_CALLERS_CAPACITY)
/* V1 fields0..15 unchanged. Append source wrapper,frame mode(0/1/2),verified
 * detour length and original-slot marker. Mode2 is the versioned scene handoff. */
struct MnmCopyCallersV2 {uint32_t header[32];uint32_t rows[MNM_COPY_CALLERS_CAPACITY][20];};
#if defined(__cplusplus)
static_assert(sizeof(MnmCopyCallersV2)==MNM_COPY_CALLERS_V2_BYTES,"COPY caller V2 ABI");
#else
_Static_assert(sizeof(struct MnmCopyCallersV2)==MNM_COPY_CALLERS_V2_BYTES,"COPY caller V2 ABI");
#endif
static inline void mnm_copy_callers_v2_record(struct MnmCopyCallersV2* c,const uint32_t* row){
 uint32_t* h=c->header;
 if(h[8]==UINT32_MAX){h[7]=1;return;}++h[8];
 if(h[6]>=MNM_COPY_CALLERS_CAPACITY){h[7]=1;return;}
 if(row[0]!=h[6]+1 || row[9]<1 || row[9]>6){h[9]|=1;return;}
 uint32_t* out=c->rows[h[6]];for(uint32_t i=0;i<20;++i)out[i]=row[i];
 ++h[9+row[9]];++h[6];
}
#endif
