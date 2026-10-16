#include "owner_planner_features.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <stdexcept>
namespace mnm::reconstruction::combat {
void validateOwnerPlannerActor(const OwnerPlannerActor &a) {
  for (int v : a.combatValue)
    if (v < 0 || v > 1000000)
      throw std::invalid_argument("owner task value domain");
  if (a.danger < 0 || a.danger > 1000000)
    throw std::invalid_argument("owner task danger domain");
  for (int v : {a.speed, a.power, a.bravery, a.defense5c5, a.factor5a9,
                a.factor5ad, a.motionB63})
    if (v < 0 || v > 10000)
      throw std::invalid_argument("owner task feature domain");
}
std::array<int, 67> hostileTaskFeatures(const OwnerPlannerActor &a,
                                        const OwnerPlannerActor &b, Extent d) {
  validateOwnerPlannerActor(a);
  validateOwnerPlannerActor(b);
  if (d.x < 1 || d.y < 1 || d.x > 1024 || d.y > 1024 || d.z < 1 || d.z > 1024)
    throw std::invalid_argument("owner task feature dimensions");
  std::array<int, 67> out{};
  for (unsigned i = 0; i < out.size(); ++i) {
    unsigned id = hostileTaskFeatureIds[i];
    // 26/84 measure within-group spread; singleton groups have zero spread.
    if (id == 26 || id == 84)
      continue;
    const auto &actor = id >= 59 ? b : a;
    if (id >= 59)
      id -= 58;
    const auto &v = actor.combatValue;
    switch (id) {
    case 3:
      out[i] = actor.speed;
      break;
    case 8:
      out[i] = actor.power;
      break;
    case 13:
      out[i] = actor.bravery;
      break;
    case 27:
      out[i] = actor.defense5c5;
      break;
    case 28:
      out[i] = std::max(v[0], v[1]);
      break;
    case 29:
      out[i] = std::max(v[2], v[3]);
      break;
    case 30:
      out[i] = v[1];
      break;
    case 31:
      out[i] = v[4];
      break;
    case 32:
      out[i] = actor.flag104 ? 100 : 0;
      break;
    case 33:
      out[i] = actor.physical ? 100 : 0;
      break;
    case 34:
      out[i] = actor.factor5a9 * actor.factor5ad;
      break;
    case 55:
      out[i] = actor.danger;
      break;
    case 58:
      out[i] = 100;
      break;
    default:
      break;
    }
  }
  return out;
}
std::vector<int> hostileTaskCandidateFeatures(const OwnerPlannerActor &a,
                                              const OwnerPlannerActor &b,
                                              Extent d, int radius,
                                              const std::array<int, 3> &path,
                                              bool assigned) {
  if (radius < 0 || radius > 1000000)
    throw std::invalid_argument("owner task uncertainty");
  for (int value : path)
    if (value < 0 || value > 1000000)
      throw std::invalid_argument("owner task path feature");
  const auto prefix = hostileTaskFeatures(a, b, d);
  std::vector<int> out(prefix.begin(), prefix.end());
  out.push_back(int(std::sqrt(static_cast<long double>(radius))));
  out.insert(out.end(), path.begin(), path.end());
  out.push_back(assigned);
  return out;
}
} // namespace mnm::reconstruction::combat
