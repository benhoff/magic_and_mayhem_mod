#include "cached_attack.hpp"
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
int wrap(int n, int d) {
  if (d <= 0)
    throw std::invalid_argument("obstacle dimension");
  n %= d;
  return n < 0 ? n + d : n;
}
} // namespace
std::optional<unsigned> opponentFacing(const AutonomousCreature &a,
                                       const AutonomousCreature &b, Extent d) {
  static constexpr int tables[4][25] = {
      {-1, -1, -1, -1, -1, -1, 5,  6,  7,  -1, -1, 4, 0,
       0,  -1, -1, 3,  2,  1,  -1, -1, -1, -1, -1, -1},
      {-1, 5, 6,  6, 7, -1, 4, 4,  6,  0,  -1, 4, 2,
       0,  0, -1, 3, 2, 2,  1, -1, -1, -1, -1, -1},
      {-1, -1, -1, -1, -1, 5, 6,  6, 7, -1, 4, 4, 6,
       0,  -1, 4,  2,  0,  0, -1, 3, 2, 2,  1, -1},
      {5, 6, 6, 6, 7, 4, 5, 6, 7, 0, 4, 4, 0,
       0, 0, 4, 3, 2, 1, 0, 3, 2, 2, 2, 1}};
  auto delta = [](int a, int b, int size) {
    if (size <= 0)
      throw std::invalid_argument("opponent facing dimensions");
    int v = a - b;
    if (v > size / 2)
      v -= size;
    if (v < -size / 2)
      v += size;
    return v;
  };
  const int x = 2 - delta(a.ai.position.x, b.ai.position.x, d.x),
            y = 2 + delta(a.ai.position.y, b.ai.position.y, d.y);
  if (x < 0 || x > 4 || y < 0 || y > 4)
    return {};
  const int value =
      tables[(a.width == 2 ? 2 : 0) + (b.width == 2 ? 1 : 0)][5 * x + y];
  return value < 0 || value > 7 ? std::nullopt
                                : std::optional<unsigned>(unsigned(value));
}
unsigned footprintObstacle(const AutonomousCreature &a, Cell p, Extent d,
                           const std::function<ObstacleCell(Cell)> &cell) {
  if (p.z < 0 || a.width > unsigned(d.x) || a.width > unsigned(d.y) ||
      a.height > 256)
    throw std::invalid_argument("obstacle footprint domain");
  p.x = wrap(p.x, d.x);
  p.y = wrap(p.y, d.y);
  // Original51b130 enumerates Y, X, then vertical cells and returns the first
  // foreign actor token; terrain-only flags without an actor are ignored.
  for (unsigned y = 0; y < a.width; ++y)
    for (unsigned x = 0; x < a.width; ++x)
      for (unsigned z = 0; z < a.height && p.z + int(z) < d.z; ++z) {
        auto v = cell(
            {wrap(p.x + int(x), d.x), wrap(p.y + int(y), d.y), p.z + int(z)});
        if ((v.flags & 3) && v.occupant && v.occupant != a.ai.slot + 1)
          return v.occupant;
      }
  return 0;
}
bool attackCachedOpponent(AutonomousCreature &a, CreatureMaintenance &m,
                          AutonomousWorld &w, const CachedAttackServices &s) {
  if (!a.targetMovingAttacker)
    return false;
  auto &b = w.creatures.at(a.targetMovingAttacker - 1);
  if (a.ai.owner < 0 || a.ai.owner >= 8 || b.ai.owner < 0 || b.ai.owner >= 8)
    throw std::invalid_argument("cached attack owner");
  if (b.ai.active && b.ai.health && a.ai.slot != b.ai.slot &&
      !w.relation[a.ai.owner][b.ai.owner] && s.reach(a, b, a.ai.position) &&
      b.ai.action != 14) {
    const auto target =
        Cell{wrap(a.destination.x, w.dimensions.x),
             wrap(a.destination.y, w.dimensions.y), a.destination.z};
    const bool blocks = s.obstacle(a, target) == b.ai.slot + 1;
    if (a.ai.enemyRadius.at(b.ai.type) > 0 || blocks) {
      if (b.ai.motion || a.ai.incoming) {
        s.action(a, 15);
        a.cooldown = 1;
      } else {
        a.alternateTarget = int(a.targetMovingAttacker - 1);
        s.face(a, b);
        s.action(a, 20);
      }
      return true;
    }
    m.recent6b0 = 30;
  }
  a.targetMovingAttacker = 0;
  a.alternateTarget = -1;
  return false;
}
} // namespace mnm::reconstruction::combat
