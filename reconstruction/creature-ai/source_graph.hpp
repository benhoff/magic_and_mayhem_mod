#pragma once
#include "../combat/autonomy.hpp"
namespace mnm::reconstruction::combat {
struct SourceGraphNode {
  std::optional<unsigned> actor, parent;
  std::vector<unsigned> children;
  unsigned activity = 0, roundRobin = 0;
  std::array<std::uint8_t, 8> free{};
  std::array<unsigned, 8> position{}, nested{};
};
using SourceGraph = std::vector<SourceGraphNode>;
// Whole407920/4079f0: eight mutable traversal slots. Actor leaves use a
// signed activity>=mode gate; no health or active-actor filter is introduced.
unsigned beginSourceTraversal(SourceGraph &, unsigned node);
std::optional<unsigned> nextSourceActor(SourceGraph &, unsigned node,
                                        unsigned cursor, int mode);
// Whole514810/5183f0/51e790 with owner0..7 and root families0..1.
void assignCreatureSourceRoot(int owner, unsigned family,
                              std::optional<unsigned>,
                              const std::array<unsigned, 8> &ownerFlag4,
                              std::array<std::optional<unsigned>, 2> &roots);
std::optional<unsigned>
terminalCreatureSourceRoot(const SourceGraph &, unsigned family,
                           const std::array<std::optional<unsigned>, 2> &roots);
bool creatureControllerAbsent(int owner, const std::array<int, 8> &controllers);
} // namespace mnm::reconstruction::combat
