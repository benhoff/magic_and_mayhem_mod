// Execute the precise signature-pinned generic raster body suppressed live.
// Its before canvas is reconstructed native history, never an original oracle.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
int main(int argc,char **argv)try{
  if(argc!=6)throw std::runtime_error("Expected pinned PE, producer record, native before, output, entry");
  map_image(read(argv[1]));const auto record=read(argv[2]),before=read(argv[3]);
  const auto entry=std::stoul(argv[5],nullptr,0);
  if(entry!=0x5947b2&&entry!=0x59521a)throw std::runtime_error("Unsupported original generic entry");
  const unsigned char expected[6]={0x55,0x8b,0xec,0x56,0x57,0x53};
  if(std::memcmp(reinterpret_cast<void*>(entry),expected,6))throw std::runtime_error("Original generic entry signature changed");
  if(record.size()<136||u32(record,8)!=9||u32(record,56)||u32(record,60)!=1||u32(record,68)!=1||u32(record,80)!=512||record.size()!=u32(record,0))throw std::runtime_error("Unclosed selected producer record");
  const auto width=u32(record,20),height=u32(record,24),size=u32(record,76);
  if(!width||width>2048||!height||height>2048||before.size()!=std::size_t(width)*height*2||size<40||record.size()!=96+size+512||u32(record,96)!=size||u32(record,124))throw std::runtime_error("Producer source/extent admission");
  std::vector<std::uint16_t> pixels(width*height);std::memcpy(pixels.data(),before.data(),before.size());
  Bytes frame(record.begin()+96,record.begin()+96+size);std::vector<std::uint32_t> palette(3+128,0);std::memcpy(palette.data()+3,record.data()+96+size,512);
  const auto pointer=std::uint32_t(reinterpret_cast<std::uintptr_t>(palette.data()));std::memcpy(frame.data()+28,&pointer,4);
  global(0x658174,reinterpret_cast<std::uintptr_t>(pixels.data()));global(0x6a2dc8,width);global(0x6e1f68,0);global(0x6e1f88,0);
  for(unsigned i=0;i<4;++i)global(i==0?0x6e0008:i==1?0x6cbb6c:i==2?0x6a49b8:0x656618,u32(record,40+i*4));
  using Draw=unsigned (*)(void*,int,int,int,int);
  reinterpret_cast<Draw>(entry)(frame.data(),std::int32_t(u32(record,32)),std::int32_t(u32(record,36)),0,0);
  save(argv[4],pixels);std::cout<<"Exact original generic entry completed from native producer history\n";return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
