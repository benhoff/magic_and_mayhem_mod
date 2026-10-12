#pragma once
#include "ai.hpp"

namespace mnm::reconstruction::combat {
// Logical values, never a shared packed creature or host-pointer layout.
struct InventoryEntry {
  std::uint32_t kind = 0, magnitude = 0, count = 0, available = 0;
};
struct AutonomousCreature {
  AiCreature ai;
  std::uint32_t width = 1, height = 1, animationKind = 0, inventoryCount = 0,
                block7c = 0, configFlags = 0, hazardSensitive = 0,
                protection8c9 = 0, mode = 0, form = 0, formCapability = 0,
                block784 = 0, blockDE3 = 0, block71c = 0, block723 = 0,
                specialVision = 0, routePhase = 0, substate = 0,
                movementKind = 0, flag104 = 0, selectedSpell = 0,
                targetMovingAttacker = 0, routeMode = 0, castSpell = 0;
  std::int32_t alternateTarget = -1, rangedDistance = 0, region = -1, range = 0,
               castSource = -1, castTarget = -1, cooldown = 0;
  Cell destination, chosenDestination, finePosition;
  std::int32_t aimXY = 0, aimZ = 0;
  std::array<InventoryEntry, 42> inventory{};
};
struct AutonomousWorld {
  Extent dimensions;
  std::vector<AutonomousCreature> creatures;
  std::array<std::array<std::uint8_t, 8>, 8> relation{};
  std::array<bool, 8> human{},
      ownerEligible{{true, true, true, true, true, true, true, true}};
  std::int32_t localOwner = 0;
};
struct AiObject {
  Cell position;
  std::uint32_t kind = 0;
};
struct AiThreat {
  Cell position;
  std::uint32_t spell = 0;
  std::int32_t owner = 0;
};
struct AiRegionEdge {
  std::uint32_t active = 0;
  std::int32_t destination = -1;
  std::array<std::int8_t, 28> bounds{};
};
struct AiRegion {
  Cell position;
  std::array<AiRegionEdge, 18> edges{};
};
struct AiTerrainCell {
  std::uint16_t occupant = 0xffff, flags = 0;
  bool tilePropertyA0 = false;
};
// The arrays are the original ordered tables, not an authored neighbor policy.
struct AiGeometry {
  std::array<Cell, 27> hazardOffsets{}, escapeOffsets{}, retreatOffsets{};
  std::array<Cell, 26> routeOffsets{};
  std::function<AiTerrainCell(Cell)> cell;
  std::function<bool(const AutonomousCreature &, Cell)> hazardous;
  std::function<bool(const AutonomousCreature &, Cell, Cell, unsigned)> legal;
  std::function<bool(const AutonomousCreature &, Cell, Cell)> objectReachable;
  std::function<bool(const AutonomousCreature &, const AutonomousCreature &,
                     Cell)>
      reach;
  std::function<bool(const AutonomousCreature &, Cell)> lineOfSight;
  std::function<bool(const AutonomousCreature &, const AiThreat &)>
      threatReaches;
};
enum class AutonomousEffect {
  reset,
  move,
  beginMotion,
  orient,
  useInventory,
  refreshInventory,
  behavior,
  action,
  prepareInteraction,
  rangedAttack,
  faceTarget,
  health
};
struct AutonomousEvent {
  AutonomousEffect kind;
  Cell position;
  std::int32_t argument = 0;
  bool operator==(const AutonomousEvent &o) const {
    return kind == o.kind && position == o.position && argument == o.argument;
  }
};
std::int32_t localHazard(const AutonomousCreature &, Cell, Extent,
                         const AiGeometry &);
bool targetAdmitted(const AutonomousCreature &, const AutonomousCreature &,
                    Extent, const AiGeometry &);
std::vector<AutonomousEvent> escapeLocal(const AutonomousCreature &, Extent,
                                         const AiGeometry &,
                                         const std::function<bool(Cell)> &move);
bool chooseInteraction(AutonomousCreature &, const AutonomousWorld &,
                       const std::vector<AiObject> &, const AiGeometry &);
bool chooseDestination(AutonomousCreature &, const AutonomousWorld &,
                       const std::vector<AiObject> &, const AiGeometry &);
std::vector<AutonomousEvent>
healFromInventory(AutonomousCreature &, const AutonomousWorld &,
                  const std::function<void(InventoryEntry &)> &use,
                  const std::function<void(std::uint32_t)> &refresh = {});
bool immediateOpponent(AutonomousCreature &, const AutonomousWorld &,
                       const AiGeometry &,
                       const std::function<std::uint32_t()> &random);
std::vector<AutonomousEvent> orientToThreat(const AutonomousCreature &,
                                            const AutonomousWorld &,
                                            const AiGeometry &);
bool assistAdmitted(const AutonomousCreature &ally,
                    const AutonomousCreature &enemy, Extent);
void attackResponse(AutonomousCreature &, const AutonomousWorld &);
struct AutonomousServices {
  AiGeometry geometry;
  std::vector<AiObject> objects;
  std::vector<AiThreat> threats;
  std::vector<AiRegion> regions;
  // These services execute consequences and can reenter/mutate the actor.
  std::function<void(AutonomousCreature &, std::uint32_t)> behavior;
  std::function<void(AutonomousCreature &, std::uint32_t)> action;
  std::function<void(AutonomousCreature &)> reset, prepareInteraction;
  std::function<void(AutonomousCreature &)> beginMotion, rangedAttack;
  std::function<void(AutonomousCreature &, const AutonomousCreature &)>
      faceTarget;
  std::function<void(AutonomousCreature &, std::int32_t)> changeHealth;
  std::function<void(AutonomousCreature &, Cell)> move;
  std::function<bool(AutonomousCreature &, Cell)> routeMove;
  std::function<void(AutonomousCreature &, InventoryEntry &)> useInventory;
  std::function<void(Cell)> orient;
  std::function<void(std::uint32_t)> refreshInventory;
  std::function<std::uint32_t()> random;
};
void setAutonomousSubstate(AutonomousCreature &, AutonomousWorld &,
                           std::uint32_t, const AutonomousServices &);
bool priorityResponse(AutonomousCreature &, AutonomousWorld &,
                      const AutonomousServices &);
void autonomousDecision(AutonomousCreature &, AutonomousWorld &,
                        const AutonomousServices &);
bool retreatLocal(AutonomousCreature &, const AutonomousWorld &,
                  const AutonomousServices &);
void autonomousBehavior(AutonomousCreature &, AutonomousWorld &,
                        const AutonomousServices &);
} // namespace mnm::reconstruction::combat
