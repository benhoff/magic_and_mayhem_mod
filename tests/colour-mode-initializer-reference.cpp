// Execute the original shape initializer only; no native implementation credit.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include <algorithm>
#include <array>
int main(int argc,char** argv)try{
 if(argc!=2)throw std::runtime_error("Expected pinned original PE");
 map_image(read(argv[1]));unsigned cases=0;
 using Initializer=void* (__attribute__((thiscall)) *)(void*,int,int,int,int,int);
 for(unsigned format:{0u,1u})for(const auto& extent:std::array<std::array<int,2>,3>{{{1,1},{2,2},{65,3}}})
  for(const auto& rgb:std::array<std::array<int,3>,3>{{{192,0,0},{255,255,255},{-3,127,511}}}){
   std::array<unsigned char,22> shape;shape.fill(0xa5);global(0x6e1f88,format);
   auto* returned=reinterpret_cast<Initializer>(0x547950)(shape.data(),extent[0],extent[1],rgb[0],rgb[1],rgb[2]);
   auto get=[&](unsigned at,unsigned bytes){unsigned value=0;std::memcpy(&value,shape.data()+at,bytes);return value;};
   if(returned!=shape.data()||get(0,4)!=2||get(4,4)!=unsigned(extent[0])||get(8,4)!=unsigned(extent[1])||get(18,4))throw std::runtime_error("Original initializer mode/extent/plane/return differs");
   for(unsigned channel=0;channel<3;++channel){const auto expected=unsigned(std::clamp(rgb[channel],0,255))>>((channel==1&&!format)?2:3);if(get(12+channel*2,2)!=expected)throw std::runtime_error("Original initializer channel differs");}
   ++cases;
  }
 std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"initializer_mode\":2,\"native_implementation_executed\":false}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
