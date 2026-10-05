// Offline PE32 oracle. The builder and drawing code remain unchanged; only
// its checked private allocator dependency is replaced with host allocation.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include "palette_shading.hpp"
#include <cstdlib>
static void* allocation=nullptr;
static void* allocate(unsigned size){if(allocation)throw std::runtime_error("Unexpected nested allocation");return allocation=std::calloc(1,size);}
int main(int argc,char** argv)try{
 if(argc!=6)throw std::runtime_error("PE SPR count queue output expected");
 map_image(read(argv[1]));
 const unsigned char expected[]={0x53,0x8b,0x0d,0x94,0x20,0x5f,0x00};
 auto* allocator=reinterpret_cast<unsigned char*>(0x597880);
 if(std::memcmp(allocator,expected,sizeof(expected)))throw std::runtime_error("Allocator entry mismatch");
 allocator[0]=0xe9;const auto relative=std::uint32_t(reinterpret_cast<std::uintptr_t>(&allocate)-0x597885);std::memcpy(allocator+1,&relative,4);
 auto sprite=read(argv[2]);const unsigned count=std::stoul(argv[3]);
 if(u32(sprite,8)!=4 || count<2 || count>256 || (count&(count-1)))throw std::runtime_error("SPR/count bounds");
 const auto pals=u32(sprite,16),frames=u32(sprite,12),table=24+pals*768,base=table+frames*4;
 if(!pals || pals>4)throw std::runtime_error("Indexed palettes required");
 std::vector<void*> nodes;std::ofstream colours(std::string(argv[5])+".palettes",std::ios::binary);
 using Build=unsigned (__attribute__((fastcall)) *)(unsigned,void**,const void*);
 for(unsigned pi=0;pi<pals;++pi){
  mnm::reconstruction::PaletteRgb rgb{};std::memcpy(rgb.data(),sprite.data()+24+pi*768,768);
  for(unsigned format=0;format<2;++format){global(0x6e1f88,format);allocation=nullptr;void* node=nullptr;
   if(!reinterpret_cast<Build>(0x582440)(count,&node,rgb.data()) || !node)throw std::runtime_error("Original palette construction failed");
   const auto native=mnm::reconstruction::buildShadedPalette(rgb,{count,1,1,2,2},format);
   auto* header=static_cast<std::uint32_t*>(node);
   if(header[0] || header[1]!=native.shift || header[2]!=native.neutral)throw std::runtime_error("Palette header differs");
   for(unsigned t=0;t<native.tables.size();++t)for(unsigned i=0;i<256;++i)
    if(native.tables[t][i]!=reinterpret_cast<std::uint16_t*>(header+3)[t*256+i]){std::cerr<<"palette "<<pi<<" count "<<count<<" format "<<format<<" table "<<t<<" colour "<<i<<" native "<<native.tables[t][i]<<" original "<<reinterpret_cast<std::uint16_t*>(header+3)[t*256+i]<<'\n';throw std::runtime_error("Palette bytes differ");}
   colours.write(reinterpret_cast<const char*>(header+3),native.tables.size()*512);
   if(format)std::free(node);else nodes.push_back(node);
  }
 }
 global(0x6e1f88,0);
 for(unsigned i=0;i<frames;++i){const auto offset=base+u32(sprite,table+i*4);const auto pi=u32(sprite,offset+28);if(pi>=pals)throw std::runtime_error("Frame palette bounds");const auto ptr=reinterpret_cast<std::uintptr_t>(nodes[pi]);std::memcpy(sprite.data()+offset+28,&ptr,4);}
 auto queue=read(argv[4]);if(queue.size()%16 || queue.size()>16384*3*16)throw std::runtime_error("Queue bounds");
 std::vector<std::uint16_t> pixels(512*256,0x2124);global(0x658174,reinterpret_cast<std::uintptr_t>(pixels.data()));global(0x6a2dc8,512);
 global(0x6e0008,0);global(0x6cbb6c,0);global(0x6a49b8,512);global(0x656618,256);global(0x6e1f68,0);
 using Draw=unsigned (__attribute__((fastcall)) *)(void*,int,int,int,int);
 for(unsigned at=0;at<queue.size();at+=16){const auto index=u32(queue,at);if(index>=frames)throw std::runtime_error("Frame index");const auto offset=base+u32(sprite,table+index*4);const auto shade=std::int32_t(u32(queue,at+12));
  // Native checked selection also prevents the original from reading outside tables.
  mnm::reconstruction::ShadedPalette shape;shape.shift=static_cast<std::uint32_t*>(nodes[0])[1];shape.neutral=(count-1)/2;shape.tables.resize(count-1);shape.tableIndex(shade);
  reinterpret_cast<Draw>(0x57e1b0)(sprite.data()+offset,std::int32_t(u32(queue,at+4)),std::int32_t(u32(queue,at+8)),shade,0);
 }
 save(argv[5],pixels);for(auto* node:nodes)std::free(node);std::cout<<"Palette bytes and original scene completed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
