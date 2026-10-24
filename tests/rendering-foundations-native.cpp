#include "../reconstruction/rendering/minimap_camera_dispatch.hpp"
#include "../reconstruction/rendering/displacement_table.h"
#include <array>
#include <iostream>
#include <stdexcept>
#include <vector>

int main()try{
  const std::array<int32_t,16> baseline{{8,11,13,15,15,15,13,11,8,5,3,1,1,1,3,5}};
  std::array<int32_t,16> table{};
  __asm__ volatile("fninit");
  if(!mnm_initialize_displacement_table(table.data(),table.size())||table!=baseline)
    throw std::runtime_error("Owned FNINIT table differs");
  unsigned calls=0;
  for(unsigned selector: {0u,1u,2u,3u,4u,0xffffffffu}){
    std::vector<unsigned> trace;
    mnm::reconstruction::dispatchOriginalMinimapCamera(selector,
      [&](unsigned camera){trace.push_back(camera);},[&]{trace.push_back(4);});
    const auto selected=mnm::reconstruction::originalMinimapCamera(selector);
    if(trace.back()!=4||trace.size()!=std::size_t(selected>=0?2:1))
      throw std::runtime_error("Owned dispatch composition differs");
    calls+=trace.size();
  }
  std::cout<<"{\"success\":true,\"owned_words\":16,\"callback_events\":"<<calls<<",\"original_image_consumed\":false}\n";
  return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
