#include "full_segment_setup.hpp"
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
int bits(unsigned u) { return u <= 0x7fffffffu ? int(u) : -1 - int(~u); }
int add(int a, int b) { return bits(unsigned(a) + unsigned(b)); }
int neg(int v) { return bits(0u - unsigned(v)); }
int difference(int a, int b, int dimension) {
  if (dimension <= 0 || a < 0 || b < 0 || a >= dimension || b >= dimension)
    throw std::invalid_argument("owned segment coordinates");
  int d = a - b;
  if (d < 0)
    d += dimension;
  if (d > dimension / 2)
    d -= dimension;
  return d;
}
} // namespace
void initializeMotionSegment(AutonomousCreature &a, CreatureExecution &e,
                             MotionActionState &m,
                             SegmentInitializationState &p,
                             const SegmentCompletionState &r,
                             AutonomousWorld &w,
                             const SegmentInitializationEnvironment &env,
                             const SegmentInitializationServices &s) {
  if (r.count > r.route.size())
    throw std::invalid_argument("owned segment count");
  if (r.cursor >= r.count) {
    a.ai.marker = 0;
    return;
  }
  if (a.ai.type != 27 && a.ai.type != 22 && s.precheck(a) && !e.routeRenewal &&
      s.immediate(a))
    return;
  const auto &q = r.route[r.cursor];
  if (unsigned(q.remainder[3]) > 4)
    throw std::invalid_argument("owned segment movement mode");
  const unsigned oldDirection = m.direction, oldMode = e.mode10c;
  const int oldVertical = p.vertical;
  p.previousDirection = oldDirection;
  m.direction = unsigned(q.remainder[0]);
  p.vertical = q.remainder[1];
  m.vertical = p.vertical != 0;
  e.animationFacing = (m.direction - 2u * env.cameraTurn) & 7u;
  if (e.block722)
    e.animationFacing = (e.animationFacing - 4u) & 7u;
  s.visualFacing(a);
  a.ai.flying = unsigned(q.remainder[2]);
  e.mode10c = unsigned(q.remainder[3]);
  const auto from = a.ai.position, to = q.position;
  const Cell delta{difference(to.x, from.x, w.dimensions.x),
                   difference(to.y, from.y, w.dimensions.y),
                   add(to.z, neg(from.z))};
  if (w.dimensions.z <= 0 || to.z < 0 || to.z >= w.dimensions.z || from.z < 0 ||
      from.z >= w.dimensions.z)
    throw std::invalid_argument("owned segment layer");
  unsigned argument = 0;
  int length = 0;
  const auto rate = [&] {
    if (a.block723) {
      e.moveVariant = p.specialRate73c;
      return;
    }
    argument = s.scalarArgument(from, to);
    m.duration = bits(s.base(a, e.mode10c, delta));
    int adjusted = m.duration;
    if (p.slow778 > 0)
      adjusted = add(adjusted, adjusted);
    if (p.fast77c > 0)
      adjusted /= 2;
    s.scalar(a, argument, e.mode10c, delta, adjusted, e.moveVariant, length);
  };
  if (a.block723) {
    e.moveVariant = p.specialRate73c;
    m.duration = bits(s.base(a, e.mode10c, delta));
  } else
    rate();
  e.marker6a0 = 0;
  unsigned accepted = 0;
  if (!s.accept(a, from, to, accepted) || accepted != e.mode10c) {
    bool fallback = s.fallback(a);
    s.action(a, 15);
    if (fallback)
      a.cooldown = 7;
    else {
      a.routePhase = 2;
      a.ai.marker = 0;
    }
    return;
  }
  if (a.hazardSensitive && !a.protection8c9 && !a.block723 && s.hazard(a, to) &&
      !s.hazard(a, from)) {
    s.action(a, 15);
    a.cooldown = 5;
    return;
  }
  s.release(a, from);
  s.reserve(a, to);
  a.ai.motion = true;
  m.heightDelta = add(bits(unsigned(to.z) * 16u), neg(a.finePosition.z));
  const unsigned tile = s.tileKind(to);
  if (tile)
    m.heightDelta = add(m.heightDelta, s.tileHeight(tile));
  m.heightOrigin = a.finePosition.z;
  if (oldDirection == m.direction && oldVertical == p.vertical &&
      a.ai.action == 2 && oldMode == e.mode10c) {
    m.progress = add(m.progress, -192);
    if (m.deltaX)
      m.travelX = add(m.travelX, m.deltaX > 0 ? -192 : 192);
    if (m.deltaY)
      m.travelY = add(m.travelY, m.deltaY > 0 ? -192 : 192);
    s.action(a, 2);
    return;
  }
  m.travelX = m.travelY = m.progress = 0;
  e.actionRouteCounters.fill(0);
  if (a.ai.action != 2) {
    bool reset = !e.markerB73 || oldVertical != p.vertical;
    if (!reset) {
      const int turn = s.turn(oldDirection, m.direction);
      reset = (turn < 0 ? neg(turn) : turn) > 1;
    }
    if (reset) {
      m.accumulator = 0;
      e.moveVariant = 0;
    }
  }
  rate();
  auto select = [&](unsigned animation, unsigned bank, int count) {
    e.animationOffset = animation;
    s.animation(a, animation + e.animationFacing, e.block722);
    p.sampleBank = bank;
    m.sampleCursor = 0;
    length = count;
  };
  if (e.mode10c == 0 || e.mode10c == 4) {
    select(0, 0xd8 + (e.animationFacing & 1u) * 48u, 12);
  } else if (e.mode10c >= 1 && e.mode10c <= 3) {
    if (!a.formCapability) {
      e.animationOffset = 0x120;
      s.animation(a, 0x120 + e.animationFacing, e.block722);
      const unsigned distance = s.distance(delta);
      if (!distance)
        throw std::invalid_argument("segment sample divisor");
      p.inlineSample = bits(((env.defaultScalar * 4u) / distance) / 12u);
      p.sampleBank = 0x71e;
      m.sampleCursor = 0;
      length = 1;
      if (a.block723 && a.form && (a.ai.type != 18 || !a.ai.flying) &&
          a.rangedDistance && a.ai.target != -1) {
        const auto &target = w.creatures.at(unsigned(a.ai.target));
        if (target.ai.active && target.ai.health &&
            target.ai.slot != a.ai.slot) {
          if (a.ai.owner < 0 || a.ai.owner >= 8 || target.ai.owner < 0 ||
              target.ai.owner >= 8)
            throw std::invalid_argument("owned segment relationship");
          if (!w.relation[unsigned(a.ai.owner)][unsigned(target.ai.owner)])
            s.attack(a, target);
        }
      }
    } else {
      const bool type12 = a.ai.type == 12;
      if (p.vertical) {
        if (e.mode10c == 1)
          select(0xf8, 0x138, 12);
        else if (e.mode10c == 2)
          select(type12 ? 0xd0 : 0xf8, type12 ? 0x168 : 0x138, 12);
        else
          select(type12 ? 0xd8 : 0xf8, type12 ? 0x168 : 0xd8, 12);
      } else {
        if (e.mode10c == 1)
          select(0xb0, 0x138 + (e.animationFacing & 1u) * 48u, 12);
        else if (e.mode10c == 2)
          select(type12 ? 0xd0 : 0xb0, 0x168, 12);
        else
          select(type12 ? 0xd8 : 0xb0,
                 (type12 ? 0x138 : 0xd8) + (e.animationFacing & 1u) * 48u, 12);
      }
    }
  }
  if (length < 0 || length > 48)
    throw std::invalid_argument("owned segment sample span");
  if (e.block722) {
    if (!p.vertical) {
      int sum = 0;
      for (int i = 0; i < length; ++i)
        sum = add(
            sum, p.sampleBank == 0x71e
                     ? p.inlineSample
                     : s.sample(a, p.sampleBank, m.sampleCursor + unsigned(i)));
      const int residual = sum / 6;
      constexpr int dx[8] = {0, 1, 1, 1, 0, -1, -1, -1},
                    dy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
      if (m.direction <= 7) {
        e.actionRouteCounters[1] = unsigned(dx[m.direction] < 0 ? neg(residual)
                                            : dx[m.direction]   ? residual
                                                                : 0);
        e.actionRouteCounters[2] = unsigned(dy[m.direction] < 0 ? neg(residual)
                                            : dy[m.direction]   ? residual
                                                                : 0);
      }
    }
    m.sampleCursor += unsigned(length) - 1u;
  }
  m.initialResidualX = bits(e.actionRouteCounters[1]);
  m.initialResidualY = bits(e.actionRouteCounters[2]);
  m.initialCursor = m.sampleCursor;
  p.initialSampleBank = p.sampleBank;
  s.action(a, 2);
}
} // namespace mnm::reconstruction::combat
