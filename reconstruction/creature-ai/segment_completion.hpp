#pragma once
#include "full_motion_action.hpp"
namespace mnm::reconstruction::combat {
struct SegmentRoutePoint {
  Cell position;
  std::array<int, 4> remainder{};
};
struct SegmentContributionNode {
  std::optional<unsigned> parent, owner;
};
struct SegmentRegion {
  Cell position;
  unsigned parameter = 0;
};
struct SegmentCompletionState {
  std::array<SegmentRoutePoint, 16> route{};
  unsigned cursor = 0, count = 0, landing110 = 0, overlayFrame809 = 0;
  bool landingOverlay71d = false, scorePendingFc = false, regionPending = false;
  std::optional<unsigned> overlayLink805;
  std::array<std::optional<unsigned>, 2> contributionRoots{};
  std::vector<SegmentContributionNode> contributionNodes;
  std::vector<unsigned> contributionTotals;
  std::array<int, 50> regionPath{};
  unsigned regionCursor = 0;
};
struct SegmentCompletionEnvironment {
  bool regionContinuation = false;
  int scoreNumerator = 1, scoreDenominator = 1;
  std::vector<SegmentRegion> regions;
};
struct SegmentCompletionServices {
  std::function<void(AutonomousCreature &)> stopOverlay, environment, animation,
      terrain, reset;
  std::function<void(unsigned)> releaseOverlay;
  std::function<unsigned(AutonomousCreature &, unsigned, int)> fallDamage;
  std::function<void(AutonomousCreature &, unsigned, int, bool)> defended;
  std::function<void(AutonomousCreature &, Cell, unsigned)> snap;
  std::function<unsigned(unsigned, float)> score;
  std::function<void(AutonomousCreature &, Cell &, unsigned, Cell)>
      regionCandidate;
  std::function<bool(const AutonomousCreature &, Cell)> legal;
};
void completeMotionSegment(AutonomousCreature &, CreatureExecution &,
                           MotionActionState &, MovementBehaviorState &,
                           SegmentCompletionState &,
                           const SegmentCompletionEnvironment &,
                           const SegmentCompletionServices &);
struct PositionSnapServices {
  std::function<unsigned(Cell)> tileKind;
  std::function<int(unsigned)> tileHeight;
  std::function<int(Cell)> largeHeight;
};
// Definition width is the global type record+8, not an assumed actor override.
void snapCreaturePosition(AutonomousCreature &, Cell, unsigned definitionWidth,
                          const PositionSnapServices &);
} // namespace mnm::reconstruction::combat
