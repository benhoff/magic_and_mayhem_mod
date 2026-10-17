#ifndef MNM_NATIVE_COPY_ROUTES_V1_H
#define MNM_NATIVE_COPY_ROUTES_V1_H
#include <stdint.h>
/* Observation only. Object words are opaque tokens, never host pointers. */
#define MNM_COPY_ROUTES_CAPACITY 65536u
#define MNM_COPY_ROUTES_WORDS 40u
#define MNM_COPY_ROUTES_BYTES (128u+160u*MNM_COPY_ROUTES_CAPACITY)
struct MnmCopyRoutes {uint32_t header[32];uint32_t rows[MNM_COPY_ROUTES_CAPACITY][40];};
#if defined(__cplusplus)
static_assert(sizeof(MnmCopyRoutes)==MNM_COPY_ROUTES_BYTES,"copy route census ABI");
#else
_Static_assert(sizeof(struct MnmCopyRoutes)==MNM_COPY_ROUTES_BYTES,"copy route census ABI");
#endif
/* Terminal callbacks no longer enter the observation/ownership domain. Count
 * that explicit exclusion; a skipped active callback still invalidates proof. */
static void mnm_copy_routes_skipped(struct MnmCopyRoutes* c,uint32_t reason,uint32_t retired){
 if(retired){
  uint32_t at=__atomic_load_n(c->header+21,__ATOMIC_RELAXED);
  for(;;){
   if(at==UINT32_MAX)break;
   if(__atomic_compare_exchange_n(c->header+21,&at,at+1,0,__ATOMIC_RELAXED,__ATOMIC_RELAXED))return;
  }
 }
 __atomic_store_n(c->header+7,1,__ATOMIC_RELAXED);
 __atomic_store_n(c->header+9,1,__ATOMIC_RELAXED);
 __atomic_fetch_or(c->header+22,reason,__ATOMIC_RELAXED);
}
/* The existing ordered copy lease serializes writers. Exhaustion invalidates
 * evidence, without altering drawing admission or allowing counter wrap. */
static void mnm_copy_routes_record(struct MnmCopyRoutes* c,const uint32_t* row){
 uint32_t* h=c->header;
 if(h[8]==UINT32_MAX){h[7]=1;return;}
 ++h[8];
 if(h[6]>=MNM_COPY_ROUTES_CAPACITY){h[7]=1;return;}
 uint32_t* out=c->rows[h[6]];
 for(uint32_t i=0;i<40;++i)out[i]=row[i];
 if(row[15]==2)++h[10];else if(row[15]==1)++h[11];else if(row[15]==3)++h[13];else ++h[12];
 if(row[10]&128)++h[17];if(row[10]&32)++h[18];
 if(row[11]?(row[12]&1):(row[12]&0x8000))++h[19];
 if(row[10]&4)++h[20];
 out[0]=h[8];++h[6];
}
#endif
