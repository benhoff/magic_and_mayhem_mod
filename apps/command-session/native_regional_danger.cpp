#include "native_regional_danger.hpp"
#include <algorithm>
#include <cstring>
#include <stdexcept>
namespace mnm::command_session {
namespace {
void observe(Continuation &c, const NativeCreatureProfile &p) {
  if (!c.native.regionalDanger)
    return;
  auto &d = *c.native.regionalDanger;
  auto &in = d.inputs;
  in.tick = int(c.native.tick);
  in.dimensions = c.world.dimensions;
  in.allied = c.world.relation;
  std::vector<model::RegionalDangerActor> live;
  for (const auto &a : c.world.creatures) {
    const auto old =
        std::find_if(in.actors.begin(), in.actors.end(),
                     [&](auto &v) { return v.slot == int(a.ai.slot); });
    model::RegionalDangerActor out =
        old == in.actors.end() ? model::RegionalDangerActor{} : *old;
    out.slot = int(a.ai.slot);
    out.owner = a.ai.owner;
    out.region = a.region;
    out.position = a.ai.position;
    out.type = a.ai.type;
    out.typeClass = a.ai.kind;
    out.height = a.height;
    out.width = a.width;
    out.movementKind = a.movementKind;
    out.health = a.ai.health;
    out.active = a.ai.active;
    out.threat = c.execution.at(a.ai.slot).combatValue[5];
    out.inventoryResource = 0;
    for (const auto &entry : a.inventory)
      if (entry.kind == 19 && entry.available)
        out.inventoryResource += entry.magnitude * 256u;
    out.animationKind = a.animationKind;
    out.ranged = a.ai.ranged != 0;
    out.attackRange = int(a.ai.ranged);
    out.flying = a.ai.flying != 0;
    out.flag104 = a.flag104 != 0;
    out.status114 = c.maintenance.at(a.ai.slot).status114 != 0;
    out.block7c = a.block7c != 0;
    out.block723 = a.block723 != 0;
    out.behavior14 = a.ai.behavior == 14;
    unsigned age = unsigned(p.types.at(a.ai.type).routeCostFactor);
    if (c.maintenance.at(a.ai.slot).status778)
      age *= 2;
    std::memcpy(&out.aging, &age, 4);
    if (c.maintenance.at(a.ai.slot).status77c)
      out.aging /= 2;
    live.push_back(out);
  }
  in.actors = std::move(live);
  d.state.actorSupport.resize(in.actors.size());
  for (unsigned owner = 0; owner < 8; ++owner) {
    auto &memories = in.memories[owner];
    for (const auto &a : in.actors) {
      const auto &actor = c.world.creatures.at(unsigned(a.slot));
      if (!actor.ai.visible[owner])
        continue;
      auto at = std::lower_bound(memories.begin(), memories.end(), a.slot,
                                 [](auto &v, int key) { return v.key < key; });
      if (a.owner == int(owner) || in.allied[owner][unsigned(a.owner)]) {
        if (at != memories.end() && at->key == a.slot)
          memories.erase(at);
        continue;
      }
      model::RegionalDangerMemory memory{a.slot, in.tick, a};
      if (at != memories.end() && at->key == a.slot)
        *at = memory;
      else
        memories.insert(at, memory);
    }
  }
}
} // namespace
void prepareNativeRegionalDanger(Continuation &c,
                                 const NativeCreatureProfile &p,
                                 model::RegionalDangerInputs inputs) {
  if (!c.native.present || !c.native.route.useRegions ||
      c.native.regions.empty() || c.world.creatures.size() > 125)
    throw std::invalid_argument(
        "native danger requires an admitted regional world");
  auto staged = c;
  inputs.dimensions = c.world.dimensions;
  inputs.tick = int(c.native.tick);
  staged.native.regionalDanger.emplace();
  auto &v = *staged.native.regionalDanger;
  v.inputs = std::move(inputs);
  const auto count = c.native.regions.size();
  v.state.pendingDanger.resize(count);
  v.state.pendingDeduction.resize(count);
  v.state.pendingBase.resize(count);
  v.state.actorSupport.resize(v.inputs.actors.size());
  observe(staged, p);
  validate(staged);
  c = std::move(staged);
}

void updateNativeRegionalDanger(Continuation &c,
                                const NativeCreatureProfile &p) {
  if (!c.native.regionalDanger)
    return;
  observe(c, p);
  auto &d = *c.native.regionalDanger;
  auto &in = d.inputs;
  model::updateRegionalDanger(c.native.regions, d.state, in);
  model::updateRegionalDeduction(c.native.regions, d.state, in);
}
} // namespace mnm::command_session
