#pragma once
#include "segment_completion.hpp"
namespace mnm::reconstruction::combat {
struct SegmentInitializationState {
  int vertical = 0, slow778 = 0, fast77c = 0;
  unsigned specialRate73c = 0;
  // Definition-relative byte offset; inline71e is a separate logical bank.
  // Composition must bind the selected owned bank to MotionActionState.samples.
  unsigned sampleBank = 0xd8, initialSampleBank = 0xd8;
  int inlineSample = 0;
};
struct SegmentInitializationEnvironment {
  unsigned cameraTurn = 0, defaultScalar = 720;
};
struct SegmentInitializationServices {
  std::function<bool(AutonomousCreature &)> precheck, immediate, fallback;
  std::function<void(AutonomousCreature &)> visualFacing;
  std::function<unsigned(const AutonomousCreature &, unsigned, Cell)> base;
  std::function<void(const AutonomousCreature &, unsigned, unsigned, Cell, int,
                     unsigned &, int &)>
      scalar;
  std::function<bool(AutonomousCreature &, Cell, Cell, unsigned &)> accept;
  std::function<bool(const AutonomousCreature &, Cell)> hazard;
  std::function<void(AutonomousCreature &, Cell)> release, reserve;
  std::function<int(unsigned, unsigned)> turn;
  std::function<unsigned(Cell, Cell)> scalarArgument;
  std::function<unsigned(Cell)> distance;
  std::function<unsigned(Cell)> tileKind;
  std::function<int(unsigned)> tileHeight;
  std::function<void(AutonomousCreature &, unsigned, unsigned)> animation;
  std::function<int(const AutonomousCreature &, unsigned, unsigned)> sample;
  std::function<void(AutonomousCreature &, const AutonomousCreature &)> attack;
  std::function<void(AutonomousCreature &, unsigned)> action;
};
void initializeMotionSegment(AutonomousCreature &, CreatureExecution &,
                             MotionActionState &, SegmentInitializationState &,
                             const SegmentCompletionState &, AutonomousWorld &,
                             const SegmentInitializationEnvironment &,
                             const SegmentInitializationServices &);
} // namespace mnm::reconstruction::combat
