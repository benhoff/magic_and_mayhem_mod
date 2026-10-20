#pragma once
#include <cstdint>
#include <stdexcept>
namespace mnm::reconstruction {
// Only after admission of the declared valid owned-frame/clip domain.
// Upper EAX, flags and malformed original pointers are outside this contract.
inline std::uint16_t effectRasterReturn(unsigned backend) {
    switch(backend) {
    case 7: case 13: case 14: case 15: case 16: return 0;
    default: throw std::invalid_argument("Unrecovered effect raster return");
    }
}
}
