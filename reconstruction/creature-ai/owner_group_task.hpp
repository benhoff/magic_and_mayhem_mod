#pragma once
#include "owner_hostile_task.hpp"
#include "owner_memory_groups.hpp"
namespace mnm::reconstruction::combat {
struct OwnerGroupTask {
  OwnerMemoryGroups groups;
  unsigned root = 0;
  int risk = 0;
};
struct OwnerGroupOrder {
  bool continuing = false, stop = false;
  OwnerHostileOrderKind kind = OwnerHostileOrderKind::none;
  int target = -1;
  Cell position;
};
// One ordinary nonranged friendly actor, two live pool slots; anonymous memory
// leaves may resolve repeatedly to the enemy slot. No opposite task assignment.
OwnerGroupOrder executeOwnerGroupTask(OwnerGroupTask &, const OwnerKnowledge &,
                                      int tick, unsigned owner, unsigned actor,
                                      const AutonomousWorld &, int rawDanger,
                                      int netDanger, int excessPercent,
                                      const std::array<int, 2> &threat,
                                      int &budget);
} // namespace mnm::reconstruction::combat
