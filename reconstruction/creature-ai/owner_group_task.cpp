#include "owner_group_task.hpp"
#include <algorithm>
#include <climits>
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
OwnerGroupOrder executeOwnerGroupTask(
    OwnerGroupTask &task, const OwnerKnowledge &knowledge, int tick,
    unsigned owner, unsigned slot, const AutonomousWorld &world, int raw,
    int net, int percent, const std::array<int, 2> &threat, int &budget) {
  validateOwnerMemoryGroups(task.groups, world.dimensions);
  if (world.creatures.size() != 2 || slot >= 2 || owner >= 8 ||
      task.root == 0 || task.root >= task.groups.nodes.size() || raw < 0 ||
      net < -100000 || net > raw || percent < 0 || percent > 10000 ||
      raw > 100000 || budget < INT_MIN + 8)
    throw std::invalid_argument("owner group task domain");
  const auto &a = world.creatures.at(slot);
  for (const auto &b : world.creatures)
    if (b.ai.kind != 1 || b.ai.ranged || b.ai.health > 46340)
      throw std::invalid_argument("owner group task ordinary actor");
  budget -= 8;
  if (!a.ai.active || !a.ai.health || a.ai.owner != int(owner))
    return {};
  if (std::int64_t(percent) * (raw - net) / 100 < raw - task.risk)
    return {false, true, OwnerHostileOrderKind::none, -1, {}};
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
        norm(root.position.x +
                 delta(b.ai.position.x, root.position.x, world.dimensions.x) /
                     count,
             world.dimensions.x),
        norm(root.position.y +
                 delta(b.ai.position.y, root.position.y, world.dimensions.y) /
                     count,
             world.dimensions.y),
        root.position.z + (b.ai.position.z - root.position.z) / count};
  }
  if (!usable)
    return {};
  unsigned activeChildren = 0;
  for (auto id : root.children)
    activeChildren += nodes[id].memoryKey < 0 || nodes[id].activity != 0;
  if (square(a.ai.position, root.position, world.dimensions) < 10 &&
      !activeChildren)
    return {};
  OwnerGroupOrder out;
  out.continuing = true;
  if (a.ai.engaged || a.ai.incoming || a.ai.readiness || a.ai.dirty932)
    return out;
  int best = INT_MIN;
  for (auto id : targets) {
    const auto &b = world.creatures[id];
    auto score = std::int32_t((unsigned(threat[id]) * 1000u) /
                              (b.ai.health * b.ai.health));
    if (!a.ai.physical && b.ai.physical)
      score /= 2;
    if (score > best) {
      best = score;
      out.target = int(id);
    }
  }
  if (best != INT_MIN)
    out.kind = OwnerHostileOrderKind::target;
  else {
    out.kind = OwnerHostileOrderKind::move;
    out.position = root.position;
  }
  return out;
}
} // namespace mnm::reconstruction::combat
