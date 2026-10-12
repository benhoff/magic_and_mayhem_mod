#include "execution.hpp"
#include <algorithm>
#include <stdexcept>

namespace mnm::reconstruction::combat {
namespace {
void release(std::optional<std::uint32_t> &token, const ExecutionServices &s) {
  if (token) {
    s.releaseEffect(*token);
    token.reset();
  }
}
bool orderBlocked(const AutonomousCreature &a, const CreatureExecution &) {
  return !a.ai.health || a.ai.behavior == 15 || a.ai.behavior == 6 ||
         a.ai.behavior == 8 || a.ai.behavior == 16 || a.mode || a.block784 ||
         a.blockDE3 || a.form || a.block71c || a.block723;
}
void disengage(AutonomousCreature &a, AutonomousWorld &w,
               const ExecutionServices &s) {
  if (!a.ai.engaged)
    return;
  if (a.ai.action == 10) {
    auto &b = w.creatures.at(unsigned(a.ai.target));
    a.ai.engaged = 0;
    auto n = std::int32_t(b.ai.incoming - 1u);
    b.ai.incoming = n < 0 ? 0 : unsigned(n);
  } else if (a.ai.action == 20) {
    auto &b = w.creatures.at(unsigned(a.alternateTarget));
    a.ai.engaged = 0;
    s.peerEngagement(b, 0);
  }
}
} // namespace

void scheduleCreatures(AutonomousWorld &w, CreatureScheduler &p,
                       const SchedulerEnvironment &env,
                       const SchedulerServices &s) {
  p.budget = env.suppressRoutes ? 0 : 53;
  bool wrapped = false, yielded = false;
  while (!wrapped && p.budget > 0) {
    auto &a = w.creatures.at(unsigned(p.cursor));
    if (a.ai.active && a.ai.health) {
      yielded = s.route(unsigned(p.cursor), p);
      if (!yielded && p.continuation)
        p.continuation = false;
      if (p.continuation)
        break;
    }
    ++p.cursor;
    if (p.cursor >= int(w.creatures.size())) {
      p.cursor = 0;
      wrapped = true;
    }
    if (yielded)
      break;
  }
  for (unsigned i = 0; i < w.creatures.size(); ++i) {
    auto &a = w.creatures[i];
    if (!a.ai.active || !a.ai.health)
      continue;
    if (int(i % 20) == p.phase20) {
      s.preDecision(i);
      s.decision(i);
      s.valuation(i);
      if (env.updateRegions && a.ai.type != 22)
        a.region = s.region(i);
    }
    if (!env.network && int(i % 90) == p.phase90) {
      if (a.ai.owner < 0 || a.ai.owner >= 8)
        throw std::out_of_range("original scheduler owner domain");
      if ((env.ownerController[unsigned(a.ai.owner)] == -1 || a.ai.flag704) &&
          a.ai.owner != env.localOwner)
        s.ownerWork(i);
    }
  }
  if (++p.phase20 >= 20)
    p.phase20 = 0;
  if (++p.phase90 >= 90)
    p.phase90 = 0;
}

std::optional<unsigned> behaviorHandler(unsigned kind) {
  // 50a930 has a null callback for12 and out-of-range values.
  if (kind >= 1 && kind <= 16 && kind != 12)
    return kind;
  return std::nullopt;
}

std::optional<unsigned> actionHandler(unsigned kind) {
  constexpr unsigned handlers[25] = {1,  2,  3,  0,  0,  1,  0, 8, 9,
                                     10, 0,  0,  13, 14, 15, 1, 1, 18,
                                     1,  20, 21, 1,  23, 24, 1};
  if (kind < 1 || kind > 25 || !handlers[kind - 1])
    return std::nullopt;
  return handlers[kind - 1];
}

void changeBehavior(AutonomousCreature &a, CreatureExecution &e, unsigned kind,
                    const ExecutionServices &s) {
  if (!a.ai.health && !(kind == 5 && a.ai.behavior != 5) && kind != 16)
    return;
  if (a.ai.motion) {
    a.ai.pending = kind;
    return;
  }
  if (a.ai.behavior == 2) {
    release(e.routeEffect, s);
    for (auto &o : e.orders)
      if (o.kind == 2)
        release(o.effect, s);
  }
  a.ai.behavior = kind;
  if (kind != 8 && kind != 14)
    a.ai.pending = 0;
  if (auto handler = behaviorHandler(kind))
    s.behavior(a, *handler);
}

void resetBehavior(AutonomousCreature &a, CreatureExecution &e,
                   AutonomousWorld &w, const ExecutionServices &s) {
  if (a.mode) {
    if (a.mode == 2)
      s.restoreColor(1);
    else if (a.mode == 1)
      s.restoreColor(4);
    a.mode = 0;
    e.absorptionTimer = 0;
  }
  if (a.ai.pending == 8)
    return;
  a.block784 = 0;
  if (e.mode10c == 3)
    a.ai.flying = 0;
  e.markerB73 = a.ai.readiness = 0;
  disengage(a, w, s);
  a.ai.home = a.ai.position;
  a.block71c = a.blockDE3 = e.block962 = e.block963 = a.block723 = e.block722 =
      a.ai.dirty932 = e.marker6a0 = a.ai.marker = a.substate = e.routeRenewal =
          e.commandMode = 0;
  changeBehavior(a, e, 1, s);
}

void clearOrders(CreatureExecution &e, const ExecutionServices &s) {
  for (auto &o : e.orders)
    if (o.kind == 2)
      release(o.effect, s);
  e.orders.clear();
  e.queuedFlag = e.queuedCount = 0;
  // 5243b0 leaves8fd untouched. The native cursor is a serial, not a dangling
  // pointer; queue delivery must check whether its serial still exists.
}

void setFollow(AutonomousCreature &a, CreatureExecution &e, AutonomousWorld &w,
               std::int32_t target, const ExecutionServices &s) {
  if (target == -1) {
    e.follow = -1;
    return;
  }
  if (orderBlocked(a, e))
    return;
  resetBehavior(a, e, w, s);
  e.follow = target;
}

void orderMove(AutonomousCreature &a, CreatureExecution &e, AutonomousWorld &w,
               Cell destination, const ExecutionServices &s) {
  if (a.ai.pending == 8 || orderBlocked(a, e))
    return;
  a.ai.marker = a.ai.readiness = 0;
  disengage(a, w, s);
  const bool alreadyRenewing = a.ai.behavior == 2 && e.routeRenewal;
  e.routeRenewal = 0;
  if (s.movementRenewal(a))
    e.routeRenewal = 1;
  a.destination = destination;
  if (a.ai.motion) {
    e.routeChanged = 1;
    a.ai.pending = 2;
  } else {
    if (a.ai.health) {
      s.action(a, 15);
      a.routePhase = 1;
      if (e.routeRenewal && !alreadyRenewing) {
        s.orient(a, {destination.x * 32 + 16, destination.y * 32 + 16, 0});
        a.cooldown = 10;
      }
    }
    changeBehavior(a, e, 2, s);
  }
  release(e.routeEffect, s);
  for (auto &o : e.orders)
    if (o.kind == 2)
      release(o.effect, s);
  if (e.marker68)
    s.updateQueue(a);
}

void deliverOrder(AutonomousCreature &, const CreatureOrder &o,
                  AutonomousWorld &w, const std::function<void(Cell)> &move,
                  const std::function<void(AutonomousCreature &)> &target,
                  const std::function<void(Cell)> &coordinate) {
  if (o.kind == 2)
    move(o.destination);
  else if (o.kind == 3) {
    auto &b = w.creatures.at(unsigned(o.target));
    if (b.ai.health)
      target(b);
  } else if (o.kind == 4)
    coordinate(o.destination);
}
} // namespace mnm::reconstruction::combat
