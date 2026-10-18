#pragma once
#include "../combat/autonomy.hpp"
namespace mnm::reconstruction::combat {
struct ResourceContributionNode {
  std::optional<unsigned> parent, recipient;
};
struct ResourceContributionState {
  std::array<std::int32_t, 8> controllers{};
  std::vector<std::uint32_t> resource, totals;
  std::vector<ResourceContributionNode> ancestors;
  std::vector<std::array<std::optional<unsigned>, 2>> roots;
};
struct ResourceContributionServices {
  std::function<std::optional<unsigned>(int)> subject;
  std::function<void(AutonomousCreature &, unsigned)> change;
  std::function<unsigned(unsigned, float, float)> account;
};
// Inline509fd1..50a0d5 after outer509e50 admission/deadline update. Names
// preserve numeric roles; counter2f and the account return are not asserted
// veterancy.
void contributeCreatureResource(AutonomousCreature &, AutonomousWorld &,
                                ResourceContributionState &, unsigned grant,
                                const ResourceContributionServices &);
} // namespace mnm::reconstruction::combat
