#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
static unsigned unlocks;
static unsigned __attribute__((stdcall)) unlock_surface(void *,void *) { ++unlocks;return 0; }
int main(int argc,char **argv) try {
  if(argc!=4)throw std::runtime_error("Expected pinned PE, fade corpus, output");
  map_image(read(argv[1]));const auto b=read(argv[2]);
  if(b.size()<12 || std::memcmp(b.data(),"MNMFADE1",8))throw std::runtime_error("Fade corpus header");
  if(std::memcmp(reinterpret_cast<void *>(0x58ed80),"\x51\x55\x56\x8b\x35\x88\x1f\x6e\x00",9))throw std::runtime_error("Fade entry bytes changed");
  auto *lock=reinterpret_cast<unsigned char *>(0x58b660);
  if(std::memcmp(lock,"\x83\xec\x08\x33\xc0",5))throw std::runtime_error("Lock entry changed");
  // Only the external Lock boundary is supplied by this private adapter.
  lock[0]=0xb8;lock[5]=0xc3;
  std::uint32_t table[33]={},iface[1]={},surface[12]={};
  table[32]=reinterpret_cast<std::uintptr_t>(unlock_surface);iface[0]=reinterpret_cast<std::uintptr_t>(table);
  surface[1]=reinterpret_cast<std::uintptr_t>(iface);
  std::ofstream out(argv[3],std::ios::binary);std::size_t at=12;
  for(unsigned c=0;c<u32(b,8);++c){
    const auto w=u32(b,at),h=u32(b,at+4),stride=u32(b,at+8),format=u32(b,at+12);at+=16;
    if(!w || !h || w>2048 || h>2048 || stride<w || stride>4096 || at+stride*h*2>b.size())throw std::runtime_error("Fade case extent");
    std::vector<std::uint16_t> plane(stride*h+64,0xb37d);
    std::memcpy(plane.data()+32,b.data()+at,stride*h*2);at+=stride*h*2;
    auto pointer=std::uint32_t(reinterpret_cast<std::uintptr_t>(plane.data()+32));std::memcpy(lock+1,&pointer,4);
    surface[4]=w;surface[5]=h;surface[6]=stride*2;global(0x6e1f88,format);unlocks=0;
    using Draw=void(__attribute__((thiscall)) *)(void *);reinterpret_cast<Draw>(0x58ed80)(surface);
    if(unlocks!=1)throw std::runtime_error("Unlock adapter count");
    for(unsigned i=0;i<32;++i)if(plane[i]!=0xb37d || plane[stride*h+32+i]!=0xb37d)throw std::runtime_error("Fade guard modified");
    out.write(reinterpret_cast<const char *>(plane.data()+32),stride*h*2);
    for(unsigned y=0;y<h;++y)out.write(reinterpret_cast<const char *>(plane.data()+32+y*stride),w*2);
  }
  if(at!=b.size() || !out)throw std::runtime_error("Fade output/input extent");
  std::cout<<"Unchanged original fade physical words, guards and visible projection pass\n";return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
