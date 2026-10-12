#include "death_behavior.hpp"
#include <algorithm>
namespace mnm::reconstruction::combat {
namespace {
bool ordinaryClass(unsigned kind) {
  return kind != 0 && kind != 24 && kind != 25 && kind != 26;
}
} // namespace
void deathBehavior(AutonomousCreature &a, const CreatureMaintenance &m,
                   const MovementBehaviorState &motion,
                   const DeathEnvironment &env, const DeathServices &s) {
  bool fresh = !a.ai.health && !a.ai.corpseB && !a.ai.corpseA;
  bool animate = a.ai.type != 0 && a.ai.type != 22 && a.ai.type != 24 &&
                 a.ai.type != 25 && a.ai.type != 26;
  if (a.ai.action != 19 && a.ai.action != 3 && animate && fresh) {
    s.action(a, 19);
    return;
  }
  if (a.ai.action == 19 || !animate) {
    if (a.ai.type == 8) {
      int power =
          std::int32_t(unsigned(m.status78c) * unsigned(env.explosionScale) +
                       unsigned(env.explosionBase));
      s.explode(a, std::min(150, power), env.explosionMode);
      return;
    }
    if (a.ai.type == 6) {
      s.specialRemoval(a);
      return;
    }
    if (a.ai.type != 14 && a.ai.type != 11 &&
        (a.ai.physical || !env.corpseConfigured || a.block7c ||
         !ordinaryClass(a.ai.kind))) {
      s.remove(a);
      return;
    }
  }
  if (a.ai.gateFC) {
    a.ai.gateFC = 0;
    s.releaseUpkeep(a);
  }
  if (fresh) {
    if (ordinaryClass(a.ai.kind))
      s.registerCorpse(a);
    s.releaseOccupancy(a, a.ai.position);
    if (a.ai.motion)
      s.releaseOccupancy(a, motion.nextRouteCell);
  }
  a.ai.corpseA = env.corpseLifetime;
  if (a.ai.corpseB == 4) {
    s.releaseReference(a);
    s.remove(a);
  } else
    s.action(a, 3);
}
} // namespace mnm::reconstruction::combat
