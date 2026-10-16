#include "owner_memory_groups.hpp"
#include <algorithm>
#include <climits>
#include <set>
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
void need(bool v, const char *s) {
  if (!v)
    throw std::invalid_argument(s);
}
int norm(int n, int extent) {
  n %= extent;
  return n < 0 ? n + extent : n;
}
int delta(int to, int from, int extent) {
  int v = norm(to - from, extent);
  return v > extent / 2 ? v - extent : v;
}
int square(Cell a, Cell b, Extent d) {
  int x = delta(b.x, a.x, d.x), y = delta(b.y, a.y, d.y), z = b.z - a.z;
  return x * x + y * y + z * z / 4;
}
} // namespace
void validateOwnerMemoryGroups(const OwnerMemoryGroups &s, Extent d) {
  need(d.x > 0 && d.x <= 1024 && d.y > 0 && d.y <= 1024 && d.z > 0 &&
           d.z <= 1024,
       "owner groups dimensions");
  if (s.nodes.empty())
    return;
  need(s.nodes.size() <= 251 && s.nodes[0].parent == -1 &&
           s.nodes[0].memoryKey == -1,
       "owner groups root");
  std::vector<unsigned> seen(s.nodes.size());
  std::vector<unsigned> stack{0};
  std::set<int> keys;
  while (!stack.empty()) {
    auto id = stack.back();
    stack.pop_back();
    need(id < s.nodes.size() && !seen[id]++, "owner groups topology");
    const auto &n = s.nodes[id];
    need(n.memoryKey >= -1 && n.activity <= 3 && n.children.size() <= 125,
         "owner groups activity/children");
    if (id) {
      need(n.position.x >= 0 && n.position.x < d.x && n.position.y >= 0 &&
               n.position.y < d.y && n.position.z >= 0 && n.position.z < d.z &&
               n.flag104 >= 0 && n.flag104 <= 1 && n.region >= -1 &&
               n.uncertaintySquared >= 0 && n.uncertaintySquared <= 1024000000,
           "owner groups node");
    }
    if (n.memoryKey >= 0)
      need(n.children.empty() && keys.insert(n.memoryKey).second,
           "owner groups leaf key");
    else
      need(id == 0 || !n.children.empty(), "owner groups empty group");
    for (auto child : n.children) {
      need(child < s.nodes.size() && s.nodes[child].parent == int(id),
           "owner groups parent");
      stack.push_back(child);
    }
  }
  need(std::all_of(seen.begin(), seen.end(), [](auto v) { return v == 1; }),
       "owner groups unreachable node");
}
void buildOwnerMemoryGroups(
    OwnerMemoryGroups &out, const OwnerKnowledge &knowledge, unsigned owner,
    int tick, bool stale, Extent d,
    const std::array<std::array<std::uint8_t, 8>, 8> &allied,
    const OwnerMemoryGroupServices &services) {
  validateOwnerMemoryGroups(out, d);
  need(owner < 8 && services.regional && services.line,
       "owner groups services");
  if (!out.nodes.empty())
    return;
  need(knowledge.records[owner].size() <= 125, "owner groups record bound");
  OwnerMemoryGroups s;
  s.nodes.emplace_back();
  for (const auto &r : knowledge.records[owner]) {
    const auto &a = r.memory.actor;
    need(a.owner >= 0 && a.owner < 8, "owner groups memory owner");
    if (!a.health || allied[owner][unsigned(a.owner)] ||
        (!stale && r.markedTick != tick))
      continue;
    need(r.memory.key >= 0 && r.planning.has_value(), "owner groups snapshot");
    const auto radius = (unsigned(tick) - unsigned(r.memory.observedTick)) *
                        unsigned(r.planning->motionB63) / 72u;
    need(radius <= 32000, "owner groups uncertainty bound");
    OwnerMemoryGroupNode leaf;
    leaf.memoryKey = r.memory.key;
    leaf.position = {norm(a.position.x, d.x), norm(a.position.y, d.y),
                     a.position.z};
    leaf.region = a.region;
    leaf.uncertaintySquared = int(radius * radius);
    leaf.flag104 = a.flag104;
    const unsigned l = unsigned(s.nodes.size()), g = l + 1;
    leaf.parent = int(g);
    s.nodes.push_back(leaf);
    auto group = leaf;
    group.memoryKey = -1;
    group.parent = 0;
    group.children = {l};
    s.nodes.push_back(group);
    s.nodes[0].children.push_back(g);
  }
  auto &roots = s.nodes[0].children;
  for (;;) {
    int best = INT_MAX, middle = -1;
    unsigned first = 0, second = 0;
    for (unsigned i = 0; i < roots.size(); ++i)
      for (unsigned j = i + 1; j < roots.size(); ++j) {
        const auto &a = s.nodes[roots[i]], &b = s.nodes[roots[j]];
        const int metric = square(a.position, b.position, d) +
                           (a.uncertaintySquared + b.uncertaintySquared) / 2;
        if (metric >= best || a.flag104 != b.flag104)
          continue;
        int center = -1;
        const bool reachable =
            a.region < 0 || b.region < 0
                ? services.line(a.position, b.position)
                : services.regional(a.region, b.region, center);
        if (reachable) {
          best = metric;
          first = i;
          second = j;
          middle = center;
        }
      }
    if (best > 100)
      break;
    auto &a = s.nodes[roots[first]];
    auto &b = s.nodes[roots[second]];
    a.children.push_back(roots[second]);
    b.parent = int(roots[first]);
    a.position = {
        norm(a.position.x + delta(b.position.x, a.position.x, d.x) / 2, d.x),
        norm(a.position.y + delta(b.position.y, a.position.y, d.y) / 2, d.y),
        a.position.z + (b.position.z - a.position.z) / 2};
    a.region = middle;
    a.uncertaintySquared = best;
    roots.erase(roots.begin() + second);
  }
  validateOwnerMemoryGroups(s, d);
  out = std::move(s);
}
std::vector<int> ownerGroupKeys(const OwnerMemoryGroups &s, unsigned root) {
  std::vector<int> result;
  std::vector<unsigned> stack{root};
  unsigned visits = 0;
  while (!stack.empty()) {
    need(++visits <= s.nodes.size(), "owner group key traversal");
    auto id = stack.back();
    stack.pop_back();
    const auto &n = s.nodes.at(id);
    if (n.memoryKey >= 0)
      result.push_back(n.memoryKey);
    else
      stack.insert(stack.end(), n.children.rbegin(), n.children.rend());
  }
  return result;
}
} // namespace mnm::reconstruction::combat
