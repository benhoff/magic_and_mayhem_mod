#pragma once
#include "owner_planner_features.hpp"
namespace mnm::reconstruction::combat {
std::array<int, 67>
hostileTaskGroupFeatures(const std::vector<OwnerPlannerActor> &friendly,
                         const std::vector<OwnerPlannerActor> &enemy, Extent);
} // namespace mnm::reconstruction::combat
