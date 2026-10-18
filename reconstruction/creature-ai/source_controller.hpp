#pragma once
#include "../combat/melee.hpp"
#include "source_graph.hpp"
namespace mnm::reconstruction::combat {
struct SourceNodeInformation {
  unsigned kind = 0, tag9 = 0;
  std::uint8_t hint25 = 0;
  std::optional<unsigned> recipient;
};
struct SourceControllerState {
  SourceGraph graph;
  std::vector<SourceNodeInformation> information;
  std::vector<std::array<std::optional<unsigned>, 2>> roots;
  std::vector<unsigned> actorFlag8e5;
  std::vector<unsigned> recipientRoot;
  std::array<unsigned, 8> ownerFlag4{};
  std::array<int, 8> controllers{};
  int mode = 1;
  RandomState random;
};
struct SourceManager {
  unsigned kind = 0;
  std::vector<unsigned> nodes;
  std::array<std::optional<unsigned>, 2> excluded;
};
struct SourceControllerConfiguration {
  int players = 1, networkOwner = 0, rootThreshold = 0;
  // Opaque input/allocator representation, separate from recovered decisions.
  // Zero-filled supplied allocations and actor-argument lowbyte0 are compared;
  // ordinary native storage initializes its own representation
  // deterministically.
  std::uint8_t initialHint25 = 0, coordinateHint25 = 0;
};
struct SourceControllerServices {
  std::function<bool(unsigned bytes)> allocate;
  std::function<void(unsigned node)> registerRoot;
};
// Whole416590 logical control/grouping projection. Lower successful allocation,
// vector storage and recipient registration remain explicit owned boundaries.
void updateCreatureSourceController(unsigned actor, AutonomousWorld &,
                                    SourceControllerState &, SourceManager &,
                                    const SourceControllerConfiguration &,
                                    const SourceControllerServices &);
} // namespace mnm::reconstruction::combat
