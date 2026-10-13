#pragma once
#include <array>
#include <cstdint>
namespace mnm::reconstruction::casting {
using PackedCastCommand = std::array<std::uint8_t, 33>;
using CastDescriptor = std::array<std::uint32_t, 12>;
//534000: input/output are distinct owned values. Output word9 remains owned by
//the caller; zero-initialization here would change the recovered contract.
void expandCastDescriptor(const PackedCastCommand &, CastDescriptor &);
}
