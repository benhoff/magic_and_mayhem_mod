#include "special_flight_actions.hpp"
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
int bits(unsigned u) { return u <= 0x7fffffffu ? int(u) : -1 - int(~u); }
int add(int a, int b) { return bits(unsigned(a) + unsigned(b)); }
int normalize(int n, int size) {
  n %= size;
  return n < 0 ? n + size : n;
}
void requireWorld(const SpecialFlightEnvironment &env) {
  if (env.dimensions.x <= 0 || env.dimensions.y <= 0 || env.dimensions.z <= 0)
    throw std::invalid_argument("owned special-action dimensions");
}
Cell normalized(Cell p, const SpecialFlightEnvironment &env) {
  return {normalize(p.x, env.dimensions.x), normalize(p.y, env.dimensions.y),
          p.z};
}
void moveLive(AutonomousCreature &a, Cell destination,
              const SegmentCompletionState &r, const SpecialFlightServices &s) {
  const Cell from = a.ai.position;
  s.release(a, from);
  if (a.ai.motion)
    s.release(a, r.route.at(r.cursor).position);
  s.snap(a, destination, a.ai.type);
  s.reserve(a, destination);
  s.environment(a);
  s.animationUpdate(a);
  s.terrain(a);
}
Cell descendColumn(const AutonomousCreature &a, Cell p,
                   const SpecialFlightServices &s) {
  unsigned steps = 0;
  while (!s.ground(a, p) && p.z > 0) {
    if (++steps > 4096)
      throw std::invalid_argument("owned descent column bound");
    Cell below = p;
    below.z = add(below.z, -1);
    if (!s.passable(a, below))
      break;
    p = below;
  }
  return p;
}
void disengage(AutonomousCreature &a, AutonomousWorld &w) {
  if (a.ai.engaged && (a.ai.action == 10 || a.ai.action == 20)) {
    auto &peer = w.creatures.at(
        unsigned(a.ai.action == 10 ? a.ai.target : a.alternateTarget));
    a.ai.engaged = 0;
    const int incoming = bits(peer.ai.incoming - 1u);
    peer.ai.incoming = incoming < 0 ? 0 : unsigned(incoming);
  }
}
} // namespace
void tickFinishingAction(AutonomousCreature &a, CreatureExecution &e,
                         const SpecialFlightServices &s) {
  if (a.animationKind == 1)
    s.poll(a, true);
  if (s.poll(a, false) != 1)
    return;
  e.block722 = 0;
  s.completeSegment(a);
  a.ai.behavior = a.ai.pending;
  s.stun(a, 20, 0, 0);
}
void tickAscendingAction(AutonomousCreature &a, CreatureExecution &e,
                         MotionActionState &m, MovementBehaviorState &landing,
                         const SegmentCompletionState &r, AutonomousWorld &w,
                         const SpecialFlightEnvironment &env,
                         const SpecialFlightServices &s) {
  requireWorld(env);
  m.direction = unsigned(add(bits(m.direction), 1));
  if (bits(m.direction) > 7)
    m.direction = 0;
  s.facing(a);
  e.animationOffset = 0x120;
  s.animation(a, 0x120 + e.animationFacing, 0);
  if (s.poll(a, false) == 1)
    s.rewind(a);
  Cell above = a.ai.position;
  above.z = add(above.z, 1);
  if (unsigned(add(a.ai.position.z, bits(a.height)) + 1u) <
          unsigned(env.dimensions.z) &&
      !s.occupied(a, above) && s.passable(a, normalized(above, env))) {
    if (a.block71c)
      return;
    moveLive(a, above, r, s);
    return;
  }
  bool relocated = false;
  for (unsigned attempt = 0; attempt < 5; ++attempt) {
    Cell raw{int((s.random() & 0x7fffffffu) % unsigned(env.dimensions.y)), 0,
             a.ai.position.z};
    if (env.diagnostic)
      s.diagnostic(0x42d6);
    raw.y = int((s.random() & 0x7fffffffu) % unsigned(env.dimensions.x));
    if (env.diagnostic)
      s.diagnostic(0x42d7);
    if (s.occupied(a, raw) || !s.passable(a, normalized(raw, env)))
      continue;
    if (a.ai.kind == 0 || a.ai.kind == 24 || a.ai.kind == 25 ||
        a.ai.kind == 26) {
      const Cell source = descendColumn(a, a.ai.position, s);
      const int sourceRegion = s.regionAt(a, normalized(source, env));
      if (sourceRegion != -1) {
        const Cell endpoint = descendColumn(a, normalized(raw, env), s);
        const int destinationRegion = s.regionAt(a, normalized(endpoint, env));
        if (destinationRegion == -1 ||
            !s.regionRoute(a, sourceRegion, destinationRegion))
          continue;
      }
    }
    // The relocation keeps the actor's current layer, independently of the
    // descended columns used only for connectivity admission.
    raw.z = a.ai.position.z;
    moveLive(a, normalized(raw, env), r, s);
    relocated = true;
    break;
  }
  if (relocated && !a.ai.active)
    return;
  if (a.flag104 && !s.ground(a, normalized(a.ai.position, env)))
    a.ai.flying = 1;
  if (a.ai.type == 27) {
    s.reset(a);
    return;
  }
  landing.landingKind = unsigned(a.ai.position.z);
  a.ai.marker = 0;
  a.blockDE3 = 0;
  e.block962 = 0;
  e.block963 = 0;
  a.ai.readiness = 0;
  disengage(a, w);
  s.behavior(a, 16);
}
void tickDescendingAction(AutonomousCreature &a, CreatureExecution &e,
                          const SegmentCompletionState &r,
                          const MovementBehaviorState &landing,
                          const SpecialFlightEnvironment &env,
                          const SpecialFlightServices &s) {
  requireWorld(env);
  if (s.poll(a, false) == 1)
    s.rewind(a);
  const bool dead = !a.ai.health && (a.ai.corpseA || a.ai.corpseB);
  Cell below = a.ai.position;
  below.z = add(below.z, -1);
  if (a.ai.position.z > 0 && !s.occupied(a, below) &&
      s.passable(a, normalized(below, env)) &&
      !s.ground(a, normalized(a.ai.position, env))) {
    if (a.movementKind == 2 &&
        unsigned(a.ai.position.z) <= env.waterline - a.height) {
      if (!a.ai.health)
        s.behavior(a, 5);
      else
        s.reset(a);
      return;
    }
    if (dead) {
      s.relocateDead(a, below);
      return;
    }
    moveLive(a, below, r, s);
    return;
  }
  const unsigned flight = a.flag104;
  a.flag104 = 0;
  Cell candidate = normalized(a.ai.position, env);
  if (s.adjustedLanding(a, candidate, 3, true)) {
    if (dead)
      s.relocateDead(a, candidate);
    else
      moveLive(a, candidate, r, s);
    if (!a.ai.active)
      return;
  }
  a.destination = a.ai.position;
  a.flag104 = flight;
  a.ai.flying = 0;
  if (!a.ai.health) {
    s.behavior(a, 5);
    return;
  }
  if (a.mode) {
    if (s.response(a))
      return;
    const unsigned mode = a.mode;
    a.mode = 0;
    s.stun(a, e.absorptionTimer, mode, 1);
  } else
    s.reset(a);
  const unsigned damage =
      a.ai.type == 15 ? a.ai.health
                      : s.fallDamage(a, landing.landingKind, a.ai.position.z);
  s.defended(a, damage, -1, false);
}
} // namespace mnm::reconstruction::combat
