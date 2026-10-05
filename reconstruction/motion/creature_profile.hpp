#pragma once
#include "animation.hpp"
#include "creature_movement.hpp"
#include <array>
namespace mnm::reconstruction {
// Selected No-CD 502b20 numeric clamps; native CFG parsing retains raw values.
assets::CreatureMovementConfig normalizeCreatureMovementConfig(assets::CreatureMovementConfig);
// Ground-only slice of 505220: two parity banks derived from normalized ANI +8.
std::array<std::uint32_t,48> groundMovementSamples(const assets::Animation&);
// Ground-only 505160 maximum; flying/water-category admission remains separate.
std::uint32_t groundMovementMaximum(const std::array<std::uint32_t,48>&);
}
