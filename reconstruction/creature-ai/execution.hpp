#pragma once
#include "../combat/autonomy.hpp"
#include <optional>

namespace mnm::reconstruction::combat {
// Logical ownership and state. Tokens identify owned effects, never addresses.
struct CreatureOrder {
  std::uint32_t kind = 2, serial = 0;
  Cell destination;
  std::int32_t target = -1;
  std::optional<std::uint32_t> effect;
  Cell referenceDestination;
  bool repeat = false;
};
struct CreatureExecution {
  std::vector<CreatureOrder> orders;
  std::optional<std::uint32_t> orderCursor, routeEffect;
  std::int32_t follow = -1;
  Cell castAim;
  Cell displacementDestination;
  Cell roamHome;
  std::int32_t roamRange = 0, roamKind = 0, roamAnnulus = 0, roamInterval = 0,
               roamCooldown = 0, fearBias = 0;
  std::array<std::int32_t, 6> combatValue{};
  std::array<std::uint32_t, 3> actionRouteCounters{};
  std::optional<unsigned> actionCallback;
  std::uint32_t queuedCount = 0, queuedFlag = 0, absorptionTimer = 0,
                block962 = 0, block963 = 0, block722 = 0, markerB73 = 0,
                marker6a0 = 0, routeRenewal = 0, routeChanged = 0, mode10c = 0,
                marker68 = 0, terrainRelocation = 0, dirtyValue = 0,
                commandMode = 0, nextOrderSerial = 1, targetDistance = 0,
                valueScale8ed = 0, bonusMode124 = 0, power84 = 0,
                mitigation90 = 0, rangedDeadline = 0, rangedLatch = 0,
                animationOffset = 0, animationFacing = 0, previousMove = 0,
                previousMoveVariant = 0, moveVariant = 0, block855 = 0,
                strategyGuard = 0, displacementActive = 0;
};
struct CreatureScheduler {
  std::int32_t budget = 0, cursor = 0, phase20 = 0, phase90 = 0;
  bool continuation = false;
};
struct SchedulerEnvironment {
  bool suppressRoutes = false, updateRegions = false, network = false;
  std::int32_t localOwner = 0;
  std::array<std::int32_t, 8> ownerController{};
};
struct SchedulerServices {
  std::function<bool(unsigned, CreatureScheduler &)> route;
  std::function<void(unsigned)> preDecision, decision, valuation, ownerWork;
  std::function<std::int32_t(unsigned)> region;
};
// Original52a780: phase values rotate independently of wall-clock frequency.
void scheduleCreatures(AutonomousWorld &, CreatureScheduler &,
                       const SchedulerEnvironment &, const SchedulerServices &);

struct ExecutionServices {
  std::function<void(AutonomousCreature &, unsigned)> behavior, action;
  std::function<void(std::uint32_t)> releaseEffect;
  std::function<void(unsigned)> restoreColor;
  std::function<void(AutonomousCreature &, unsigned)> peerEngagement;
  std::function<bool(AutonomousCreature &)> movementRenewal;
  std::function<void(AutonomousCreature &, Cell)> orient;
  std::function<void(AutonomousCreature &)> deliverQueue, updateQueue;
  std::function<void(const CreatureOrder &)> performOrder;
  std::function<bool(Cell &)> adjustDuplicate;
  std::function<Cell(AutonomousCreature &, Cell)> duplicateMove;
  std::function<int(Cell)> terrainHeight;
};
std::optional<unsigned> behaviorHandler(unsigned);
std::optional<unsigned> actionHandler(unsigned);
void changeBehavior(AutonomousCreature &, CreatureExecution &, unsigned,
                    const ExecutionServices &);
void resetBehavior(AutonomousCreature &, CreatureExecution &, AutonomousWorld &,
                   const ExecutionServices &);
void clearOrders(CreatureExecution &, const ExecutionServices &);
void setFollow(AutonomousCreature &, CreatureExecution &, AutonomousWorld &,
               std::int32_t, const ExecutionServices &);
void orderMove(AutonomousCreature &, CreatureExecution &, AutonomousWorld &,
               Cell, const ExecutionServices &);
void deliverOrder(AutonomousCreature &, const CreatureOrder &,
                  AutonomousWorld &, const std::function<void(Cell)> &,
                  const std::function<void(AutonomousCreature &)> &,
                  const std::function<void(Cell)> &);
bool orderTarget(AutonomousCreature &, CreatureExecution &, AutonomousWorld &,
                 std::int32_t, const ExecutionServices &);
void orderCoordinateAttack(AutonomousCreature &, CreatureExecution &,
                           AutonomousWorld &, Cell, const ExecutionServices &);
void appendOrder(AutonomousCreature &, CreatureExecution &, AutonomousWorld &,
                 CreatureOrder, const ExecutionServices &);
bool advanceOrders(AutonomousCreature &, CreatureExecution &,
                   const ExecutionServices &);
struct FollowServices {
  std::function<bool(const AutonomousCreature &, Cell &)> adjust;
  std::function<void(AutonomousCreature &, Cell)> move;
};
bool followReference(AutonomousCreature &, const CreatureExecution &,
                     AutonomousWorld &, const FollowServices &);
void repairCreatureReference(AutonomousCreature &, CreatureExecution &,
                             AutonomousWorld &, int removed,
                             const ExecutionServices &);
struct RouteExecutionServices {
  // 5128a0:0 failed,1 ready,2 retained continuation. Shared budget lives in
  // the route backend; neither the scheduler nor this layer invents a quota.
  std::function<unsigned(AutonomousCreature &, Cell, unsigned, unsigned)>
      request;
  std::function<bool(AutonomousCreature &)> resumeDisplacement, queueFallback;
  std::function<bool(const AutonomousCreature &, const AutonomousCreature &,
                     Cell)>
      reach;
  std::function<bool(const AutonomousCreature &, const AutonomousCreature &,
                     Cell &)>
      adjust;
  std::function<bool(const AutonomousCreature &, Cell)> legal, localReach;
  std::function<void(AutonomousCreature &)> beginMotion, reset;
  std::function<void(AutonomousCreature &, unsigned)> action;
  bool displacementPending = false;
};
bool continueCreatureRoute(AutonomousCreature &, CreatureExecution &,
                           AutonomousWorld &, const RouteExecutionServices &);
struct PeriodicEnvironment {
  bool terrainHasRegions = false;
  std::uint32_t relocationStrength = 0;
  std::vector<Cell> regions;
  std::int32_t regionCapacity = 0;
};
struct PeriodicServices {
  std::function<bool(Cell)> terrain;
  std::function<std::uint32_t()> random;
  std::function<void(AutonomousCreature &)> reset;
  std::function<void(AutonomousCreature &, Cell)> move;
};
void periodicTerrain(AutonomousCreature &, CreatureExecution &,
                     std::uint32_t resistance, const PeriodicEnvironment &,
                     const PeriodicServices &);
struct ValuationEnvironment {
  std::array<bool, 28> configured{};
  std::array<std::int32_t, 28> response{};
  std::array<std::int32_t, 2> statusBonus{};
  std::int32_t hthBonus = 0, responseBonus = 0, factor5a9 = 0, factor5ad = 0,
               factor5c5 = 0, factor589 = 0;
};
void updateCreatureValue(AutonomousCreature &, CreatureExecution &,
                         const ValuationEnvironment &);
struct RangedExecutionServices {
  std::function<bool(AutonomousCreature &)> blocked, prepareResponse;
  std::function<bool(AutonomousCreature &, const AutonomousCreature &,
                     unsigned)>
      inRange;
  std::function<bool(AutonomousCreature &, Cell, Cell)> closeResponse;
  std::function<void(AutonomousCreature &)> maintainTarget;
  std::function<void(AutonomousCreature &, Cell)> orient;
  std::function<void(AutonomousCreature &, unsigned)> action;
};
void rangedTactic(AutonomousCreature &, CreatureExecution &, AutonomousWorld &,
                  unsigned style, std::uint32_t tick,
                  const RangedExecutionServices &);
unsigned rangedStyle(unsigned creatureClass);
struct ActionEnvironment {
  unsigned regularOverlay = 0, regularOverlayOffset = 0, meleeOverlay = 0,
           meleeOverlayOffset = 0;
};
struct ActionExecutionServices {
  std::function<std::uint32_t()> random;
  std::function<void(AutonomousCreature &, unsigned, unsigned)> animation;
  std::function<void(AutonomousCreature &, unsigned)> overlay;
  std::function<void(AutonomousCreature &, unsigned, unsigned)>
      overlayAnimation;
  std::function<void(AutonomousCreature &, const AutonomousCreature *)>
      attackAnimation;
  std::function<bool(AutonomousCreature &)> flightAnimation;
  std::function<void(AutonomousCreature &)> repairOverlay;
};
void changeAction(AutonomousCreature &, CreatureExecution &, AutonomousWorld &,
                  unsigned, const ActionEnvironment &,
                  const ActionExecutionServices &);
struct AnimationActionServices {
  std::function<int(AutonomousCreature &, bool overlay)> poll;
  std::function<void(AutonomousCreature &)> cast, behavior;
  std::function<void(AutonomousCreature &, unsigned)> restoreForm;
  std::function<void(AutonomousCreature &, unsigned, unsigned)> animation;
};
void tickRangedAction(AutonomousCreature &, CreatureExecution &, bool alternate,
                      unsigned tick, unsigned recovery, unsigned restoreMode,
                      const AnimationActionServices &);
void tickReadyAction(AutonomousCreature &, CreatureExecution &,
                     const AnimationActionServices &);
struct StrategicScenario {
  std::uint32_t active = 0, kind = 0, state = 0, key = 0;
};
struct StrategyEnvironment {
  std::vector<Cell> regions, blockedColumns;
  std::vector<StrategicScenario> scenarios;
  std::int32_t regionCapacity = 0, waterline = 0;
  std::function<std::optional<unsigned>(const StrategicScenario &)> scenarioActor;
};
struct StrategyServices {
  std::function<std::uint32_t()> random;
  std::function<bool(const AutonomousCreature &, const AutonomousCreature &)>
      admit;
  std::function<void(AutonomousCreature &, const AutonomousCreature &)> attack;
  std::function<void(AutonomousCreature &, unsigned)> substate, action;
  std::function<void(AutonomousCreature &, const AutonomousCreature &)>
      releaseAlternate;
  std::function<bool(Cell, Cell, int)> insideInnerRadius;
  std::function<bool(const AutonomousCreature &, Cell)> legal, water;
  std::function<bool(const AutonomousCreature &, int, int, unsigned)>
      regionReach;
  std::function<void(AutonomousCreature &, Cell)> move;
};
struct RoamChoice {
  bool admitted = false;
  int region = 0;
};
RoamChoice chooseRoamRegion(const AutonomousCreature &,
                            const CreatureExecution &, const AutonomousWorld &,
                            const StrategyEnvironment &,
                            const StrategyServices &);
void strategicCreatureWork(AutonomousCreature &, CreatureExecution &,
                           AutonomousWorld &, const StrategyEnvironment &,
                           const StrategyServices &);
struct BehaviorExecutionServices {
  std::function<bool(AutonomousCreature &)> immediate, interaction, follow,
      queue, blocked, rangedOpponent;
  std::function<void(AutonomousCreature &, unsigned)> action, behavior, restoreForm;
  std::function<void(AutonomousCreature &)> reset, beginMotion, ranged, maintainTarget;
  std::function<bool(const AutonomousCreature &, const AutonomousCreature &, Cell)> reach;
  std::function<void(AutonomousCreature &, const AutonomousCreature &)> face;
  std::function<bool(AutonomousCreature &, Cell, unsigned)> coordinateRange;
  std::function<void(AutonomousCreature &, Cell)> coordinateFace;
};
void idleBehavior(AutonomousCreature &, const BehaviorExecutionServices &);
void maintainTargetBehavior(AutonomousCreature &, CreatureExecution &,
                             AutonomousWorld &, unsigned restoreMode,
                             const BehaviorExecutionServices &);
void targetAttackBehavior(AutonomousCreature &, CreatureExecution &,
                           AutonomousWorld &, const BehaviorExecutionServices &);
void coordinateAttackBehavior(AutonomousCreature &, CreatureExecution &,
                               unsigned tick, const BehaviorExecutionServices &);
void immediateAttackBehavior(AutonomousCreature &, AutonomousWorld &,
                              const BehaviorExecutionServices &);
void standingRangedBehavior(AutonomousCreature &, CreatureExecution &,
                            AutonomousWorld &, unsigned tick,
                            const BehaviorExecutionServices &);
void displacementBehavior(AutonomousCreature &, CreatureExecution &,
                           const BehaviorExecutionServices &);
void specialBehavior(AutonomousCreature &, CreatureExecution &, unsigned kind,
                      const BehaviorExecutionServices &);
} // namespace mnm::reconstruction::combat
