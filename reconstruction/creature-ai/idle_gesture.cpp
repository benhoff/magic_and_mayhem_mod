#include "idle_gesture.hpp"
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
int bits(unsigned n) { return n <= 0x7fffffffu ? int(n) : -1 - int(~n); }
int add(int a, int b) { return bits(unsigned(a) + unsigned(b)); }
} // namespace
bool idleGesture(
    AutonomousCreature &a, AutonomousWorld &w, const AiGeometry &g,
    const std::array<int, 28> &scores, const std::function<unsigned()> &random,
    const std::function<void(AutonomousCreature &, Cell)> &orient) {
  if (a.mode || (a.form && !a.formCapability && a.animationKind != 3) ||
      a.blockDE3 || a.block71c || a.block723)
    return false;
  if ((random() & 0x7fffffffu) % 100 > 10)
    return false;
  constexpr bool admitted[27] = {false, true,  true, true,  true,  true, true,
                                 true,  true,  true, true,  true,  true, true,
                                 true,  false, true, false, true,  true, true,
                                 true,  false, true, false, false, false};
  if (a.ai.type >= 27 || !admitted[a.ai.type])
    return false;
  if (a.ai.owner < 0 || a.ai.owner >= 8)
    throw std::invalid_argument("idle gesture owner");
  AutonomousCreature *best = nullptr;
  int maximum = -1;
  for (auto &b : w.creatures) {
    if (!b.ai.active || b.ai.slot == a.ai.slot || !b.ai.health ||
        b.ai.owner < 0 || b.ai.owner >= 8 ||
        w.relation[a.ai.owner][b.ai.owner] || !b.ai.visible[a.ai.owner])
      continue;
    Cell aim{add(b.finePosition.x, b.aimXY), add(b.finePosition.y, b.aimXY),
             add(b.finePosition.z, b.aimZ)};
    if (!g.lineOfSight(a, aim))
      continue;
    const int score = scores.at(b.ai.type);
    if (creatureDistance(a.ai.position, b.ai.position, w.dimensions) >
        bits(unsigned(score) * 2u))
      continue;
    const int value =
        add(bits(unsigned(score) * 4u), int((random() & 0x7fffffffu) % 50));
    if (value > maximum) {
      maximum = value;
      best = &b;
    }
  }
  if (!best)
    return false;
  orient(a, {add(best->finePosition.x, best->aimXY),
             add(best->finePosition.y, best->aimXY), 0});
  best->ai.history = a.ai.slot + 1;
  return true;
}
} // namespace mnm::reconstruction::combat
