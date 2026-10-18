#pragma once
#include "../combat/autonomy.hpp"
namespace mnm::reconstruction::combat {
// Original517580, called at segment completion before behavior continuation.
bool motionOpponentResponse(AutonomousCreature &, AutonomousWorld &,
                            const AiGeometry &);
// Original516ee0: validate the cached slot+1 before starting another segment.
bool cachedMotionOpponent(AutonomousCreature &, AutonomousWorld &,
                          const AiGeometry &);
} // namespace mnm::reconstruction::combat
