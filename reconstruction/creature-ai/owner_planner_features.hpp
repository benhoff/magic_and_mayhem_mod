#pragma once
#include "../combat/control.hpp"
#include <array>
namespace mnm::reconstruction::combat {
// Deliberately bounded ordinary, noncasting, empty-resource feature projection.
// Full species/configuration, inventories, fog points and spell admission
// remain separate contracts. Snapshot copies retain the observation-time
// values.
struct OwnerPlannerActor {
  std::array<int, 6> combatValue{};
  Cell position;
  int speed = 0, power = 0, bravery = 0, defense5c5 = 0, factor5a9 = 0,
      factor5ad = 0, danger = 0, motionB63 = 0;
  bool physical = false, flag104 = false;
};
inline constexpr std::array<unsigned, 67> hostileTaskFeatureIds{
    {28,  29,  30,  31,  8,   13,  27,  3,   32,  33,  26, 34, 35,  36,
     37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47, 48, 49,  50,
     51,  52,  53,  54,  55,  57,  58,  86,  87,  88,  89, 66, 71,  85,
     61,  90,  91,  84,  92,  93,  94,  95,  96,  97,  98, 99, 100, 101,
     102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112}};
std::array<int, 67> hostileTaskFeatures(const OwnerPlannerActor &friendly,
                                        const OwnerPlannerActor &enemy, Extent);
std::vector<int> hostileTaskCandidateFeatures(const OwnerPlannerActor &,
                                              const OwnerPlannerActor &, Extent,
                                              int uncertaintySquared,
                                              const std::array<int, 3> &path,
                                              bool alreadyAssigned);
void validateOwnerPlannerActor(const OwnerPlannerActor &);
} // namespace mnm::reconstruction::combat
