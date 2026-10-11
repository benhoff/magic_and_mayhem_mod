#ifndef MNM_WORLD_SOURCE_REGIONS_H
#define MNM_WORLD_SOURCE_REGIONS_H
/* Intentional native permission policy, separate from readable_extent's fresh
 * synchronous admission. The caller owns a single guarded World epoch and
 * original thread; all bytes/signatures remain copied/checked on every draw.
 * Permission mutations by external writers within that epoch are excluded.
 * Reset before/after every guard. Unknown/guarded/uncommitted regions never
 * become reusable proofs. MNM_WORLD_REGION_QUERY performs the actual query. */
#define MNM_WORLD_SOURCE_REGIONS 64u
struct MnmWorldSourceRegions {unsigned fields[MNM_WORLD_SOURCE_REGIONS][7],count,next;};
static void mnm_world_source_regions_reset(struct MnmWorldSourceRegions* cache){cache->count=cache->next=0;}
static unsigned mnm_world_source_regions_query(struct MnmWorldSourceRegions* cache,unsigned p,unsigned* output){
 for(unsigned i=0;i<cache->count;++i){const unsigned* m=cache->fields[i];
  if(p>=m[0] && p-m[0]<m[3]){for(unsigned k=0;k<7;++k)output[k]=m[k];return 28;}}
 unsigned result=MNM_WORLD_REGION_QUERY(p,output);
 if(result==28 && output[0] && output[0]<=p && output[3] && output[3]<=~0u-output[0] &&
    p-output[0]<output[3] && output[4]==0x1000 && !(output[5]&0x101) && (output[5]&0xee)){
  unsigned index=cache->count<MNM_WORLD_SOURCE_REGIONS?cache->count++:cache->next;
  if(cache->count==MNM_WORLD_SOURCE_REGIONS)cache->next=(index+1)%MNM_WORLD_SOURCE_REGIONS;
  for(unsigned k=0;k<7;++k)cache->fields[index][k]=output[k];
 }
 return result;
}
#endif
