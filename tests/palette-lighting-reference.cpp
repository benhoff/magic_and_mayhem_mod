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
 if(argc!=6)throw std::runtime_error("PE and four settings (or missing) expected");map_image(read(argv[1]));
 auto* end=reinterpret_cast<unsigned char*>(0x4e5173);const unsigned char expected[]={0x68,0x80,0x27,0x6e,0x00};
 if(std::memcmp(end,expected,5))throw std::runtime_error("Lighting window boundary mismatch");*end=0xc3;
 mnm::reconstruction::PaletteLightingOverrides overrides;
 std::optional<int>* ints[]={&overrides.lightCurve,&overrides.colourFactor};std::optional<double>* doubles[]={&overrides.lightPower,&overrides.colourPower};
 for(unsigned i=0;i<4;++i){fields[i]=std::strcmp(argv[i+2],"missing")?argv[i+2]:nullptr;if(fields[i]){if(i<2)*ints[i]=std::stoi(fields[i]);else *doubles[i-2]=std::stod(fields[i]);}}
 const auto native=mnm::reconstruction::applyPaletteLighting({},overrides);lighting_window();
 const int curve=*reinterpret_cast<int*>(0x5f18d0),factor=*reinterpret_cast<int*>(0x5f18d4);
 const double light=*reinterpret_cast<double*>(0x5f18d8),colour=*reinterpret_cast<double*>(0x5f18e0);
 if(curve!=native.intensityLevel || factor!=native.saturationLevel || std::memcmp(&light,&native.intensityPower,8) || std::memcmp(&colour,&native.saturationPower,8))throw std::runtime_error("Original lighting admission differs");
 std::cout<<curve<<' '<<factor<<' '<<light<<' '<<colour<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
