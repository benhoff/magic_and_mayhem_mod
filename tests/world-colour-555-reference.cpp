// Independent original PE32 execution from authored packed World requests.
// No native renderer code or original destination image is an input.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include <array>
int main(int argc,char** argv)try{
 if(argc!=4)throw std::runtime_error("Expected pinned original PE, owned World inputs, new output");
 map_image(read(argv[1]));const auto input=read(argv[2]);
 if(input.size()<80||std::memcmp(input.data(),"MNMWRLD1",8)||u32(input,8)!=1||u32(input,12)!=80||u32(input,16)!=input.size()||u32(input,40)||u32(input,44)!=0x40209ca7||u32(input,72)>1)throw std::runtime_error("Owned World envelope");
 const auto width=u32(input,24),height=u32(input,28),format=u32(input,72);
 if(!width||width>2048||!height||height>2048||u32(input,32)!=width||!u32(input,36)||u32(input,36)>12320)throw std::runtime_error("Owned World extent");
 std::vector<std::uint16_t> pixels(std::size_t(width)*height,0xffff);global(0x658174,reinterpret_cast<std::uintptr_t>(pixels.data()));global(0x6a2dc8,width);global(0x6e1f88,format);
 const unsigned shift=format?10:11,green=format?31:63;std::size_t at=80;
 for(unsigned n=0;n<u32(input,36);++n){
  const auto size=u32(input,at),w=u32(input,at+64),h=u32(input,at+68),mode=u32(input,at+72);
  if(u32(input,at+4)!=7||u32(input,at+32)||u32(input,at+36)||u32(input,at+52)!=12||!w||w>64||!h||h>64||mode>3||size!=76+w*h*2||at+size>input.size())throw std::runtime_error("Owned colour record");
  std::vector<std::uint16_t> triples(w*h*3);
  for(unsigned i=0;i<w*h;++i){const auto word=input.at(at+76+i*2)|unsigned(input.at(at+77+i*2))<<8;if(format&&word>0x7fff)throw std::runtime_error("Noncanonical RGB555 source");triples[i*3]=(word>>shift)&31;triples[i*3+1]=(word>>5)&green;triples[i*3+2]=word&31;}
  unsigned char shape[22]={};const std::uint32_t fields[]={mode,w,h};std::memcpy(shape,fields,12);const auto ptr=std::uint32_t(reinterpret_cast<std::uintptr_t>(triples.data()));std::memcpy(shape+18,&ptr,4);
  for(unsigned i=0;i<4;++i)global(std::array<std::uintptr_t,4>{0x6e0008,0x6cbb6c,0x6a49b8,0x656618}[i],u32(input,at+16+i*4));
  using Colour=unsigned (__attribute__((thiscall)) *)(void*,int,int);
  reinterpret_cast<Colour>(0x547d80)(shape,std::int32_t(u32(input,at+8)),std::int32_t(u32(input,at+12)));at+=size;
 }
 if(at!=input.size())throw std::runtime_error("Trailing owned World data");
 save(argv[3],pixels);std::cout<<"Original packed colour World words "<<pixels.size()<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
