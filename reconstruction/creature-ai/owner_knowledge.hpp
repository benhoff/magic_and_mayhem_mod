#pragma once
#include "regional_danger.hpp"
#include "owner_planner_features.hpp"
namespace mnm::reconstruction::combat {
struct OwnerObservation {
  RegionalDangerActor actor;
  Cell finePosition;
  std::array<unsigned, 8> visible{};
  int speed = 0; // Global type CFG +589, not snapshot configuration.
  std::optional<OwnerPlannerActor> planning{};
};
struct OwnerMemory {
  RegionalDangerMemory memory;
  int markedTick = 0;
  Cell finePosition;
  std::optional<OwnerPlannerActor> planning{};
};
struct OwnerKnowledge {
  std::array<std::vector<OwnerMemory>, 8> records;
  std::array<int, 8> nextKey{};
  int previousSweep = 0, currentSweep = 0, owner = -1, cursor = -1;
  bool reset = true;
  std::array<bool, 125> matched{};
};
// 4181d0 modes 1/2, absent task/notification lists. Visibility is an input;
// this contract does not generate line of sight or strategic task consequences.
void markOwnerKnowledge(OwnerKnowledge &, int tick);
void updateOwnerKnowledge(OwnerKnowledge &, int tick, Extent,
                         const std::array<bool, 8> &enabled,
                         const std::array<std::array<std::uint8_t, 8>, 8> &allied,
                         const std::vector<OwnerObservation> &);
void validateOwnerKnowledge(const OwnerKnowledge &, unsigned actors);
} // namespace mnm::reconstruction::combat
