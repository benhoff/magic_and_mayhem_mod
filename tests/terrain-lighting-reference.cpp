// Isolated original CFG admission, initialization, kernel build and source stamp.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include "terrain_lighting.hpp"
#include <algorithm>
#include <sstream>
static const char* settings[2];
extern "C" unsigned __attribute__((stdcall)) terrain_profile(const char* section,const char* key,const char*,char* output,unsigned size,const char*){
 if(std::strcmp(section,"GLOBAL_OPTIONS"))throw std::runtime_error("Unexpected lighting section");
 unsigned i;if(!std::strcmp(key,"AmbientLight"))i=0;else if(!std::strcmp(key,"LightRamp"))i=1;else throw std::runtime_error("Unexpected lighting key");
 const auto n=settings[i]?std::min<std::size_t>(std::strlen(settings[i]),size?size-1:0):0;if(size){if(n)std::memcpy(output,settings[i],n);output[n]=0;}return n;
}
extern "C" void __attribute__((naked)) controls_window(){asm volatile("push %ebp\n push %ebx\n push %esi\n push %edi\n mov $terrain_profile,%ebp\n mov $0x4e46f9,%eax\n call *%eax\n pop %edi\n pop %esi\n pop %ebx\n pop %ebp\n ret");}
extern "C" void __attribute__((naked)) publish_window(void*){asm volatile("push %ebx\n push %esi\n push %edi\n mov 16(%esp),%ebx\n mov $0x4f2a42,%eax\n call *%eax\n pop %edi\n pop %esi\n pop %ebx\n ret");}
static void put(Bytes& object,unsigned offset,unsigned value){std::memcpy(object.data()+offset,&value,4);}
int main(int argc,char** argv)try{
 if(argc!=9)throw std::runtime_error("Expected PE width height layers ambient/missing ramp/missing sources prefix");map_image(read(argv[1]));
 auto* end=reinterpret_cast<unsigned char*>(0x4e47ef);const unsigned char expected[]={0x68,0x80,0x27,0x6e,0};if(std::memcmp(end,expected,5))throw std::runtime_error("Config window boundary mismatch");*end=0xc3;
 const auto width=unsigned(std::stoul(argv[2])),height=unsigned(std::stoul(argv[3])),layers=unsigned(std::stoul(argv[4]));
 settings[0]=std::strcmp(argv[5],"missing")?argv[5]:nullptr;settings[1]=std::strcmp(argv[6],"missing")?argv[6]:nullptr;
 mnm::reconstruction::TerrainLightingConfig initial;initial.ambient=-108;initial.ramp=-16;global(0x5e18e8,unsigned(initial.ambient));global(0x5e18ec,unsigned(initial.ramp));
 const auto config=mnm::reconstruction::applyTerrainLighting(initial,settings[0]?std::optional<int>(std::stoi(settings[0])):std::nullopt,settings[1]?std::optional<int>(std::stoi(settings[1])):std::nullopt);controls_window();
 if(std::int32_t(*reinterpret_cast<unsigned*>(0x5e18e8))!=config.ambient || std::int32_t(*reinterpret_cast<unsigned*>(0x5e18ec))!=config.ramp)throw std::runtime_error("Control admission mismatch");
 mnm::reconstruction::TerrainLightField native(width,height,layers,config);const auto count=native.buffers()[0].size();Bytes object(0x66c0);std::array<Bytes,5> buffers;
 put(object,4,width);put(object,8,height);put(object,12,layers);put(object,16,width*height);put(object,0x7e0,count);
 for(unsigned i=0;i<5;++i){buffers[i].assign(count+32,0xa5);put(object,0x7cc+4*i,reinterpret_cast<std::uintptr_t>(buffers[i].data()+16));}
 for(unsigned y=0;y<height;++y)put(object,0x64b2+4*y,y*width);
 for(unsigned z=0;z<(layers+1)/2;++z)put(object,0x6432+4*z,z*width*height);
 using Init=void(__attribute__((thiscall)) *)(void*);using Stamp=void(__attribute__((thiscall)) *)(void*,unsigned,unsigned,unsigned,unsigned);
 if(*reinterpret_cast<unsigned char*>(0x4ecdd0)!=0x53 || *reinterpret_cast<unsigned char*>(0x4f2140)!=0x81)throw std::runtime_error("Original entry mismatch");
 reinterpret_cast<Init>(0x4ecdd0)(object.data());
 const auto compare=[&]{for(unsigned i=0;i<5;++i){if(std::memcmp(buffers[i].data()+16,native.buffers()[i].data(),count))throw std::runtime_error("Light buffer differs: "+std::to_string(i));for(unsigned j=0;j<16;++j)if(buffers[i][j]!=0xa5 || buffers[i][count+16+j]!=0xa5)throw std::runtime_error("Original buffer guard changed");}};
 compare();std::size_t words=0;
 for(unsigned n=2;n<=17;++n){const unsigned index=2*n-1,ptr=u32(object,0x637d+4*index);if(object[0x6404+index]!=n || std::memcmp(reinterpret_cast<void*>(ptr),native.kernels()[n].data(),n*n*n)){for(unsigned i=0;i<n*n*n;++i)if(reinterpret_cast<std::int8_t*>(ptr)[i]!=native.kernels()[n][i])std::cerr<<"Mismatch "<<n<<" xyz "<<i%n<<','<<(i/n)%n<<','<<i/(n*n)<<" original "<<int(reinterpret_cast<std::int8_t*>(ptr)[i])<<" native "<<int(native.kernels()[n][i])<<'\n';throw std::runtime_error("Kernel mismatch size "+std::to_string(n));}words+=n*n*n;}
 std::ofstream kernelFile(std::string(argv[8])+".kernels",std::ios::binary);for(unsigned n=2;n<=17;++n){const auto ptr=u32(object,0x637d+4*(2*n-1));kernelFile.write(reinterpret_cast<const char*>(ptr),n*n*n);}if(!kernelFile)throw std::runtime_error("Kernel output failed");
 std::istringstream sources(argv[7]);std::string source;unsigned stamps=0;
 while(std::getline(sources,source,';')){if(source=="none")continue;std::replace(source.begin(),source.end(),',',' ');std::istringstream fields(source);mnm::reconstruction::TerrainLightSource s;if(!(fields>>s.column>>s.row>>s.layer>>s.size))throw std::runtime_error("Source syntax");native.stamp(s);reinterpret_cast<Stamp>(0x4f2140)(object.data(),s.column,s.row,s.layer,2*s.size-1);compare();++stamps;}
 native.publish();auto* publicationEnd=reinterpret_cast<unsigned char*>(0x4f2a62);const unsigned char publicationExpected[]={0xc7,0x83,0x2e,0x64,0,0};if(std::memcmp(publicationEnd,publicationExpected,6))throw std::runtime_error("Publication boundary mismatch");*publicationEnd=0xc3;publish_window(object.data());compare();
 std::ofstream file(std::string(argv[8])+".field",std::ios::binary);file.write(reinterpret_cast<const char*>(buffers[0].data()+16),count);if(!file)throw std::runtime_error("Field output failed");
 std::ofstream bufferFile(std::string(argv[8])+".buffers",std::ios::binary);for(const auto& buffer:buffers)bufferFile.write(reinterpret_cast<const char*>(buffer.data()+16),count);if(!bufferFile)throw std::runtime_error("Buffers output failed");
 std::cout<<"{\"kernel_bytes\":"<<words<<",\"field_bytes\":"<<count<<",\"stamps\":"<<stamps<<",\"ambient\":"<<config.ambient<<",\"ramp\":"<<config.ramp<<"}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
