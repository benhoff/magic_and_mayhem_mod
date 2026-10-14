#include "execution.hpp"
#include <algorithm>
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
void release(std::optional<unsigned> &token, const ExecutionServices &s) {
  if (token) {
    s.releaseEffect(*token);
    token.reset();
  }
}
void disengage(AutonomousCreature &a, AutonomousWorld &w,
               const ExecutionServices &s, bool direct) {
  if (a.ai.engaged && (a.ai.action == 10 || a.ai.action == 20)) {
    auto &b = w.creatures.at(
        unsigned(a.ai.action == 10 ? a.ai.target : a.alternateTarget));
    a.ai.engaged = 0;
    if (a.ai.action == 20 && !direct)
      s.peerEngagement(b, 0);
    else {
      int n = std::int32_t(b.ai.incoming - 1u);
      b.ai.incoming = n < 0 ? 0 : unsigned(n);
    }
  }
}
bool blocked(const AutonomousCreature &a) {
  return a.ai.pending == 8 || a.ai.behavior == 15 || a.ai.behavior == 6 ||
         a.ai.behavior == 8 || a.ai.behavior == 16 || a.mode || a.block784 ||
         a.blockDE3 || a.block71c;
}
} // namespace
bool orderTarget(AutonomousCreature &a, CreatureExecution &e,
                 AutonomousWorld &w, int target, const ExecutionServices &s) {
  if (a.ai.type == 27) {
    resetBehavior(a, e, w, s);
    return false;
  }
  if (a.ai.type == 22 || !a.ai.health || blocked(a) || (a.block723 && !a.form))
    return false;
  auto &b = w.creatures.at(unsigned(target));
  if (a.ai.behavior == 3 && a.ai.target == target)
    return true;
  a.ai.readiness = 0;
  disengage(a, w, s, true);
  a.ai.marker = 0;
  if (&a == &b)
    return false;
  a.ai.target = target;
  e.targetDistance =
      unsigned(creatureDistance(a.ai.position, b.ai.position, w.dimensions));
  a.castTarget = target;
  a.castSource = int(a.ai.slot);
  a.castSpell = a.selectedSpell;
  e.routeRenewal = s.movementRenewal(a) ? 1 : 0;
  if (!a.ai.motion && e.routeRenewal) {
    s.orient(a, {b.ai.position.x * 32 + 16, b.ai.position.y * 32 + 16, 0});
    a.cooldown = 10;
  }
  changeBehavior(a, e, 3, s);
  return true;
}
void orderCoordinateAttack(AutonomousCreature &a, CreatureExecution &e,
                           AutonomousWorld &w, Cell p,
                           const ExecutionServices &s) {
  if (a.ai.type == 27) {
    resetBehavior(a, e, w, s);
    return;
  }
  if (a.ai.type == 22 || blocked(a) || a.form || a.block723)
    return;
  a.ai.readiness = 0;
  disengage(a, w, s, false);
  a.ai.marker = 0;
  a.destination = p;
  // Cast aim and current fine position are independent projections.
  e.castAim = {p.x * 32 + 16, p.y * 32 + 16, p.z * 16 + s.terrainHeight(p)};
  a.castSpell = a.selectedSpell;
  a.castSource = int(a.ai.slot);
  a.castTarget = -1;
  changeBehavior(a, e, 4, s);
}
void appendOrder(AutonomousCreature &a, CreatureExecution &e,
                 AutonomousWorld &w, CreatureOrder order,
                 const ExecutionServices &s) {
  if (std::int32_t(e.queuedCount) >= 15)
    return;
  auto wrap = [](int n, int size) {
    n %= size;
    return n < 0 ? n + size : n;
  };
  order.destination.x = wrap(order.destination.x, w.dimensions.x);
  order.destination.y = wrap(order.destination.y, w.dimensions.y);
  order.referenceDestination = order.destination;
  if (order.target != -1)
    (void)w.creatures.at(unsigned(order.target));
  auto insert = [&](CreatureOrder o) {
    o.serial = e.nextOrderSerial++;
    e.orders.push_back(o);
    ++e.queuedCount;
  };
  if (e.orders.empty()) {
    if (a.ai.behavior != 2)
      s.performOrder(order);
    else {
      CreatureOrder current;
      current.kind = 2;
      current.destination = {wrap(a.destination.x, w.dimensions.x),
                             wrap(a.destination.y, w.dimensions.y),
                             a.destination.z};
      current.referenceDestination = current.destination;
      insert(current);
    }
    insert(order);
    e.orderCursor = e.orders.front().serial;
    s.deliverQueue(a);
    return;
  }
  auto &last = e.orders.back();
  if (last.kind == order.kind &&
      last.referenceDestination == order.destination &&
      last.target == order.target) {
    if (!a.flag104)
      return;
    Cell adjusted=order.destination;
    if (!s.adjustDuplicate(adjusted))
      return;
    Cell actual = s.duplicateMove(a, adjusted);
    last.destination = actual;
    release(last.effect, s);
    if (e.queuedCount > 1)
      s.deliverQueue(a);
    if (e.queuedCount == 1)
      s.performOrder(last);
    return;
  }
  auto same =
      std::find_if(e.orders.begin(), e.orders.end(), [&](const auto &o) {
        return o.kind == order.kind &&
               o.referenceDestination == order.destination &&
               o.target == order.target;
      });
  if (same != e.orders.end()) {
    if (std::int32_t(e.queuedCount) > 1)
      same->repeat = true;
    return;
  }
  insert(order);
  s.deliverQueue(a);
}
bool advanceOrders(AutonomousCreature &a, CreatureExecution &e,
                   const ExecutionServices &s) {
  if (e.orders.empty() || a.ai.behavior == 15 || a.ai.behavior == 6 ||
      a.ai.behavior == 8 || a.ai.type == 22 || a.block784 || a.blockDE3 ||
      (a.form && !a.formCapability && a.animationKind != 3))
    return false;
  auto current =
      std::find_if(e.orders.begin(), e.orders.end(),
                   [&](const auto &o) { return e.orderCursor == o.serial; });
  unsigned next = 0;
  if (current == e.orders.end()) {
    // Logical end represents an explicitly zeroed sentinel payload. A cycling
    // end increments to the first live node without purging target nodes.
    if (e.orderCursor || !e.queuedFlag)
      throw std::invalid_argument("queue cursor does not identify a live node");
  } else {
    if (current->repeat) {
      current->repeat = false;
      e.queuedFlag = 1;
    }
    next = unsigned(current - e.orders.begin()) + 1;
    if (!e.queuedFlag) {
      release(current->effect, s);
      e.orders.erase(current);
      --e.queuedCount;
      --next;
    }
  }
  if (next == e.orders.size()) {
    if (!e.queuedFlag) {
      for (auto &o : e.orders)
        if (o.kind == 2)
          release(o.effect, s);
      e.orders.clear();
      e.queuedFlag = e.queuedCount = 0;
      e.orderCursor.reset();
      return false;
    }
    e.orders.erase(std::remove_if(e.orders.begin(), e.orders.end(),
                                  [&](const auto &o) {
                                    if (o.kind == 3) {
                                      --e.queuedCount;
                                      return true;
                                    }
                                    return false;
                                  }),
                   e.orders.end());
    next = 0;
  }
  if (e.orders.empty()) {
    e.orderCursor.reset();
    // 524130 still dispatches the zero sentinel command and returns1 after a
    // cycling all-target purge. This does not recover uninitialized sentinels.
    CreatureOrder sentinel;
    sentinel.kind = 0;
    s.performOrder(sentinel);
    return true;
  }
  e.orderCursor = e.orders.at(next).serial;
  auto command = e.orders[next];
  s.performOrder(command);
  if (e.orders[next].kind == 3) {
    e.orders.erase(e.orders.begin() + next);
    --e.queuedCount;
    e.orderCursor = next < e.orders.size()
                        ? std::optional<unsigned>(e.orders[next].serial)
                        : std::nullopt;
  }
  return true;
}
} // namespace mnm::reconstruction::combat
namespace mnm::reconstruction::combat {
bool followReference(AutonomousCreature &a, const CreatureExecution &e,
                     AutonomousWorld &w, const FollowServices &s) {
  if (e.follow == -1)
    return false;
  const auto &b = w.creatures.at(unsigned(e.follow));
  // 513850 compares raw coordinate separations before normalization. A target
  // across the map seam is not treated as adjacent by this particular gate.
  auto distance = [](int x, int y) {
    unsigned n = unsigned(x) - unsigned(y);
    unsigned mask = 0u - (n >> 31);
    return std::int32_t((n ^ mask) - mask);
  };
  if (distance(a.ai.position.x, b.ai.position.x) < 3 &&
      distance(a.ai.position.y, b.ai.position.y) < 3)
    return false;
  if (!b.ai.health)
    return false;
  auto wrap = [](int n, int size) {
    n %= size;
    return n < 0 ? n + size : n;
  };
  Cell p{wrap(b.ai.position.x, w.dimensions.x),
         wrap(b.ai.position.y, w.dimensions.y), b.ai.position.z};
  if (!s.adjust(a, p))
    return false;
  s.move(a, p);
  return true;
}
} // namespace mnm::reconstruction::combat
namespace mnm::reconstruction::combat {
void repairCreatureReference(AutonomousCreature &a, CreatureExecution &e,
                             AutonomousWorld &w, int removed,
                             const ExecutionServices &s) {
  if (e.follow == removed)
    e.follow = -1;
  auto disengage = [&](int target) {
    auto &b = w.creatures.at(unsigned(target));
    a.ai.engaged = 0;
    int count = std::int32_t(b.ai.incoming - 1u);
    b.ai.incoming = count < 0 ? 0 : unsigned(count);
  };
  if (a.ai.target == removed && a.ai.health) {
    if (a.ai.action == 10 && a.ai.engaged)
      disengage(a.ai.target);
    bool special = a.ai.type == 5 && a.ai.action == 10;
    a.ai.target = -1;
    a.block784 = 0;
    if (a.ai.behavior == 3 || a.ai.behavior == 9 || a.ai.behavior == 10)
      resetBehavior(a, e, w, s);
    if (a.ai.pending == 3)
      a.ai.pending = 0;
    if (a.ai.action == 10)
      s.action(a, 15);
    if (special)
      s.action(a, 25);
  }
  if (a.alternateTarget == removed && a.ai.health) {
    if (a.ai.action == 20 && a.ai.engaged)
      disengage(a.alternateTarget);
    a.alternateTarget = -1;
    if (a.ai.action == 20)
      s.action(a, a.ai.type == 5 ? 25 : 15);
  }
}
} // namespace mnm::reconstruction::combat
