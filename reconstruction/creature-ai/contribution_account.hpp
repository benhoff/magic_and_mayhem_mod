#pragma once
#include <array>
#include <cstdint>
namespace mnm::reconstruction::combat {
struct ContributionProfile {
  std::uint32_t kind = 0, capacity = 1, flag58 = 0;
  std::array<std::uint32_t, 28> first{}, second{};
};
using ContributionProfiles = std::array<ContributionProfile, 28>;
// Whole505bf0/505c50 numeric profile access, without legacy pointers.
unsigned contributionProfileFirst(const ContributionProfiles &, unsigned source,
                                  unsigned target);
unsigned contributionProfileSecond(const ContributionProfiles &,
                                   unsigned source, unsigned target);
// Whole416d80 numeric return. Signed quantities, float arguments and pinned
// constants retain their original arithmetic roles. Require x87-equivalent
// 64-significand precision/nearest rounding and finite valid divisors/results.
// This API does not assign a scoring/veterancy interpretation to the word.
unsigned contributionAccount(const ContributionProfiles &, unsigned type,
                             int level, int quantity, float amount, float delta,
                             float extra);
} // namespace mnm::reconstruction::combat
