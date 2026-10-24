// Execute only the original four GLOBAL_OPTIONS reads, admissions and setters.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include "palette_shading.hpp"
#include <algorithm>
#include <optional>
#include <cmath>
static const char* fields[4];
extern "C" unsigned __attribute__((stdcall)) lighting_profile(const char* section,const char* key,const char*,char* output,unsigned size,const char*){
 if(std::strcmp(section,"GLOBAL_OPTIONS"))throw std::runtime_error("Unexpected section");
 const char* keys[]={"LightCurve","ColourFactor","LightPower","ColourPower"};
 for(unsigned i=0;i<4;++i)if(!std::strcmp(key,keys[i])){if(!fields[i]){if(size)output[0]=0;return 0;}const auto n=std::min<std::size_t>(std::strlen(fields[i]),size?size-1:0);if(size){std::memcpy(output,fields[i],n);output[n]=0;}return n;}
 throw std::runtime_error("Unexpected lighting key");
}
extern "C" void __attribute__((naked)) lighting_window(){
 asm volatile("push %ebp\n push %ebx\n push %esi\n push %edi\n sub $48,%esp\n mov $lighting_profile,%ebp\n mov $1,%esi\n mov $0x4e4f75,%eax\n call *%eax\n add $48,%esp\n pop %edi\n pop %esi\n pop %ebx\n pop %ebp\n ret");
}
int main(int argc,char** argv)try{
 if(argc!=2 && argc!=6)throw std::runtime_error("PE and four settings (or missing) expected");map_image(read(argv[1]));
 auto* end=reinterpret_cast<unsigned char*>(0x4e5173);const unsigned char expected[]={0x68,0x80,0x27,0x6e,0x00};
 if(std::memcmp(end,expected,5))throw std::runtime_error("Lighting window boundary mismatch");*end=0xc3;
 const auto compare=[](const std::array<const char*,4>& values){
  mnm::reconstruction::PaletteLightingOverrides overrides;
  std::optional<int>* ints[]={&overrides.lightCurve,&overrides.colourFactor};std::optional<double>* doubles[]={&overrides.lightPower,&overrides.colourPower};
  for(unsigned i=0;i<4;++i){fields[i]=values[i];if(fields[i]){if(i<2)*ints[i]=std::stoi(fields[i]);else *doubles[i-2]=std::stod(fields[i]);}}
  global(0x5f18d0,1);global(0x5f18d4,1);const double power=2.;
  std::memcpy(reinterpret_cast<void*>(0x5f18d8),&power,8);std::memcpy(reinterpret_cast<void*>(0x5f18e0),&power,8);
  const auto native=mnm::reconstruction::applyPaletteLighting({},overrides);lighting_window();
  const int curve=*reinterpret_cast<int*>(0x5f18d0),factor=*reinterpret_cast<int*>(0x5f18d4);
  const double light=*reinterpret_cast<double*>(0x5f18d8),colour=*reinterpret_cast<double*>(0x5f18e0);
  if(curve!=native.intensityLevel || factor!=native.saturationLevel || std::memcmp(&light,&native.intensityPower,8) || std::memcmp(&colour,&native.saturationPower,8))throw std::runtime_error("Original lighting admission differs");
  return native;
 };
 if(argc==2){
  unsigned cases=0;
  const std::array<const char*,4> missing{{nullptr,nullptr,nullptr,nullptr}};compare(missing);++cases;
  for(unsigned field=0;field<4;++field){
   const std::vector<const char*> integers={"-2147483648","-1","0","1","2","999","1000","1001","2147483647","+50"," 17"};
   const std::vector<const char*> powers={"-1","-0","0","0.5","1","2.2","999.5","1000","1001","1e300","+2.2"," 2"};
   for(auto value:field<2?integers:powers){auto values=missing;values[field]=value;compare(values);++cases;}
  }
  for(const char* level:{"-1","1","50","1001"})for(const char* power:{"-1","0","2.2","1001"}){compare({level,level,power,power});++cases;}
  std::cout<<"{\"all_match\":true,\"cases\":"<<cases<<",\"fields\":4,\"native_strict_malformed_policy_compared\":false}\n";
  return 0;
 }
 const auto native=compare({std::strcmp(argv[2],"missing")?argv[2]:nullptr,std::strcmp(argv[3],"missing")?argv[3]:nullptr,std::strcmp(argv[4],"missing")?argv[4]:nullptr,std::strcmp(argv[5],"missing")?argv[5]:nullptr});
 std::cout<<native.intensityLevel<<' '<<native.saturationLevel<<' '<<native.intensityPower<<' '<<native.saturationPower<<'\n';return 0;

}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
