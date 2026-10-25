#pragma once
#include <cstdint>
#include <initializer_list>
#include <stdexcept>
namespace mnm::reconstruction {
// Whole 0056a8d0: authored script strings select owner changes, directed table
// consumers and symmetric pair setters. Lifecycle/reveal/name services remain
// explicit guest endpoints; this class never stores host pointers.
template<class Memory> class TerrainLightScriptRelations {
 using U=std::uint32_t;Memory& m;
 U call(U a,U p,std::initializer_list<U> args={}){return m.call(a,p,args);}
 bool same(U a,U b){for(U i=0;i<256;++i){auto x=m.get8(a+i),y=m.get8(b+i);if(x!=y)return false;if(!x)return true;}throw std::invalid_argument("Unterminated script name");}
 bool named(U p){return !same(p,0x5fc874);}
 U owner(U name){return call(0x466900,0x643a80,{name});}
 U index(U p){return U(std::int32_t(p-0x643a80)/0x154);}
 U character(U name){U base=m.get32(0x6f37f8);auto count=std::int32_t(m.get32(0x6f37fc));for(std::int32_t i=0;i<count;++i){U p=base+U(i)*0x40;if(m.get32(p)&&same(p+4,name))return p;}return 0;}
 U creature(U i){return i<m.get32(0x6def5c)?m.get32(0x6def58)+i*0xe4b:0;}
 bool shares(U p,U i){return m.get8(p+0x18+i)&&m.get8(p+0x20+i);}
 void revealCharacter(U c,U counterpart){
  if(!c||!m.get32(c+0x3c)||!shares(counterpart,m.get32(0x644520))||m.get32(c+0x28)==~U(0))return;
  U value=call(0x463cb0,0);call(0x463eb0,value,{creature(m.get32(c+0x28)),c+4});
 }
 void cleanup(U c){call(0x51edf0,c);U state=m.get32(c+0x5ec);if(state==3||state==9||state==10||state==11)call(0x50ab30,c);if(m.get32(c+0x5f0)==3)m.set32(c+0x5f0,0);}
public:
 explicit TerrainLightScriptRelations(Memory& memory):m(memory){}
 U execute(U manager,U p){
  if(named(p+0x36c)&&!named(p+0x30c))throw std::invalid_argument("Shared sight requires a partner owner name");
  if(named(p+0x2ec)&&named(p+0x2cc)){
   U to=owner(p+0x2ec),c=character(p+0x2cc);
   if(c&&m.get32(c+0x28)!=~U(0)){
    U source=creature(m.get32(c+0x28));call(0x51ec80,source,{index(to)});
    if(m.get32(p+0x2c)!=~U(0)&&!m.get32(p+0x30))call(0x571310,0x658178,{m.get32(p+0x2c),m.get32(source+8),m.get32(source+12),m.get32(source+16),0,0,0});
   }
  }
  if(named(p+0x32c)&&named(p+0x30c)){U a=owner(p+0x32c),b=owner(p+0x30c);call(0x465500,a,{index(b),0});}
  if(named(p+0x34c)&&named(p+0x30c)){
   U a=owner(p+0x34c),b=owner(p+0x30c),ai=index(a),bi=index(b);
   call(0x465500,a,{bi,1});
   for(std::int32_t i=0;i<std::int32_t(m.get32(0x6df180));++i){
    U c=creature(U(i));if(!m.get32(c+4)||!m.get32(c+0xe4))continue;
    if(m.get32(c+0x174)==ai)cleanup(c);if(m.get32(c+0x174)==bi)cleanup(c);
    if(shares(a,bi)){if(m.get32(c+0x174)==ai)call(0x508ca0,c,{1,bi,0});if(m.get32(c+0x174)==bi)call(0x508ca0,c,{1,ai,0});}
   }
   U c=0,other=0;
   if(m.get32(a+4)){c=character(p+0x30c);other=b;}else if(m.get32(b+4)){c=character(p+0x34c);other=a;}
   revealCharacter(c,other);
  }
  if(named(p+0x36c)&&named(p+0x30c)){
   U a=owner(p+0x36c),b=owner(p+0x30c),ai=index(a),bi=index(b);
   call(0x4655e0,a,{bi,m.get32(p+0x38c)?1u:0u});
   if(m.get32(p+0x38c)&&m.get8(a+0x18+bi))for(std::int32_t i=0;i<std::int32_t(m.get32(0x6df180));++i){U c=creature(U(i));if(m.get32(c+4)&&m.get32(c+0xe4)){if(m.get32(c+0x174)==ai)call(0x508ca0,c,{1,bi,0});if(m.get32(c+0x174)==bi)call(0x508ca0,c,{1,ai,0});}}
   U c=0,other=0;if(m.get32(a+4)){c=character(b+0x50);other=b;}else if(m.get32(b+4)){c=character(a+0x50);other=a;}revealCharacter(c,other);
  }
  return call(0x564d60,manager,{p});
 }
};
}
