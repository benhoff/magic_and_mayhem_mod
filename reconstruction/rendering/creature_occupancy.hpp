#pragma once
#include <array>
#include <cstdint>
namespace mnm::reconstruction {
// Owned input to recovered 004e1200; each MSB-first row spans 64 fine units.
// Height is a raw DWORD, shifted left four with original 32-bit wrapping.
struct CreatureOccupancy {
    std::array<std::uint16_t,16> rows{};
    std::uint32_t height=0;
    bool occupied(std::int32_t x,std::int32_t y,std::int32_t z) const noexcept;
};
}
