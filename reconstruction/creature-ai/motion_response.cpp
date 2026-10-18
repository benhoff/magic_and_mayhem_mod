#include "motion_response.hpp"
#include <cstdlib>
#include <stdexcept>
namespace mnm::reconstruction::combat {
bool cachedMotionOpponent(AutonomousCreature &a, AutonomousWorld &w,
                          const AiGeometry &g) {
  if (a.mode || a.block784 || !a.targetMovingAttacker)
    return false;
  auto &b = w.creatures.at(a.targetMovingAttacker - 1);
  if (a.ai.owner < 0 || a.ai.owner >= 8 || b.ai.owner < 0 || b.ai.owner >= 8)
    throw std::invalid_argument("cached motion response owner");
  if (b.ai.active && b.ai.health && a.ai.slot != b.ai.slot &&
      !w.relation[a.ai.owner][b.ai.owner] && g.reach(a, b, a.ai.position) &&
      b.ai.action != 14)
    return true;
  a.targetMovingAttacker = 0;
  a.alternateTarget = -1;
  return false;
}
bool motionOpponentResponse(AutonomousCreature &a, AutonomousWorld &w,
                            const AiGeometry &g) {
  if (a.specialVision && !a.ai.engaged && !a.ai.incoming && !a.ai.readiness)
    return false;
  if (a.ai.owner < 0 || a.ai.owner >= 8)
    throw std::invalid_argument("motion response owner");
  auto distance = [](int x, int y, int size) {
    int v = std::abs(x - y);
    return v > size / 2 ? size - v : v;
  };
  for (auto &b : w.creatures) {
    if (!b.ai.active || !b.ai.health || a.ai.slot == b.ai.slot ||
        b.ai.owner < 0 || b.ai.owner >= 8 ||
        w.relation[a.ai.owner][b.ai.owner] || !b.ai.visible[a.ai.owner])
      continue;
    if (std::abs(a.ai.position.z - b.ai.position.z) > 2 ||
        distance(a.ai.position.x, b.ai.position.x, w.dimensions.x) > 3 ||
        distance(a.ai.position.y, b.ai.position.y, w.dimensions.y) > 3)
      continue;
    if (!g.reach(a, b, a.ai.position))
      continue;
    a.targetMovingAttacker = b.ai.slot + 1;
    if (b.ai.motion)
      b.targetMovingAttacker = a.ai.slot + 1;
    return true;
  }
  return false;
}
} // namespace mnm::reconstruction::combat
