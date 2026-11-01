// Compare effective-table oracle reconstruction with raw original phase/state.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include "world-raster-reference.hpp"
#include <algorithm>
namespace {
void put(Bytes& b,unsigned at,unsigned value){std::memcpy(b.data()+at,&value,4);}
}
int main(int argc,char** argv)try{
 if(argc!=2)throw std::runtime_error("Expected pinned original PE");
 map_image(read(argv[1]));
 constexpr unsigned width=64,height=64,w=7,h=47,table=40+h*8;
 Bytes frame(table+h*2+w*h,0);put(frame,0,frame.size());put(frame,4,w);put(frame,8,h);put(frame,12,3);put(frame,16,4);
 for(unsigned row=0;row<h;++row){put(frame,40+row*8,table+row*2);put(frame,44+row*8,table+h*2+row*w);frame[table+row*2+1]=w;}
 unsigned cases=0,clipped=0,mismatches=0,unclippedMismatches=0;std::uint64_t words=0;
 for(auto entry:{0x596490u,0x5968a4u})for(unsigned phase=0;phase<16;++phase)
 for(int top:{-31,-19,-3,0,7,16})for(int clipTop:{0,5,11}){
  const unsigned skip=unsigned(std::max(0,clipTop-top));
  std::vector<std::uint16_t> before(width*height);for(unsigned i=0;i<before.size();++i)before[i]=std::uint16_t(i*97+phase*53+13);
  auto expected=before;global(0x658174,reinterpret_cast<std::uintptr_t>(expected.data()));global(0x6a2dc8,width);
  global(0x6e0008,0);global(0x6cbb6c,clipTop);global(0x6a49b8,width);global(0x656618,height);global(0x6e1f68,0);global(0x6e1f88,0);global(0x6c4830,phase);
  for(unsigned j=0;j<16;++j)global(0x656640+j*4,(j*5+3)&15);
  using Draw=unsigned (*)(void*,int,int);const auto ax=reinterpret_cast<Draw>(entry)(frame.data(),20,top+4)&65535u;
  Bytes record(96+frame.size()+64,0);put(record,0,record.size());put(record,8,9);put(record,20,width);put(record,24,height);
  put(record,32,20);put(record,36,unsigned(top+4));put(record,44,clipTop);put(record,48,width);put(record,52,height);put(record,56,3);put(record,60,9);put(record,64,16);put(record,68,1);put(record,76,frame.size());put(record,80,64);put(record,84,ax);
  std::copy(frame.begin(),frame.end(),record.begin()+96);
  for(unsigned j=0;j<16;++j)put(record,96+frame.size()+j*4,(((j+phase+1-skip)&15)*5+3)&15);
  auto actual=before;const auto reconstructed=originalRaster(record,entry,actual)&65535u;
  if(reconstructed!=ax)throw std::runtime_error("Original AX differs");
  const bool different=actual!=expected;++cases;clipped+=skip>0;mismatches+=different;unclippedMismatches+=different&&!skip;words+=before.size();
 }
 std::cout<<"{\"success\":"<<(mismatches?"false":"true")<<",\"cases\":"<<cases<<",\"clipped_cases\":"<<clipped<<",\"mismatching_cases\":"<<mismatches<<",\"unclipped_mismatches\":"<<unclippedMismatches<<",\"words_compared\":"<<words<<",\"entries\":2,\"phases\":16}\n";
 return mismatches?1:0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
