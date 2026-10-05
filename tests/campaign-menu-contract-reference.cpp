// Pinned original Main New Game and realm admission in a private PE32 mapping.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include "campaign_fixture_bytes.h"
#include <array>
static void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
static unsigned word(unsigned at){unsigned v;std::memcpy(&v,(void*)at,4);return v;}
static std::string calls;static unsigned samples,cues,pushes,pops;
static void __attribute__((thiscall)) transition(void* self){check(self==(void*)0x6e0100,"Main transition receiver");calls+='T';}
static void __attribute__((thiscall)) config_reset(void* self){check(self==(void*)0x6ddd58&&!std::strcmp((char*)0x6dde58,"Celtic"),"realm copied before config reset");calls+='C';}
static void __attribute__((thiscall)) wizard_reset(void* self){check(self==(void*)0x65b250,"wizard reset receiver");calls+='W';}
static void __attribute__((thiscall)) wizard_load(void* self,const char* name,int a,int b){check(self==(void*)0x65b250&&std::string(name)=="_start.wzd"&&!a&&b==-1,"wizard initial load");calls+='L';}
static void __attribute__((thiscall)) controller_reset(void* self){check(self==(void*)0x6f2aa0,"controller reset receiver");calls+='P';}
static void __attribute__((thiscall)) realm_load(void* self,const char* name){check(self==(void*)0x659e51&&std::string(name)=="Celtic"&&word(0x6e0133)==0x659408,"realm load after requested screen");calls+='R';}
static void __attribute__((thiscall)) script_reset(void* self){check(self==(void*)0x6f37f8&&!word(0x6e2038)&&!word(0x6e2034),"script reset counters");calls+='S';}
static void __attribute__((thiscall)) sample(void* self,int id,int value,int a,int b,int c,int d,int e){check(self==(void*)0x6b0198&&id==99999&&!value&&!a&&!b&&!c&&d==-1&&e==-1,"full-region rejection cue");++samples;}
static void __attribute__((thiscall)) cue(unsigned id){check(id==22,"auxiliary cue ID");++cues;}
static void __attribute__((thiscall)) push(void* self){check(self==(void*)0x6a5088&&word(0x6a50db)==4,"Realm Escape Mini context");++pushes;}
static void pop(){check(!word(0x657d37),"return clears active realm pointer");++pops;}
static void stub(unsigned at,const unsigned char* expected,unsigned n,void* target){check(n>=5&&!std::memcmp((void*)at,expected,n),"dependency original bytes");auto* p=(unsigned char*)at;p[0]=0xe9;global(at+1,unsigned((uintptr_t)target-at-5));}
int main(int argc,char** argv)try{
 check(argc==2,"Pinned PE required");map_image(read(argv[1]));
#define STUB(name) stub(name##_address,name##_bytes,sizeof(name##_bytes),(void*)&name)
 STUB(transition);STUB(config_reset);STUB(wizard_reset);STUB(wizard_load);STUB(controller_reset);STUB(realm_load);STUB(script_reset);STUB(sample);STUB(cue);STUB(push);STUB(pop);
 using Main=int (__attribute__((thiscall)) *)(void*,unsigned);auto action=(Main)0x4a75c0;
 unsigned mainCases=0;
 for(int repetition=0;repetition<3;++repetition){
  std::memset((void*)0x6e0100,0,0x80);global(0x6e0104,3);std::strcpy((char*)0x6dde58,"Greek");
  for(unsigned at:{0x68991cu,0x689920u,0x6f2b30u,0x6f2cf0u,0x6e2038u,0x6e2034u})global(at,0xdeadbeef);
  *(unsigned char*)0x6f2d08=1;calls.clear();check(action((void*)0x6e0100,0)==0,"New Game return");
  check(calls=="TCWLPRS"&&word(0x6e0133)==0x659408&&word(0x68991c)==0&&word(0x689920)==5,"New Game ordering and request");
  check(word(0x6f2b30)==0x65b250&&!word(0x6f2cf0)&&!*(unsigned char*)0x6f2d08,"controller bindings reset");++mainCases;
 }
 for(unsigned index:{1u,2u,6u,0xffffffffu}){
  global(0x6e0133,0x11223344);global(0x68991c,123);global(0x689920,456);std::strcpy((char*)0x6dde58,"Greek");calls.clear();
  check(action((void*)0x6e0100,index)==0&&calls=="T"&&!std::strcmp((char*)0x6dde58,"Greek"),"noncampaign branch must not reset realm");
  check(word(0x6e0133)==(index==1?0x6de4e8u:index==2?0x6e0020u:0x11223344u),"other pending screen");
  if(index==1)check(word(0x6de548)==3&&word(0x68991c)==0&&word(0x689920)==5,"Load caller and mode");
  ++mainCases;
 }
 using Count=int (__attribute__((thiscall)) *)(void*,int);auto count=(Count)0x54e690;unsigned countCases=0;
 for(int n:{-1,0,1,5,80})for(int region=0;region<5;++region){
  std::memset((void*)0x659e51,0,0x13dc);global(0x659f55,n);int expected=0;
  for(int i=0;i<std::max(0,n);++i){int live=i%3!=0;global(0x65ab9d+i*4,live);global(0x65ae1d+i*4,i%5);if(live&&i%5==region)++expected;}
  check(count((void*)0x659e51,region)==expected,"occupancy count/eligibility");++countCases;
 }
 using Admit=void (__attribute__((thiscall)) *)(unsigned);auto admit=(Admit)0x54eec0;unsigned admissionCases=0;
 for(unsigned player=0;player<3;++player)for(unsigned owner=0;owner<3;++owner)for(unsigned here=0;here<2;++here)for(unsigned otherHere=0;otherHere<2;++otherHere)for(unsigned occupants=0;occupants<=5;++occupants){
  const unsigned region=3;std::memset((void*)0x659e51,0,0x13dc);global(0x659f51,player);global(0x659f55,8);global(0x65b1d9+region*4,owner);
  for(unsigned i=0;i<8;++i){global(0x65ae1d+i*4,9);global(0x65af5d+i*4,0x11223344);}
  global(0x65ae1d+player*4,here?region:2);if(owner!=player)global(0x65ae1d+owner*4,otherHere?region:2);
  unsigned actual=0;for(unsigned i=0;i<8&&actual<occupants;++i)if(word(0x65ae1d+i*4)==region){global(0x65ab9d+i*4,1);++actual;}
  for(unsigned i=3;i<8&&actual<occupants;++i){if(word(0x65ab9d+i*4))continue;global(0x65ae1d+i*4,region);global(0x65ab9d+i*4,1);++actual;}
  global(0x659e33,0);*(unsigned char*)0x659e50=0;global(0x6f2d0c,0x11223344);global(0x6f2d10,0x11223344);global(0x6f2d14,0x11223344);samples=0;
  admit(region);const bool battle=here&&owner!=player&&otherHere;
  check(word(0x659e33)==(battle?4u:0u)&&*(unsigned char*)0x659e50==unsigned(battle),"battle admission mode/return byte");
  if(battle)check(word(0x6f2d0c)==region&&word(0x6f2d10)==player&&word(0x6f2d14)==owner&&!samples&&word(0x65af5d+player*4)==0x11223344,"battle participant selection precedes capacity");
  else check(word(0x65af5d+player*4)==(actual<4?region:0x11223344u)&&samples==unsigned(actual>=4)&&word(0x6f2d0c)==0x11223344,"destination capacity or refusal cue");
  ++admissionCases;
 }
 unsigned auxCases=0;using Aux=void (__attribute__((stdcall)) *)(unsigned);auto aux=(Aux)0x54fd20;
 const std::array<unsigned,4> flags{0x6f2d28,0x6f2d34,0x6f2d2c,0x6f2d30};
 for(unsigned index:{0u,1u,2u,3u,4u,5u,0xffffffffu}){
  for(auto at:flags){global(at,0);}
  cues=0;aux(index);check(cues==1,"auxiliary callback cue including unknown indices");
  for(unsigned i=0;i<4;++i)check(word(flags[i])==unsigned(index==i+1),"auxiliary request flag mapping");
  ++auxCases;
 }
 unsigned navigationCases=0;using Input=int (__attribute__((thiscall)) *)(void*,unsigned,unsigned,unsigned,unsigned);auto input=(Input)0x552310;
 for(unsigned message:{0x100u,0x104u,0x200u})for(unsigned key:{0x1bu,0x56u,0x76u,0x7fu}){
  std::memset((void*)0x659408,0,0xa49);global(0x6a50db,0);pushes=0;
  check(input((void*)0x659408,0,message,key,0)==0,"Realm input return");check(pushes==unsigned(message==0x100&&key==0x1b),"Escape opens Mini only for WM_KEYDOWN");
  check(*(unsigned char*)0x659460==unsigned(message==0x104&&(key==0x56||key==0x76)),"system V flag");++navigationCases;
 }
 using Tick=void (__attribute__((thiscall)) *)(void*);
 for(unsigned entry:{0x5517a0u,0x552210u}){global(0x659e3f,1);global(0x657d37,0x659408);pops=0;((Tick)entry)((void*)0x659408);
  check(!word(0x659e3f)&&pops==1,"pending Realm return consumed once");++navigationCases;}
 std::cout<<"{\"success\":true,\"main_cases\":"<<mainCases<<",\"occupancy_cases\":"<<countCases<<",\"admission_cases\":"<<admissionCases<<",\"auxiliary_cases\":"<<auxCases<<",\"navigation_cases\":"<<navigationCases<<"}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
