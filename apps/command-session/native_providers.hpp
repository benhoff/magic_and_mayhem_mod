#pragma once
#include "runtime.hpp"
#include "../../reconstruction/pathfinding/route_scalar.hpp"
#include "../../reconstruction/pathfinding/route_cell_validity.hpp"
#include "../../reconstruction/creature-ai/death_behavior.hpp"
#include "../../reconstruction/motion/creature_profile.hpp"
namespace mnm::command_session {
struct NativeCreatureType {
  unsigned bodyAsset=0,regularOverlayAsset=0,meleeOverlayAsset=0;
  int width=1,height=1,movementKind=0;
  unsigned periodicResistance=0;
  int routeCostFactor=1;
  bool routeFlagPenalty=false;
  std::array<int,28> gestureScores{};
  reconstruction::CreatureScalarState scalar;
  unsigned regionalFlightClearance=0;
};
// Immutable decoded inputs. Applications may load actual ANI/CFG/MAP assets;
// profile construction/import and complete campaign construction are separate.
struct NativeCreatureProfile {
  std::vector<assets::Animation> animation;
  std::array<NativeCreatureType,28> types;
  std::vector<reconstruction::TerrainValidityRecord> terrain;
  std::vector<bool> terrainPropertyA0;
  model::MaintenanceEnvironment maintenance;
  model::EnvironmentPassConfiguration environment;
  model::ValuationEnvironment valuation;
  model::PeriodicEnvironment periodic;
  unsigned cameraTurn=0,defaultScalar=720,recovery=9;
  int boundary=0;
};
// These own staged health/effect/World/profile-specific consequences. ANI,
// geometry, route motion, action dispatch and creature pass ordering are
// supplied by nativeProviders, never by a synthetic frame callback.
struct NativeConsequences {
  // Operation-local context. No address enters the continuation.
  std::function<void(model::OwnedCreatureCommands &)> bindCommands;
  model::ActionCallbackServices callbacks;
  model::ExecutionServices execution;
  model::SegmentInitializationServices segment;
  model::SegmentCompletionServices completion;
  model::MotionActionServices motion;
  model::RangedExecutionServices ranged;
  model::SpecialFlightServices flight;
  model::MaintenanceServices maintenance;
  model::BehaviorExecutionServices behavior;
  model::MovementBehaviorServices movement;
  model::DeathServices death;
  model::AutonomousServices autonomy;
  std::function<void(model::AutonomousCreature &)> preDecision,ownerWork;
  std::function<int(model::AutonomousCreature &)> region;
};
using NativeConsequenceFactory=std::function<NativeConsequences(Continuation &)>;
// Copies immutable inputs and binds their exact deterministic profile stamp.
// All provider objects borrow only an operation's staged continuation.
HookFactory nativeProviders(NativeCreatureProfile,NativeConsequenceFactory);
void prepareNativeContinuation(Continuation &,const NativeCreatureProfile &);
} // namespace mnm::command_session
