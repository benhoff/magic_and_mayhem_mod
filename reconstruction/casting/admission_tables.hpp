#pragma once
#include <array>
namespace mnm::reconstruction::casting {
// Pinned compressed table57c678; indices select original57c634 arms.
inline constexpr std::array<unsigned, 103> castArms{
    {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  16, 16, 16, 16, 16, 16, 16, 16, 16, 0,  1,  0,
     16, 16, 0,  16, 16, 2,  2,  0,  2,  16, 2,  16, 16, 16, 2,  16, 2,  16,
     2,  3,  16, 16, 0,  16, 2,  16, 4,  16, 16, 5,  16, 16, 0,  5,  5,  6,
     7,  8,  2,  0,  0,  0,  2,  2,  2,  0,  2,  2,  2,  2,  2,  9,  10, 6,
     11, 5,  5,  12, 4,  12, 4,  13, 16, 6,  14, 15, 4}};
} // namespace mnm::reconstruction::casting
