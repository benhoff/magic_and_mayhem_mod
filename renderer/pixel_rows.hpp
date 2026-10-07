#pragma once
#include "blit.hpp"
#include <cstddef>

namespace mnm::render {
// Owned byte storage. firstRow is the logical top-row offset, including for
// negative pitch; padding and guard bytes are not native pixels.
struct PixelRows {
    int width=0,height=0;
    std::int32_t pitch=0;
    std::size_t firstRow=0;
    std::vector<std::uint8_t> bytes;
};
void validateSurfaceFormat(PixelFormat);
Image unpackPixelRows(const PixelRows&,PixelFormat);
// Validate the complete layout and every word before changing any byte.
// Only active pixel bytes are written; caller padding/guards stay intact.
void packPixelRows(const Image&,PixelFormat,PixelRows&);
}
