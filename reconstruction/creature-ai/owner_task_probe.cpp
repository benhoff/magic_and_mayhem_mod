#include "owner_task_probe.hpp"
#include <algorithm>
#include <climits>
#include <set>
#include <stdexcept>
namespace mnm::reconstruction::combat {
OwnerTaskProbeResult
advanceOwnerTaskProbe(OwnerTaskProbe &s, std::vector<NavigationRegion> &g,
                      int from, int to, const RegionalQuery &q, Extent d,
                      int &budget, const RegionalServices &services) {
  if (budget < 0 || s.remaining < 0 || s.remaining > 20 || g.size() > 4096 ||
      q.trafficPenalty)
    throw std::invalid_argument("owner task probe domain");
  if (s.result != OwnerTaskProbeResult::pending)
    return s.result;
  const int granted = std::min(budget, s.remaining);
  budget -= granted;
  s.remaining -= granted;
  // Pinned binary32 0.2; preserving its value avoids i386 constant folding.
  constexpr long double factor = 0x1.99999ap-3L;
  int expansions = int(static_cast<long double>(granted) / factor);
  std::array<int, 50> ignored{};
  const bool reached = searchNavigationRegions(
      g, from, to, q, d, ignored, expansions, false, s.search, services);
  s.path.clear();
  std::set<int> seen;
  for (int node = s.search.best;;) {
    if (!seen.insert(node).second)
      throw std::invalid_argument("owner probe predecessor cycle");
    s.path.push_back(node);
    if (node == from)
      break;
    const auto at =
        std::find_if(s.search.records.begin(), s.search.records.end(),
                     [&](const auto &r) { return r.node == node; });
    if (at == s.search.records.end())
      throw std::invalid_argument("owner probe missing predecessor");
    node = at->predecessor;
  }
  const int refund =
      expansions > 0
          ? int(std::max(1.L, static_cast<long double>(expansions) * factor))
          : expansions;
  budget += refund;
  s.remaining += refund;
  if (budget > 0)
    s.result = reached && expansions > 0 ? OwnerTaskProbeResult::reached
                                         : OwnerTaskProbeResult::failed;
  return s.result;
}
int ownerTaskTravelTime(int cost, int speed) {
  const auto v = unsigned(cost);
  return speed > 0 ? int((((v >> 2) * 9u & 0xffffffu) << 7) / unsigned(speed))
                   : (cost ? INT_MAX : 0);
}
} // namespace mnm::reconstruction::combat
