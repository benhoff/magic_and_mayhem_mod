#pragma once
#include <array>
#include <cstdint>
#include <functional>
namespace mnm::reconstruction {
//4f3260 visits NW,NE,SW,SE and returns early at zero. Classification compares
// DWORDs unsigned against the initial16 ceiling. Kind0 represents absent data.
std::int32_t large_footprint_height(const std::array<std::uint16_t,4>&,
    const std::function<std::int32_t(std::uint16_t)>&);
}
