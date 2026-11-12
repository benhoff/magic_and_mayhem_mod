#ifndef MNM_NATIVE_COPY_CALLERS_V1_H
#define MNM_NATIVE_COPY_CALLERS_V1_H
#include <stdint.h>
#define MNM_COPY_CALLERS_CAPACITY 65536u
#define MNM_COPY_CALLERS_BYTES (128u+64u*MNM_COPY_CALLERS_CAPACITY)
struct MnmCopyCallers {uint32_t header[32];uint32_t rows[MNM_COPY_CALLERS_CAPACITY][16];};
#if defined(__cplusplus)
static_assert(sizeof(MnmCopyCallers)==MNM_COPY_CALLERS_BYTES,"COPY caller ABI");
#else
_Static_assert(sizeof(struct MnmCopyCallers)==MNM_COPY_CALLERS_BYTES,"COPY caller ABI");
#endif
/* Same serialized writer/ordinal as the companion V2 routes. Refuse lost rows. */
static inline void mnm_copy_callers_record(struct MnmCopyCallers* c,const uint32_t* row){
 uint32_t* h=c->header;
 if(h[8]==UINT32_MAX){h[7]=1;return;}
 ++h[8];
 if(h[6]>=MNM_COPY_CALLERS_CAPACITY){h[7]=1;return;}
 if(row[0]!=h[6]+1 || row[9]<1 || row[9]>6){h[9]|=1;return;}
 uint32_t* out=c->rows[h[6]];
 for(uint32_t i=0;i<16;++i)out[i]=row[i];
 ++h[9+row[9]];++h[6];
}
#endif
