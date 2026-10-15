#pragma once
#include "../combat/autonomy.hpp"
namespace mnm::reconstruction::combat {
struct ActionObject {
  unsigned kind = 0;
  std::uint16_t next = 0xffff;
};
struct ObjectActionWorld {
  Extent dimensions;
  std::vector<std::uint16_t> heads;
  std::vector<ActionObject> objects;
};
struct ObjectActionServices {
  std::function<int(AutonomousCreature &, bool)> poll;
  std::function<void(AutonomousCreature &)> behavior, reset;
  std::function<void(unsigned, Cell)> cue;
};
// 50f3b0: footprint/object interaction action13. Supplied callbacks can mutate
// the actor before the current profile width/position are sampled.
void tickObjectAction(AutonomousCreature &, const ObjectActionWorld &,
                      const ObjectActionServices &);
} // namespace mnm::reconstruction::combat
