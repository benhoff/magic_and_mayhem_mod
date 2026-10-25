#include "terrain_lighting_cycle.hpp"
#include <cstdio>
#include <cstdlib>
#include <new>
#include <stdexcept>
using namespace mnm::reconstruction;
namespace {
void* owned[8192]{};unsigned outstanding=0,failAt=0,attempt=0;
void remember(void* p){for(auto& slot:owned)if(!slot){slot=p;++outstanding;return;}std::abort();}
void forget(void* p){if(!p)return;for(auto& slot:owned)if(slot==p){slot=nullptr;--outstanding;std::free(p);return;}std::abort();}
void require(bool value){if(!value)std::abort();}
std::uint64_t fingerprint(const TerrainLightingCycle& cycle){
 std::uint64_t h=1469598103934665603ULL;
 const auto add=[&](std::uint64_t n){for(unsigned i=0;i<8;++i){h^=(n>>(i*8))&255;h*=1099511628211ULL;}};
 for(const auto& buffer:cycle.field().buffers())for(auto v:buffer)add(static_cast<unsigned char>(v));
 const auto c=cycle.creatures();const auto o=cycle.objects();
 for(auto v:{c.phase,c.remaining,c.quota,c.cursor,unsigned(c.published),o.phase,o.remaining,o.quota,o.cursor,unsigned(o.active),unsigned(o.requested),cycle.ticks()})add(v);
 return h;
}
}
void* operator new(std::size_t n){if(failAt && ++attempt==failAt)throw std::bad_alloc();auto* p=std::malloc(n?n:1);if(!p)throw std::bad_alloc();remember(p);return p;}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete(void* p) noexcept{forget(p);}
void operator delete[](void* p) noexcept{forget(p);}
void operator delete(void* p,std::size_t) noexcept{forget(p);}
void operator delete[](void* p,std::size_t) noexcept{forget(p);}
int main(){
 unsigned constructionFailures=0,restoreFailures=0,stepFailures=0,invalidCases=0;
 const auto initial=outstanding;
 for(unsigned layers:{1u,4u,7u}){
  for(unsigned n=1;n<128;++n){
   const auto baseline=outstanding;bool succeeded=false;failAt=n;attempt=0;
   try{TerrainLightingCycle cycle(32,32,layers);succeeded=true;}catch(const std::bad_alloc&){++constructionFailures;}
   failAt=0;require(outstanding==baseline);if(succeeded)break;require(n<127);
  }
  TerrainLightingCycle cycle(32,32,layers);auto buffers=cycle.field().buffers();
  for(unsigned i=0;i<5;++i)for(unsigned j=0;j<buffers[i].size();++j)buffers[i][j]=std::int8_t((j*7+i*13)%128-64);
  TerrainCreatureLightState c{3,17,5,7,true};TerrainStaticLightState o{1,19,3,11,false,true};
  for(unsigned n=1;n<128;++n){
   const auto baseline=outstanding;const auto before=fingerprint(cycle);bool succeeded=false;failAt=n;attempt=0;
   try{cycle.restore(buffers,c,o);succeeded=true;}catch(const std::bad_alloc&){++restoreFailures;}
   failAt=0;require(outstanding==baseline);if(succeeded)break;require(fingerprint(cycle)==before);require(n<127);
  }
  require(cycle.field().buffers()==buffers && cycle.creatures().cursor==7 && cycle.objects().cursor==11 && cycle.ticks()==0);
  auto before=fingerprint(cycle);
  for(unsigned i=0;i<5;++i){auto bad=buffers;bad[i].pop_back();try{cycle.restore(bad,c,o);std::abort();}catch(const std::invalid_argument&){++invalidCases;}require(fingerprint(cycle)==before);}
  for(unsigned i=0;i<4;++i){auto badC=c;auto badO=o;if(i==0)badC.phase=8;if(i==1)badC.cursor=65537;if(i==2)badO.phase=2;if(i==3)badO.cursor=65537;try{cycle.restore(buffers,badC,badO);std::abort();}catch(const std::invalid_argument&){++invalidCases;}require(fingerprint(cycle)==before);}
  TerrainLightingSnapshot snapshot;snapshot.changed=1;
  for(unsigned n=1;n<128;++n){
   const auto baseline=outstanding;before=fingerprint(cycle);bool succeeded=false;failAt=n;attempt=0;
   try{cycle.step(snapshot);succeeded=true;}catch(const std::bad_alloc&){++stepFailures;}
   failAt=0;require(outstanding==baseline);if(succeeded)break;require(fingerprint(cycle)==before);require(n<127);
  }
  require(cycle.ticks()==1);
 }
 require(outstanding==initial);
 std::printf("{\"success\":true,\"construction_failures\":%u,\"restore_failures\":%u,\"step_failures\":%u,\"invalid_restores\":%u,\"outstanding_delta\":0}\n",constructionFailures,restoreFailures,stepFailures,invalidCases);
}
