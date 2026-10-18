#include "owner_group_task.hpp"
#include <algorithm>
#include <climits>
#include <cmath>
#include <set>
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
int norm(int x, int n) {
  x %= n;
  return x < 0 ? x + n : x;
}
int delta(int to, int from, int n) {
  const int v = norm(to - from, n);
  return v > n / 2 ? v - n : v;
}
int square(Cell a, Cell b, Extent d) {
  int x = delta(b.x, a.x, d.x), y = delta(b.y, a.y, d.y), z = b.z - a.z;
  return x * x + y * y + z * z / 4;
}
} // namespace
OwnerSourceGroupResult executeOwnerSourceGroupTask(
    OwnerGroupTask &task, const OwnerKnowledge &knowledge, int tick,
    unsigned owner, const std::vector<unsigned> &members,
    const AutonomousWorld &world, const std::vector<int> &raw,
    const std::vector<int> &net, int percent, const std::vector<int> &threat,
    int &budget) {
  validateOwnerMemoryGroups(task.groups, world.dimensions);
  const auto size = world.creatures.size();
  if (!size || size > 125 || members.empty() || members.size() > size ||
      raw.size() != members.size() || net.size() != members.size() ||
      threat.size() != size || owner >= 8 || task.root == 0 ||
      task.root >= task.groups.nodes.size() || percent < 0 || percent > 10000 ||
      budget < INT_MIN + 8)
    throw std::invalid_argument("owner source group task domain");
  std::set<unsigned> seen;
  for (unsigned i = 0; i < members.size(); ++i) {
    auto id = members[i];
    if (id >= size || !seen.insert(id).second || raw[i] < 0 ||
        raw[i] > 100000 || net[i] < -100000 || net[i] > raw[i])
      throw std::invalid_argument("owner source group member domain");
    const auto &a = world.creatures[id].ai;
    if (!a.active || !a.health || a.owner != int(owner))
      throw std::invalid_argument("owner source group inactive member");
  }
  for (const auto &b : world.creatures)
    if (b.ai.kind != 1 || b.ai.ranged || b.ai.health > 46340)
      throw std::invalid_argument("owner group task ordinary actor");
  budget -= 8;
  OwnerSourceGroupResult out;
  out.orders.resize(members.size());
  for (unsigned i = 0; i < members.size(); ++i)
    if (std::int64_t(percent) * (raw[i] - net[i]) / 100 < raw[i] - task.risk) {
      for (auto &order : out.orders)
        order.stop = true;
      return out;
    }
  auto &root = task.groups.nodes[task.root];
  auto &nodes = task.groups.nodes;
  std::vector<unsigned> leaves, stack{task.root};
  while (!stack.empty()) {
    auto id = stack.back();
    stack.pop_back();
    if (nodes[id].memoryKey >= 0)
      leaves.push_back(id);
    else
      stack.insert(stack.end(), nodes[id].children.rbegin(),
                   nodes[id].children.rend());
  }
  bool usable = false;
  int count = 0;
  std::vector<unsigned> targets;
  for (auto id : leaves) {
    auto &leaf = nodes[id];
    leaf.activity = 3;
    const auto &rows = knowledge.records[owner];
    auto r = std::find_if(rows.begin(), rows.end(), [&](const auto &v) {
      return v.memory.key == leaf.memoryKey;
    });
    if (r == rows.end() || r->markedTick != tick) {
      leaf.activity = 0;
      usable = true;
      continue;
    }
    if (r->memory.actor.slot < 0 ||
        unsigned(r->memory.actor.slot) >= world.creatures.size())
      throw std::invalid_argument("owner group memory slot");
    const auto &b = world.creatures[unsigned(r->memory.actor.slot)];
    if (!b.ai.health || b.ai.owner < 0 || b.ai.owner >= 8 ||
        world.relation[owner][unsigned(b.ai.owner)]) {
      leaf.activity = 0;
      continue;
    }
    usable = true;
    targets.push_back(unsigned(r->memory.actor.slot));
    ++count;
    root.position = {
        norm(root.position.x + (b.ai.position.x - root.position.x) / count,
             world.dimensions.x),
        norm(root.position.y + (b.ai.position.y - root.position.y) / count,
             world.dimensions.y),
        root.position.z + (b.ai.position.z - root.position.z) / count};
  }
  if (!usable)
    return out;
  unsigned activeChildren = 0;
  for (auto id : root.children)
    activeChildren += nodes[id].memoryKey < 0 || nodes[id].activity != 0;
  int minimum = INT_MAX, maximum = 0;
  for (auto id : members) {
    const int d = square(world.creatures[id].ai.position, root.position,
                         world.dimensions);
    minimum = std::min(minimum, d);
    maximum = std::max(maximum, d);
  }
  if (minimum < 10 && !activeChildren)
    return out;
  const int near = int(std::sqrt(static_cast<long double>(minimum)));
  const int far = int(std::sqrt(static_cast<long double>(maximum)));
  const int spread = far - near > int(members.size()) + 1
                         ? (far + near) * (far + near) / 4
                         : 0;
  out.continuing = true;
  std::vector<unsigned> assigned(size);
  for (unsigned i = 0; i < members.size(); ++i) {
    const auto &a = world.creatures[members[i]].ai;
    auto &order = out.orders[i];
    order.continuing = true;
    if (a.engaged || a.incoming || a.readiness || a.dirty932)
      continue;
    int best = INT_MIN;
    for (auto id : targets) {
      const auto &b = world.creatures[id];
      auto score =
          std::int32_t(((unsigned(threat[id]) - assigned[id]) * 1000u) /
                       (b.ai.health * b.ai.health));
      if (!a.physical && b.ai.physical)
        score /= 2;
      if (score > best) {
        best = score;
        order.target = int(id);
      }
    }
    if (square(a.position, root.position, world.dimensions) < spread) {
      order.stop = true;
      order.target = -1;
    } else if (best != INT_MIN) {
      order.kind = OwnerHostileOrderKind::target;
      assigned[unsigned(order.target)] += unsigned(threat[members[i]]);
    } else {
      order.kind = OwnerHostileOrderKind::move;
      order.position = root.position;
    }
  }
  return out;
}
OwnerGroupOrder executeOwnerGroupTask(
    OwnerGroupTask &task, const OwnerKnowledge &knowledge, int tick,
    unsigned owner, unsigned slot, const AutonomousWorld &world, int raw,
    int net, int percent, const std::array<int, 2> &threat, int &budget) {
  if (world.creatures.size() != 2 || slot >= 2)
    throw std::invalid_argument("owner singleton group task domain");
  validateOwnerMemoryGroups(task.groups, world.dimensions);
  if (owner >= 8 || task.root == 0 || task.root >= task.groups.nodes.size() ||
      raw < 0 || raw > 100000 || net < -100000 || net > raw || percent < 0 ||
      percent > 10000 || budget < INT_MIN + 8)
    throw std::invalid_argument("owner singleton group task domain");
  for (const auto &b : world.creatures)
    if (b.ai.kind != 1 || b.ai.ranged || b.ai.health > 46340)
      throw std::invalid_argument("owner group task ordinary actor");
  const auto &a = world.creatures[slot].ai;
  // Preserve the historical singleton admission of an inactive source.
  if (!a.active || !a.health || a.owner != int(owner)) {
    if (budget < INT_MIN + 8)
      throw std::invalid_argument("owner group task budget");
    budget -= 8;
    return {};
  }
  return executeOwnerSourceGroupTask(task, knowledge, tick, owner, {slot},
                                     world, {raw}, {net}, percent,
                                     {threat[0], threat[1]}, budget)
      .orders[0];
}
} // namespace mnm::reconstruction::combat
