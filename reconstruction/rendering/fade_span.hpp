#pragma once
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace mnm::reconstruction::rendering {
// NoCD58ed80 advances one contiguous DWORD pointer for all rows. The original
// ignores pitch and leaves the unprocessed tail at the end of that span.
inline std::size_t fadeWordCount(unsigned width,unsigned height,unsigned stride) {
  if(!width || !height || width>2048 || height>2048 || stride<width || stride>4096)
    throw std::invalid_argument("Fade plane outside bounded dimensions");
  return std::size_t(width/2)*2*height;
}
inline std::uint16_t fadeWord(std::uint16_t p,std::int32_t format) {
  return (p>>1)&(format ? 0x7def : 0x7bef);
}
inline void fadePhysicalWords(std::vector<std::uint16_t> &plane,unsigned width,
                              unsigned height,unsigned stride,std::int32_t format) {
  const auto count=fadeWordCount(width,height,stride);
  if(plane.size()!=std::size_t(stride)*height)
    throw std::invalid_argument("Incomplete fade physical plane");
  for(std::size_t i=0;i<count;++i)plane[i]=fadeWord(plane[i],format);
}
}
