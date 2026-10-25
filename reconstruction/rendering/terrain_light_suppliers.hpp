#pragma once
#include "terrain_creature_lighting.hpp"
#include <stdexcept>
namespace mnm::reconstruction {
// 00465440 initializes relation18 to one, share20 to zero. Keep the consumer's
// zero-initialized fixture defaults distinct from original owner construction.
inline TerrainLightRelations initialTerrainLightRelations() {
 TerrainLightRelations r;
 for(auto& row:r.first)row.fill(1);
 return r;
}
// Whole 00465500 / 004655e0 for admitted owners 0..7. Preserve raw bytes.
// Setters are symmetric and ignore self; loaded/observed tables can be directed.
inline void setTerrainLightRelation(TerrainLightRelations& r,unsigned a,unsigned b,
                                   std::uint8_t value,bool sharing) {
 if(a>=8 || b>=8)throw std::invalid_argument("Terrain light relation owner outside 0..7");
 if(a==b)return;
 auto& table=sharing?r.second:r.first;
 table[a][b]=table[b][a]=value;
}
// Build-specific resource contract, separate from native TerrainLightField RAII.
// Memory supplies owned byte storage and explicit allocation/free/kernel services;
// addresses are guest addresses, never host pointers. Allocation failure and
// malformed/overflowing dimensions are outside this admitted original contract.
template<class Memory> class TerrainLightResourceContracts {
 using U=std::uint32_t;
 Memory& m;
public:
 explicit TerrainLightResourceContracts(Memory& memory):m(memory){}
 // Whole 004ecdd0 except the separately supplied 004f1890 kernel service.
 // Existing published storage gates all five allocations; repeated init retains
 // its original byte count even if map dimensions changed.
 void initialize(U p) {
  if(!m.get32(p+0x7cc)) {
   U count=((m.get32(p+12)+1)/2)*m.get32(p+16);
   m.set32(p+0x7e0,count);
   for(U offset:{0x7ccu,0x7d0u,0x7d4u,0x7d8u,0x7dcu})
    m.set32(p+offset,m.allocate(count));
  }
  U count=m.get32(p+0x7e0);
  const auto fill=m.get8(0x5e18f0);
  for(U offset:{0x7ccu,0x7d0u,0x7d4u,0x7d8u,0x7dcu})
   for(U i=0;i<count;++i)m.set8(m.get32(p+offset)+i,fill);
  m.rebuildKernels(p);
  m.set32(p+0x6426,1);m.set32(p+0x642a,1);m.set32(p+0x642e,0);
 }
 // Whole 004ecbe0. The ancillary +4c allocation is independently freed. A null
 // published handle skips sibling frees AND count clearing, as in the binary.
 void clear(U p) {
  for(U i=0;i<0x4c;i+=4)m.set32(p+i,0);
  for(U i=0x1b0;i<0x390;i+=4)m.set32(p+i,~U(0));
  if(U handle=m.get32(p+0x4c)){m.release(handle);m.set32(p+0x4c,0);}
  if(!m.get32(p+0x7cc))return;
  for(U offset:{0x7ccu,0x7d4u,0x7dcu,0x7d0u,0x7d8u})m.release(m.get32(p+offset));
  for(U offset:{0x7ccu,0x7d4u,0x7dcu,0x7d0u,0x7d8u,0x7e0u})m.set32(p+offset,0);
 }
 // Whole 004f2dd0; relation setters themselves do not request a refresh.
 void requestRefresh(){m.set32(0x5e41a8,1);}
};
}
