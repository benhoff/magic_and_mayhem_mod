#ifndef MNM_NATIVE_COPY_ROUTES_V2_H
#define MNM_NATIVE_COPY_ROUTES_V2_H
#include "native_copy_routes_v1.h"
/* V1 fields0..39 retain their meanings. Four appended scalar words bind an
 * emitted COPY interval and its logical source/target IDs, never host pointers.
 * Diagnostic observation only; no admission or rendering input uses these. */
#define MNM_COPY_ROUTES_V2_WORDS 44u
#define MNM_COPY_ROUTES_V2_BYTES (128u+176u*MNM_COPY_ROUTES_CAPACITY)
struct MnmCopyRoutesV2 {uint32_t header[32];uint32_t rows[MNM_COPY_ROUTES_CAPACITY][44];};
#if defined(__cplusplus)
static_assert(sizeof(MnmCopyRoutesV2)==MNM_COPY_ROUTES_V2_BYTES,"copy route v2 ABI");
#else
_Static_assert(sizeof(struct MnmCopyRoutesV2)==MNM_COPY_ROUTES_V2_BYTES,"copy route v2 ABI");
#endif
static void mnm_copy_routes_v2_skipped(struct MnmCopyRoutesV2* c,uint32_t reason,uint32_t retired){
 uint32_t* h=c->header;
 if(retired){
  uint32_t at=__atomic_load_n(h+21,__ATOMIC_RELAXED);
  for(;;){if(at==UINT32_MAX)break;
   if(__atomic_compare_exchange_n(h+21,&at,at+1,0,__ATOMIC_RELAXED,__ATOMIC_RELAXED))return;}
 }
 __atomic_store_n(h+7,1,__ATOMIC_RELAXED);__atomic_store_n(h+9,1,__ATOMIC_RELAXED);
 __atomic_fetch_or(h+22,reason,__ATOMIC_RELAXED);
}
static void mnm_copy_routes_v2_record(struct MnmCopyRoutesV2* c,const uint32_t* row){
 uint32_t* h=c->header;
 if(h[8]==UINT32_MAX){h[7]=1;return;}++h[8];
 if(h[6]>=MNM_COPY_ROUTES_CAPACITY){h[7]=1;return;}
 uint32_t* out=c->rows[h[6]];
 for(uint32_t i=0;i<44;++i)out[i]=row[i];
 if(row[15]==2)++h[10];else if(row[15]==1)++h[11];else if(row[15]==3)++h[13];else ++h[12];
 if(row[10]&128)++h[17];if(row[10]&32)++h[18];
 if(row[11]?(row[12]&1):(row[12]&0x8000))++h[19];if(row[10]&4)++h[20];
 out[0]=h[8];++h[6];
}
#endif
