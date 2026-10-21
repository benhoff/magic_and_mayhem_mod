// Unchanged original palette builder and whole-cell camera movement. The
// checked allocator dependency has bounded extra space to observe the original
// odd-count underallocation without corrupting a real host heap allocation.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include "palette_shading.hpp"
#include "terrain_camera.hpp"
#include <array>
#include <cstdlib>
#include <limits>

using namespace mnm::reconstruction;
static unsigned requested;
static std::vector<unsigned char> allocation;
static void* allocate(unsigned bytes){
 if(bytes>256*512 || !allocation.empty())throw std::runtime_error("Unexpected allocator request");
 requested=bytes;allocation.assign(bytes+1024,0xa5);return allocation.data();
}
static void require(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
static void put(void* p,unsigned off,std::uint32_t v){std::memcpy(static_cast<unsigned char*>(p)+off,&v,4);}
static std::uint32_t get(const void* p,unsigned off){std::uint32_t v;std::memcpy(&v,static_cast<const unsigned char*>(p)+off,4);return v;}
static std::int32_t signedBits(std::uint32_t bits){std::int32_t v;std::memcpy(&v,&bits,4);return v;}
int main(int argc,char** argv)try{
 require(argc==2,"Expected pinned PE");map_image(read(argv[1]));
 const unsigned char prefix[]={0x53,0x8b,0x0d,0x94,0x20,0x5f,0x00};
 auto* entry=reinterpret_cast<unsigned char*>(0x597880);
 require(!std::memcmp(entry,prefix,sizeof(prefix)),"Allocator entry mismatch");
 entry[0]=0xe9;const auto delta=std::uint32_t(reinterpret_cast<std::uintptr_t>(&allocate)-0x597885);
 std::memcpy(entry+1,&delta,4);
 const unsigned char builder[]={0x55,0x8b,0xec,0x83,0xe4,0xf8};
 require(!std::memcmp(reinterpret_cast<void*>(0x582440),builder,sizeof(builder)),"Palette entry mismatch");
 PaletteRgb rgb{};for(unsigned i=0;i<256;++i)rgb[i]={std::uint8_t(i),std::uint8_t((i*73+19)&255),std::uint8_t(255-i)};
 std::uint64_t words=0;unsigned palettes=0,underallocated=0,cameras=0;
 using Build=unsigned(__attribute__((fastcall)) *)(unsigned,void**,const void*);
 for(unsigned format=0;format<2;++format)for(unsigned count=2;count<=256;++count){
  global(0x6e1f88,format);allocation.clear();void* node=nullptr;
  require(reinterpret_cast<Build>(0x582440)(count,&node,rgb.data())==1 && node==allocation.data(),"Original palette result");
  const auto native=buildShadedPalette(rgb,{count,1,1,2,2},format!=0);
  require(requested==count*512-496 && get(node,0)==0 && get(node,4)==native.shift && get(node,8)==native.neutral,"Original palette header/request");
  const auto bytes=12+native.tables.size()*512;
  for(unsigned t=0;t<native.tables.size();++t)for(unsigned i=0;i<256;++i){
   std::uint16_t original;std::memcpy(&original,allocation.data()+12+t*512+2*i,2);
   if(original!=native.tables[t][i]){
    std::cerr<<"count="<<count<<" format="<<format<<" table="<<t<<" color="<<i<<" native="<<native.tables[t][i]<<" original="<<original<<'\n';
    throw std::runtime_error("Palette output mismatch");
   }
   ++words;
  }
  for(std::size_t i=bytes;i<allocation.size();++i)require(allocation[i]==0xa5,"Palette guard changed");
  if(bytes>requested){require(count>=5 && count%2==1 && bytes-requested==508,"Unexpected original allocation overrun");++underallocated;}
  ++palettes;
 }
 using Clamp=void(__attribute__((thiscall)) *)(void*);
 using Move=void(__attribute__((thiscall)) *)(void*,int,int);
 const unsigned char clampPrefix[]={0x8b,0x51,0x25,0x56,0x57};
 const unsigned char movePrefix[]={0x8b,0x44,0x24,0x04,0x8b,0x51,0x39};
 require(!std::memcmp(reinterpret_cast<void*>(0x4f79b0),clampPrefix,sizeof(clampPrefix)) &&
         !std::memcmp(reinterpret_cast<void*>(0x4f7b30),movePrefix,sizeof(movePrefix)),"Camera entry mismatch");
 const std::array<std::int32_t,11> values{{std::numeric_limits<int>::min(),-1000,-6,-5,-4,0,5,37,133,1000,std::numeric_limits<int>::max()}};
 for(unsigned width:{1u,17u,128u})for(unsigned height:{1u,29u,128u})for(auto x:values)for(auto y:values)for(unsigned op=0;op<3;++op){
  std::array<unsigned char,0x260> receiver;receiver.fill(0xa5);
  std::array<unsigned,5> map{{0,width,height,3,width*height}};
  put(receiver.data(),0x25,reinterpret_cast<std::uintptr_t>(map.data()));
  put(receiver.data(),0x39,std::uint32_t(x));put(receiver.data(),0x3d,std::uint32_t(y));
  put(receiver.data(),0x45,19);put(receiver.data(),0x49,23);
  const auto before=receiver;TerrainCamera native;native.column=x;native.row=y;native.f45=19;native.f49=23;
  if(!op){reinterpret_cast<Clamp>(0x4f79b0)(receiver.data());clampTerrainCamera(native,width,height);}
  else{const auto dx=op==1?-37:std::numeric_limits<int>::max(),dy=op==1?41:std::numeric_limits<int>::min();
   reinterpret_cast<Move>(0x4f7b30)(receiver.data(),dx,dy);moveTerrainCameraCells(native,width,height,dx,dy);
  }
  require(signedBits(get(receiver.data(),0x39))==native.column && signedBits(get(receiver.data(),0x3d))==native.row &&
          signedBits(get(receiver.data(),0x45))==native.f45 && signedBits(get(receiver.data(),0x49))==native.f49,"Camera state mismatch");
  for(unsigned i=0;i<receiver.size();++i)if(!((i>=0x39&&i<0x41)||(i>=0x45&&i<0x4d)))
   require(receiver[i]==before[i],"Unrelated camera state changed");
  ++cameras;
 }
 std::cout<<"{\"success\":true,\"palettes\":"<<palettes<<",\"palette_words\":"<<words
          <<",\"original_underallocated_outputs\":"<<underallocated<<",\"camera_cases\":"<<cameras<<",\"entry_bytes\":{\"004f79b0\":\"8b51255657\",\"004f7b30\":\"8b4424048b5139\"}}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
