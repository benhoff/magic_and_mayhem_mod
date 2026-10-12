#include "actor_routing.hpp"
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
const AutonomousCreature *actorOrNull(const AutonomousWorld &w, int slot) {
  return unsigned(slot) < w.creatures.size() ? &w.creatures[unsigned(slot)]
                                             : nullptr;
}
int signedWord(unsigned u) { return u <= 0x7fffffffu ? int(u) : -1 - int(~u); }
} // namespace
bool displaceCreatureRoute(AutonomousCreature &a, AutonomousWorld &w,
                           const std::vector<int> &owners,
                           const std::vector<CreatureExecution> &peers,
                           ActorRouteContext &c,
                           const ActorDisplacementServices &s) {
  if (c.active == 1 && !c.displacementContinuation)
    return false;
  if (a.ai.behavior == 3) {
    if (s.meleeReached(a, actorOrNull(w, a.ai.target), a.ai.position))
      return false;
  } else if (a.ai.behavior == 11 &&
             (a.substate == 1 ||
              (signedWord(a.substate) > 2 && signedWord(a.substate) <= 5)) &&
             a.ai.target != -1 &&
             s.rangedReached(a, actorOrNull(w, a.ai.target)))
    return false;
  const unsigned blocker = s.blocker(a);
  if (!blocker || !s.canDisplace(a, blocker - 1u))
    return false;
  const unsigned slot = blocker - 1u;
  if (!c.active) {
    const auto &b = w.creatures.at(slot);
    if (b.ai.behavior == 13 && owners.at(slot) == int(a.ai.slot) &&
        b.ai.position == peers.at(slot).displacementDestination) {
      const unsigned roll = (s.random() & 0x7fffffffu) % 100u;
      if (c.diagnostic)
        s.diagnostic(0x333b);
      if (roll >= 10) {
        s.action(a, 15);
        a.cooldown = 6;
        a.ai.marker = 1;
        a.routePhase = 0;
        return true;
      }
    }
  }
  unsigned updateResult = 0;
  if (s.obstacle(a, a.destination) != blocker) {
    updateResult = s.update(a, c);
    if (c.active) {
      c.displacementContinuation = 1;
      return true;
    }
  } else
    c.active = 0;
  c.displacementContinuation = 0;
  if (updateResult)
    return false;
  if (a.ai.behavior == 13 && owners.at(a.ai.slot) == int(slot)) {
    a.routePhase = 0;
    s.reset(a);
    a.destination = a.ai.position;
    return true;
  }
  if (!s.issue(a, slot))
    return false;
  a.routePhase = 0;
  return true;
}
} // namespace mnm::reconstruction::combat
