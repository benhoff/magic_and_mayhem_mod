#ifndef MNM_NATIVE_CPU_READER_JOURNAL_V1_H
#define MNM_NATIVE_CPU_READER_JOURNAL_V1_H
#include <stdint.h>
#define MNM_CRJ_CAPACITY 65536u
#define MNM_CRJ_BYTES (128u+MNM_CRJ_CAPACITY*128u)
#define MNM_CRJ_BEGIN 1u
#define MNM_CRJ_REQUEST 2u
#define MNM_CRJ_COMPLETE 3u
#define MNM_CRJ_DC_RETURN 4u
#define MNM_CRJ_DC_RELEASE 5u
#define MNM_CRJ_END 6u
#define MNM_CRJ_DC_TRACKED 7u
#define MNM_CRJ_GET_DC 1u
#define MNM_CRJ_PREFERENCES 2u
#define MNM_CRJ_SAVE 3u
#define MNM_CRJ_LOAD 4u
#define MNM_CRJ_EXPLICIT 5u
#define MNM_CRJ_RELEASE_DC 6u
#define MNM_CRJ_OTHER 7u
#define MNM_CRJ_VALIDATED 256u
#define MNM_CRJ_CHECKPOINT 512u
#define MNM_CRJ_TRACKED 1024u
struct MnmCpuReaderJournal {uint32_t header[32];uint32_t rows[MNM_CRJ_CAPACITY][32];};
static inline int mnm_crj_gate(uint32_t enabled,uint32_t diagnostic,uint32_t pilot){return enabled==1 && diagnostic==1 && pilot==1;}
/* Exactly one reservation attempt. Contention/overflow refuses evidence, never
 * waits, overwrites a row, changes a game result or extends a deadline. */
static inline int mnm_crj_record(struct MnmCpuReaderJournal* j,const uint32_t* row){
 if(!j)return 0;
 if(__atomic_load_n(j->header+10,__ATOMIC_ACQUIRE)){__atomic_fetch_or(j->header+9,4,__ATOMIC_RELAXED);return 0;}
 uint32_t at=__atomic_load_n(j->header+6,__ATOMIC_RELAXED);
 if(at>=MNM_CRJ_CAPACITY){__atomic_fetch_add(j->header+8,1,__ATOMIC_RELAXED);__atomic_fetch_or(j->header+9,1,__ATOMIC_RELAXED);return 0;}
 if(!__atomic_compare_exchange_n(j->header+6,&at,at+1,0,__ATOMIC_ACQ_REL,__ATOMIC_RELAXED)){
  __atomic_fetch_add(j->header+8,1,__ATOMIC_RELAXED);__atomic_fetch_or(j->header+9,2,__ATOMIC_RELAXED);return 0;
 }
 for(unsigned n=1;n<32;++n)j->rows[at][n]=row[n];
 __atomic_store_n(j->rows[at],at+1,__ATOMIC_RELEASE);
 __atomic_fetch_add(j->header+7,1,__ATOMIC_RELEASE);return 1;
}
#endif
