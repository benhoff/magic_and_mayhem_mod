#ifndef MNM_WORLD_SOURCE_EPOCH_H
#define MNM_WORLD_SOURCE_EPOCH_H
#include "world_source_regions.h"
/* Shared by the actual producer and its deterministic admission fixture.
 * The guard owner supplies eligibility; this does not grant byte ownership. */
static void mnm_world_source_epoch_begin(struct MnmWorldSourceRegions* cache,unsigned* owner,unsigned eligible,unsigned thread){
 mnm_world_source_regions_reset(cache);
 __atomic_store_n(owner,eligible?thread:0,__ATOMIC_RELEASE);
}
static void mnm_world_source_epoch_end(struct MnmWorldSourceRegions* cache,unsigned* owner){
 __atomic_store_n(owner,0,__ATOMIC_RELEASE);mnm_world_source_regions_reset(cache);
}
static unsigned mnm_world_source_epoch_query(struct MnmWorldSourceRegions* cache,const unsigned* owner,unsigned thread,unsigned p,unsigned* output){
 if(thread && __atomic_load_n(owner,__ATOMIC_ACQUIRE)==thread)return mnm_world_source_regions_query(cache,p,output);
 return MNM_WORLD_REGION_QUERY(p,output);
}
#endif
