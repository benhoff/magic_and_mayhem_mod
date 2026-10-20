#include <array>
#include <cassert>
#include <iostream>
#include <vector>
struct Region {unsigned base,size,state,protection;};
static std::vector<Region> regions;
static unsigned calls;
static unsigned query(unsigned p,unsigned* m){
 ++calls;for(auto r:regions)if(p>=r.base && p-r.base<r.size){
  std::array<unsigned,7> a{r.base,r.base,0,r.size,r.state,r.protection,0};for(unsigned i=0;i<7;++i)m[i]=a[i];return 28;}
 return 0;
}
#define MNM_WORLD_REGION_QUERY(p,m) query(p,m)
#include "../runtime/scene/world_source_epoch.h"
#include "../runtime/scene/world_source_copy.h"
int main(){
 MnmWorldSourceRegions cache{};unsigned m[7];regions={{4096,4096,0x1000,4},{8192,4096,0x1000,2},{12288,4096,0x1000,1}};
 mnm_world_source_regions_reset(&cache);
 for(unsigned i=0;i<10000;++i){unsigned p=4096+(i%8192);assert(mnm_world_source_regions_query(&cache,p,m)==28);assert(m[0]==(p<8192?4096u:8192u));}
 assert(calls==2 && cache.count==2);
 assert(!mnm_world_source_regions_query(&cache,4095,m));assert(mnm_world_source_regions_query(&cache,12288,m)==28 && m[5]==1);
 assert(mnm_world_source_regions_query(&cache,12289,m)==28 && cache.count==2);assert(calls==5);
 // Permission changes between guarded epochs must not reuse the prior proof.
 regions[0].protection=0x104;mnm_world_source_regions_reset(&cache);assert(mnm_world_source_regions_query(&cache,4096,m)==28 && m[5]==0x104 && !cache.count);
 regions[0].protection=4;regions[0].state=0;assert(mnm_world_source_regions_query(&cache,4096,m)==28 && !cache.count);
 regions[0].state=0x1000;assert(mnm_world_source_regions_query(&cache,4096,m)==28 && cache.count==1);
 mnm_world_source_regions_reset(&cache);regions.clear();for(unsigned i=0;i<65;++i)regions.push_back({4096+i*4096,4096,0x1000,4});
 for(unsigned i=0;i<65;++i)assert(mnm_world_source_regions_query(&cache,4096+i*4096,m)==28);
 assert(cache.count==64);const auto before=calls;assert(mnm_world_source_regions_query(&cache,4096,m)==28 && calls==before+1);
 // Invalid overflowing region answers cannot become proofs.
 regions={{~0u-15,32,0x1000,4}};mnm_world_source_regions_reset(&cache);assert(mnm_world_source_regions_query(&cache,~0u-7,m)==28 && !cache.count);
 // This is the same dispatch/begin/end policy called by canvas_producers.c.
 unsigned owner=0;regions={{4096,4096,0x1000,4}};
 mnm_world_source_epoch_begin(&cache,&owner,1,17);auto at=calls;
 assert(mnm_world_source_epoch_query(&cache,&owner,17,4096,m)==28);
 assert(mnm_world_source_epoch_query(&cache,&owner,17,4096,m)==28&&calls==at+1);
 assert(mnm_world_source_epoch_query(&cache,&owner,18,4096,m)==28&&calls==at+2);
 assert(mnm_world_source_epoch_query(&cache,&owner,18,4096,m)==28&&calls==at+3);
 // Permission reuse never caches the content: the consumer reads it per request.
 unsigned char content[7]={1,2,3,4,5,6,7},first[7],second[7];
 mnm_world_source_copy(first,content,7,1);
 content[2]=29;assert(mnm_world_source_epoch_query(&cache,&owner,17,4096,m)==28);
 mnm_world_source_copy(second,content,7,1);
 assert(first[2]==3&&second[2]==29&&calls==at+3);
 for(unsigned i=0;i<7;++i)assert(first[i]==i+1&&second[i]==(i==2?29:i+1));
 mnm_world_source_epoch_end(&cache,&owner);assert(!cache.count&&!owner);
 assert(mnm_world_source_epoch_query(&cache,&owner,17,4096,m)==28&&calls==at+4);
 mnm_world_source_epoch_begin(&cache,&owner,0,17);
 assert(mnm_world_source_epoch_query(&cache,&owner,17,4096,m)==28);
 assert(mnm_world_source_epoch_query(&cache,&owner,17,4096,m)==28&&!cache.count&&!owner&&calls==at+6);
 regions[0].protection=1;mnm_world_source_epoch_begin(&cache,&owner,1,17);
 assert(mnm_world_source_epoch_query(&cache,&owner,17,4096,m)==28&&m[5]==1&&!cache.count);
 // Unknown protection combinations must never become reusable proofs.
 for(unsigned protection:{6u,0x12u,0x200u}){
  regions[0].protection=protection;mnm_world_source_epoch_begin(&cache,&owner,1,17);const auto queried=calls;
  for(unsigned repeat=0;repeat<2;++repeat)
   assert(mnm_world_source_epoch_query(&cache,&owner,17,4096,m)==28&&m[5]==protection&&!cache.count);
  assert(calls==queried+2);
 }
 for(unsigned protection:{2u,4u,8u,0x20u,0x40u,0x80u}){
  regions[0].protection=protection;mnm_world_source_epoch_begin(&cache,&owner,1,17);const auto queried=calls;
  for(unsigned repeat=0;repeat<2;++repeat)
   assert(mnm_world_source_epoch_query(&cache,&owner,17,4096,m)==28&&m[5]==protection&&cache.count==1);
  assert(calls==queried+1);
 }
 std::cout<<"{\"success\":true,\"repeated_queries\":10000,\"physical_queries\":2,\"region_capacity\":64,\"epoch_permission_revalidated\":true,\"producer_dispatch_cases\":8,\"content_rechecked\":true}\n";
}
