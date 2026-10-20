#pragma once
#include <algorithm>
#include <cstdint>
#include <stdexcept>
namespace mnm::render {
// Selected581200 RGB565 colour conversion. Inputs retain their raw DWORDs;
// the original consumes each low byte. This bounded shade domain avoids the
// original's wider signed arithmetic and low-WORD comparison policies.
inline std::uint16_t tintColour565(std::uint32_t red,std::uint32_t green,std::uint32_t blue,int shade){
    if(shade< -127||shade>127)throw std::invalid_argument("Tint shade outside recovered domain");
    const auto scaled=[shade](std::uint32_t value){return std::min(((value&255u)*unsigned(shade+127))/127u,255u);};
    return std::uint16_t((scaled(red)>>3)<<11|(scaled(green)>>2)<<5|(scaled(blue)>>3));
}
}
