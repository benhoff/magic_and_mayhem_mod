// Reuse the pinned builder dependencies; execute the whole original dispatcher.
#define MNM_PALETTE_EFFECTS_FIXTURE_NO_MAIN
#include "palette-effects-reference.cpp"
#undef MNM_PALETTE_EFFECTS_FIXTURE_NO_MAIN
#include "palette_effect_chain.hpp"
#include <memory>
#ifdef ORIGINAL_REFERENCE
static std::vector<std::unique_ptr<Bytes>> chainAllocations;
static std::vector<unsigned> chainRequests;
static unsigned failAfter=~0u;
static void* chain_allocate(unsigned n){
 require(n<=256*1024,"Chain allocator extent");if(chainAllocations.size()==failAfter)return nullptr;
 chainAllocations.push_back(std::make_unique<Bytes>(n+2048,0xa5));chainRequests.push_back(n);return chainAllocations.back()->data();
}
static unsigned word(const void* p,unsigned off){unsigned n;std::memcpy(&n,static_cast<const unsigned char*>(p)+off,4);return n;}
static void compareChains(const PaletteEffectChain& native,const std::array<unsigned,12>& receiver,const std::vector<PaletteRgb>& source){
 require(native.paletteCount()==source.size(),"Palette chain denominator");
 for(std::size_t p=0;p<source.size();++p){
  require(native.source(p)==source[p],"Palette chain source propagation");const void* node=reinterpret_cast<void*>(receiver[4+p]);
  for(std::size_t ordinal=0;ordinal<native.ordinals();++ordinal){
   require(node!=nullptr,"Missing original effect ordinal");const auto& set=native.tableSet(ordinal,p);require(word(node,4)==set.shift&&word(node,8)==set.neutral,"Chain table header");
   const auto stride=native.originalEntryBytes(ordinal);for(std::size_t t=0;t<set.tables.size();++t)for(unsigned c=0;c<256;++c){unsigned actual=0;std::memcpy(&actual,static_cast<const unsigned char*>(node)+12+(t*256+c)*stride,stride);require(actual==set.tables[t][c],"Original chain table value");}
   std::size_t allocation=0;while(allocation<chainAllocations.size()&&chainAllocations[allocation]->data()!=node)++allocation;require(allocation<chainAllocations.size(),"Original chain allocator identity");const auto produced=12+set.tables.size()*256*stride;for(std::size_t i=produced;i<chainAllocations[allocation]->size();++i)require(chainAllocations[allocation]->at(i)==0xa5,"Original chain guard");
   node=reinterpret_cast<void*>(word(node,0));
  }
  require(node==nullptr,"Extra original effect ordinal");
 }
}
#endif
int main(int argc,char** argv)try{
#ifdef ORIGINAL_REFERENCE
 require(argc==2,"Expected pinned image");map_image(read(argv[1]));const unsigned char prefix[]={0x53,0x8b,0x0d,0x94,0x20,0x5f,0x00};auto* entry=reinterpret_cast<unsigned char*>(0x597880);require(!std::memcmp(entry,prefix,sizeof(prefix)),"Allocator dependency prefix");entry[0]=0xe9;const auto delta=std::uint32_t(reinterpret_cast<std::uintptr_t>(&chain_allocate)-0x597885);std::memcpy(entry+1,&delta,4);
 PrivateFs fs;require(*reinterpret_cast<unsigned*>(6049980)==1926766u&&*reinterpret_cast<unsigned*>(6049916)==1926738u,"Original IAT values");global(6049980,reinterpret_cast<std::uintptr_t>(&directory));global(6049916,reinterpret_cast<std::uintptr_t>(&profile));
 using Append=unsigned(__attribute__((thiscall)) *)(void*,unsigned,unsigned);
#else
 (void)argc;(void)argv;
#endif
 std::mt19937 random(0x57d690);unsigned sessions=0,appends=0,noops=0,rejected=0;words=0;valueHash=14695981039346656037ULL;
 const std::array<std::pair<unsigned,unsigned>,12> modes{{{0,16},{5,16},{2,4},{7,32},{8,17},{1,8},{9,64},{6,2},{4,16},{3,128},{99,0},{0,3}}};
 for(unsigned format=0;format<2;++format)for(unsigned pals=0;pals<=4;++pals)for(unsigned pattern=0;pattern<4;++pattern){
  std::vector<PaletteRgb> input(pals);for(auto& pal:input)for(unsigned c=0;c<256;++c)pal[c]={{std::uint8_t(random()),std::uint8_t(random()),std::uint8_t(random())}};
  for(auto& pal:input){pal[0]={0,0,0};pal[1]={255,255,0};pal[2]={255,0,0};}
  PaletteEffectChain native(input);auto originalSource=input;settings={};variant=pattern;
  if(pattern==1)settings={.25,1,9,.5,2,7,-3,.25,.5};if(pattern==2)settings={-1,-3,256,-.25,10.9,-2,511,-1.5,1.25};if(pattern==3)settings={1,0,5,0,1,5,0,200,0};
#ifdef ORIGINAL_REFERENCE
  chainAllocations.clear();chainRequests.clear();std::array<unsigned,12> receiver{};std::array<unsigned,6> header{};header[4]=pals;receiver[0]=reinterpret_cast<std::uintptr_t>(header.data());receiver[1]=reinterpret_cast<std::uintptr_t>(originalSource.data());
#endif
  for(const auto [mode,count]:modes){PaletteShadingConfig c{count,7,11,1.3,2.2};const auto result=native.append(mode,c,settings,format!=0);if(result)++appends;else ++noops;
#ifdef ORIGINAL_REFERENCE
   lighting(c);global(0x6e1f88,format);require(reinterpret_cast<Append>(0x57d690)(receiver.data(),mode,count)==1,"Original append result");fs.check();compareChains(native,receiver,originalSource);
#endif
  }
  bool refused=false;try{native.append(2,{17,1,1,2,2});}catch(const std::invalid_argument&){refused=true;}
  require(refused==bool(pals),"Native invalid count/no-palette behavior");rejected+=refused;
#ifdef ORIGINAL_REFERENCE
  require(reinterpret_cast<Append>(0x57d690)(receiver.data(),2,17)==unsigned(!pals),"Original invalid-count result");compareChains(native,receiver,originalSource);
#endif
  for(std::size_t ordinal=0;ordinal<native.ordinals();++ordinal){hash(native.mode(ordinal),4);hash(native.originalEntryBytes(ordinal),4);for(std::size_t p=0;p<native.paletteCount();++p){const auto& set=native.tableSet(ordinal,p);hash(set.shift,4);hash(set.neutral,4);for(const auto& table:set.tables)for(auto value:table){hash(value,2);++words;}}}
  for(std::size_t p=0;p<native.paletteCount();++p)for(const auto& colour:native.source(p))for(auto channel:colour)hash(channel,1);++sessions;
 }
#ifdef ORIGINAL_REFERENCE
 fs.check();
#endif
 std::cout<<"{\"success\":true,\"sessions\":"<<sessions<<",\"appends\":"<<appends<<",\"noops\":"<<noops<<",\"refusals\":"<<rejected<<",\"words\":"<<words<<",\"value_hash\":"<<valueHash<<"}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
