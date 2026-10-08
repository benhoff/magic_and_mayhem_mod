#pragma once
#include "world_frame_v1.h"
/* Little-endian aligned u32 words; no host pointers. Two independent slots.
 * FREE -> WRITING -> READY -> READING -> FREE, release/acquire publication.
 * A full channel drops a frame without waiting for the host. */
#define MNM_WCH_MAGIC "MNMWCH01"
#define MNM_WCH_HEADER 128u
#define MNM_WCH_ORACLE (8u*1024u*1024u)
#define MNM_WCH_SLOT (16u+MNM_WORLD_MAX_BYTES+MNM_WCH_ORACLE)
#define MNM_WCH_SIZE (MNM_WCH_HEADER+2u*MNM_WCH_SLOT)
#define MNM_WCH_VERIFY 1u
#define MNM_WCH_WAITING 0u
#define MNM_WCH_ACTIVE 1u
#define MNM_WCH_ENDED 2u
#define MNM_WCH_FAILED 3u
/* Header: version8,size12,session16,state20,cancel24,published28,
 * consumed32,dropped36,reason40,flags44,presented48,slots52,
 * input-cap56,oracle-cap60,build64,reserved68..127.
 * Slot: state0,sequence4,input-bytes8,oracle-bytes12,inputs16,
 * optional tight RGB565 diagnostic oracle after input-cap. */
static inline unsigned int mnm_wch_load(const unsigned int* p){return __atomic_load_n(p,__ATOMIC_ACQUIRE);}
static inline void mnm_wch_store(unsigned int* p,unsigned int v){__atomic_store_n(p,v,__ATOMIC_RELEASE);}
static inline int mnm_wch_cas(unsigned int* p,unsigned int old,unsigned int value){return __atomic_compare_exchange_n(p,&old,value,0,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE);}
