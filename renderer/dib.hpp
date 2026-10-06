#pragma once
#include <array>
#include <cstdint>
#include <vector>

namespace mnm::render {
struct Image;
// Owned original loader inputs, independent of file offsets and host pointers.
struct DibInput {
    std::array<std::uint8_t,40> header{};
    std::vector<std::uint8_t> palette,pixels;
    unsigned usage=0;
};
// Bounded BI_RGB, positive-height 1/4/8-bit RGB palette and 24-bit BGR.
// Returns top-down 0x00RRGGBB pixels. Other layouts/usage are explicit refusals.
Image decodeDibRgb(const DibInput&);
}
