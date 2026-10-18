#include "../reconstruction/creature-ai/attack_selection.hpp"
#include "../reconstruction/pathfinding/route_footprint_height.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>
#ifdef MNM_ENCOUNTER_SUPPORT_REFERENCE
#define main unused_support_mapper_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
namespace m=mnm::reconstruction::combat;
namespace {
[[maybe_unused]] void requireSupport(bool value,const char *why){if(!value)throw std::runtime_error(why);}
std::uint64_t resultHash=1469598103934665603ull;
void hashWord(unsigned value){for(unsigned i=0;i<4;++i)resultHash=(resultHash^((value>>(8*i))&255))*1099511628211ull;}
#ifdef MNM_ENCOUNTER_SUPPORT_REFERENCE
std::vector<m::AttackSelectionEvent> observed;
std::vector<std::pair<unsigned,Bytes>> protectedBytes;
extern "C" void __attribute__((thiscall)) supportPlay(void *,unsigned sequence,unsigned reverse){requireSupport(!reverse,"unexpected reverse attack");observed.push_back({m::AttackSelectionEffect::play,sequence});}
extern "C" void __attribute__((thiscall)) supportVisual(void *){observed.push_back({m::AttackSelectionEffect::visualRepair,0});}
extern "C" void __attribute__((thiscall)) supportAttachment(void *,unsigned facing){observed.push_back({m::AttackSelectionEffect::attachmentFacing,facing});}
extern "C" void __attribute__((thiscall)) supportSecondary(void *,unsigned sequence){observed.push_back({m::AttackSelectionEffect::secondarySwitch,sequence});}
template<class T>void supportPut(void *base,unsigned at,T value){std::memcpy(static_cast<unsigned char*>(base)+at,&value,sizeof(value));}
void supportRedirect(unsigned at,std::uintptr_t target){auto *p=reinterpret_cast<unsigned char*>(at);p[0]=0xe9;const auto delta=std::uint32_t(target-at-5);std::memcpy(p+1,&delta,4);}
void guardSupport(){for(const auto &g:protectedBytes)requireSupport(!std::memcmp(reinterpret_cast<void*>(g.first),g.second.data(),g.second.size()),"protected support body/table changed");}
#endif
}
int main(int argc,char **argv)try {
#ifdef MNM_ENCOUNTER_SUPPORT_REFERENCE
  requireSupport(argc==2,"expected pinned executable");map_image(read(argv[1]));
  for(auto span:{std::pair<unsigned,unsigned>{0x50e400,0x50e66d},{0x4f3260,0x4f331d},{0x510480,0x5104a7},{0x5e51e4,0x5e5310}})
    protectedBytes.push_back({span.first,Bytes(reinterpret_cast<unsigned char*>(span.first),reinterpret_cast<unsigned char*>(span.second))});
  supportRedirect(0x464cb0,reinterpret_cast<std::uintptr_t>(&supportPlay));supportRedirect(0x513ff0,reinterpret_cast<std::uintptr_t>(&supportVisual));supportRedirect(0x494a50,reinterpret_cast<std::uintptr_t>(&supportAttachment));supportRedirect(0x464e20,reinterpret_cast<std::uintptr_t>(&supportSecondary));
  supportPut(reinterpret_cast<void*>(0x6cb8c2),0,0u);supportPut(reinterpret_cast<void*>(0x6cb942),0,0u);
#else
  (void)argc;(void)argv;
#endif
  unsigned selections=0,heights=0,predicates=0;
  for(unsigned n=0;n<65536;++n){
    const m::Extent d{16,18,4};m::AttackSelectionState s;
    s.position={int(n%16),int((n/16)%18),1};s.width=1+(n/288)%2;s.direction=(n/576)%8;s.facing=(n/4608)%8;
    s.animationKind=(n/5)%4;s.reverse=n&1;s.attached=n&2;s.secondary=n&4;s.offset=123;
    const unsigned camera=(n/7)%4, mode=(n/11)%4;
    m::Cell p{(s.position.x+int((n/3)%9)+12)%16,(s.position.y+int((n/29)%9)+14)%18,1};
    std::optional<m::AttackSelectionTarget> target;if(mode<2)target=m::AttackSelectionTarget{p,1+mode};
    std::optional<m::AttackSelectionObject> object;if(mode==2||mode==3)object=m::AttackSelectionObject{p,mode==2?26u:1u};
    auto r=m::selectAttackAnimation(s,d,camera,target,object);
#ifdef MNM_ENCOUNTER_SUPPORT_REFERENCE
    Bytes actor(0xe4b),other(0xe4b),definition(0x5c9),otherDefinition(0x5c9),cell(16*18*12),objectPool(558);
    supportPut(definition.data(),8,s.width);supportPut(otherDefinition.data(),8,target?target->width:1u);
    supportPut(actor.data(),0xac,reinterpret_cast<std::uintptr_t>(definition.data()));supportPut(other.data(),0xac,reinterpret_cast<std::uintptr_t>(otherDefinition.data()));
    supportPut(actor.data(),8,s.position.x);supportPut(actor.data(),12,s.position.y);supportPut(actor.data(),0x608,s.direction);supportPut(actor.data(),0x60c,s.facing);supportPut(actor.data(),0xe0,s.offset);supportPut(actor.data(),0x7cd,s.animationKind);supportPut(actor.data(),0x722,std::uint8_t(s.reverse));supportPut(actor.data(),0x7d1,s.attached?0x12000u:0u);supportPut(actor.data(),0x855,unsigned(s.secondary));
    supportPut(actor.data(),0x96b,p.x);supportPut(actor.data(),0x96f,p.y);supportPut(actor.data(),0x973,0u);
    for(unsigned y=0;y<18;++y)supportPut(reinterpret_cast<void*>(0x6cb942),y*4,y*16);
    supportPut(other.data(),8,p.x);supportPut(other.data(),12,p.y);supportPut(objectPool.data(),8,p.x);supportPut(objectPool.data(),12,p.y);supportPut(objectPool.data(),0x28,object?object->kind:0u);
    supportPut(cell.data(),12*(p.y*16+p.x)+2,std::uint16_t(object?0:0xffff));supportPut(reinterpret_cast<void*>(0x6c54dc),0,reinterpret_cast<std::uintptr_t>(cell.data()));supportPut(reinterpret_cast<void*>(0x689498),0,reinterpret_cast<std::uintptr_t>(objectPool.data()));supportPut(reinterpret_cast<void*>(0x68949c),0,1u);supportPut(reinterpret_cast<void*>(0x6c5494),0,d.x);supportPut(reinterpret_cast<void*>(0x6c5498),0,d.y);supportPut(reinterpret_cast<void*>(0x6e9850),0,camera);
    auto expected=actor;const auto immutableOther=other,immutableDefinition=definition,immutableObjects=objectPool;observed.clear();
    using Select=void(__attribute__((thiscall)) *)(void *,void *);reinterpret_cast<Select>(0x50e400)(actor.data(),target?other.data():nullptr);
    supportPut(expected.data(),0x608,r.state.direction);supportPut(expected.data(),0x60c,r.state.facing);supportPut(expected.data(),0xe0,r.state.offset);
    if(actor!=expected || observed!=r.events){unsigned v[3];for(unsigned i=0;i<3;++i)std::memcpy(&v[i],actor.data()+std::array<unsigned,3>{0x608,0x60c,0xe0}[i],4);std::cerr<<"case "<<n<<" width "<<s.width<<" pos "<<s.position.x<<","<<s.position.y<<" target "<<p.x<<","<<p.y<<" mode "<<mode<<" facing "<<s.facing<<" camera "<<camera<<" original "<<v[0]<<","<<v[1]<<","<<v[2]<<" native "<<r.state.direction<<","<<r.state.facing<<","<<r.state.offset<<"\n";for(auto e:observed)std::cerr<<"original event "<<unsigned(e.kind)<<" "<<e.argument<<"\n";for(auto e:r.events)std::cerr<<"native event "<<unsigned(e.kind)<<" "<<e.argument<<"\n";}
    requireSupport(actor==expected&&observed==r.events,"original attack selection projection mismatch");requireSupport(other==immutableOther&&definition==immutableDefinition&&objectPool==immutableObjects,"attack inputs changed");guardSupport();
#endif
    hashWord(r.state.direction);hashWord(r.state.facing);hashWord(r.state.offset);for(auto e:r.events){hashWord(unsigned(e.kind));hashWord(e.argument);}++selections;
  }
  for(unsigned n=0;n<16384;++n){
    std::array<std::uint16_t,4> kinds{};std::array<int,5> values{};const int table[]={0,1,7,16,17,32,-1,INT32_MIN};
    for(unsigned i=0;i<4;++i){kinds[i]=(n>>(i*3))%5;values[i+1]=table[(n>>(i*2))%8];}
    const auto native=mnm::reconstruction::large_footprint_height(kinds,[&](auto kind){return values.at(kind);});
#ifdef MNM_ENCOUNTER_SUPPORT_REFERENCE
    std::array<unsigned char,48> cells{};Bytes terrain(5*356);std::array<unsigned,2> rows{0,2};
    for(unsigned i=0;i<4;++i)supportPut(cells.data(),i*12,kinds[i]);
    for(unsigned i=0;i<5;++i)supportPut(terrain.data(),i*356+0x94,values[i]);
    supportPut(reinterpret_cast<void*>(0x65660c),0,reinterpret_cast<std::uintptr_t>(terrain.data()));const auto beforeCells=cells;const auto beforeTerrain=terrain;
    using Height=int(__attribute__((thiscall)) *)(void *,void *);const auto original=reinterpret_cast<Height>(0x4f3260)(rows.data(),cells.data());
    requireSupport(native==original&&cells==beforeCells&&terrain==beforeTerrain,"original footprint height mismatch");guardSupport();
#endif
    hashWord(unsigned(native));++heights;
  }
  for(unsigned n=0;n<256;++n){const unsigned health=n%4==0?0:n, a=(n/4)%4,b=(n/16)%4;const auto native=m::corpseFlightAnimation(health,a,b);
#ifdef MNM_ENCOUNTER_SUPPORT_REFERENCE
    Bytes actor(0xe4b);supportPut(actor.data(),0xe4,health);supportPut(actor.data(),0xf0,a);supportPut(actor.data(),0xf4,b);const auto before=actor;using Predicate=unsigned(__attribute__((thiscall)) *)(void *);requireSupport(native==bool(reinterpret_cast<Predicate>(0x510480)(actor.data()))&&actor==before,"original corpse-flight predicate mismatch");guardSupport();
#endif
    hashWord(native);++predicates;
  }
  std::cout<<"{\"success\":true,\"cases\":"<<selections+heights+predicates<<",\"functions\":3,\"attack_selections\":"<<selections<<",\"footprint_heights\":"<<heights<<",\"corpse_predicates\":"<<predicates<<",\"digest\":"<<resultHash<<"}\n";return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
