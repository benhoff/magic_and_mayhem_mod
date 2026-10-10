#include "../renderer/canvas_sequence.hpp"
#include "../reconstruction/rendering/fade_span.hpp"
#include <iostream>
static void require(bool v){if(!v)throw std::runtime_error("Fade test assertion");}
int main() try {
  using mnm::render::CanvasSequence;using mnm::reconstruction::rendering::fadePhysicalWords;
  unsigned refusals=0;
  CanvasSequence c;c.create(1,3,3);c.fill(1,{0,0,3,2},0xabcd);
  try{c.fade(1,3,8);throw std::runtime_error("Missing unknown refusal");}catch(const std::runtime_error &e){if(std::string(e.what())=="Missing unknown refusal")throw;++refusals;}
  c.fill(1,{0,2,3,3},0x9876);const auto before=c.read(1).pixels;
  require(before[0]==0xabcd); // Late undefined input cannot partially dim earlier words.
  for(auto invalid : {0u,2u,4097u}){
    try{c.fade(1,invalid,10);throw std::runtime_error("Missing extent refusal");}catch(const std::invalid_argument &){++refusals;}
    require(c.read(1).pixels==before);
  }
  c.fade(1,5,6);auto pixels=c.read(1).pixels;
  require(pixels[0]==((0xabcd>>1)&0x7bef) && pixels[3]==((0xabcd>>1)&0x7bef) && pixels[4]==0xabcd && pixels[6]==0x9876);
  CanvasSequence untouched;untouched.create(1,1,2);untouched.fade(1,2,0);untouched.fill(1,{0,0,1,2},0x9988);require(untouched.read(1).pixels[0]==0x9988);
  std::vector<std::uint16_t> plane(9,0xabcd);const auto original=plane;
  for(auto dims: {std::array<unsigned,3>{0,3,3},{3,0,3},{2049,3,2049},{3,3,2},{3,3,4097},{3,2,3}}){
    try{fadePhysicalWords(plane,dims[0],dims[1],dims[2],0);throw std::runtime_error("Missing model refusal");}catch(const std::invalid_argument &){++refusals;}
    require(plane==original);
  }
  std::cout<<"{\"success\":true,\"atomic_refusals\":"<<refusals<<"}\n";return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
