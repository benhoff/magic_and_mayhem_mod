// Independent original pixel oracle: only closed authored sources and a zero
// destination are inputs. The native process never consumes this output as input.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
int main(int argc,char** argv)try{
 if(argc<5)throw std::invalid_argument("Expected original PE, output, and closed records");
 map_image(read(argv[1]));std::vector<std::uint16_t> pixels(800*600,0);unsigned generic=0,clipped=0,word=0;
 for(int i=3;i<argc;++i){const auto r=read(argv[i]);if(r.size()<136||u32(r,0)!=r.size()||u32(r,8)!=9||u32(r,16)||u32(r,20)!=800||u32(r,24)!=600||u32(r,28)!=800||u32(r,72)!=r.size()-96||u32(r,124))throw std::invalid_argument("Unclosed owned sprite record");
  const auto size=u32(r,76),backend=u32(r,60);if(size<40||u32(r,96)!=size||r.size()!=96+size+u32(r,80))throw std::invalid_argument("Sprite source extent");
  Bytes frame(r.begin()+96,r.begin()+96+size);frame.resize(size+4,0);std::vector<std::uint32_t> palette(131,0);
  std::uint32_t entry=0;
  if((backend==1||backend==3)&&u32(r,68)==1&&u32(r,80)==512){std::memcpy(palette.data()+3,r.data()+96+size,512);const auto p=std::uint32_t(reinterpret_cast<std::uintptr_t>(palette.data()));std::memcpy(frame.data()+28,&p,4);}
  else if(backend==8&&!u32(r,68)&&!u32(r,80)){const std::uint32_t sentinel=0xffffffff;std::memcpy(frame.data()+28,&sentinel,4);}
  else throw std::invalid_argument("Unsupported original sprite route");
  // The driver supplies the exact entry in r[22] of the standalone oracle
  // copy. This word never enters native decoding or the actual producer wire.
  entry=u32(r,88);if(!((backend==1&&(entry==0x5947b2||entry==0x59521a))||(backend==3&&entry==0x57de00)||(backend==8&&(entry==0x597086||entry==0x596cb8))))throw std::invalid_argument("Wrong original entry/backend pair");
  global(0x658174,reinterpret_cast<std::uintptr_t>(pixels.data()));global(0x6a2dc8,800);global(0x6e1f68,0);global(0x6e1f88,0);
  for(unsigned k=0;k<4;++k)global(k==0?0x6e0008:k==1?0x6cbb6c:k==2?0x6a49b8:0x656618,u32(r,40+k*4));
  const auto x=std::int32_t(u32(r,32)),y=std::int32_t(u32(r,36));
  if(backend==8){using Draw=unsigned(*)(void*,int,int);reinterpret_cast<Draw>(entry)(frame.data(),x,y);++word;}
  else {using Draw=unsigned(*)(void*,int,int,int,int);const auto result=reinterpret_cast<Draw>(entry)(frame.data(),x,y,0,0);if(backend==1){++generic;if((result&65535)==1){reinterpret_cast<Draw>(0x57de00)(frame.data(),x,y,0,0);++clipped;}}else ++clipped;}
 }
 save(argv[2],pixels);std::cout<<"{\"success\":true,\"draws\":"<<argc-3<<",\"generic\":"<<generic<<",\"clipped\":"<<clipped<<",\"word\":"<<word<<"}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
