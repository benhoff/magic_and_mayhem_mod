#pragma once
#include <array>
#include <cstdint>

namespace mnm::reconstruction {
// Owned 0x38-byte trajectory state consumed by original 004df500.
// Initialization is deliberately separate from the recovered stepping contract.
struct EffectTrajectory {
    std::array<std::uint32_t,14> words{};
    void step(std::array<std::uint32_t,3>& units,std::uint32_t& changes) noexcept;
};
}
