#pragma once
#include "../combat/autonomy.hpp"
#include "owner_knowledge.hpp"
namespace mnm::reconstruction::combat {
enum class OwnerHostileOrderKind { none, target, move };
struct OwnerHostileTask {
  int memoryKey = -1;
  Cell position;
};
struct OwnerHostileOrder {
  bool continuing = false;
  OwnerHostileOrderKind kind = OwnerHostileOrderKind::none;
  int target = -1;
  Cell position;
};
// 432900 single ordinary melee actor projection. Empty opposite-brain tasks,
// empty resources, zero regional danger and no wizard formations. Actor
// engagement/dirty state are read live; memory marker controls slot resolution
// (407c10).
OwnerHostileOrder executeOwnerHostileTask(OwnerHostileTask &,
                                          const OwnerKnowledge &, int tick,
                                          unsigned owner, unsigned actor,
                                          const AutonomousWorld &, int &budget);
} // namespace mnm::reconstruction::combat
