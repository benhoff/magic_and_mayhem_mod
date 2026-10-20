#define main reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include <set>
int main(int argc,char** argv)try{
 if(argc!=2)throw std::runtime_error("Expected pinned PE");
 map_image(read(argv[1]));
 std::vector<std::uint16_t> pixels(37*19);global(0x658174,reinterpret_cast<std::uintptr_t>(pixels.data()));global(0x6a2dc8,37);global(0x6e1f88,0);global(0x6e0008,0);global(0x6cbb6c,0);global(0x6a49b8,37);global(0x656618,19);
 using Method=unsigned (__attribute__((thiscall)) *)(void*,int,int);std::set<unsigned> values;unsigned cases=0;
 for(unsigned increment:{0u,1u,12u,31u,63u,65520u,65535u})for(unsigned w:{1u,2u,17u,64u})for(unsigned h:{1u,3u,21u})for(int y:{-32,-3,-1,0,1,16,18,19,32})for(int x:{-65,-17,-2,-1,0,1,18,35,36,37,65}){
  std::uint32_t object[26]={};object[13]=w;object[14]=h;object[15]=increment;object[16]=increment;object[17]=increment;
  for(unsigned i=0;i<pixels.size();++i)pixels[i]=(i*97+13)&65535;
  const auto result=reinterpret_cast<Method>(0x54aaa0)(object,x,y);values.insert(result&65535);++cases;
 }
 std::cout<<"{\"cases\":"<<cases<<",\"low16_values\":[";bool first=true;for(auto v:values){if(!first)std::cout<<',';first=false;std::cout<<v;}std::cout<<"]}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
