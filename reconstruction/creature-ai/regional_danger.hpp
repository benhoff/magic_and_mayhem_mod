#pragma once
#include "regional_navigation.hpp"
namespace mnm::reconstruction::combat {
using DangerRow = std::array<std::uint8_t, 7>;
// Logical projections of the live pool and owner memory; no legacy pointers.
struct RegionalDangerActor {
  int slot = -1, owner = -1, region = -1;
  Cell position;
  unsigned type = 0, typeClass = 1, height = 1, width = 1, movementKind = 0;
  unsigned health = 0, resource = 0, maximumResource = 0, inventoryResource = 0;
  int threat = 0, aging = 0, attackRange = 0;
  unsigned animationKind = 0;
  bool active = false, ranged = false, flying = false, flag104 = false,
       status114 = false, block7c = false, block723 = false, behavior14 = false;
  std::array<unsigned, 8> estimatedResource{};
};
struct RegionalDangerMemory {
  int key = 0, observedTick = 0;
  RegionalDangerActor actor;
};
struct RegionalDangerEffect {
  unsigned kind = 0;
  int owner = -1, radius = 0;
  Cell position;
};
struct RegionalDangerInputs {
  Extent dimensions;
  int tick = 0;
  std::array<bool, 8> enabled{};
  std::array<int, 8> commander{{-1, -1, -1, -1, -1, -1, -1, -1}};
  std::array<std::array<std::uint8_t, 8>, 8> allied{}, shared{};
  std::array<std::vector<RegionalDangerMemory>, 8> memories;
  std::vector<RegionalDangerActor> actors;
  std::vector<RegionalDangerEffect> effects;
  std::vector<Cell> observationPoints;
  // Original global-list base hazards, with their recovered per-entry amount.
  struct Hazard {
    Cell position;
    int radius = 0;
    unsigned amount = 0;
  };
  std::vector<Hazard> hazards;
  int firePower = 0, fireRange = 0, animationRange = 0;
};
struct RegionalDangerState {
  bool resetDanger = true, resetDeduction = true;
  int dangerOwner = -1, memoryKey = -1, dangerRegion = 0, commander = -1,
      dangerTotal = 0;
  int deductionOwner = -1, actorCursor = 125, deductionRegion = 0,
      deductionTotal = 0;
  std::vector<DangerRow> pendingDanger, pendingDeduction;
  std::vector<std::uint8_t> pendingBase;
  std::array<int, 8> dangerTotals{}, deductionTotals{};
  std::vector<int> actorSupport;
};
// 542810: strict squared-distance descent, ordered ties, middle path entry.
bool regionalDangerCenter(const std::vector<NavigationRegion> &, int, int,
                          const CreatureMovementParameters &, Extent, int &);
// 41a060 / 41d270: separate slices, four records/qualified actors and up to
// 32 effect regions per publication phase. Caller owns observation production.
void updateRegionalDanger(std::vector<NavigationRegion> &,
                          RegionalDangerState &, const RegionalDangerInputs &,
                          bool blocked = false);
void updateRegionalDeduction(std::vector<NavigationRegion> &,
                             RegionalDangerState &,
                             const RegionalDangerInputs &,
                             bool blocked = false);
void validateRegionalDanger(const std::vector<NavigationRegion> &,
                            const RegionalDangerState &,
                            const RegionalDangerInputs &);
} // namespace mnm::reconstruction::combat
