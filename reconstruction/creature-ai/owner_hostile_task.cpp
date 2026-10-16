#include "owner_hostile_task.hpp"
#include <algorithm>
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
int distanceSquared(Cell a, Cell b, Extent d) {
  auto delta = [](int a, int b, int n) {
    auto v = (std::int64_t(a) - b) % n;
    if (v > n / 2)
      v -= n;
    if (v < -n / 2)
      v += n;
    return int(v);
  };
  const int x = delta(a.x, b.x, d.x), y = delta(a.y, b.y, d.y), z = a.z - b.z;
  return x * x + y * y + z * z;
}
} // namespace
OwnerHostileOrder
executeOwnerHostileTask(OwnerHostileTask &task, const OwnerKnowledge &knowledge,
                        int tick, unsigned owner, unsigned slot,
                        const AutonomousWorld &world, int &budget) {
  if (owner >= 8 || slot >= world.creatures.size() || task.memoryKey < 0 ||
      world.dimensions.x < 1 || world.dimensions.y < 1 ||
      world.dimensions.x > 1024 || world.dimensions.y > 1024 ||
      world.dimensions.z < 1 || world.dimensions.z > 1024 ||
      budget < -2147483640)
    throw std::invalid_argument("owner hostile task roots");
  auto cell = [&](Cell p) {
    if (p.x < 0 || p.x >= world.dimensions.x || p.y < 0 ||
        p.y >= world.dimensions.y || p.z < 0 || p.z >= world.dimensions.z)
      throw std::invalid_argument("owner hostile task cell");
  };
  cell(task.position);
  for (const auto &actor : world.creatures)
    cell(actor.ai.position);
  budget -= 8;
  const auto &a = world.creatures.at(slot);
  if (!a.ai.active || !a.ai.health || a.ai.owner != int(owner))
    return {};
  const auto &records = knowledge.records[owner];
  const auto r =
      std::find_if(records.begin(), records.end(), [&](const auto &v) {
        return v.memory.key == task.memoryKey;
      });
  const AutonomousCreature *target = nullptr;
  if (r != records.end() && r->markedTick == tick) {
    const auto i = unsigned(r->memory.actor.slot);
    if (i >= world.creatures.size())
      throw std::invalid_argument("owner hostile task memory slot");
    const auto &b = world.creatures[i];
    if (!b.ai.health || b.ai.owner < 0 || b.ai.owner >= 8 ||
        world.relation[owner][unsigned(b.ai.owner)])
      return {};
    target = &b;
    task.position = b.ai.position;
  }
  if (!target &&
      distanceSquared(a.ai.position, task.position, world.dimensions) < 10)
    return {};
  OwnerHostileOrder result;
  result.continuing = true;
  if (a.ai.engaged || a.ai.incoming || a.ai.readiness || a.ai.dirty932)
    return result;
  if (target) {
    result.kind = OwnerHostileOrderKind::target;
    result.target = int(target->ai.slot);
  } else {
    result.kind = OwnerHostileOrderKind::move;
    result.position = task.position;
  }
  return result;
}
} // namespace mnm::reconstruction::combat
