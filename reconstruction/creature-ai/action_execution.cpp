#include "execution.hpp"
#include <cstdint>
namespace mnm::reconstruction::combat {
void changeAction(AutonomousCreature &a, CreatureExecution &e,
                  AutonomousWorld &w, unsigned action,
                  const ActionEnvironment &env,
                  const ActionExecutionServices &s) {
  if (a.ai.motion && action != 2)
    return;
  if (a.ai.action == 2) {
    e.previousMove = 1;
    e.previousMoveVariant = e.moveVariant;
  } else if (a.ai.action != 15)
    e.previousMove = 0;
  a.ai.action = action;
  if (action != 2)
    e.actionRouteCounters = {};
  auto animation = [&](unsigned offset, unsigned loop = 0) {
    e.animationOffset = offset;
    s.animation(a, offset + e.animationFacing, loop);
  };
  auto overlay = [&](bool melee = false) {
    if (a.animationKind == 1) {
      s.overlay(a, melee ? env.meleeOverlay : env.regularOverlay);
      s.overlayAnimation(a,
                         e.animationFacing + (melee ? env.meleeOverlayOffset
                                                    : env.regularOverlayOffset),
                         0);
    }
  };
  auto engage = [&](int target) {
    auto &b = w.creatures.at(unsigned(target));
    s.attackAnimation(a, &b);
    a.ai.engaged = 1;
    int n = std::int32_t(b.ai.incoming + 1u);
    b.ai.incoming = n > 0 ? unsigned(n) : 1;
    overlay(true);
  };
  switch (action) {
  case 1:
    if (!a.ai.flying)
      animation(64 + ((s.random() & 0x7fffffffu) % 6) * 8);
    else
      animation(a.formCapability ? 0xf8 : 0x120);
    overlay();
    break;
  case 2:
    overlay();
    break;
  case 3:
    animation(0x128 + a.ai.corpseB * 8);
    break;
  case 6:
    animation(0x70);
    overlay();
    break;
  case 8:
  case 9:
    e.animationOffset = 0x100;
    a.ai.readiness = 1;
    s.animation(a, 0x100 + e.animationFacing, 0);
    overlay();
    break;
  case 10:
    engage(a.ai.target);
    break;
  case 13:
    animation(0x10);
    overlay();
    break;
  case 14:
    animation(0x90);
    overlay();
    break;
  case 15:
    animation(0x120);
    overlay();
    break;
  case 16:
    animation(0x108);
    overlay();
    break;
  case 17:
    animation(0x100);
    overlay();
    break;
  case 18:
    s.attackAnimation(a, nullptr);
    overlay();
    break;
  case 19:
    animation(0xe8);
    overlay();
    break;
  case 20:
    engage(a.alternateTarget);
    break;
  case 21:
    animation(0x38);
    if (a.animationKind == 1)
      s.repairOverlay(a);
    break;
  case 22:
    animation(0xe8, 1);
    break;
  case 23:
    animation(0x120);
    if (a.animationKind == 1)
      s.repairOverlay(a);
    break;
  case 24:
    if (!s.flightAnimation(a))
      animation(0x120);
    if (a.animationKind == 1)
      s.repairOverlay(a);
    break;
  case 25:
    animation(0x100);
    if (a.animationKind == 1)
      s.repairOverlay(a);
    break;
  default:
    break;
  }
  e.actionCallback = actionHandler(action);
}
void tickRangedAction(AutonomousCreature &a, CreatureExecution &e,
                      bool alternate, unsigned tick, unsigned recovery,
                      unsigned restoreMode, const AnimationActionServices &s) {
  int event = s.poll(a, false);
  if (a.animationKind == 1)
    s.poll(a, true);
  if (event == 2) {
    if (alternate || !e.block855)
      s.cast(a);
  } else if (event == 1) {
    a.ai.readiness = 0;
    e.rangedDeadline = tick + recovery;
    if (alternate && a.form && !a.formCapability && a.animationKind != 3) {
      s.restoreForm(a, restoreMode);
      if (a.ai.behavior == 2)
        return;
    }
    s.behavior(a);
  }
}
void tickReadyAction(AutonomousCreature &a, CreatureExecution &e,
                     const AnimationActionServices &s) {
  if (a.animationKind == 1)
    s.poll(a, true);
  int event = s.poll(a, false);
  if (event == 1) {
    if (a.cooldown > 0) {
      e.animationOffset = 0x120;
      s.animation(a, 0x120 + e.animationFacing, 0);
    } else
      s.behavior(a);
  }
  if (a.cooldown > 0)
    --a.cooldown;
  if (a.cooldown <= 0)
    s.behavior(a);
}
} // namespace mnm::reconstruction::combat
