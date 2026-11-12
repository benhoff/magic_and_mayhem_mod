// Execute only original rectangle calls from independently authored inputs.
// Queue markers describe the test order; no original World/simulation claim.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include <array>
int main(int argc,char** argv)try{
 if(argc!=4)throw std::runtime_error("Expected original PE, owned producers, output");
 map_image(read(argv[1]));const auto input=read(argv[2]);
 if(input.size()<64||std::memcmp(input.data(),"MNMPRO03",8))throw std::runtime_error("Expected producer V3 fixture");
 std::vector<std::uint16_t> pixels;unsigned width=0,height=0,calls=0,sequence=0;
 for(std::size_t at=64;at<input.size();){
  if(input.size()-at<96)throw std::runtime_error("Short fixture record");
  const unsigned size=u32(input,at),op=u32(input,at+8);
  if(size<96||size>input.size()-at||u32(input,at+4)!=++sequence)throw std::runtime_error("Invalid fixture order");
  auto field=[&](unsigned n){return u32(input,at+n*4);};
  if(op==1){width=field(5);height=field(6);if(!width||width>2048||!height||height>2048)throw std::runtime_error("Fixture extent");pixels.assign(width*height,0);}
  else if(op==5||op==26){
   if(!width)throw std::runtime_error("Missing fixture canvas");
   global(0x658174,reinterpret_cast<std::uintptr_t>(pixels.data()));global(0x6a2dc8,width);global(0x6e1f88,0);
   const std::array<unsigned,4> clip=op==5?std::array<unsigned,4>{0,0,width,height}:std::array<unsigned,4>{field(10),field(11),field(12),field(13)};
   for(unsigned k=0;k<4;++k)global(std::array<std::uintptr_t,4>{0x6e0008,0x6cbb6c,0x6a49b8,0x656618}[k],clip[k]);
   const int left=std::int32_t(field(10)),top=std::int32_t(field(11)),right=std::int32_t(field(12)),bottom=std::int32_t(field(13));
   unsigned char shape[22]={};std::vector<std::uint16_t> triples;
   const unsigned mode=op==5?0:field(15),w=op==5?unsigned(right-left):field(19),h=op==5?unsigned(bottom-top):field(20);
   const int x=op==5?left:std::int32_t(field(8)),y=op==5?top:std::int32_t(field(9));
   if(!w||w>2048||!h||h>2048)throw std::runtime_error("Invalid original shape");
   std::memcpy(shape,&mode,4);std::memcpy(shape+4,&w,4);std::memcpy(shape+8,&h,4);
   if(op==5){const std::uint16_t rgb[]={std::uint16_t(field(14)>>11),std::uint16_t((field(14)>>5)&63),std::uint16_t(field(14)&31)};std::memcpy(shape+12,rgb,6);}
   else{
    if(size!=96+w*h*2||w>64||h>64||mode>3)throw std::runtime_error("Invalid owned colour input");
    triples.resize(w*h*3);
    for(unsigned i=0;i<w*h;++i){const auto word=input.at(at+96+i*2)|unsigned(input.at(at+97+i*2))<<8;triples[i*3]=word>>11;triples[i*3+1]=(word>>5)&63;triples[i*3+2]=word&31;}
    const auto pointer=std::uint32_t(reinterpret_cast<std::uintptr_t>(triples.data()));std::memcpy(shape+18,&pointer,4);
   }
   using Colour=unsigned (__attribute__((thiscall)) *)(void*,int,int);
   reinterpret_cast<Colour>(0x547d80)(shape,x,y);++calls;
  }else if(op!=11&&op!=12)throw std::runtime_error("Unexpected fixture producer");
  at+=size;
 }
 save(argv[3],pixels);std::cout<<"Original rectangle calls "<<calls<<"; complete output words "<<pixels.size()<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
