// Independent unchanged PE32 and native64/sanitized effect-builder matrix.
#ifdef ORIGINAL_REFERENCE
#define main sprite_effect_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include <asm/ldt.h>
#include <sys/syscall.h>
#include <unistd.h>
#else
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#endif
#include "palette_effects.hpp"
#include <algorithm>
#include <array>
#include <random>
using namespace mnm::reconstruction;
static PaletteEffectProfile settings;
static unsigned variant;
static void require(bool b,const char* why){if(!b)throw std::runtime_error(why);}
#ifdef ORIGINAL_REFERENCE
static std::vector<unsigned char> storage;
static unsigned requested;
static void* allocate_palette(unsigned n){require(n<=256*1024&&storage.empty(),"Allocator bound");requested=n;storage.assign(n+2048,0xa5);return storage.data();}
class PrivateFs {
 unsigned short saved=0;unsigned head=~0u;
public:
 PrivateFs(){asm volatile("mov %%fs,%0":"=r"(saved));user_desc d{};d.entry_number=unsigned(-1);d.base_addr=reinterpret_cast<std::uintptr_t>(&head);d.limit=3;d.seg_32bit=1;d.useable=1;require(syscall(SYS_set_thread_area,&d)==0,"Private FS descriptor");const unsigned short selector=(d.entry_number<<3)|3;asm volatile("mov %0,%%fs"::"r"(selector):"memory");}
 ~PrivateFs(){asm volatile("mov %0,%%fs"::"r"(saved):"memory");}
 void check(){require(head==~0u,"Original SEH restoration");}
};
static unsigned __attribute__((stdcall)) directory(unsigned size,char* out){require(size>=4,"Directory extent");std::memcpy(out,"Z:\\",4);return 3;}
static unsigned __attribute__((stdcall)) profile(const char* section,const char* key,const char*,char* out,unsigned size,const char*){
 require(size==256,"Profile extent");const char* sections[]={"STONE_EFFECT","STONE_EFFECT","STONE_EFFECT","METAL_EFFECT","METAL_EFFECT","METAL_EFFECT","METAL_EFFECT","METAL_EFFECT","REDTINT_EFFECT"};const char* keys[]={"Reducer","SpecalValue","Darkest","Reducer","SpecalValue","Darkest","BlueTint","Contrast","Reducer"};const double values[]={settings.stoneReducer,settings.stoneSpecalValue,settings.stoneDarkest,settings.metalReducer,settings.metalSpecalValue,settings.metalDarkest,settings.metalBlueTint,settings.metalContrast,settings.redTintReducer};
 for(unsigned i=0;i<9;++i)if(!std::strcmp(section,sections[i])&&!std::strcmp(key,keys[i])){if(!variant){out[0]=0;return 0;}const auto text=std::to_string(values[i]);require(text.size()<size,"Profile string bound");std::memcpy(out,text.c_str(),text.size()+1);return text.size();}
 throw std::runtime_error("Unexpected profile key");
}
static void lighting(const PaletteShadingConfig& c){
 for(unsigned base:{0x5f18d0u,0x5f18f8u}){global(base,c.intensityLevel);global(base+4,c.saturationLevel);std::memcpy(reinterpret_cast<void*>(base+8),&c.intensityPower,8);std::memcpy(reinterpret_cast<void*>(base+16),&c.saturationPower,8);}
}
static const std::array<unsigned,10> builders{{0x582440,0x582a80,0x583330,0x583bb0,0x586660,0x5870d0,0x585d80,0x587e70,0x588730,0x588bf0}};
#endif
static std::uint64_t valueHash=14695981039346656037ULL,words=0;
static unsigned palettes=0,refusals=0,mutations=0,underallocated=0;
static void hash(unsigned value,unsigned bytes){for(unsigned i=0;i<bytes;++i){valueHash^=(value>>(i*8))&255;valueHash*=1099511628211ULL;}}
static void fixture(const PaletteRgb& input,unsigned mode,PaletteShadingConfig c,unsigned format){
 auto originalSource=input;bool admitted=true;PaletteEffectResult native;
 try{native=buildPaletteEffect(input,mode,c,settings,format!=0);}catch(const std::invalid_argument&){admitted=false;}
#ifdef ORIGINAL_REFERENCE
 storage.clear();requested=0;void* node=reinterpret_cast<void*>(1);lighting(c);global(0x6e1f88,format);
 using Build=unsigned(__attribute__((fastcall)) *)(unsigned,void**,const void*);
 const auto accepted=reinterpret_cast<Build>(builders[mode])(c.count,&node,originalSource.data());
 require(bool(accepted)==admitted,"Original/native count admission");
 if(!admitted){require(node==reinterpret_cast<void*>(1)&&storage.empty()&&originalSource==input,"Rejected count changed palette/node");++refusals;return;}
 require(node==storage.data()&&node,"Original allocation identity");
 unsigned next,shift,neutral;std::memcpy(&next,storage.data(),4);std::memcpy(&shift,storage.data()+4,4);std::memcpy(&neutral,storage.data()+8,4);
 require(!next&&shift==native.palette.shift&&neutral==native.palette.neutral,"Original palette header");
 if(native.source!=originalSource){
  for(unsigned i=0;i<256;++i)if(native.source[i]!=originalSource[i]){std::cerr<<"Source mode="<<mode<<" count="<<c.count<<" variant="<<variant<<" format="<<format<<" colour="<<i<<" input="<<int(input[i][0])<<','<<int(input[i][1])<<','<<int(input[i][2])<<" native="<<int(native.source[i][0])<<','<<int(native.source[i][1])<<','<<int(native.source[i][2])<<" original="<<int(originalSource[i][0])<<','<<int(originalSource[i][1])<<','<<int(originalSource[i][2])<<'\n';break;}
  throw std::runtime_error("Mode5 source mutation mismatch");
 }
 for(unsigned table=0;table<native.palette.tables.size();++table)for(unsigned colour=0;colour<256;++colour){
  unsigned actual=0;std::memcpy(&actual,storage.data()+12+(table*256+colour)*native.originalEntryBytes,native.originalEntryBytes);
  if(actual!=native.palette.tables[table][colour]){std::cerr<<"mode="<<mode<<" count="<<c.count<<" variant="<<variant<<" format="<<format<<" levels="<<c.intensityLevel<<','<<c.saturationLevel<<" table="<<table<<" colour="<<colour<<" native="<<native.palette.tables[table][colour]<<" original="<<actual<<'\n';throw std::runtime_error("Palette effect value mismatch");}
 }
 const auto produced=12+native.palette.tables.size()*256*native.originalEntryBytes;
 if(produced>requested){require(mode==0||mode==8,"Unexpected original underallocation");++underallocated;}
 for(std::size_t i=produced;i<storage.size();++i)require(storage[i]==0xa5,"Original palette tail guard");
#else
 (void)originalSource;
 if(!admitted){++refusals;return;}
 if((mode==0&&c.count>=5&&c.count%2)||(mode==8&&c.count%2))++underallocated;
#endif
 ++palettes;mutations+=native.source!=input;hash(mode,4);hash(native.originalEntryBytes,4);hash(native.palette.shift,4);hash(native.palette.neutral,4);hash(native.palette.tables.size(),4);
 for(const auto& table:native.palette.tables)for(auto value:table){hash(value,2);++words;}
 for(const auto& colour:native.source)for(auto channel:colour)hash(channel,1);
}
#ifndef MNM_PALETTE_EFFECTS_FIXTURE_NO_MAIN
int main(int argc,char** argv)try{
#ifdef ORIGINAL_REFERENCE
 require(argc==2,"Expected pinned PE");map_image(read(argv[1]));
 const unsigned char prefix[]={0x53,0x8b,0x0d,0x94,0x20,0x5f,0x00};auto* entry=reinterpret_cast<unsigned char*>(0x597880);require(!std::memcmp(entry,prefix,sizeof(prefix)),"Allocator entry mismatch");entry[0]=0xe9;const auto delta=std::uint32_t(reinterpret_cast<std::uintptr_t>(&allocate_palette)-0x597885);std::memcpy(entry+1,&delta,4);
 PrivateFs fs;require(*reinterpret_cast<unsigned*>(6049980)==1926766u,"Directory IAT expected value");global(6049980,reinterpret_cast<std::uintptr_t>(&directory));require(*reinterpret_cast<unsigned*>(6049916)==1926738u,"Profile IAT expected value");global(6049916,reinterpret_cast<std::uintptr_t>(&profile));
#else
 (void)argc;(void)argv;
#endif
 std::array<PaletteRgb,3> inputs{};std::mt19937 random(0x582a80);
 for(unsigned p=0;p<inputs.size();++p)for(unsigned i=0;i<256;++i){inputs[p][i]=p?std::array<std::uint8_t,3>{{std::uint8_t(random()),std::uint8_t(random()),std::uint8_t(random())}}:std::array<std::uint8_t,3>{{std::uint8_t(i),std::uint8_t(i*73+19),std::uint8_t(255-i)}};}
 const std::array<std::array<std::uint8_t,3>,10> special{{{{0,0,0}},{{1,0,0}},{{10,0,0}},{{127,0,0}},{{0,10,0}},{{0,0,10}},{{10,20,30}},{{255,0,0}},{{0,255,0}},{{0,0,255}}}};std::copy(special.begin(),special.end(),inputs[0].begin());
 for(unsigned format=0;format<2;++format)for(unsigned mode:{0u,8u})for(unsigned count=2;count<=256;++count)fixture(inputs[0],mode,{count,1,1,2,2},format);
 const std::array<PaletteShadingConfig,3> lights{{{16,1,1,2,2},{16,7,11,1.3,2.2},{16,9,3,2,1}}};
 for(unsigned mode=1;mode<10;++mode)if(mode!=8)for(variant=0;variant<((mode==4||mode==5||mode==7)?4u:1u);++variant){
  settings={};
  if(variant==1)settings={.25,1,9,.5,2,7,-3,.25,.5};
  if(variant==2)settings={-1,-3,256,-.25,10.9,-2,511,-1.5,1.25};
  if(variant==3)settings={1,0,5,0,1,5,0,200,0};
  for(auto c:lights)for(unsigned format=0;format<2;++format)for(const auto& input:inputs)for(unsigned count:{2u,4u,8u,16u,32u,64u,128u,256u}){c.count=count;fixture(input,mode,c,format);}
 }
 settings={};variant=0;
 for(unsigned mode=1;mode<10;++mode)if(mode!=8)for(unsigned count:{3u,17u,255u})fixture(inputs[0],mode,{count,1,1,2,2},0);
#ifdef ORIGINAL_REFERENCE
 fs.check();
#endif
 std::cout<<"{\"success\":true,\"palettes\":"<<palettes<<",\"words\":"<<words<<",\"refusals\":"<<refusals<<",\"source_mutations\":"<<mutations<<",\"original_underallocated_outputs\":"<<underallocated<<",\"value_hash\":"<<valueHash<<"}\n";
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}


#endif
