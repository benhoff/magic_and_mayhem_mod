// Whole unchanged original initializer and both index-one stamping routines.
#ifdef ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
#include "terrain_lighting.hpp"
#include <array>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace mnm::reconstruction;
#ifdef ORIGINAL_REFERENCE
static void put(Bytes& b,unsigned offset,unsigned value){std::memcpy(b.data()+offset,&value,4);}
#endif
int main(int argc,char** argv)try{
#ifdef ORIGINAL_REFERENCE
 if(argc!=3)throw std::runtime_error("Expected PE and output");map_image(read(argv[1]));const char* output=argv[2];
 if(*reinterpret_cast<unsigned char*>(0x4ecdd0)!=0x53 || *reinterpret_cast<unsigned char*>(0x4f2140)!=0x81)throw std::runtime_error("Original light entry mismatch");
 using Init=void(__attribute__((thiscall)) *)(void*);
 using Stamp=void(__attribute__((thiscall)) *)(void*,unsigned,unsigned,unsigned,unsigned);
#else
 if(argc!=2)throw std::runtime_error("Expected output");const char* output=argv[1];
#endif
 std::ofstream file(output,std::ios::binary);std::uint64_t bytes=0;unsigned stamps=0;
 for(int inherited=-128;inherited<=127;++inherited){
  const unsigned w=1+unsigned(inherited+128)%19,h=1+unsigned(inherited+128)*7%19,layers=1+unsigned(inherited+128)%7;
  TerrainLightField field(w,h,layers,{-50,7,inherited});
#ifdef ORIGINAL_REFERENCE
  Bytes object(0x66c0);std::array<Bytes,5> buffers;const auto count=field.buffers()[0].size();
  put(object,4,w);put(object,8,h);put(object,12,layers);put(object,16,w*h);put(object,0x7e0,count);object[0x6380]=static_cast<unsigned char>(inherited);
  for(unsigned i=0;i<5;++i){buffers[i].assign(count+32,0xa5);put(object,0x7cc+4*i,reinterpret_cast<std::uintptr_t>(buffers[i].data()+16));}
  for(unsigned y=0;y<h;++y)put(object,0x64b2+4*y,y*w);
  for(unsigned z=0;z<(layers+1)/2;++z)put(object,0x6432+4*z,z*w*h);
  global(0x5e18e8,unsigned(-50));global(0x5e18ec,7);reinterpret_cast<Init>(0x4ecdd0)(object.data());
  if(object[0x6380]!=static_cast<unsigned char>(inherited)||object[0x6405]!=1||u32(object,0x6381)!=reinterpret_cast<std::uintptr_t>(object.data()+0x6380))throw std::runtime_error("Single-cell kernel inheritance differs");
#endif
  const auto compare=[&]{for(unsigned i=0;i<5;++i){const auto& data=field.buffers()[i];
#ifdef ORIGINAL_REFERENCE
   if(std::memcmp(buffers[i].data()+16,data.data(),data.size()))throw std::runtime_error("Single-cell buffer mismatch: "+std::to_string(inherited)+" buffer "+std::to_string(i));
   for(unsigned j=0;j<16;++j)if(buffers[i][j]!=0xa5||buffers[i][data.size()+16+j]!=0xa5)throw std::runtime_error("Single-cell buffer guard changed");
#endif
   file.write(reinterpret_cast<const char*>(data.data()),data.size());bytes+=data.size();
  }};
  compare();
  for(unsigned n=0;n<3;++n){const unsigned x=n==1?w-1:0,y=n==1?h-1:0,z=n==1?layers-1:layers/2;
   field.stamp({x,y,z,1});
#ifdef ORIGINAL_REFERENCE
   reinterpret_cast<Stamp>(0x4f2140)(object.data(),x,y,z,1);
#endif
   compare();++stamps;field.stampStatic({x,y,z,1});
#ifdef ORIGINAL_REFERENCE
   reinterpret_cast<Stamp>(0x4f1c60)(object.data(),x,y,z,1);
#endif
   compare();++stamps;
  }
 }
 if(!file)throw std::runtime_error("Single-cell output failed");
 std::cout<<"{\"inherited_bytes\":256,\"stamps\":"<<stamps<<",\"buffer_bytes\":"<<bytes<<"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
