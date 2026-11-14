#include "../tools/broader_finite_edge_oracle.hpp"
#include <iostream>
using namespace broader_edge;
static void put(std::vector<unsigned char>& b,unsigned at,unsigned n){std::memcpy(b.data()+at,&n,4);}
static std::vector<unsigned char> fixture(){
 std::vector<unsigned char> b(96+60+64);put(b,0,b.size());put(b,8,9);put(b,20,4);put(b,24,2);put(b,48,4);put(b,52,2);put(b,56,3);put(b,60,9);put(b,64,2);put(b,76,60);put(b,80,64);put(b,96,60);put(b,100,4);put(b,104,2);put(b,136,56);put(b,140,60);put(b,144,58);put(b,148,64);b[152]=0;b[153]=4;b[154]=0;b[155]=4;return b;
}
int main()try{
 unsigned cases=0;
 for(unsigned delta=0;delta<=16;++delta){auto b=fixture();put(b,160,delta);std::vector<std::uint16_t> pixels{101,102,103,104,105,106,107,108};const auto before=pixels;taps.clear();{
  Guard guard(7,0x596490,b,pixels);for(unsigned i=4;i<8;++i)pixels[i]=i+delta<8?before[i+delta]:0;
 }for(unsigned i=0;i<8;++i){const auto expected=i<4||i+delta>=8?before[i]:before[i+delta];if(pixels[i]!=expected)throw std::runtime_error("Policy changed valid sample or failed retained edge");}
 if(taps.size()!=std::min(delta,4u))throw std::runtime_error("Policy coverage certificate count");
 ++cases;}
 auto b=fixture();put(b,160,8);b[154]=4;b[155]=0;taps.clear();std::vector<std::uint16_t> pixels(8,123);{Guard guard(8,0x5968a4,b,pixels);}if(!taps.empty())throw std::runtime_error("Transparent edge gained policy credit");++cases;
 b=fixture();put(b,160,8);put(b,52,1);taps.clear();{Guard guard(9,0x596490,b,pixels);}if(!taps.empty())throw std::runtime_error("Clipped edge gained policy credit");++cases;
 for(unsigned variant=0;variant<7;++variant){b=fixture();if(variant==0)put(b,64,0);if(variant==1)put(b,160,17);if(variant==2)put(b,48,5);if(variant==3)put(b,136,39);if(variant==4)b[155]=5;if(variant==5)put(b,140,61);if(variant==6)put(b,60,8);bool refused=false;try{Guard guard(1,0x596490,b,pixels);}catch(const std::runtime_error&){refused=true;}if(!refused)throw std::runtime_error("Malformed policy input admitted");++cases;}
 std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"original_game_runs\":0}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
