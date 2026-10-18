#include "owner_group_features.hpp"
#include <algorithm>
#include <climits>
#include <stdexcept>
namespace mnm::reconstruction::combat {
std::array<int, 67>
hostileTaskGroupFeatures(const std::vector<OwnerPlannerActor> &a,
                         const std::vector<OwnerPlannerActor> &b, Extent d) {
  if (a.empty() || b.empty() || a.size() > 125 || b.size() > 125)
    throw std::invalid_argument("task feature group extent");
  std::array<std::int64_t, 67> sum{};
  for (unsigned side = 0; side < 2; ++side) {
    const auto &group = side ? b : a;
    for (const auto &actor : group) {
      auto values = hostileTaskFeatures(actor, actor, d);
      for (unsigned i = 0; i < 67; ++i)
        if ((hostileTaskFeatureIds[i] >= 59) == bool(side))
          sum[i] += values[i];
    }
  }
  std::array<int, 67> result{};
  for (unsigned i = 0; i < 67; ++i) {
    auto id = hostileTaskFeatureIds[i];
    const auto &group = id >= 59 ? b : a;
    if (id >= 59)
      id -= 58;
    if (id == 26) {
      auto delta = [](int a, int b, int n) {
        auto v = std::abs(a - b) % n;
        return std::min(v, n - v);
      };
      for (const auto &a : group)
        for (const auto &b : group) {
          const int x = delta(a.position.x, b.position.x, d.x),
                    y = delta(a.position.y, b.position.y, d.y),
                    z = a.position.z - b.position.z;
          sum[i] = std::max(sum[i], std::int64_t(x * x + y * y + z * z / 4));
        }
    } else if (id < 28 || id > 31)
      sum[i] /= int(group.size());
    if (sum[i] < INT_MIN || sum[i] > INT_MAX)
      throw std::invalid_argument("task feature group arithmetic");
    result[i] = int(sum[i]);
  }
  return result;
}
} // namespace mnm::reconstruction::combat
