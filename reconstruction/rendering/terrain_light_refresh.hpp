#pragma once
#include <array>
#include <cstdint>
#include <initializer_list>
#include <stdexcept>
namespace mnm::reconstruction {
// Build-specific caller contracts. Memory owns guest bytes and supplies the
// named external services; this does not reconstruct those service internals.
template<class Memory> class TerrainLightRefreshCallers {
 using U=std::uint32_t;
 Memory& m;
 U call(U address,U receiver,std::initializer_list<U> args={}){return m.call(address,receiver,args);}
public:
 explicit TerrainLightRefreshCallers(Memory& memory):m(memory){}
 // Whole 0048f690, restricted to an existing, valid linked creature (+44).
 // New-owner selection/script creation and invalid original handles are outside
 // this domain. Service failures keep their original early-return boundaries.
 U createFromLinkedCreature(U p,U kind,U inherit) {
  const U slot=m.get32(p+0x44);
  if(slot>=m.get32(0x6def5c))throw std::invalid_argument("Refresh creation needs a valid linked creature");
  const U source=m.get32(0x6def58)+slot*0xe4b;
  const U definition=0x6a5f88+kind*0x5c9;
  if(call(0x4f13f0,0x6c5490,{m.get32(p+8),m.get32(p+12),m.get32(p+16),m.get32(definition+4),m.get32(definition),0xfffe}))return 0;
  const U owner=call(0x466870,0x643a80,{m.get32(source+0x174)});
  if(owner && std::int32_t(m.get32(owner+0x8f5))>=std::int32_t(m.get32(owner+0x8f1)))return 0;
  const U created=call(0x52a640,0x6def58);
  m.set32(p+0x198,created);
  if(!created)return 0;
  call(0x5070e0,created,{m.get32(p+8),m.get32(p+12),m.get32(p+16),kind});
  call(0x506290,created,{kind,m.get32(source+0x174),inherit?1u:0u});
  call(0x5183e0,created,{inherit?1u:0u});
  if(m.get32(created+0x174)==m.get32(0x644520))m.requestRefresh();
  m.set32(p+0x48,m.get32(created));
  return 1;
 }
 // Whole 0051e890 only for event9 (position transition) with +718=0,
 // and default events. Events5/8/14 and the +718 continuation remain open.
 U positionEvent(U p,U event) {
  if(event==5 || event==8 || event==14 || m.get32(p+0x718))
   throw std::invalid_argument("Unsupported refresh position event continuation");
  std::array<U,3> position{m.convertColumn(m.get32(p+0x70c)),m.convertRow(m.get32(p+0x710)),m.get32(p+0x714)};
  if(event!=9)return 0;
  U saved=m.get32(p+0x104);
  const U resolved=m.resolvePosition(p,position);
  if(call(0x4f5230,0x6c5490,{m.get32(p),resolved})) {
   call(0x5093e0,p,{m.get32(p+8),m.get32(p+12),m.get32(p+16)});
   if(m.get32(p+0x618)) {
    U at=p+0x97f+m.get32(p+0x977)*28;
    call(0x5093e0,p,{m.get32(at),m.get32(at+4),m.get32(at+8)});
   }
   call(0x5070e0,p,{position[0],position[1],position[2],m.get32(p+0xa8)});
   call(0x509230,p,{position[0],position[1],position[2]});
   call(0x51a8f0,p);call(0x50f850,p);call(0x51aa30,p);
   if(!m.get32(p+4))return 0;
   m.set32(p+0x618,0);call(0x51a880,p);call(0x5178c0,p);
   U state=m.get32(p+0xa0);
   if(state) {
    if(state==2)call(0x54dce0,p+0xbf3,{1});
    else if(state==1)call(0x54dce0,p+0xbf3,{4});
    m.set32(p+0xa0,0);m.set32(p+0x9c,0);
   }
   m.set32(p+0x108,0);
   if(m.get32(p+0x798)){call(0x520e70,p);saved=0;}
   call(0x50ab30,p);m.requestRefresh();
   m.set32(p+0xde3,0);m.set32(p+0x963,0);
   m.set8(p+0x962,0);m.set8(p+0x723,0);
   for(U i=0;i<3;++i)m.set32(p+0x96b+4*i,m.get32(p+8+4*i));
  }
  m.set32(p+0x104,saved);
  call(0x54dad0,p+0xbf3,{10});
  return 0;
 }
 // Selected unmodified 0046afc0 region [0046b836,0046b91a), with
 // EBX=0 and ESI=parent. A zero timer result exits at0046bca3. This is
 // local ordering/gates only, not a claim about the entire world loop.
 void schedulerRegion(U p) {
  if(m.get32(p+0xfd5a)){m.schedulerExit(false);return;}
  call(0x534380,0x6a49c0);call(0x4f2e20,0x6c5490);call(0x4f3900,0x6c5490);
  call(0x544d10,0x6b1248);call(0x545910,p+0x12);call(0x4e0f00,p+0x1c1a);call(0x46f5d0,p);
  if(!m.get32(0x6deed0) && !m.get32(0x6f2aa8))call(0x571300,0x658178);
  if(!call(0x536f00,0x6a2928,{m.get32(p+0xfd62),1})){m.schedulerExit(true);return;}
  if(m.get32(0x6c4830)>m.get32(p+0xfdea) && call(0x4f7540,0x6c5490))
   m.set32(p+0xfdea,m.get32(0x657cdc)+m.get32(0x6c4830));
  if(m.get32(0x5e1404))m.updateLighting();
  m.schedulerExit(false);
 }
};
}
