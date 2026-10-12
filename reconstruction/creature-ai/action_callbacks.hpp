#pragma once
#include "death_behavior.hpp"
namespace mnm::reconstruction::combat {
enum class ActionCue { idle, interaction, strike, pain, remove };
struct ActionCallbackState {
  unsigned sequence = 0;
  std::array<int, 2> contributors{{-1, -1}};
  bool animationBreak = false;
};
struct ActionCallbackEnvironment {
  unsigned overlayDamage = 0;
  bool diagnostic = false;
};
struct ActionCallbackServices {
  std::function<int(AutonomousCreature &, bool)> poll;
  std::function<bool(AutonomousCreature &)> immediate, rangedOpponent;
  std::function<void(AutonomousCreature &, unsigned)> behavior, action;
  std::function<void(AutonomousCreature &)> runBehavior, reset, rewind, hit,
      interactionHit, notifyHit;
  std::function<void(AutonomousCreature &, unsigned, Cell)> effect;
  std::function<void(AutonomousCreature &, ActionCue, unsigned)> cue;
  std::function<void(AutonomousCreature &, unsigned, int,
                     const std::array<int, 2> &)>
      defendedDamage;
  std::function<void(AutonomousCreature &)> releaseEngagement;
  std::function<unsigned()> random;
  std::function<void(unsigned)> diagnostic;
  std::function<void(AutonomousCreature &, unsigned, unsigned)> stun;
  std::function<void(AutonomousCreature &, unsigned)> drainForm, clearColor,
      pollColor;
  std::function<void(unsigned)> releaseEffect;
  std::function<void(AutonomousCreature &)> updateOrders;
};
void tickIdleAction(AutonomousCreature &, CreatureExecution &,
                    AutonomousWorld &, const ActionCallbackState &,
                    const ActionCallbackServices &);
void tickCorpseAction(AutonomousCreature &, const ActionCallbackServices &);
void tickMeleeAction(AutonomousCreature &, CreatureExecution &,
                     AutonomousWorld &, const CreatureMaintenance &,
                     const ActionCallbackState &, bool alternate,
                     const ActionCallbackEnvironment &,
                     const ActionCallbackServices &);
void tickInteractionAction(AutonomousCreature &,
                           const ActionCallbackEnvironment &,
                           const ActionCallbackServices &);
void tickStunAction(AutonomousCreature &, CreatureExecution &,
                    ActionCallbackState &, const ActionCallbackServices &);
} // namespace mnm::reconstruction::combat
