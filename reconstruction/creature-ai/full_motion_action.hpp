#pragma once
#include "action_callbacks.hpp"
namespace mnm::reconstruction::combat {
struct MotionActionState {
  int accumulator = 0, duration = 1, progress = 0, travelX = 0, travelY = 0,
      heightDelta = 0, heightStep = 0, heightOrigin = 0;
  unsigned direction = 0, sampleCursor = 0, initialCursor = 0;
  std::array<int, 48> samples{};
  int initialResidualX = 0, initialResidualY = 0;
  bool vertical = false, displacementPause = false, auxiliarySprite = false;
};
struct MotionActionServices {
  std::function<int(AutonomousCreature &, bool)> poll;
  std::function<void(AutonomousCreature &)> rewind, completeSegment,
      runBehavior, reset, updateOrders, visualFacing;
  std::function<void(AutonomousCreature &, unsigned)> behavior, bindBehavior,
      action;
  std::function<bool(AutonomousCreature &)> response;
  std::function<bool(const AutonomousCreature &, Cell)> legal;
  std::function<void(AutonomousCreature &, unsigned, unsigned)> stun;
  std::function<void(AutonomousCreature &, unsigned, Cell)> effect;
  std::function<void(AutonomousCreature &)> footstep;
  std::function<void(unsigned)> releaseEffect;
  unsigned cameraTurn = 0;
};
void tickMotionAction(AutonomousCreature &, CreatureExecution &,
                      MotionActionState &, MovementBehaviorState &,
                      AutonomousWorld &, const MotionActionServices &);
} // namespace mnm::reconstruction::combat
