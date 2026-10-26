// Production capture inputs and independent unchanged-PE32 outer-updater outputs.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include "../protocols/include/mnm/terrain_light_v1.h"
#include <algorithm>
#include <array>
using U=std::uint32_t;
constexpr U Map=0x6c5490,Guard=16,Capacity=6;
extern "C" unsigned char* singleCellCapture(U,U,U*);
extern "C" void singleCellResult(unsigned char*,U,U);
extern "C" void singleCellFree(unsigned char*);
static std::vector<std::pair<U,U>> regions;
extern "C" U singleCellQuery(U p,U* info,U bytes){
 if(bytes!=28)return 0;
 for(const auto& region:regions)if(p>=region.first && p-region.first<region.second){
  std::fill(info,info+7,0);info[0]=region.first;info[3]=region.second;
  info[4]=0x1000;info[5]=4;return 28;
 }
 return 0;
}
static void put(Bytes& b,U at,U value){std::memcpy(b.data()+at,&value,4);}
static U address(const void* p){return U(reinterpret_cast<std::uintptr_t>(p));}
static U get(U p){U value;std::memcpy(&value,reinterpret_cast<const void*>(p),4);return value;}
static void protectCode(const Bytes& pe){
 const U head=u32(pe,0x3c),opt=head+24;
 const unsigned count=pe[head+6]|(pe[head+7]<<8),size=pe[head+20]|(pe[head+21]<<8);
 for(unsigned i=0;i<count;++i){const U at=opt+size+i*40;
  if(u32(pe,at+36)&0x20000000){const U start=0x400000+u32(pe,at+12),n=(std::max(u32(pe,at+8),u32(pe,at+16))+4095)&~4095u;
   if(mprotect(reinterpret_cast<void*>(start),n,PROT_READ|PROT_EXEC))throw std::runtime_error("Original RX protection failed");
  }
 }
}
int main(int argc,char** argv)try{
 if(argc!=3)throw std::runtime_error("Expected pinned PE and new bundle");
 const auto pe=read(argv[1]);map_image(pe);protectCode(pe);
 const auto imageBytes=u32(pe,u32(pe,0x3c)+24+56);
 using Call=void(__attribute__((thiscall))*)(void*);
 using Lookup=U(__attribute__((thiscall))*)(void*,U);
 const auto initialize=reinterpret_cast<Call>(0x4ecdd0),update=reinterpret_cast<Call>(0x4f27a0);
 std::ofstream output(argv[2],std::ios::binary);output.write("MNMSZ101",8);
 U records=0;output.write(reinterpret_cast<const char*>(&records),4);
 std::uint64_t comparedBytes=0;U initializations=0;
 const std::array<std::array<U,3>,4> dimensions{{{1,1,1},{2,3,4},{17,19,5},{19,17,6}}};
 const std::array<std::array<int,2>,4> configurations{{{-127,-127},{-50,7},{0,-16},{127,127}}};
 for(int inherited=-128;inherited<=127;++inherited)for(U shape=0;shape<dimensions.size();++shape){
  const auto d=dimensions[shape];const U w=d[0],h=d[1],layers=d[2],count=w*h*((layers+1)/2);
  std::memset(reinterpret_cast<void*>(Map),0,0x66c0);
  global(Map+4,w);global(Map+8,h);global(Map+12,layers);global(Map+16,w*h);global(Map+0x7e0,count);
  std::array<Bytes,5> buffers;Bytes objects(Capacity*0x22e + 2*Guard,0xa5);
  regions={{0x400000,imageBytes},{address(objects.data()),U(objects.size())}};
  for(U i=0;i<5;++i){buffers[i].assign(count+2*Guard,0xa5);
   regions.emplace_back(address(buffers[i].data()),U(buffers[i].size()));
   global(Map+0x7cc+4*i,address(buffers[i].data()+Guard));
  }
  for(U y=0;y<h;++y)global(Map+0x64b2+4*y,y*w);
  for(U z=0;z<(layers+1)/2;++z)global(Map+0x6432+4*z,z*w*h);
  global(0x5e18e8,U(configurations[shape][0]));global(0x5e18ec,U(configurations[shape][1]));global(0x5e18f0,U(-127));
  *reinterpret_cast<unsigned char*>(Map+0x6380)=static_cast<unsigned char>(inherited);
  for(U repeat=0;repeat<2;++repeat){
   initialize(reinterpret_cast<void*>(Map));++initializations;
   if(*reinterpret_cast<unsigned char*>(Map+0x6380)!=static_cast<unsigned char>(inherited) ||
      get(Map+0x6381)!=Map+0x6380 || *reinterpret_cast<unsigned char*>(Map+0x6405)!=1)
    throw std::runtime_error("Size-one inherited byte/metadata changed at initialization");
   for(const auto& b:buffers)if(!std::all_of(b.begin()+Guard,b.end()-Guard,[](unsigned char x){return x==129;}))
    throw std::runtime_error("Original initializer buffer reset differs");
  }
  for(U i=0;i<5;++i)for(U j=0;j<count;++j)buffers[i][Guard+j]=static_cast<unsigned char>(-127+int((j+29*i+inherited+128)%128));
  global(0x6def58,0);global(0x6def5c,0);global(0x6df180,0);global(0x644520,0);
  global(0x689498,address(objects.data()+Guard));global(0x68949c,Capacity);global(0x6898dc,Capacity);
  global(0x6894a4,1);global(0x6894b0,0);global(0x5fbf28,0x643a80);
  for(U owner=0;owner<8;++owner)std::memset(reinterpret_cast<void*>(0x643a80+owner*0x154+0x18),0,16);
  for(U p:{0x6e8d90u,0x6e8d94u,0x6e8d98u,0x6e8d9cu,0x6e8da0u,0x6e8da4u,0x6e8da8u,0x6e8dacu})global(p,0);
  global(Map+0x808,0);global(Map+0x642e,shape%2);
  if(reinterpret_cast<Lookup>(0x49cf60)(reinterpret_cast<void*>(0x689498),0)!=1 ||
     reinterpret_cast<Lookup>(0x49cf60)(reinterpret_cast<void*>(0x689498),1)!=0)
   throw std::runtime_error("Original light-index selection differs");
  for(U tick=0;tick<8;++tick){
   const bool enabled=tick!=2;global(Map+0x80c,enabled);
   global(0x5e41a8,tick==2?7:tick==3?1:tick==4?2:tick==6?U(-1):0);
   global(0x6898dc,tick==5?0:Capacity);
   global(0x689930+0x39,tick==7?w-1:0);global(0x689930+0x3d,tick==7?h-1:0);
   global(0x689930+0x59,tick==7?0:std::min(w,h));
   for(U i=0;i<Capacity;++i){const U at=Guard+i*0x22e;
    const U x=i%2?w-1:0,y=i%2?h-1:0,z=i%2?layers-1:0;
    put(objects,at+4,(i!=2 && !(tick==7 && i==4)));put(objects,at+0x28,i==3?1:0);
    put(objects,at+8,x);put(objects,at+12,y);put(objects,at+16,z);
    put(objects,at+0x1c2,tick==4?0:x);put(objects,at+0x1c6,tick==4?0:y);
   }
   global(0x6c4830,records);U length=0;auto* input=singleCellCapture(Map,records+1,&length);
   if(!input)throw std::runtime_error("Production capture refused fixture");
   Bytes source(input,input+length);singleCellFree(input);
   if(std::int32_t(u32(source,56))!=inherited || u32(source,120)!=mnm_light_checksum(source.data(),length))
    throw std::runtime_error("Captured byte sign/identity differs");
   // The oracle runs only original instructions after capture. No native model
   // contributes expected state or pixels to this output.
   update(reinterpret_cast<void*>(Map));
   Bytes reply(80+5*count,0);std::memcpy(reply.data(),MNM_LIGHT_V1_REPLY,8);
   put(reply,8,1);put(reply,12,U(reply.size()));put(reply,16,u32(source,20));
   put(reply,20,u32(source,24));put(reply,24,u32(source,28));put(reply,28,u32(source,120));
   singleCellResult(reply.data()+32,Map,count);
   for(const auto& b:buffers)for(U j=0;j<Guard;++j)if(b[j]!=0xa5 || b[Guard+count+j]!=0xa5)
    throw std::runtime_error("Original light buffer guard changed");
   for(U j=0;j<Guard;++j)if(objects[j]!=0xa5 || objects[objects.size()-Guard+j]!=0xa5)
    throw std::runtime_error("Original object guard changed");
   const U resultBytes=U(reply.size());output.write(reinterpret_cast<const char*>(&length),4);
   output.write(reinterpret_cast<const char*>(&resultBytes),4);
   output.write(reinterpret_cast<const char*>(source.data()),source.size());
   output.write(reinterpret_cast<const char*>(reply.data()),reply.size());
   comparedBytes+=5*count;++records;
  }
 }
 output.seekp(8);output.write(reinterpret_cast<const char*>(&records),4);
 if(!output)throw std::runtime_error("Fixture bundle write failed");
 std::cout<<"{\"inherited_bytes\":256,\"dimensions\":4,\"paired_configurations\":4,\"initializations\":"<<initializations
          <<",\"requests\":"<<records<<",\"buffer_bytes\":"<<comparedBytes<<"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
