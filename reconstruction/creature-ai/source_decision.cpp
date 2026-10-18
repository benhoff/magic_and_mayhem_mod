#include "source_decision.hpp"
#include <algorithm>
#include <cmath>
#include <set>
#include <stdexcept>
namespace mnm::reconstruction::combat {
void validateOwnerSourceDecision(const OwnerSourceDecision &s) {
  if (s.stage > 5 ||
      (s.winner != -1 && s.winner != 0 && s.winner != 2 && s.winner != 4) ||
      !std::isfinite(s.bias) || s.bias < -2 || s.bias > 0 ||
      !std::isfinite(s.winningBias) || s.winningBias < -2 || s.winningBias > 0)
    throw std::invalid_argument("source decision state");
}
std::vector<unsigned>
ownerSourceSplitMembers(const OwnerSourceDecisionInputs &i) {
  if (i.members.size() < 2 || i.members.size() > 3 ||
      i.speeds.size() != i.members.size())
    throw std::invalid_argument("source decision partition domain");
  // 423590 inserts speed->traversal-index into a unique-key tree. Equal
  // speeds retain the first inserted actor, rather than another rank entry.
  std::vector<unsigned> order;
  std::set<int> keys;
  for (unsigned j = 0; j < i.members.size(); ++j)
    if (keys.insert(i.speeds[j]).second)
      order.push_back(j);
  std::stable_sort(order.begin(), order.end(), [&](unsigned a, unsigned b) {
    return i.speeds[a] > i.speeds[b];
  });
  order.resize((order.size() + 1) / 2);
  std::sort(order.begin(), order.end());
  std::vector<unsigned> out;
  for (auto j : order)
    out.push_back(i.members[j]);
  return out;
}
void validateOwnerSourceDecisionInputs(const OwnerSourceDecisionInputs &i) {
  std::set<unsigned> unique(i.members.begin(), i.members.end());
  unique.insert(i.otherMembers.begin(), i.otherMembers.end());
  if (i.members.empty() || i.members.size() + i.otherMembers.size() > 3 ||
      i.members.size() != i.speeds.size() ||
      unique.size() != i.members.size() + i.otherMembers.size() ||
      (i.otherTask && i.otherMembers.empty()))
    throw std::invalid_argument("source decision input domain");
  if (i.otherTask)
    validateOwnerPreviousTask(*i.otherTask);
}
Cell ownerSourceCenter(const std::vector<Cell> &points, Extent d) {
  if (points.empty() || points.size() > 3 || d.x < 1 || d.y < 1 || d.z < 1 ||
      d.x > 1024 || d.y > 1024 || d.z > 1024)
    throw std::invalid_argument("source center domain");
  Cell out{};
  int count = 0;
  for (auto p : points) {
    if (p.x < 0 || p.x >= d.x || p.y < 0 || p.y >= d.y || p.z < 0 || p.z >= d.z)
      throw std::invalid_argument("source center position");
    ++count;
    out.x += (p.x - out.x) / count;
    out.y += (p.y - out.y) / count;
    out.z += (p.z - out.z) / count;
  }
  return out;
}
int ownerSourceDistance(const std::vector<Cell> &a, const std::vector<Cell> &b,
                        Extent d) {
  const auto x = ownerSourceCenter(a, d), y = ownerSourceCenter(b, d);
  int dx = std::abs(x.x - y.x), dy = std::abs(x.y - y.y), dz = x.z - y.z;
  dx = std::min(dx, d.x - dx);
  dy = std::min(dy, d.y - dy);
  return dx * dx + dy * dy + dz * dz;
}
bool replaceOwnerSourceTask(float candidate, const OwnerPreviousTask *old,
                            int tick, const OwnerReplacementPolicy &policy) {
  if (candidate <= 1)
    return replaceOwnerTask(candidate, old, tick, policy);
  validateOwnerReplacementPolicy(policy);
  if (!std::isfinite(candidate) || candidate > 3 || tick < 0)
    throw std::invalid_argument("source commit value");
  if (!old)
    return true;
  validateOwnerPreviousTask(*old);
  if (tick < old->started)
    throw std::invalid_argument("source commit time");
  if (old->completed)
    return true;
  if (old->weighted)
    return false;
  const long double denominator =
      1.L - static_cast<long double>(candidate) * old->value;
  if (denominator > 0 &&
      (static_cast<long double>(candidate) - old->value) / denominator >
          policy.improvement)
    return true;
  if (old->value >= 1)
    return false;
  const double age = double(static_cast<long double>(tick - old->started) /
                            (old->expires - old->started));
  return std::pow(double(ownerTaskProgress(*old, policy)), age) < policy.expiry;
}
std::optional<unsigned>
chooseOwnerMergeCandidate(std::vector<OwnerMergeCandidate> candidates,
                          RandomState &random) {
  if (candidates.size() > 128)
    throw std::invalid_argument("source merge candidate bound");
  std::set<unsigned> seen;
  for (const auto &v : candidates)
    if (v.distance < 0 || !seen.insert(v.source).second)
      throw std::invalid_argument("source merge candidate domain");
  if (candidates.empty())
    return {};
  std::stable_sort(
      candidates.begin(), candidates.end(),
      [](const auto &a, const auto &b) { return a.distance < b.distance; });
  candidates.erase(std::unique(candidates.begin(), candidates.end(),
                               [](const auto &a, const auto &b) {
                                 return a.distance == b.distance;
                               }),
                   candidates.end());
  candidates.resize(std::min<std::size_t>(2, candidates.size()));
  return candidates[(nextRandom(random) & 0x7fffffffu) % candidates.size()]
      .source;
}
bool advanceOwnerSourceDecision(OwnerSourceDecision &s, OwnerTaskSelection &q,
                                const OwnerSourceDecisionInputs &i, int &budget,
                                RandomState &random,
                                const OwnerSourceDecisionServices &services) {
  validateOwnerSourceDecision(s);
  validateOwnerSourceDecisionInputs(i);
  if (!services.select || budget < 0)
    throw std::invalid_argument("source decision services/budget");
  if (s.reset) {
    s = {};
    s.reset = false;
    s.stage = i.splitFirst && i.members.size() >= 2 ? 3 : 0;
    q = {};
    q.weighted = i.weighted;
  }
  while (s.stage < 5) {
    if (s.stage == 1) {
      if (services.chooseOther)
        services.chooseOther(random);
      if (i.otherMembers.empty()) {
        s.stage = 5;
        continue;
      }
      // Partial shuffle consumes one draw even for the sole merge candidate.
      if (!services.chooseOther)
        (void)nextRandom(random);
      s.bias = -(i.otherTask ? i.otherTask->value + 1.f : 0.f);
      s.stage = 2;
    }
    if (s.stage == 3) {
      if (i.members.size() < 2) {
        s.stage = 5;
        continue;
      }
      s.bias = 0;
      s.stage = 4;
    }
    auto members = i.members;
    const OwnerPreviousTask *old = nullptr;
    if (s.stage == 2) {
      members.insert(members.end(), i.otherMembers.begin(),
                     i.otherMembers.end());
      old = i.otherTask ? &*i.otherTask : nullptr;
    } else if (s.stage == 4)
      members = ownerSourceSplitMembers(i);
    const bool changed =
        services.select(members, true, s.bias, old, s.stage == 2, budget);
    if (changed) {
      s.winner = int(s.stage);
      s.winningBias = s.bias;
    }
    // 423590 tests budget before moving its stage cursor, even if the
    // evaluator sweep just completed on the last unit.
    if (budget < 1)
      return false;
    if (!q.reset)
      throw std::invalid_argument("source evaluator yielded with spare budget");
    if (s.stage == 0)
      s.stage = 1;
    else if (s.stage == 2)
      s.stage = 5;
    else
      s.stage = 5;
  }
  return true;
}
} // namespace mnm::reconstruction::combat
