#include "full_motion_action.hpp"
#include <algorithm>
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
int word(unsigned n) { return std::int32_t(n); }
int add(int a, int b) { return word(unsigned(a) + unsigned(b)); }
int mul(int a, int b) { return word(unsigned(a) * unsigned(b)); }
int wrap(int n, int extent) {
  n %= extent;
  return n < 0 ? n + extent : n;
}
void releaseMotionEffects(CreatureExecution &e, const MotionActionServices &s) {
  if (e.routeEffect) {
    s.releaseEffect(*e.routeEffect);
    e.routeEffect.reset();
  }
  for (auto &o : e.orders)
    if (o.kind == 2 && o.effect) {
      s.releaseEffect(*o.effect);
      o.effect.reset();
    }
}
void disengageMotion(AutonomousCreature &a, AutonomousWorld &w) {
  if (!a.ai.engaged)
    return;
  if (a.ai.action == 10 || a.ai.action == 20) {
    auto &b = w.creatures.at(
        unsigned(a.ai.action == 10 ? a.ai.target : a.alternateTarget));
    a.ai.engaged = 0;
    int count = word(b.ai.incoming - 1u);
    b.ai.incoming = count < 0 ? 0 : unsigned(count);
  }
}
} // namespace
void tickMotionAction(AutonomousCreature &a, CreatureExecution &e,
                      MotionActionState &m, MovementBehaviorState &landing,
                      AutonomousWorld &w, const MotionActionServices &s) {
  if (a.animationKind == 1)
    s.poll(a, true);
  m.accumulator = add(m.accumulator, word(e.moveVariant));
  if (m.duration == 0 || (m.accumulator == INT32_MIN && m.duration == -1))
    throw std::invalid_argument("original motion division domain");
  int count = m.accumulator / m.duration;
  if (count <= 0)
    return;
  if (count > 4096)
    throw std::invalid_argument("bounded original motion substep domain");
  bool force32 = a.ai.type == 12 && (e.mode10c == 2 || e.mode10c == 3);
  if (force32)
    count = 1;
  constexpr int dx[] = {0, 1, 1, 1, 0, -1, -1, -1},
                dy[] = {-1, -1, 0, 1, 1, 1, 0, -1};
  for (int i = 0; i < count; ++i) {
    int step =
        force32 ? 32 : mul(m.samples.at(m.sampleCursor), m.vertical ? 2 : 1);
    int sx = 0, sy = 0;
    if (!m.vertical && m.direction < 8) {
      sx = mul(step, dx[m.direction]);
      sy = mul(step, dy[m.direction]);
    }
    m.progress = add(m.progress, step);
    m.travelX = add(m.travelX, sx);
    m.travelY = add(m.travelY, sy);
    m.heightStep = std::clamp(mul(m.heightDelta, m.progress) / 192, -16, 16);
    a.finePosition.z = add(m.heightOrigin, m.heightStep);
    Cell old = a.finePosition;
    a.finePosition.x = add(mul(a.ai.position.x, 32), m.travelX / 6);
    a.finePosition.y = add(mul(a.ai.position.y, 32), m.travelY / 6);
    e.actionRouteCounters[1] += unsigned(old.x) - unsigned(a.finePosition.x);
    e.actionRouteCounters[2] += unsigned(old.y) - unsigned(a.finePosition.y);
    if (a.ai.flying && !a.formCapability)
      e.actionRouteCounters = {};
    else {
      m.sampleCursor += e.block722 ? unsigned(-1) : 1u;
      ++e.actionRouteCounters[0];
      unsigned polls = 0;
      for (int event = s.poll(a, false); event; event = s.poll(a, false)) {
        if (++polls > 4096)
          throw std::invalid_argument("bounded original motion event domain");
        if (event == 1 && e.animationOffset <= 7) {
          s.rewind(a);
          e.actionRouteCounters[1] = unsigned(m.initialResidualX);
          e.actionRouteCounters[2] = unsigned(m.initialResidualY);
          e.actionRouteCounters[0] = 0;
          m.sampleCursor = m.initialCursor;
          break;
        }
        if (event == 1)
          s.rewind(a);
        else if (event == 2) {
          s.rewind(a);
          e.actionRouteCounters[1] = unsigned(m.initialResidualX);
          e.actionRouteCounters[2] = unsigned(m.initialResidualY);
          e.actionRouteCounters[0] = 0;
          m.sampleCursor = m.initialCursor;
          break;
        } else if (event == 5)
          s.footstep(a);
        else if (event == 6) {
          if (a.ai.type == 4)
            s.effect(a, 0x203, a.ai.position);
          else if (a.ai.type == 12 || a.ai.type == 15)
            s.effect(a, 0x20e, a.ai.position);
        }
      }
    }
    m.accumulator = add(m.accumulator, word(0u-unsigned(m.duration)));
    if (unsigned(m.progress) < 192u)
      continue;
    if (a.ai.behavior == 14)
      e.block722 = 0;
    s.completeSegment(a);
    if (!a.ai.active)
      return;
    if (a.ai.behavior == 14) {
      e.block722 = 0;
      a.ai.marker = 0;
      m.direction = (m.direction - 4u) & 7u;
      e.animationFacing = (m.direction - 2u * s.cameraTurn) & 7u;
      s.visualFacing(a);
      unsigned pending = a.ai.pending;
      if (pending != 5) {
        if (pending == 8) {
          unsigned mode = a.mode;
          a.mode = 0;
          s.stun(a, e.absorptionTimer + 10u, mode);
          a.ai.pending = 0;
        } else {
          if (pending == 2 || (pending == 3 && a.ai.target != -1)) {
            a.ai.behavior = pending;
            s.bindBehavior(a, pending);
            a.ai.pending = 0;
          } else
            s.reset(a);
          disengageMotion(a, w);
          if (!a.ai.motion) {
            s.action(a, 15);
            a.cooldown = 10;
          }
        }
      }
    }
    if (a.ai.pending) {
      unsigned pending = a.ai.pending;
      a.ai.marker = 0;
      if (pending == 8) {
        unsigned old = a.ai.behavior;
        s.behavior(a, 8);
        a.ai.pending = old == 8 ? 0 : old;
        releaseMotionEffects(e, s);
        if (e.marker68)
          s.updateOrders(a);
      } else {
        s.behavior(a, pending);
        a.ai.pending = 0;
        if (!a.ai.active)
          return;
      }
    } else if (a.targetMovingAttacker || s.response(a)) {
      s.action(a, 15);
      if (a.cooldown < 2)
        a.cooldown = 2;
    } else
      s.runBehavior(a);
    if (a.ai.behavior == 13 && m.displacementPause && !a.ai.motion) {
      s.action(a, 15);
      a.cooldown = 20;
      m.displacementPause = false;
    }
    Cell p{wrap(a.ai.position.x, w.dimensions.x),
           wrap(a.ai.position.y, w.dimensions.y), a.ai.position.z};
    if (!s.legal(a, p)) {
      if (a.ai.type == 27) {
        s.reset(a);
        return;
      }
      landing.landingKind = unsigned(a.ai.position.z);
      a.ai.marker = a.blockDE3 = e.block962 = e.block963 = a.ai.readiness = 0;
      disengageMotion(a, w);
      s.behavior(a, 16);
    }
    return;
  }
}
} // namespace mnm::reconstruction::combat
