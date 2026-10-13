#include "contribution_account.hpp"
#include <cmath>
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
int word(unsigned v) { return v <= 0x7fffffffu ? int(v) : -1 - int(~v); }
unsigned truncateWord(long double v) {
  if (!std::isfinite(v) || v < -9223372036854775808.0L ||
      v >= 9223372036854775808.0L)
    throw std::invalid_argument("account integer conversion domain");
  return unsigned(std::int64_t(v));
}
void validTypes(unsigned source, unsigned target) {
  if (source >= 28 || target >= 28)
    throw std::invalid_argument("account profile domain");
}
unsigned profileValue(const ContributionProfiles &p, unsigned source,
                      unsigned target, unsigned v) {
  return p[target].flag58 && !p[source].flag58 ? unsigned(word(v) / 3) : v;
}
} // namespace
unsigned contributionProfileFirst(const ContributionProfiles &p,
                                  unsigned source, unsigned target) {
  validTypes(source, target);
  if (p[source].kind == 8 || p[source].kind == 20)
    return 10;
  return profileValue(p, source, target, p[source].first[target]);
}
unsigned contributionProfileSecond(const ContributionProfiles &p,
                                   unsigned source, unsigned target) {
  validTypes(source, target);
  return profileValue(p, source, target, p[source].second[target]);
}
unsigned contributionAccount(const ContributionProfiles &p, unsigned type,
                             int level, int quantity, float amount, float delta,
                             float extra) {
  validTypes(type, 0);
  const auto &profile = p[type];
  unsigned base;
  if (profile.kind == 0 || profile.kind == 24 || profile.kind == 25 ||
      profile.kind == 26) {
    const int previous = word(unsigned(level) - unsigned(quantity) + 1u);
    const int current = word(unsigned(level) + 1u);
    if (!previous || !current)
      throw std::invalid_argument("account quantity divisor");
    const unsigned cap = p[0].capacity + 1u;
    base = truncateWord((1.0L / previous - 1.0L / current) * word(cap * cap) +
                        static_cast<long double>(0.999999));
  } else {
    if (!profile.capacity)
      throw std::invalid_argument("account capacity divisor");
    unsigned aggregate = 0;
    for (unsigned target = 0; target < 28; ++target)
      aggregate += contributionProfileFirst(p, type, target) +
                   contributionProfileSecond(p, type, target);
    const unsigned scaled = (aggregate / 28) * unsigned(quantity);
    base = (scaled * 40u + profile.capacity - 1u) / profile.capacity;
  }
  const long double denominator = static_cast<long double>(amount) + 30.0L;
  const long double before = denominator - static_cast<long double>(delta);
  if (!denominator || !before || !std::isfinite(denominator) ||
      !std::isfinite(before))
    throw std::invalid_argument("account resource divisor");
  const unsigned middle = truncateWord(
      (1.0L / before - 1.0L / denominator) * 180000.0L + word(base));
  return truncateWord(static_cast<long double>(word(middle)) +
                      static_cast<long double>(extra) * 1000000.0L);
}
} // namespace mnm::reconstruction::combat
