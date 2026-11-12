// Execute the pinned marker body with unused row entries inaccessible/poisoned.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include <array>
#include <unistd.h>
int main(int argc,char** argv) try {
 if(argc!=2)throw std::runtime_error("Expected pinned PE32 image");
 map_image(read(argv[1]));
 if(std::memcmp(reinterpret_cast<void*>(0x553850),"\x83\xec\x08\x53\x55",5))throw std::runtime_error("Marker entry changed");
 const auto page=static_cast<unsigned>(sysconf(_SC_PAGESIZE));
 auto* guard=static_cast<unsigned char*>(mmap(nullptr,page*3,PROT_NONE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0));
 if(guard==MAP_FAILED||mprotect(guard+page,page,PROT_READ|PROT_WRITE))throw std::runtime_error("Row guard setup failed");
 unsigned cases=0;
 for(unsigned format=0;format<2;++format)for(unsigned view=0;view<4;++view)for(unsigned b:{0u,2u,32u}){
  constexpr unsigned stride=199,height=128,ox=88,oy=12;
  const unsigned a=0,row=(a+b)/2;
  unsigned x=a,y=b;if(view==1){x=63-b;y=a;}else if(view==2){x=63-a;y=63-b;}else if(view==3){x=b;y=63-a;}
  std::vector<std::uint16_t> background(stride*height,0x1234),canonical,guarded;
  std::vector<unsigned char> object(0x6210),before;
  auto field=[&](unsigned at,std::uint32_t value){std::memcpy(object.data()+at,&value,4);};
  field(0xc5,view);field(0xeb,stride);field(0xf7,ox);field(0xfb,oy);field(0x103,63);field(0x107,63);
  field(0x61a1,32);field(0x61a5,32);
  std::array<unsigned,3> marker{x,y,0};std::vector<signed char> state(64*128,1);
  field(0x614d,reinterpret_cast<std::uintptr_t>(marker.data()));field(0x6151,reinterpret_cast<std::uintptr_t>(marker.data()+3));
  global(0x6c5494,64);global(0x6c5498,64);global(0x6e1f88,format);global(0x5e1404,0);global(0x5e18f0,unsigned(-1));
  global(0x6c5c5c,reinterpret_cast<std::uintptr_t>(state.data()));
  for(unsigned i=0;i<128;++i)global(0x6cb942+i*4,i*64);
  for(unsigned i=0;i<32;++i)global(0x6cb8c2+i*4,0);
  std::array<unsigned,128> rows{};
  auto draw=[&](bool bounded){
   auto pixels=background;field(0xdf,reinterpret_cast<std::uintptr_t>(pixels.data()));
   for(unsigned r=0;r<rows.size();++r)rows[r]=reinterpret_cast<std::uintptr_t>(pixels.data()+((oy+r)*stride+ox));
   if(bounded){
    std::memset(guard+page,0xa5,page);
    std::memcpy(guard+page,&rows[row],4);
    field(0xf3,reinterpret_cast<std::uintptr_t>(guard+page)-row*4);
   }else field(0xf3,reinterpret_cast<std::uintptr_t>(rows.data()));
   before=object;
   using Draw=void(__attribute__((thiscall)) *)(void*);
   reinterpret_cast<Draw>(0x553850)(object.data());
   const unsigned dim=64;std::memcpy(before.data()+0x6199,&dim,4);std::memcpy(before.data()+0x619d,&dim,4);
   if(object!=before)throw std::runtime_error("Unexpected marker object mutation");
   return pixels;
  };
  canonical=draw(false);guarded=draw(true);
  if(canonical!=guarded)throw std::runtime_error("Unused row entries changed marker output");
  unsigned changed=0;for(unsigned i=0;i<canonical.size();++i)changed+=canonical[i]!=background[i];
  if(changed!=13)throw std::runtime_error("Expected thirteen marker stores");
  ++cases;
 }
 if(munmap(guard,page*3))throw std::runtime_error("Guard teardown failed");
 std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"orientations\":4,\"formats\":2,\"center_rows\":[0,1,16],\"unused_preceding_rows_inaccessible\":true,\"unused_following_rows_poisoned\":true,\"whole_destination_equal\":true,\"stores_per_case\":13}\n";
 return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
