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
 // Original caller with existing-source and new-owner/script branches.
 // Valid records/definitions and nonzero dimensions are admitted; malformed
 // original pointer/sentinel accesses remain outside owned memory contracts.
 U createCreature(U p,U kind,U inherit) {
  const U slot=m.get32(p+0x44);
  if(slot!=~U(0) && slot>=m.get32(0x6def5c))throw std::invalid_argument("Invalid linked creature");
  const U source=slot==~U(0)?0:m.get32(0x6def58)+slot*0xe4b;
  const U definition=0x6a5f88+kind*0x5c9;
  if(call(0x4f13f0,0x6c5490,{m.get32(p+8),m.get32(p+12),m.get32(p+16),m.get32(definition+4),m.get32(definition),0xfffe}))return 0;
  if(source) {
   const U owner=call(0x466870,0x643a80,{m.get32(source+0x174)});
   if(owner && std::int32_t(m.get32(owner+0x8f5))>=std::int32_t(m.get32(owner+0x8f1)))return 0;
  }
  const U created=call(0x52a640,0x6def58);
  m.set32(p+0x198,created);
  if(!created)return 0;
  call(0x5070e0,created,{m.get32(p+8),m.get32(p+12),m.get32(p+16),kind});
  if(source) {
   call(0x506290,created,{kind,m.get32(source+0x174),inherit?1u:0u});
   call(0x5183e0,created,{inherit?1u:0u});
  }else{
   U selected=~U(0);
   for(U i=0;i<8;++i)if(m.get32(0x643a80+i*0x154) && m.get32(0x643a8c+i*0x154)==~U(0)){selected=i;break;}
   const bool scripted=m.get32(p+0x54)==4;
   if(scripted){
    const U index=m.get32(p+0x58);
    if(std::int32_t(index)<0 || index>=m.get32(0x6f3a74))throw std::invalid_argument("Invalid creation script");
    const U found=call(0x466900,0x643a80,{m.get32(0x6f3a70)+index*0x47c+0x2f0});
    if(found<0x643a80 || (found-0x643a80)%0x154 || (found-0x643a80)/0x154>=8)throw std::invalid_argument("Creation script owner lookup");
    selected=(found-0x643a80)/0x154;
   }
   call(0x506290,created,{kind,selected,0});
   for(U i=0;i<2;++i){
    const U limit=m.get32(0x6c5494+i*4);
    if(!limit || limit>0x7fffffff)throw std::invalid_argument("Creation map dimensions");
    std::int64_t value=std::int32_t(m.get32(p+8+i*4));value%=limit;if(value<0)value+=limit;
    m.set32(created+0xbcf+i*4,U(value));
   }
   m.set32(created+0xbd7,m.get32(p+16));m.set32(created+0xbdb,0);
   if(scripted)call(0x568be0,0x6f3a70,{created,m.get32(p+0x58)});
  }
  if(m.get32(created+0x174)==m.get32(0x644520))m.requestRefresh();
  m.set32(p+0x48,m.get32(created));
  return 1;
 }
 U createFromLinkedCreature(U p,U kind,U inherit) {
  if(m.get32(p+0x44)>=m.get32(0x6def5c))throw std::invalid_argument("Refresh creation needs a valid linked creature");
  return createCreature(p,kind,inherit);
 }
 // Full local event dispatch and command continuation. Coordinate, placement,
 // command and lifecycle services are explicit suppliers, not engine widgets.
 U positionEvent(U p,U event) {
  std::array<U,3> position{m.convertColumn(m.get32(p+0x70c)),m.convertRow(m.get32(p+0x710)),m.get32(p+0x714)};
  if(event==5 || event==14){
   if(m.get32(p+0x7c) && m.get32(p+0x174)==m.get32(0x644520))call(0x54dad0,p+0xbf3,{m.get32(p+0xa8)==13?12u:7u});
   return 0;
  }
  if(event==8){m.set32(p+0x8cd,0);call(0x51f5e0,p,{1});call(0x506d30,p);return 0;}
  if(event!=9)return 0;
  U saved=m.get32(p+0x104);
  const U continuation=m.get32(p+0x718);
  if(continuation)m.set32(p+0x104,1);
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
  if(continuation){
   const U dependency=m.get32(p+0xac);std::array<std::uint8_t,22> metadata{};
   const auto put=[&](U at,U v){for(U i=0;i<4;++i)metadata[at+i]=std::uint8_t(v>>(i*8));};
   put(0,m.get32(dependency+12));put(4,m.get32(dependency+8));put(8,saved);put(12,m.get32(dependency+0x44));put(17,m.get32(p+0xa8));
   metadata[21]=(m.get32(p+0x723)&255)!=0 || m.get32(p+0x5ec)==14;
   if(!m.submitCommand({m.get32(p+8),m.get32(p+12),m.get32(p+16)},metadata)){
    if(m.get32(p+0xa8)==27)call(0x50ab30,p);
    else{
     m.set32(p+0xb8b,0);m.set32(p+0xa4,m.get32(p+16));m.set32(p+0xde3,0);
     m.set8(p+0x962,0);m.set32(p+0x963,0);m.set32(p+0x164,0);
     if(m.get32(p+0x664)){
      const U action=m.get32(p+0x5fc);
      if(action==10){
       U slot=m.get32(p+0x64c);if(slot>=m.get32(0x6def5c))throw std::invalid_argument("Invalid position attachment");
       U target=m.get32(0x6def58)+slot*0xe4b;m.set32(p+0x664,0);
       U count=m.get32(target+0x668)-1;m.set32(target+0x668,std::int32_t(count)<0?0:count);
      }else if(action==20){
       U target=call(0x4970b0,0x6def58,{m.get32(p+0x6a8)});m.set32(p+0x664,0);call(0x50eb60,target,{0});
      }
     }
     call(0x50aa30,p,{16});
    }
   }
  }
  call(0x54dad0,p+0xbf3,{10});
  return 0;
 }
 // Successful reload continuation [004721f0,00472218): request the parent's
 // post-load work and update lighting unconditionally before its later services.
 void loadedSceneRegion(U p){
  m.set8(p+0x100b0,1);m.updateLighting();
  call(0x479a10,0x6c3898,{m.get32(0x5c5d48)});call(0x470d40,p);m.schedulerExit(false);
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
