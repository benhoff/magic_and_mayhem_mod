#pragma once
#include "full_segment_setup.hpp"
namespace mnm::reconstruction::combat {
// Owned counterpart of the131-DWORD route transfer. No host pointers or shared
// object layouts. Motion words retain exact bits until the actor applies them.
struct ActorRouteSnapshot {
  Cell destination;
  unsigned cursor = 0, count = 0;
  std::array<SegmentRoutePoint, 16> points{};
  std::array<unsigned, 14> motionWords{};
};
struct ActorRouteMetadata {
  Cell regionDestination;
  unsigned marker69c = 0;
};
struct ActorRouteContext {
  unsigned active = 0, displacementContinuation = 0, mode = 0;
  int budget = 0, requestBudget = 0;
  Cell destination, origin;
  ActorRouteSnapshot scratch;
  bool scratchDirty = false, useRegions = false, diagnostic = false;
};
struct ActorDisplacementServices {
  std::function<bool(const AutonomousCreature &, const AutonomousCreature *)>
      rangedReached;
  std::function<bool(const AutonomousCreature &, const AutonomousCreature *,
                     Cell)>
      meleeReached;
  // Blocker/obstacle services use the original slot+1 convention; zero means
  // absent. The owned core resolves slots only where the body dereferences
  // them.
  std::function<unsigned(AutonomousCreature &)> blocker;
  std::function<bool(AutonomousCreature &, unsigned)> canDisplace, issue;
  std::function<unsigned(AutonomousCreature &, Cell)> obstacle;
  std::function<unsigned(AutonomousCreature &, ActorRouteContext &)> update;
  std::function<unsigned()> random;
  std::function<void(unsigned)> diagnostic;
  std::function<void(AutonomousCreature &, unsigned)> action;
  std::function<void(AutonomousCreature &)> reset;
};
bool displaceCreatureRoute(AutonomousCreature &, AutonomousWorld &,
                           const std::vector<int> &displacementOwners,
                           const std::vector<CreatureExecution> &peerExecution,
                           ActorRouteContext &,
                           const ActorDisplacementServices &);
} // namespace mnm::reconstruction::combat
