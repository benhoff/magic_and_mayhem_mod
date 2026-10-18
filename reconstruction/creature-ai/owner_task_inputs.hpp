#pragma once
#include "owner_task_lifecycle.hpp"
#include "contribution_account.hpp"
namespace mnm::reconstruction::combat {
// 52a3a0: commander distress, not ordinary creature morale.
bool ownerTaskDistressed(unsigned kind, bool transformed, unsigned health,
                         unsigned maximum, int danger, int support);
bool ownerTaskOnSpecialCell(unsigned slot, const std::vector<unsigned> &occupants);
// 514990: resolved task recipients are supplied. A wizard victim doubles the
// attacker's credit, while its own task receives the undoubled cost.
void addOwnerTaskDamage(OwnerPreviousTask *source, OwnerPreviousTask *victim,
                        unsigned victimKind, unsigned accounted);
// 512460: only a pending-FC completion produces this task cost.
void addOwnerTaskCost(OwnerPreviousTask &, unsigned accounted);
void addOwnerTaskReward(OwnerPreviousTask &, unsigned accounted);
} // namespace mnm::reconstruction::combat
