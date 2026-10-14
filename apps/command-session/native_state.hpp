#pragma once
#include "../../reconstruction/animation/controller.hpp"
#include "../../reconstruction/combat/melee.hpp"
#include "../../reconstruction/creature-ai/full_segment_setup.hpp"
#include "../../reconstruction/creature-ai/environment_pass.hpp"
#include "../../reconstruction/pathfinding/route_search.hpp"
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
};
} // namespace mnm::command_session
