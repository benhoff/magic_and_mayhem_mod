// Selected original preference admission and terrain loader argument dispatch.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include "palette_shading.hpp"
#include <algorithm>
static const char* setting;
extern "C" unsigned __attribute__((stdcall)) preference_profile(const char* section,const char* key,const char*,char* output,unsigned size,const char*){
 if(std::strcmp(section,"VIDEO") || std::strcmp(key,"TerrainLightLevels"))throw std::runtime_error("Unexpected preference read");
 const auto n=setting?std::min<std::size_t>(std::strlen(setting),size?size-1:0):0;
 if(size){if(n)std::memcpy(output,setting,n);output[n]=0;}return n;
}
static unsigned observedCount;static bool observed;
static int __attribute__((thiscall)) loader(void* receiver,const char* path,unsigned count){
 if(receiver!=reinterpret_cast<void*>(0x6de558) || std::strcmp(path,"fixture\\Terrain.spr"))throw std::runtime_error("Unexpected terrain loader arguments");
 observedCount=count;observed=true;return 1;
}
extern "C" void __attribute__((naked)) preference_window(){asm volatile("push %ebp\n push %ebx\n push %esi\n push %edi\n mov $0x6de6c8,%ebp\n mov $preference_profile,%ebx\n mov $2,%edi\n mov $0x54c269,%eax\n call *%eax\n pop %edi\n pop %esi\n pop %ebx\n pop %ebp\n ret");}
extern "C" void __attribute__((naked)) dispatch_window(){asm volatile("push %ebp\n push %ebx\n push %esi\n push %edi\n xor %edx,%edx\n mov $0x46e6af,%eax\n call *%eax\n pop %edi\n pop %esi\n pop %ebx\n pop %ebp\n ret");}
static void boundary(std::uintptr_t address,const Bytes& expected){auto* target=reinterpret_cast<unsigned char*>(address);if(std::memcmp(target,expected.data(),expected.size()))throw std::runtime_error("Original boundary mismatch");*target=0xc3;}
int main(int argc,char** argv)try{
 if(argc!=4)throw std::runtime_error("Expected PE, initial count, setting or missing");map_image(read(argv[1]));
 boundary(0x54c2de,{0x68,0x8c,0x22,0x6f,0});boundary(0x46e6cb,{0x85,0xc0});
 auto* target=reinterpret_cast<unsigned char*>(0x57d310);const unsigned char expected[]={0x83,0xec,0x1c,0x53,0x55,0x56};
 if(std::memcmp(target,expected,sizeof(expected)))throw std::runtime_error("Loader entry mismatch");
 target[0]=0xe9;const auto relative=std::uint32_t(reinterpret_cast<std::uintptr_t>(&loader)-(0x57d310+5));std::memcpy(target+1,&relative,4);
 const auto initial=unsigned(std::stoul(argv[2]));global(0x6de6e5,initial);setting=std::strcmp(argv[3],"missing")?argv[3]:nullptr;
 mnm::reconstruction::PaletteShadingConfig config;config.count=initial;config=mnm::reconstruction::applyTerrainPalettePreference(config,setting?std::optional<int>(std::stoi(setting)):std::nullopt);
 preference_window();if(*reinterpret_cast<unsigned*>(0x6de6e5)!=config.count)throw std::runtime_error("Preference admission differs");
 std::strcpy(reinterpret_cast<char*>(0x5fc360),"fixture\\Terrain.spr");dispatch_window();if(!observed || observedCount!=config.count)throw std::runtime_error("Terrain dispatch differs");
 std::cout<<config.count<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
