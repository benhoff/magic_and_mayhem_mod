#include "coarse_route.hpp"
#include "../rendering/effect_trajectory.hpp"
#include <algorithm>
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
int wrap(int v, int size) {
  v %= size;
  return v < 0 ? v + size : v;
}
int bits(unsigned v) { return v <= 0x7fffffffu ? int(v) : -1 - int(~v); }
reconstruction::EffectTrajectory trajectory(Cell from, Cell to, Extent d) {
  // Original4df6e0 +4df260, strict shorter-distance wrap and >= axis ties.
  auto adjust = [](int from, int to, int size) {
    int distance = std::abs(from - to);
    if (distance > std::abs(size + from - to))
      to -= size;
    if (distance > std::abs(from - size - to))
      to += size;
    return to;
  };
  to.x = adjust(from.x, to.x, d.x);
  to.y = adjust(from.y, to.y, d.y);
  const int x = std::abs(to.x - from.x), y = std::abs(to.y - from.y),
            z = std::abs(to.z - from.z);
  const int sx = to.x >= from.x ? 1 : -1, sy = to.y >= from.y ? 1 : -1,
            sz = to.z >= from.z ? 1 : -1;
  reconstruction::EffectTrajectory t;
  auto &s = t.words;
  s[4] = unsigned(sx);
  s[5] = unsigned(sy);
  if (x >= y)
    s[6] = unsigned(sx);
  else
    s[7] = unsigned(sy);
  s[8] = 2u * unsigned(std::max(x, y));
  s[9] = 2u * unsigned(std::min(x, y));
  s[10] = unsigned(std::max(x, y));
  s[1] = unsigned(sz);
  if (z >= std::max(x, y))
    s[0] = unsigned(sz);
  else
    s[2] = 1;
  s[11] = unsigned(std::max({x, y, z}));
  s[12] = 2u * unsigned(std::min(z, std::max(x, y)));
  s[13] = 2u * s[11];
  return t;
}
} // namespace
bool coarseCreatureRoute(
    const AutonomousCreature &a, unsigned direction, Extent d, Cell target,
    ActorRouteSnapshot &out,
    const std::function<bool(Cell, Cell, unsigned &)> &move,
    const std::function<bool(Cell)> &hazard) {
  if (d.x <= 0 || d.y <= 0 || d.z <= 0)
    throw std::invalid_argument("coarse route dimensions");
  auto from = a.ai.position;
  target.x = wrap(target.x, d.x);
  target.y = wrap(target.y, d.y);
  auto stepper = trajectory(from, target, d);
  out.destination = target;
  out.count = out.cursor = 0;
  unsigned changes = 0;
  while (!(from == target)) {
    if (out.count >= 16)
      return false;
    auto old = from;
    std::array<unsigned, 3> units{unsigned(from.x), unsigned(from.y),
                                  unsigned(from.z)};
    stepper.step(units, changes);
    from = {wrap(bits(units[0]), d.x), wrap(bits(units[1]), d.y),
            bits(units[2])};
    unsigned category = 0;
    if (!move(old, from, category) ||
        (a.hazardSensitive && !a.protection8c9 && hazard(from)))
      return false;
    int vertical = from.z - old.z;
    if (old.x != from.x || old.y != from.y) {
      auto delta = [](int now, int prior, int size) {
        int v = now - prior;
        if (std::abs(v) > size / 2)
          v += v > 0 ? -size : size;
        return v;
      };
      int x = delta(from.x, old.x, d.x), y = delta(from.y, old.y, d.y);
      constexpr unsigned directions[9] = {7, 0, 1, 6, 0, 2, 5, 4, 3};
      direction = directions[x + 3 * y + 4];
      vertical = 0;
    }
    out.points[out.count++] = {from,
                               {int(direction), vertical,
                                int(category >= 1 && category <= 3),
                                int(category)}};
  }
  return true;
}
} // namespace mnm::reconstruction::combat
