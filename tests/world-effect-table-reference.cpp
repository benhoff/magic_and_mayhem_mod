#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include <array>
static void check(bool value){if(!value)throw std::runtime_error("Original non-drawing effect preparation differs");}
static unsigned loadWord(unsigned p){return *reinterpret_cast<unsigned*>(p);}
int main(int argc,char** argv)try{
 check(argc==2);map_image(read(argv[1]));
 // An unreadable canvas and palette pointer make any drawing/late-frame path fail.
 global(0x658174,1);std::array<unsigned,11> frame{};frame[0]=44;frame[7]=1;const auto before=frame;
 using Source=unsigned (__attribute__((fastcall)) *)(void*,int,int,int,int);
 using Wave=unsigned (__attribute__((fastcall)) *)(void*,int,int,int);
 unsigned cached=0;std::cout<<"{\"success\":true,\"source_tables\":[";
 for(unsigned amp=1;amp<=16;++amp){
  for(unsigned p=0x5f1440;p<0x5f14e0;p+=4)global(p,0xdeadbeef);
  global(0x5f1450,32000);global(0x5e15f8,amp);
  check((reinterpret_cast<Source>(0x580260)(frame.data(),0,0,0,0)&65535)==0&&frame==before);
  for(unsigned p=0x5f1440;p<0x5f14e0;p+=4)if(p<0x5f1450||p>=0x5f1450+amp*8)check(loadWord(p)==0xdeadbeef);
  std::array<unsigned,32> table{};for(unsigned i=0;i<amp*2;++i){table[i]=loadWord(0x5f1450+i*4);check(table[i]<=amp*2);}
  global(0x5e15f8,amp==16?1:amp+1);check((reinterpret_cast<Source>(0x580260)(frame.data(),0,0,0,0)&65535)==0&&frame==before);
  for(unsigned i=0;i<amp*2;++i)check(table[i]==loadWord(0x5f1450+i*4));
  ++cached;
  if(amp>1)std::cout<<',';
  std::cout<<"{\"amplitude\":"<<amp<<",\"words\":[";
  for(unsigned i=0;i<amp*2;++i){if(i)std::cout<<',';std::cout<<table[i];}std::cout<<"]}";
 }
 for(unsigned p=0x5f14c0;p<0x5f18e0;p+=4)global(p,0xdeadbeef);
 global(0x5f14d0,32000);check((reinterpret_cast<Wave>(0x5806f0)(frame.data(),0,0,9)&65535)==0&&frame==before);
 for(unsigned p=0x5f14c0;p<0x5f18e0;p+=4)if(p<0x5f14d0||p>=0x5f18d0)check(loadWord(p)==0xdeadbeef);
 std::array<unsigned,256> wave{};
 for(unsigned i=0;i<256;++i){wave[i]=loadWord(0x5f14d0+i*4);check(wave[i]<=(i/16)*2);}
 check((reinterpret_cast<Wave>(0x5806f0)(frame.data(),0,0,16)&65535)==0&&frame==before);
 for(unsigned i=0;i<256;++i)check(wave[i]==loadWord(0x5f14d0+i*4));
 ++cached;
 std::cout<<"],\"wave_words\":[";for(unsigned i=0;i<256;++i){if(i)std::cout<<',';std::cout<<wave[i];}
 std::cout<<"],\"cached_calls_unchanged\":"<<cached<<",\"zero_frame_and_unreadable_pixel_inputs\":true,\"wave_writes_only_groups0_to15\":true}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
