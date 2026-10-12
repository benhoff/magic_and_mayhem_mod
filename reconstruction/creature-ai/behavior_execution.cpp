#include "execution.hpp"
#include <cstdlib>
#include <stdexcept>
namespace mnm::reconstruction::combat {
void idleBehavior(AutonomousCreature &a, const BehaviorExecutionServices &s) {
  if (a.ai.type != 22 && a.ai.type != 27 && !s.immediate(a) &&
      s.interaction(a)) {
    s.action(a, 6);
    return;
  }
  if (!s.follow(a) && !s.queue(a))
    s.action(a, 1);
}
void maintainTargetBehavior(AutonomousCreature &a, CreatureExecution &,
                            AutonomousWorld &w, unsigned restoreMode,
                            const BehaviorExecutionServices &s) {
  if (a.form && !a.formCapability && a.animationKind != 3) {
    s.restoreForm(a, restoreMode);
    if (a.ai.behavior != 2) {
      s.action(a, 15);
      a.cooldown = 10;
    }
    return;
  }
  auto &b = w.creatures.at(unsigned(a.ai.target));
  auto visible = [&] {
    return a.ai.owner >= 0 && a.ai.owner < 8 &&
           b.ai.visible[unsigned(a.ai.owner)];
  };
  if (a.ai.marker)
    s.beginMotion(a);
  else if (visible() || !(a.ai.position == a.chosenDestination)) {
    s.action(a, 15);
    if (!a.routePhase)
      a.routePhase = 1;
  } else
    s.reset(a);
  if (visible())
    a.chosenDestination = b.ai.position;
}
void targetAttackBehavior(AutonomousCreature &a, CreatureExecution &e,
                          AutonomousWorld &w,
                          const BehaviorExecutionServices &s) {
  if (a.ai.target < 0 || unsigned(a.ai.target) >= w.creatures.size() ||
      !w.creatures[unsigned(a.ai.target)].ai.health) {
    s.behavior(a, 1);
    return;
  }
  auto &b = w.creatures[unsigned(a.ai.target)];
  if (s.reach(a, b, a.ai.position)) {
    e.routeRenewal = 0;
    if (a.ai.incoming) {
      s.action(a, 15);
      a.cooldown = 1;
    } else {
      e.block722 = 0;
      s.face(a, b);
      if (b.ai.motion) {
        b.targetMovingAttacker = a.ai.slot + 1;
        s.action(a, 15);
        a.cooldown = 1;
      } else
        s.action(a, 10);
    }
    e.marker6a0 = 0;
  } else if (!(a.ai.type == 18 && a.ai.flying) && a.rangedDistance != 0)
    s.ranged(a);
  else
    s.maintainTarget(a);
}
void coordinateAttackBehavior(AutonomousCreature &a, CreatureExecution &e,
                              unsigned tick,
                              const BehaviorExecutionServices &s) {
  if (s.coordinateRange(a, e.castAim, unsigned(a.rangedDistance) * 32u)) {
    if (!(a.ai.type == 18 && a.ai.flying) && a.rangedDistance != 0 &&
        tick >= e.rangedDeadline && !a.ai.readiness && !a.ai.motion) {
      s.coordinateFace(a, {int(unsigned(e.castAim.x) >> 5),
                           int(unsigned(e.castAim.y) >> 5), 0});
      s.action(a, 9);
    } else
      s.action(a, 1);
  } else if (a.ai.marker)
    s.beginMotion(a);
  else {
    s.action(a, 15);
    if (!a.routePhase)
      a.routePhase = 1;
  }
}
void immediateAttackBehavior(AutonomousCreature &a, AutonomousWorld &w,
                             const BehaviorExecutionServices &s) {
  if (a.ai.incoming) {
    s.action(a, 15);
    a.cooldown = 1;
    return;
  }
  if (!s.immediate(a)) {
    s.reset(a);
    return;
  }
  auto &b = w.creatures.at(unsigned(a.ai.target));
  s.face(a, b);
  if (b.ai.motion) {
    b.targetMovingAttacker = a.ai.slot + 1;
    s.action(a, 15);
    a.cooldown = 1;
  } else
    s.action(a, 10);
}
void standingRangedBehavior(AutonomousCreature &a, CreatureExecution &e,
                            AutonomousWorld &w, unsigned tick,
                            const BehaviorExecutionServices &s) {
  if (a.ai.incoming || s.blocked(a)) {
    s.action(a, 15);
    a.cooldown = 1;
    s.reset(a);
    return;
  }
  if ((a.ai.type == 18 && a.ai.flying) || a.rangedDistance == 0 ||
      tick < e.rangedDeadline || a.ai.readiness || a.ai.motion) {
    s.action(a, 15);
    a.cooldown = 1;
    return;
  }
  if (!s.rangedOpponent(a)) {
    s.reset(a);
    return;
  }
  const auto &b = w.creatures.at(unsigned(a.ai.target));
  s.coordinateFace(
      a, {std::int32_t(unsigned(b.finePosition.x) + unsigned(b.aimXY)),
          std::int32_t(unsigned(b.finePosition.y) + unsigned(b.aimXY)), 0});
  s.action(a, 8);
}
void displacementBehavior(AutonomousCreature &a, CreatureExecution &e,
                          const BehaviorExecutionServices &s) {
  if (a.ai.position == a.destination &&
      a.ai.position == e.displacementDestination) {
    e.displacementActive = 0;
    s.reset(a);
    return;
  }
  if (a.ai.marker) {
    s.beginMotion(a);
    return;
  }
  if (a.ai.position == a.destination) {
    a.destination = e.displacementDestination;
    a.ai.marker = 0;
  } else if (std::abs(a.ai.position.x - a.destination.x) <= 1 &&
             std::abs(a.ai.position.y - a.destination.y) <= 1 &&
             !a.routePhase) {
    e.displacementActive = 0;
    s.reset(a);
    a.destination = a.ai.position;
    return;
  } else if (std::abs(a.ai.position.x - a.destination.x) <= 1 &&
             std::abs(a.ai.position.y - a.destination.y) <= 1) {
    s.action(a, 15);
    return;
  }
  s.action(a, 15);
  if (!a.routePhase)
    a.routePhase = 1;
}
void specialBehavior(AutonomousCreature &a, CreatureExecution &e, unsigned kind,
                     const BehaviorExecutionServices &s) {
  switch (kind) {
  case 6:
    s.action(a, 13);
    break;
  case 7:
    s.action(a, 6);
    break;
  case 8:
    s.action(a, 14);
    break;
  case 14:
    e.block722 = 1;
    a.targetMovingAttacker = 0;
    s.beginMotion(a);
    break;
  case 15:
    s.action(a, 23);
    break;
  case 16:
    s.action(a, 24);
    break;
  default:
    throw std::invalid_argument("unsupported special behavior");
  }
}
} // namespace mnm::reconstruction::combat
