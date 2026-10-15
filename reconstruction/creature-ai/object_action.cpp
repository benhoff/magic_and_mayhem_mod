#include "object_action.hpp"
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
int signedBits(unsigned n) { return n <= 0x7fffffffu ? int(n) : -1 - int(~n); }
} // namespace
void tickObjectAction(AutonomousCreature &a, const ObjectActionWorld &world,
                      const ObjectActionServices &s) {
  const auto extent = world.dimensions;
  if (extent.x <= 0 || extent.y <= 0 || extent.z <= 0 || extent.x > 65535 ||
      extent.y > 65535 || extent.z > 65535 || world.objects.size() > 65535 ||
      std::uint64_t(extent.x) * unsigned(extent.y) * unsigned(extent.z) !=
          world.heads.size())
    throw std::invalid_argument("object action owned world extent");
  if (a.animationKind == 1)
    s.poll(a, true);
  const int event = s.poll(a, false);
  if (event == 1)
    s.behavior(a);
  else if (event == 3 && a.ai.type == 9)
    s.cue(0x62, a.ai.position);
  constexpr std::array<Cell, 4> offsets{
      {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0}}};
  const unsigned count = a.width == 2 ? 4 : 1;
  const Cell center = a.ai.position;
  bool matched = false;
  for (unsigned i = 0; i < count; ++i) {
    Cell p{signedBits(unsigned(center.x) + unsigned(offsets[i].x)),
           signedBits(unsigned(center.y) + unsigned(offsets[i].y)), center.z};
    if (p.x < 0 || p.x >= extent.x || p.y < 0 || p.y >= extent.y || p.z < 0 ||
        p.z >= extent.z)
      continue;
    const auto cell =
        std::uint64_t(unsigned(p.x)) +
        std::uint64_t(unsigned(p.y)) * unsigned(extent.x) +
        std::uint64_t(unsigned(p.z)) * unsigned(extent.x) * unsigned(extent.y);
    unsigned object = world.heads.at(cell);
    if (object == 0xffff)
      continue;
    unsigned steps = 0;
    while (object < world.objects.size()) {
      if (++steps > world.objects.size())
        throw std::invalid_argument("cyclic object action chain");
      const auto &o = world.objects[object];
      if (o.kind == 26 || o.kind == 62 || o.kind == 65) {
        matched = true;
        break;
      }
      object = o.next;
    }
  }
  if (!matched)
    s.reset(a);
}
} // namespace mnm::reconstruction::combat
