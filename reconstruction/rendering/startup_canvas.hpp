#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <stdexcept>

namespace mnm::reconstruction {
// Pinned NoCD 4fd446..4fd47b scalar binding calculation. This owns no storage.
struct StartupBinding { int stride, height; };
inline StartupBinding paddedStartupBinding(bool video, int xPadding, int yPadding) {
  if(xPadding<0 || xPadding>2048 || yPadding<0 || yPadding>2048)
    throw std::invalid_argument("Unsupported startup padding");
  return {(video?800:640)+2*xPadding,(video?600:480)+2*yPadding};
}
using StartupWaveTable=std::array<std::int32_t,256>;
// 58072d..58076a: original approximate double pi/8, fsin, multiplication,
// truncation toward zero and amplitude bias. Independent of captured offsets.
inline StartupWaveTable startupWaveTable() {
  StartupWaveTable table{};
  for(int amplitude=0;amplitude<16;++amplitude)
    for(int row=0;row<16;++row)
      table[amplitude*16+row]=amplitude+int(amplitude*std::sin(row*static_cast<long double>(0x1.921fb52713c02p-2)));
  return table;
}
inline std::array<int,16> startupWaveOffsets(const StartupWaveTable& table,
                                          unsigned amplitude,unsigned phase) {
  if(!amplitude || amplitude>8)throw std::invalid_argument("Unsupported effective wave amplitude");
  std::array<int,16> result{};
  for(unsigned row=0;row<16;++row)result[row]=table[amplitude*16+(row+phase)%(2*amplitude)];
  return result;
}
}
