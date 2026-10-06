#pragma once
#include "effect_trajectory.hpp"

namespace mnm::reconstruction {
// Whole 004df3a0 contract, including its 004df260 XY helper. All inputs are
// raw 32-bit words. Periods replace globals 006c5494/006c5498; the original
// caller supplies a multiplier (e.g. 32). State, inputs and counter are disjoint.
// Word 3 is intentionally preserved, not initialized by the original routine.
void initializeEffectTrajectory(EffectTrajectory& state,
    const std::array<std::uint32_t,3>& from,
    const std::array<std::uint32_t,3>& to,
    const std::array<std::uint32_t,2>& periods,
    std::uint32_t multiplier,std::uint32_t& changes) noexcept;
}
