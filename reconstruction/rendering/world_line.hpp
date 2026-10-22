#pragma once
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace mnm::reconstruction {
struct WorldLinePoint { std::int32_t x,y; };
// 4df840: strict > tie rule, inclusive endpoint and original forward direction.
// Admission bounds guarantee finite owned output and avoid original INT_MIN/
// doubled-delta overflow. No original memory address is accepted here.
inline std::vector<WorldLinePoint> worldLine(std::int32_t x,std::int32_t y,
        std::int32_t endX,std::int32_t endY) {
    const auto dx=std::int64_t(endX)-x,dy=std::int64_t(endY)-y;
    const auto ax=dx<0?-dx:dx,ay=dy<0?-dy:dy;
    if (std::max(ax,ay)>8192)throw std::invalid_argument("Unbounded World line");
    std::vector<WorldLinePoint> result;
    result.reserve(std::size_t(std::max(ax,ay))+1);
    const int sx=dx<0?-1:1,sy=dy<0?-1:1;
    std::int64_t error=0;
    if (ax>ay) {
        while(x!=endX) {
            result.push_back({x,y});x+=sx;error+=2*ay;
            if(error>ax){y+=sy;error-=2*ax;}
        }
    } else {
        while(y!=endY) {
            result.push_back({x,y});y+=sy;error+=2*ax;
            if(error>ay){x+=sx;error-=2*ay;}
        }
    }
    result.push_back({x,y});return result;
}
inline std::uint16_t addWorldPaletteWord(std::uint16_t destination,
        std::uint16_t source,bool rgb555) {
    const unsigned redShift=rgb555?10:11,greenMax=rgb555?31:63;
    const auto red=std::min(31u,unsigned(destination>>redShift&31)+unsigned(source>>redShift&31));
    const auto green=std::min(greenMax,unsigned(destination>>5&greenMax)+unsigned(source>>5&greenMax));
    const auto blue=std::min(31u,unsigned(destination&31)+unsigned(source&31));
    return std::uint16_t(red<<redShift|green<<5|blue);
}
}
