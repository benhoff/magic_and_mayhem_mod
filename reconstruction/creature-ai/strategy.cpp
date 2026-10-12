#include "execution.hpp"
#include <algorithm>
#include <cstdlib>
#include <stdexcept>
namespace mnm::reconstruction::combat {
RoamChoice chooseRoamRegion(const AutonomousCreature &a,
                            const CreatureExecution &e,
                            const AutonomousWorld &w,
                            const StrategyEnvironment &env,
                            const StrategyServices &s) {
  if (env.regionCapacity <= 1 || env.regionCapacity > int(env.regions.size()))
    throw std::invalid_argument("original roaming region domain");
  int index = int(s.random() & 0x7fffffffu) % (env.regionCapacity - 1);
  for (int tried = 0; tried < env.regionCapacity; ++tried) {
    Cell p = env.regions.at(unsigned(index));
    if (creatureDistance(e.roamHome, p, w.dimensions) <=
            std::int32_t(unsigned(e.roamRange) * 2u) &&
        !s.insideInnerRadius(a.ai.position, p,
                             std::min(e.roamAnnulus, e.roamRange) - 1)) {
      bool blocked = false;
      for (auto column : env.blockedColumns)
        if (column.x == p.x && column.y == p.y &&
            std::abs(column.z - p.z) <= 1) {
          blocked = true;
          break;
        }
      if (!blocked && s.legal(a, p)) {
        bool admitted = false;
        switch (unsigned(e.roamKind)) {
        case 0:
          admitted = true;
          break;
        case 1:
          admitted = s.water(a, p);
          break;
        case 2:
          admitted = a.flag104 && !s.water(a, p) && p.z >= env.waterline;
          break;
        case 3:
          admitted = s.water(a, p) &&
                     unsigned(p.z) >= unsigned(env.waterline - int(a.height)) &&
                     p.z < env.waterline;
          break;
        default:
          break;
        }
        if (admitted)
          return {true, index};
      }
    }
    if (++index >= env.regionCapacity)
      index = 0;
  }
  return {false, index};
}
void strategicCreatureWork(AutonomousCreature &a, CreatureExecution &e,
                           AutonomousWorld &w, const StrategyEnvironment &env,
                           const StrategyServices &s) {
  if (e.strategyGuard) {
    for (const auto &scenario : env.scenarios) {
      if (!scenario.active || scenario.kind != 1 || scenario.state != 1)
        continue;
      auto actor = env.scenarioActor(scenario);
      if (actor && *actor == a.ai.slot)
        return;
    }
  }
  const AutonomousCreature *nearest = nullptr;
  int best = 0;
  for (const auto &b : w.creatures) {
    if (!b.ai.active || !b.ai.health || b.ai.slot == a.ai.slot ||
        a.ai.owner < 0 || a.ai.owner >= 8 || b.ai.owner < 0 ||
        b.ai.owner >= 8 ||
        w.relation[unsigned(a.ai.owner)][unsigned(b.ai.owner)] ||
        !b.ai.visible[unsigned(a.ai.owner)])
      continue;
    int distance = creatureDistance(a.ai.position, b.ai.position, w.dimensions);
    if ((!nearest || distance < best) && s.admit(a, b)) {
      nearest = &b;
      best = distance;
    }
  }
  if (nearest) {
    int radius = std::int32_t(unsigned(a.ai.enemyRadius.at(nearest->ai.type)) +
                              unsigned(e.fearBias));
    if (radius > 0) {
      if (best <= std::int32_t(unsigned(radius) * 2u))
        s.attack(a, *nearest);
    } else if (best <= std::int32_t((0u - unsigned(radius)) * 2u)) {
      a.range = std::int32_t((5u - unsigned(radius)) * 2u);
      if (a.ai.engaged) {
        if (a.ai.action == 10) {
          auto &b = w.creatures.at(unsigned(a.ai.target));
          a.ai.engaged = 0;
          int n = std::int32_t(b.ai.incoming - 1u);
          b.ai.incoming = n < 0 ? 0 : unsigned(n);
        } else if (a.ai.action == 20) {
          auto &b = w.creatures.at(unsigned(a.alternateTarget));
          a.ai.engaged = 0;
          s.releaseAlternate(a, b);
        }
      }
      if (a.ai.action == 10 || a.ai.action == 20)
        s.action(a, 15);
      a.ai.target = int(nearest->ai.slot);
      s.substate(a, 2);
    }
  }
  if (a.region == -1 || e.roamRange <= 0 || a.ai.behavior != 1)
    return;
  if (e.roamCooldown > 0)
    e.roamCooldown = std::max(0, e.roamCooldown - 90);
  if (e.roamCooldown > 0)
    return;
  auto choice = chooseRoamRegion(a, e, w, env, s);
  if (!choice.admitted)
    return;
  if (s.regionReach(a, a.region, choice.region, 250)) {
    s.move(a, env.regions.at(unsigned(choice.region)));
    e.roamCooldown = e.roamInterval;
  }
}
} // namespace mnm::reconstruction::combat
