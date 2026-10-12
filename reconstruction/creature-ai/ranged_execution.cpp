#include "execution.hpp"
namespace mnm::reconstruction::combat {
unsigned rangedStyle(unsigned kind) {
  if (kind == 1 || kind == 3)
    return 2;
  if (kind == 2 || kind == 10)
    return 1;
  return 0;
}
void rangedTactic(AutonomousCreature &a, CreatureExecution &e,
                  AutonomousWorld &w, unsigned style, unsigned tick,
                  const RangedExecutionServices &s) {
  auto &target = w.creatures.at(unsigned(a.ai.target));
  const auto range = unsigned(a.rangedDistance) * 32u;
  auto ready = [&] {
    return !(a.ai.type == 18 && a.ai.flying) && a.rangedDistance != 0 &&
           tick >= e.rangedDeadline && !a.ai.readiness && !a.ai.motion;
  };
  auto cast = [&] {
    e.block722 = 0;
    s.orient(a, {target.finePosition.x + target.aimXY,
                 target.finePosition.y + target.aimXY, 0});
    s.action(a, 8);
    e.marker6a0 = 0;
  };
  auto wait = [&] {
    a.cooldown = 1;
    s.action(a, 15);
    e.marker6a0 = 0;
  };
  if (style == 0) {
    if (!e.rangedLatch && ready() && !s.blocked(a) &&
        s.inRange(a, target, range)) {
      cast();
      e.rangedLatch = 1;
    } else {
      s.maintainTarget(a);
      e.rangedLatch = 0;
    }
  } else if (style == 1) {
    if (s.blocked(a) || !s.inRange(a, target, range))
      s.maintainTarget(a);
    else if (ready())
      cast();
    else
      wait();
  } else if (style == 2) {
    if (s.blocked(a) || !s.inRange(a, target, range)) {
      s.maintainTarget(a);
      e.targetDistance = 0;
      return;
    }
    int distance =
        creatureDistance(a.ai.position, target.ai.position, w.dimensions);
    if (distance < int(e.targetDistance) &&
        distance < int((unsigned(a.rangedDistance) * 3u) >> 1) &&
        s.closeResponse(a, a.ai.position, target.ai.position) &&
        s.prepareResponse(a)) {
      e.targetDistance = 0;
      return;
    }
    e.targetDistance = unsigned(distance);
    if (ready())
      cast();
    else
      wait();
  }
}
} // namespace mnm::reconstruction::combat
