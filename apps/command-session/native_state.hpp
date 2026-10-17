#pragma once
#include "../../reconstruction/animation/controller.hpp"
#include "../../reconstruction/creature-ai/actor_route_request.hpp"
#include "../../reconstruction/combat/melee.hpp"
#include "../../reconstruction/creature-ai/full_segment_setup.hpp"
#include "../../reconstruction/creature-ai/environment_pass.hpp"
#include "../../reconstruction/pathfinding/route_search.hpp"
#include "../../reconstruction/creature-ai/regional_navigation.hpp"
#include "../../reconstruction/creature-ai/regional_maintenance.hpp"
#include "../../reconstruction/creature-ai/regional_danger.hpp"
#include "../../reconstruction/creature-ai/owner_knowledge.hpp"
#include "../../reconstruction/creature-ai/owner_visibility.hpp"
#include "../../reconstruction/creature-ai/owner_planner.hpp"
#include "../../reconstruction/creature-ai/owner_hostile_task.hpp"
#include "../../reconstruction/creature-ai/brain_inference.hpp"
#include "../../reconstruction/creature-ai/owner_group_task.hpp"
#include "../../reconstruction/creature-ai/owner_task_probe.hpp"
#include "../../reconstruction/creature-ai/owner_task_lifecycle.hpp"
namespace mnm::command_session {
struct NativeAnimationChannel {
  unsigned asset=0,sequence=0;
  bool bound=false;
  reconstruction::AnimationControllerState state;
};
struct NativeActorAnimation {
  NativeAnimationChannel body,overlay;
  std::array<NativeAnimationChannel,6> auxiliary;
};
struct NativeGeometryCell {
  std::uint16_t kind=0,occupant=0xffff,flags=0,links=0;
};
struct NativeOpenNode {int priority=0;unsigned node=0;};
struct NativeSearchNode {unsigned node=0;reconstruction::NodeRecord record;};
struct NativeSearchContinuation {
  std::vector<NativeOpenNode> open;
  std::vector<NativeSearchNode> records;
  unsigned bestNode=0;
  int bestHeuristic=0;
  bool initialized=false;
  reconstruction::combat::Cell destination;
  // Known packed search prefix carried as explicit owned bytes. It is a
  // build-specific wire projection, not a host C++ object layout/pointer.
  std::array<std::uint8_t,0x269> context{};
};
struct NativeRegionalMaintenance {
  std::array<reconstruction::combat::RegionalCreatureShape,28> shapes;
  reconstruction::combat::NavigationMaintenance state;
};
struct NativeRegionalDanger {
  reconstruction::combat::RegionalDangerInputs inputs;
  reconstruction::combat::RegionalDangerState state;
};
struct NativeOwnerVisibility {
  reconstruction::combat::VisibilityState scan;
  reconstruction::combat::VisionRelations sharing{};
  std::vector<reconstruction::combat::SightDefinition> definitions;
  std::vector<std::array<int,8>> observers;
};
struct NativeOwnerReplanning {
  reconstruction::combat::OwnerTask6Admission admission;
  reconstruction::combat::OwnerReplacementPolicy policy;
  std::optional<reconstruction::combat::OwnerPreviousTask> previous;
  int candidateDuration=0,bestDuration=0;
  bool eligible=false,prepared=false;
};
struct NativeOwnerGroupPlanner {
  reconstruction::combat::OwnerMemoryGroups groups;
  std::vector<reconstruction::combat::OwnerMemory> observations;
  unsigned cursor=0;
  int deadline=0,from=-1,to=-1,excessPercent=0;
  reconstruction::combat::OwnerTaskProbe probe;
  std::optional<reconstruction::combat::OwnerGroupTask> candidate,best,active;
  std::optional<NativeOwnerReplanning> replanning;
};
struct NativeOwnerPlanner {
  unsigned owner=0,actor=0,nextSerial=1,activeSerial=0;
  reconstruction::combat::BrainInferenceModel model;
  std::array<float,3> weights{{1,0,1}};
  int percent=100;
  reconstruction::combat::RandomState random;
  reconstruction::combat::OwnerTaskSelection selection;
  bool fallbackEmitted=false,hostileEmitted=false,candidateReady=false;
  reconstruction::combat::OwnerHostileTask candidate;
  float candidateScore=0;
  std::vector<int> features;
  std::optional<reconstruction::combat::OwnerHostileTask> active;
  std::optional<NativeOwnerGroupPlanner> grouped;
};
struct NativeProviderState {
  bool present=false,randomSeeded=false,alternate=false;
  std::array<std::uint32_t,2> profileStamp{};
  unsigned tick=0,sequence=0;
  reconstruction::combat::RandomState random;
  std::vector<NativeActorAnimation> animation;
  std::array<NativeAnimationChannel,8> worldAnimation;
  std::vector<NativeGeometryCell> cells;
  std::vector<NativeSearchContinuation> search;
  std::vector<reconstruction::combat::SegmentInitializationState> initialization;
  reconstruction::combat::EnvironmentPassState environment;
  reconstruction::combat::ActorRouteContext route;
  std::vector<reconstruction::combat::ActorRouteMetadata> routeMetadata;
  std::vector<int> displacementOwners;
  std::vector<reconstruction::combat::NavigationRegion> regions;
  reconstruction::combat::RegionalSearchState regionalSearch;
  std::array<unsigned,2> regionalRiskPowers{};
  std::optional<NativeRegionalMaintenance> regionalMaintenance;
  std::optional<NativeRegionalDanger> regionalDanger;
  std::optional<reconstruction::combat::OwnerKnowledge> ownerKnowledge;
  std::optional<NativeOwnerVisibility> ownerVisibility;
  std::optional<NativeOwnerPlanner> ownerPlanner;
};
} // namespace mnm::command_session
