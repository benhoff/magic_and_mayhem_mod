#pragma once
#include "owner_knowledge.hpp"
#include <functional>
namespace mnm::reconstruction::combat {
struct OwnerMemoryGroupNode {
  int memoryKey = -1, parent = -1;
  Cell position;
  int region = -1, uncertaintySquared = 0, flag104 = 0;
  unsigned activity = 3;
  std::vector<unsigned> children;
};
struct OwnerMemoryGroups {
  std::vector<OwnerMemoryGroupNode> nodes;
};
struct OwnerMemoryGroupServices {
  std::function<bool(int, int, int &)> regional;
  std::function<bool(Cell, Cell)> line;
};
// 413330: clear nodes at the knowledge epoch boundary; a nonempty cache is
// reused. Node0 owns ordered top-level groups. Leaves retain anonymous keys.
void buildOwnerMemoryGroups(
    OwnerMemoryGroups &, const OwnerKnowledge &, unsigned owner, int tick,
    bool includeStale, Extent,
    const std::array<std::array<std::uint8_t, 8>, 8> &allied,
    const OwnerMemoryGroupServices &);
void validateOwnerMemoryGroups(const OwnerMemoryGroups &, Extent);
std::vector<int> ownerGroupKeys(const OwnerMemoryGroups &, unsigned root);
} // namespace mnm::reconstruction::combat
