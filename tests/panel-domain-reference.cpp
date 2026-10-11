#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
static unsigned unlocks;
static unsigned __attribute__((stdcall)) unlock_surface(void *,void *) { ++unlocks;return 0; }
int main(int argc,char **argv) try {
  if(argc!=4)throw std::runtime_error("Expected pinned PE, panel corpus, output");
  map_image(read(argv[1]));const auto b=read(argv[2]);
  if(b.size()<12 || std::memcmp(b.data(),"MNMPAN01",8))throw std::runtime_error("Panel corpus header");
  auto *lock=reinterpret_cast<unsigned char *>(0x58b660);
  if(std::memcmp(lock,"\x83\xec\x08\x33\xc0",5) || std::memcmp(reinterpret_cast<void *>(0x58ddf0),"\x83\xec\x40\xa1\x88\x1f\x6e\x00",8) || std::memcmp(reinterpret_cast<void *>(0x58d8c0),"\x83\xec\x38\xa1\x88\x1f\x6e\x00",8) || std::memcmp(reinterpret_cast<void *>(0x58e2c0),"\x83\xec\x24\xa1\x88\x1f\x6e\x00",8))throw std::runtime_error("Original anchors changed");
  lock[0]=0xb8;lock[5]=0xc3;
  std::uint32_t table[33]={},iface[1]={},surface[12]={};
  table[32]=reinterpret_cast<std::uintptr_t>(unlock_surface);iface[0]=reinterpret_cast<std::uintptr_t>(table);surface[1]=reinterpret_cast<std::uintptr_t>(iface);
  global(0x6e1f88,0);*reinterpret_cast<std::uint16_t *>(0x6e1f7c)=0xf800;*reinterpret_cast<std::uint16_t *>(0x6e1f7e)=0x07e0;*reinterpret_cast<std::uint16_t *>(0x6e1f80)=0x001f;
  std::ofstream out(argv[3],std::ios::binary);std::size_t at=12;
  for(unsigned c=0;c<u32(b,8);++c){
    const auto w=u32(b,at),h=u32(b,at+4),stride=u32(b,at+8),kind=u32(b,at+12),percent=u32(b,at+32),pressed=u32(b,at+36),reverse=u32(b,at+40);
    int rectangle[4];std::memcpy(rectangle,b.data()+at+16,16);at+=44;
    if(!w||!h||w>2048||h>2048||stride<w||stride>4096||kind<15||kind>17||at+stride*h*2>b.size())throw std::runtime_error("Panel case extent");
    std::vector<std::uint16_t> plane(stride*h+64,0xb37d);std::memcpy(plane.data()+32,b.data()+at,stride*h*2);at+=stride*h*2;
    auto pointer=std::uint32_t(reinterpret_cast<std::uintptr_t>(plane.data()+32));std::memcpy(lock+1,&pointer,4);surface[4]=w;surface[5]=h;surface[6]=stride*2;unlocks=0;
    if(kind==15){using Draw=void(__attribute__((thiscall)) *)(void *,void *,int,int,int);reinterpret_cast<Draw>(0x58ddf0)(surface,rectangle,percent,pressed,reverse);}
    else {using Draw=void(__attribute__((thiscall)) *)(void *,void *);reinterpret_cast<Draw>(kind==16?0x58d8c0:0x58e2c0)(surface,rectangle);}
    if(unlocks!=1)throw std::runtime_error("Unlock adapter count");
    for(unsigned i=0;i<32;++i)if(plane[i]!=0xb37d || plane[stride*h+32+i]!=0xb37d)throw std::runtime_error("Panel guard changed");
    out.write(reinterpret_cast<const char *>(plane.data()+32),stride*h*2);
  }
  if(at!=b.size() || !out)throw std::runtime_error("Input/output extent");
  return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
