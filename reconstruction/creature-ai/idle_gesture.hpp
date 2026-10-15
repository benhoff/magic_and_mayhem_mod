#pragma once
#include "../combat/autonomy.hpp"
namespace mnm::reconstruction::combat {
// Original516800, distinct from the object selector51d7b0. The probability
// draw precedes the species table, including species that cannot gesture.
bool idleGesture(AutonomousCreature &, AutonomousWorld &, const AiGeometry &,
                 const std::array<int, 28> &scores,
                 const std::function<unsigned()> &random,
                 const std::function<void(AutonomousCreature &, Cell)> &orient);
} // namespace mnm::reconstruction::combat
