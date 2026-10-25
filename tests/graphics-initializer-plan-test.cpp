#include "../reconstruction/rendering/graphics_initializer.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
using namespace mnm::reconstruction;
static unsigned checks=0;
static void need(bool v){++checks;if(!v)throw std::runtime_error("Initializer owned request contract");}
int main()try{
 unsigned cases=0;
 for(bool video:{false,true})for(bool window:{false,true})for(auto config:{0u,1u,0x80000000u})
 for(auto x:{0u,7u,96u})for(auto y:{0u,5u}){
  const GraphicsInitializerInput input{video,window,config,x,y};const auto result=planGraphicsInitialization(input);
  const auto width=video?800u:640u,height=video?600u:480u;
  const auto px=config?96u:x,py=config?0u:y;
  std::array<std::uint32_t,27> primary{},offscreen{};
  primary[0]=offscreen[0]=108;primary[1]=1;primary[26]=0x200;
  offscreen[1]=7;offscreen[2]=height+py;offscreen[3]=width+px;offscreen[26]=0x840;
  need(result.primary==primary&&result.offscreen==offscreen);
  need(result.width==width&&result.height==height&&result.depth==16&&result.paddingX==px&&result.paddingY==py);
  using Step=GraphicsInitializerStep;
  need(result.steps==std::array<Step,5>{window?Step::setupWindow:Step::setupMode,Step::createPrimary,Step::createOffscreen,Step::clearOffscreen,Step::queryPixelFormat});
  need(result.clearFlags==0x1000400&&result.clearEffects[0]==100&&result.pixelFormatBytes==32);
  need(std::all_of(result.clearEffects.begin()+1,result.clearEffects.end(),[](auto v){return v==0;}));
  need(input.paddingX==x&&input.paddingY==y&&input.configuration==config);++cases;
 }
 for(const auto invalid:{GraphicsInitializerInput{false,false,0,0xffffffffu,0},GraphicsInitializerInput{true,true,0,0,0xffffffffu}}){bool rejected=false;try{planGraphicsInitialization(invalid);}catch(const std::invalid_argument&){rejected=true;}need(rejected);}
 const auto overridden=planGraphicsInitialization({true,true,1,0xffffffffu,0xffffffffu});need(overridden.paddingX==96&&overridden.paddingY==0);
 const auto maximum=planGraphicsInitialization({false,false,0,0xffffffffu-640,0xffffffffu-480});need(maximum.offscreen[2]==0xffffffffu&&maximum.offscreen[3]==0xffffffffu);
 auto first=planGraphicsInitialization({});auto copy=first;copy.primary[0]=0;need(first.primary[0]==108&&copy.primary[0]==0);
 std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"checks\":"<<checks<<",\"real_surfaces\":false}\n";
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
