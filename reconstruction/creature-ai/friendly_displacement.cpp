#include "friendly_displacement.hpp"
#include "../combat/autonomy_tables.hpp"
#include <algorithm>
#include <cstdlib>
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
int wrap(int n, int size) {
  n %= size;
  return n < 0 ? n + size : n;
}
int bits(unsigned n) { return n <= 0x7fffffffu ? int(n) : -1 - int(~n); }
unsigned priority(const AutonomousCreature &a) {
  return (((4u * a.width + a.height) * 28u + a.ai.type) * 125u + a.ai.slot) *
         125u;
}
bool contains(const SegmentCompletionState &r, Cell p) {
  for (unsigned i = 0; i < r.count; ++i)
    if (r.route.at(i).position == p)
      return true;
  return false;
}
bool candidateBlocked(const AutonomousCreature &a, const AutonomousCreature &b,
                      Cell p, Extent d, const FriendlyDisplacementServices &s) {
  const auto blocked = [&](Cell q) {
    return s.obstacle(b, q) == a.ai.slot + 1;
  };
  if (blocked(p))
    return true;
  if (b.width == 2)
    return false;
  int dx = p.x - b.ai.position.x, dy = p.y - b.ai.position.y,
      dz = p.z - b.ai.position.z;
  if (std::abs(dx) > d.x / 2)
    dx += dx > 0 ? -d.x : d.x;
  if (std::abs(dy) > d.y / 2)
    dy += dy > 0 ? -d.y : d.y;
  if (dx * dx + dy * dy + dz * dz <= 1)
    return false;
  auto old = b.ai.position;
  for (Cell q :
       {Cell{p.x, old.y, old.z}, Cell{old.x, p.y, old.z}, Cell{p.x, p.y, old.z},
        Cell{old.x, old.y, p.z}, Cell{p.x, old.y, p.z}, Cell{old.x, p.y, p.z}})
    if (blocked(q))
      return true;
  return false;
}
} // namespace
unsigned displacementBlocker(const AutonomousCreature &a, Cell next,
                             const FriendlyDisplacementServices &s) {
  if (!s.move(a, a.ai.position, next, true))
    return 0;
  const auto flags = s.flags(next);
  if ((flags & 0x30) && !(flags & 0x10))
    return 0;
  unsigned result = s.obstacle(a, next);
  if (result || a.width == 2)
    return result;
  auto p = a.ai.position;
  const int x = next.x - p.x, y = next.y - p.y, z = next.z - p.z;
  if (x * x + y * y + z * z <= 1)
    return 0;
  unsigned v[6];
  unsigned i = 0;
  for (Cell q : {Cell{next.x, p.y, p.z}, Cell{p.x, next.y, p.z},
                 Cell{next.x, next.y, p.z}, Cell{p.x, p.y, next.z},
                 Cell{next.x, p.y, next.z}, Cell{p.x, next.y, next.z}})
    v[i++] = s.obstacle(a, q);
  if (!(v[0] || v[1] || v[2]) || !(v[3] || v[4] || v[5]))
    return 0;
  for (auto n : v)
    if (n)
      return n;
  return 0;
}
bool displacementWait(AutonomousCreature &a, CreatureExecution &e, Cell next,
                      const AutonomousWorld &w,
                      const std::vector<CreatureExecution> &execution,
                      const std::function<ObstacleCell(Cell)> &cell,
                      const FriendlyDisplacementServices &lower) {
  auto services = lower;
  services.obstacle = [&](const auto &a, Cell p) {
    auto d = w.dimensions;
    for (unsigned y = 0; y < a.width; ++y)
      for (unsigned x = 0; x < a.width; ++x)
        for (unsigned z = 0; z < a.height && p.z + int(z) < d.z; ++z) {
          auto v = cell(
              {wrap(p.x + int(x), d.x), wrap(p.y + int(y), d.y), p.z + int(z)});
          if (!(v.flags & 3) || !v.occupant || v.occupant == a.ai.slot + 1)
            continue;
          const auto &b = w.creatures.at(v.occupant - 1);
          const auto &be = execution.at(v.occupant - 1);
          if (!displacementMoving(b, be))
            continue;
          if (b.ai.motion ||
              (b.routePhase == 1 && b.ai.action == 15 && b.ai.behavior != 13))
            return v.occupant;
          unsigned link = be.marker6a0;
          bool cycle = link == a.ai.slot + 1;
          for (unsigned i = 0; !cycle && link && i < 10; ++i) {
            link = execution.at(link - 1).marker6a0;
            cycle = link == a.ai.slot + 1;
          }
          if (!cycle)
            return v.occupant;
        }
    return 0u;
  };
  const auto blocker = displacementBlocker(a, next, services);
  if (blocker)
    e.marker6a0 = blocker;
  return blocker != 0;
}
bool canDisplaceCreature(const AutonomousCreature &a,
                         const AutonomousCreature &b,
                         const CreatureExecution &e, const AutonomousWorld &w) {
  if (b.ai.type == 22 || !b.ai.health)
    return false;
  if (a.ai.slot != b.ai.slot) {
    if (a.ai.owner < 0 || a.ai.owner >= 8 || b.ai.owner < 0 || b.ai.owner >= 8)
      throw std::out_of_range("displacement relationship");
    if (!w.relation.at(unsigned(a.ai.owner)).at(unsigned(b.ai.owner)))
      return false;
  }
  if (b.ai.behavior != 1 && b.ai.behavior != 13 && b.ai.behavior != 10 &&
      !(b.ai.action == 15 && (e.marker6a0 || b.routePhase)))
    return false;
  return b.ai.behavior != 6 && !b.mode && !b.form;
}
bool displacementMoving(const AutonomousCreature &a,
                        const CreatureExecution &e) {
  return a.ai.health &&
         (a.ai.motion ||
          (a.routePhase == 1 && a.ai.action == 15 && a.ai.behavior != 13) ||
          e.marker6a0);
}
bool displacementSweptOverlap(const AutonomousCreature &a, Cell next, Cell p,
                              unsigned width, Extent d) {
  auto axis = [&](int from, int to, int at, int size) {
    int lo, hi;
    if (std::abs(to - from) < size / 2) {
      lo = std::min(from, to);
      hi = std::max(from, to) + int(a.width);
    } else {
      lo = std::max(from, to);
      hi = std::min(from, to) + int(a.width) + size;
      if (at < size / 2)
        at += size;
    }
    return at < hi && at + int(width) > lo;
  };
  return axis(a.ai.position.x, next.x, p.x, d.x) &&
         axis(a.ai.position.y, next.y, p.y, d.y);
}
bool issueFriendlyDisplacement(AutonomousCreature &a, AutonomousCreature &b,
                               AutonomousWorld &w,
                               std::vector<CreatureExecution> &execution,
                               std::vector<SegmentCompletionState> &routes,
                               std::vector<MotionActionState> &motion,
                               std::vector<int> &owners,
                               const FriendlyDisplacementServices &s) {
  auto &ae = execution.at(a.ai.slot);
  auto &be = execution.at(b.ai.slot);
  const unsigned rank =
      a.ai.behavior == 13 ? ae.displacementActive : priority(a);
  const unsigned other = b.ai.behavior == 13         ? be.displacementActive
                         : displacementMoving(b, be) ? priority(b)
                                                     : 0;
  if (bits(rank) <= bits(other))
    return false;
  int best = -1, score = 0;
  auto d = w.dimensions;
  const auto &route = routes.at(a.ai.slot);
  const Cell next = route.route.at(route.cursor).position;
  for (unsigned i = 0; i < originalRouteOffsets.size(); ++i) {
    auto off = originalRouteOffsets[i];
    Cell p{wrap(b.ai.position.x + off.x, d.x),
           wrap(b.ai.position.y + off.y, d.y), b.ai.position.z + off.z};
    if (p.z <= 0 || p.z >= d.z)
      continue;
    if (!s.move(b, b.ai.position, p, true))
      continue;
    int value = 10;
    if (!s.move(b, b.ai.position, p, false)) {
      value = 5;
      if (candidateBlocked(a, b, p, d, s))
        continue;
    }
    if (s.hazard(a, p))
      value = 1;
    if (value > score) {
      if (contains(route, p))
        --value;
      if (value > score && displacementSweptOverlap(a, next, p, b.width, d))
        value = value > 5 ? 2 : 1;
    }
    if (best < 0 || value > score) {
      best = int(i);
      score = value;
      if (value == 10)
        break;
    }
  }
  if (best < 0)
    return false;
  if (b.ai.behavior != 13)
    be.displacementDestination = b.destination;
  auto off = originalRouteOffsets[unsigned(best)];
  Cell p{wrap(b.ai.position.x + off.x, d.x), wrap(b.ai.position.y + off.y, d.y),
         b.ai.position.z + off.z};
  if (b.ai.behavior != 13)
    s.order(b, p);
  else {
    b.destination = p;
    b.ai.marker = 0;
    if (!b.routePhase)
      b.routePhase = 1;
  }
  b.cooldown = 0;
  be.displacementActive = rank - 1;
  owners.at(b.ai.slot) = int(a.ai.slot);
  motion.at(b.ai.slot).displacementPause = true;
  s.action(a, 15);
  a.cooldown = 6;
  a.ai.marker = 1;
  return b.ai.behavior == 13;
}
} // namespace mnm::reconstruction::combat
