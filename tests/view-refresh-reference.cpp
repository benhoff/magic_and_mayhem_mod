// Whole unchanged view helpers; private byte-checked animation request adapters.
#define main previous_camera_reference_unused_main
#include "terrain-camera-reference.cpp"
#undef main
#include "view_refresh.hpp"
#include <cstdlib>
#include "effect_animation_selection.hpp"
using namespace mnm::reconstruction;
static void viewCheck(bool ok,unsigned line){if(!ok){std::cerr<<"View fixture check failed at line "<<line<<"\n";std::exit(1);}}
#define require(value) viewCheck(bool(value),__LINE__)
using Target=AnimationViewAction::Target;using Operation=AnimationViewAction::Operation;
static std::vector<AnimationViewAction> calls;
static std::uintptr_t actor=0,effect=0,catalog=0;
static Target target(void* p){const auto value=reinterpret_cast<std::uintptr_t>(p);if(value==actor+0xb0)return Target::main;if(value==actor+0x7d5)return Target::special;if(value==actor+0x825)return Target::secondary;require(value==effect+0x160);return Target::effect;}
static void __attribute__((thiscall)) bindCatalog(void* p,void* asset){calls.push_back({target(p),Operation::bindCatalog,unsigned((reinterpret_cast<std::uintptr_t>(asset)-catalog)/0x114)});}
static void __attribute__((thiscall)) start(void* p,unsigned value,unsigned reverse){require(!reverse);calls.push_back({target(p),Operation::startForward,value});}
static void __attribute__((thiscall)) switchTo(void* p,unsigned value){calls.push_back({target(p),Operation::switchSequence,value});}
static void __attribute__((thiscall)) linked(void*,unsigned value){calls.push_back({Target::linkedEffect,Operation::refreshLinkedEffect,value});}
static void redirect(std::uintptr_t at,const std::vector<unsigned char>& bytes,void* replacement){require(bytes.size()>=6&&!std::memcmp(reinterpret_cast<void*>(at),bytes.data(),bytes.size()));unsigned char stub[]={0x68,0,0,0,0,0xc3};auto address=reinterpret_cast<std::uintptr_t>(replacement);std::memcpy(stub+1,&address,4);std::memcpy(reinterpret_cast<void*>(at),stub,6);}
static void actions(const std::vector<AnimationViewAction>& expected){if(calls.size()!=expected.size())std::cerr<<"request counts "<<calls.size()<<" vs "<<expected.size()<<"\n";require(calls.size()==expected.size());for(unsigned i=0;i<calls.size();++i)require(calls[i].target==expected[i].target&&calls[i].operation==expected[i].operation&&calls[i].value==expected[i].value);}
int main(int argc,char** argv)try{
 require(argc==2);mapImage(read(argv[1]));calls.reserve(8);
 redirect(0x464ca0,{0x8b,0x44,0x24,0x04,0x89,0x41,0x1c},reinterpret_cast<void*>(&bindCatalog));redirect(0x464cb0,{0x8b,0x44,0x24,0x04,0x53,0x56},reinterpret_cast<void*>(&start));redirect(0x464e20,{0x53,0x56,0x8b,0x71,0x1c,0x57},reinterpret_cast<void*>(&switchTo));
 std::array<unsigned char,6> linkedBytes;std::memcpy(linkedBytes.data(),reinterpret_cast<void*>(0x494a50),6);redirect(0x494a50,{0x83,0x79,0x28,0x34,0x75,0x4a},reinterpret_cast<void*>(&linked));
 using One=void(__attribute__((thiscall)) *)(void*);using Linked=void(__attribute__((thiscall)) *)(void*,unsigned);using Direction=unsigned(__attribute__((stdcall)) *)(float,float);
 std::mt19937 rng(0x508a10);unsigned creatureChecks=0,effectChecks=0,linkedChecks=0,directionChecks=0,nativeRefusals=0;
 std::array<unsigned char,0x170> special{};std::array<unsigned char,3*0x114> catalogBytes{};catalog=reinterpret_cast<std::uintptr_t>(catalogBytes.data());put(reinterpret_cast<void*>(0x6894a0),0,catalog);put(reinterpret_cast<void*>(0x6898d0),0,reinterpret_cast<std::uintptr_t>(special.data()));
 std::array<unsigned char,0xe4b> creatureBytes{};std::array<unsigned char,0x22e> effectBytes{};actor=reinterpret_cast<std::uintptr_t>(creatureBytes.data());effect=reinterpret_cast<std::uintptr_t>(effectBytes.data());
 for(unsigned view=0;view<4;++view)for(unsigned direction:{0u,1u,2u,3u,4u,5u,6u,7u,8u,31u,0xffffffffu})for(unsigned kind:{0u,2u,3u,99u})for(unsigned flags=0;flags<8;++flags){
  creatureBytes.fill(0xa5);CreatureViewState state{direction,99,kind,rng(),bool(flags&1),bool(flags&2),bool(flags&4)};CreatureSpecialView selected{rng(),rng()%3};put(special.data(),0x160,selected.sequenceBase);put(special.data(),0x164,selected.catalogIndex);put(creatureBytes.data(),0x608,direction);put(creatureBytes.data(),0x60c,99);creatureBytes[0x722]=state.reverseFacing?5:0;put(creatureBytes.data(),0x7cd,kind);put(creatureBytes.data(),0x7d1,state.linkedEffect?effect:0);put(creatureBytes.data(),0x855,state.secondary?1:0);put(creatureBytes.data(),0xe0,state.mainSequenceBase);put(reinterpret_cast<void*>(0x6e9850),0,view);auto expectedBytes=creatureBytes;calls.clear();reinterpret_cast<One>(0x508a10)(creatureBytes.data());auto expected=refreshCreatureView(state,view,selected);actions(expected);put(expectedBytes.data(),0x60c,state.viewDirection);require(expectedBytes==creatureBytes);++creatureChecks;
 }
 std::memcpy(reinterpret_cast<void*>(0x494a50),linkedBytes.data(),6);
 EffectViewInputs input;input.width=17;input.height=31;input.sequenceBases.resize(89);std::array<std::array<std::uint32_t,4>,89> entries{};for(unsigned i=0;i<input.sequenceBases.size();++i){input.sequenceBases[i]=i*100+3;entries[i][0]=input.sequenceBases[i];}
 std::array<unsigned char,0x440> metadata{};put(metadata.data(),0x438,reinterpret_cast<std::uintptr_t>(entries.data()));put(effectBytes.data(),0x24,reinterpret_cast<std::uintptr_t>(metadata.data()));
 std::array<std::array<unsigned char,0xe4b>,3> ownedCreatures{};std::array<std::array<unsigned char,12>,3> descriptors{};input.creatures.resize(3);for(unsigned i=0;i<3;++i){input.creatures[i]=EffectViewCreature{int(i*3),int(i*5),i+1};put(ownedCreatures[i].data(),8,i*3);put(ownedCreatures[i].data(),12,i*5);put(ownedCreatures[i].data(),0x60c,i+1);put(ownedCreatures[i].data(),0xac,reinterpret_cast<std::uintptr_t>(descriptors[i].data()));}
 const auto base=reinterpret_cast<std::uintptr_t>(ownedCreatures.data());put(reinterpret_cast<void*>(0x6def58),0,base);put(reinterpret_cast<void*>(0x6def5c),0,3);put(reinterpret_cast<void*>(0x6c5494),0,input.width);put(reinterpret_cast<void*>(0x6c5498),0,input.height);
 const auto pointer=[&](std::optional<unsigned> ordinal){return ordinal?base+*ordinal*0xe4b:0;};
 const auto fields=[&](void* p,const EffectViewState& s){for(const auto f:std::vector<std::pair<unsigned,unsigned>>{{0x28,s.type},{0x4c,s.kind},{0x58,s.stage},{0x50,s.variant},{0x11c,s.orientationIndex},{0x54,s.mode},{0x1a6,s.worldDirection},{0x1a2,s.viewDirection},{0x44,s.targetOrdinal},{0x48,s.creatureOrdinal},{0x198,unsigned(pointer(s.cachedTarget))},{8,unsigned(s.column)},{12,unsigned(s.row)},{0x2c,unsigned(s.initialX)},{0x30,unsigned(s.initialY)},{0x38,unsigned(s.destinationX)},{0x3c,unsigned(s.destinationY)}})put(p,f.first,f.second);std::memcpy(static_cast<unsigned char*>(p)+0x6c,&s.velocityX,4);std::memcpy(static_cast<unsigned char*>(p)+0x70,&s.velocityY,4);};
 const auto compareEffect=[&](EffectViewState state,unsigned view,bool attached=false){
  effectBytes.fill(0xa5);put(effectBytes.data(),0x24,reinterpret_cast<std::uintptr_t>(metadata.data()));fields(effectBytes.data(),state);auto expectedBytes=effectBytes;put(reinterpret_cast<void*>(0x5fed78),0,view);calls.clear();auto native=state;std::vector<AnimationViewAction> expected;
  bool refusalExpected=false;
  if(!attached&&(state.type==0||state.type==4||state.type==7)){
   if(state.kind==52&&state.type!=7&&state.creatureOrdinal>=input.creatureDescriptorClasses.size())refusalExpected=true;
   else refusalExpected=selectEffectAnimation(state.type,state.kind,input.kindClasses,state.creatureOrdinal,input.creatureDescriptorClasses)==NoEffectAnimation;
  }
  if(!attached&&state.type==81&&state.mode!=4){const auto target=state.targetOrdinal==0xffffffffu?state.cachedTarget:std::optional<unsigned>(state.targetOrdinal);refusalExpected=!target||*target>=input.creatures.size()||!input.creatures[*target];}
  bool refused=false;try{expected=attached?refreshCreatureLinkedEffect(native,input):refreshEffectView(native,view,input);}catch(const std::exception&){refused=true;}
  require(refused==refusalExpected);if(refused){++nativeRefusals;return;}
  if(attached)reinterpret_cast<Linked>(0x494a50)(effectBytes.data(),99);else reinterpret_cast<One>(0x4945f0)(effectBytes.data());actions(expected);fields(expectedBytes.data(),native);
  if(expectedBytes!=effectBytes){std::cerr<<"effect state differs type="<<state.type<<" kind="<<state.kind<<" view="<<view<<'\n';throw std::runtime_error("Effect original field mismatch");}
  ++effectChecks;if(attached)++linkedChecks;
 };
 for(unsigned category=0;category<3;++category){
  input.kindClasses.assign(103,category);input.creatureDescriptorClasses.assign(3,category);for(unsigned i=0;i<103;++i)put(reinterpret_cast<void*>(0x6b13cd+i*721),0,category);for(auto& d:descriptors)put(d.data(),8,category);
  for(unsigned type=0;type<112;++type)for(unsigned view=0;view<4;++view)for(unsigned kind=0;kind<(type==0||type==4||type==7?103u:1u);++kind){
   EffectViewState state;state.type=type;state.kind=kind;state.stage=category;state.variant=view;state.orientationIndex=(type+view+category)%36;state.mode=category==2?4:0;state.targetOrdinal=(view+category)%5;state.creatureOrdinal=(kind+view)%5;state.cachedTarget=1;state.column=int(rng()%1025)-512;state.row=int(rng()%1025)-512;state.initialX=int(rng());state.initialY=int(rng());state.destinationX=int(rng());state.destinationY=int(rng());state.velocityX=float(int(rng()%1025)-512);state.velocityY=float(int(rng()%1025)-512);state.worldDirection=99;state.viewDirection=98;compareEffect(state,view);
  }
 }
 for(unsigned view=0;view<4;++view)for(unsigned orientation=0;orientation<36;++orientation){EffectViewState state;state.type=20;state.orientationIndex=orientation;compareEffect(state,view);}
 for(unsigned view=0;view<4;++view)for(unsigned variant=0;variant<4;++variant){EffectViewState state;state.type=63;state.variant=variant;compareEffect(state,view);}
 for(unsigned n=0;n<4096;++n){EffectViewState state;state.type=n%3==0?0:n%3==1?4:7;state.kind=std::array<unsigned,3>{{93,95,98}}[n%3];state.velocityX=n<4?std::array<float,4>{{0.f,-0.f,1.f,-1.f}}[n]:float(int(rng()%1048577)-524288);state.velocityY=n<4?0.f:float(int(rng()%1048577)-524288);state.stage=n%7;compareEffect(state,n%4);const auto actual=reinterpret_cast<Direction>(0x4eacc0)(state.velocityX,state.velocityY);require(actual==effectViewDirection8(state.velocityX,state.velocityY));++directionChecks;}
 for(unsigned owner=0;owner<3;++owner)for(unsigned viewDirection=0;viewDirection<8;++viewDirection){input.creatures[owner]->viewDirection=viewDirection;put(ownedCreatures[owner].data(),0x60c,viewDirection);EffectViewState state;state.type=52;state.targetOrdinal=owner;compareEffect(state,0,true);}
 std::cout<<"{\"success\":true,\"creature_checks\":"<<creatureChecks<<",\"effect_checks\":"<<effectChecks<<",\"linked_checks\":"<<linkedChecks<<",\"direction_checks\":"<<directionChecks<<",\"native_refusals_outside_original_domain\":"<<nativeRefusals<<"}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
