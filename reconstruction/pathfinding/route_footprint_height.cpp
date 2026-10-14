#include "route_footprint_height.hpp"
#include <stdexcept>
namespace mnm::reconstruction {
std::int32_t large_footprint_height(const std::array<std::uint16_t,4> &kinds,
    const std::function<std::int32_t(std::uint16_t)> &classification) {
  if(!classification)throw std::invalid_argument("footprint classification provider missing");
  std::uint32_t result=16;
  for(auto kind:kinds) {
    if(!kind)return 0;
    const auto value=std::uint32_t(classification(kind));
    if(value<=result)result=value;
    if(!result)return 0;
  }
  return std::int32_t(result);
}
}
