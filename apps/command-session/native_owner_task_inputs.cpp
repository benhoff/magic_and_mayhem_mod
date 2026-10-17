#include "native_owner_task_inputs.hpp"
#include "native_owner_sources.hpp"
#include <algorithm>
#include <stdexcept>
namespace mnm::command_session {
namespace {
NativeOwnerReplanning *inputs(Continuation &c) {
  if (!c.native.ownerPlanner || !c.native.ownerPlanner->grouped ||
      !c.native.ownerPlanner->grouped->replanning ||
      !c.native.ownerPlanner->grouped->replanning->eventInputs)
    return nullptr;
  return &*c.native.ownerPlanner->grouped->replanning;
}
} // namespace
void updateNativeOwnerTaskInputs(Continuation &c) {
  auto *r = inputs(c);
  if (!r)
    return;
  const auto &s = *c.native.ownerPlanner;
  const auto members = nativeOwnerSourceActors(s);
  const auto &danger = *c.native.regionalDanger;
  const auto owner = s.owner;
  const int commander = danger.inputs.commander.at(owner);
  r->policy.ownerAvailable = danger.inputs.enabled.at(owner) && commander >= 0;
  r->policy.ownerNumerator =
      r->policy.ownerAvailable ? danger.state.dangerTotals.at(owner) : 0;
  r->policy.ownerDenominator =
      r->policy.ownerAvailable ? danger.state.deductionTotals.at(owner) : 0;
  model::validateOwnerReplacementPolicy(r->policy);
  auto &a = r->admission;
  a.enabled = s.percent != 0;
  a.brain = 0;
  a.parent = false;
  a.allAssigned =
      std::all_of(members.begin(), members.end(), [&](unsigned slot) {
        return c.world.creatures.at(slot).region != -1;
      });
  if (members.size() > 1) {
    const auto region = c.world.creatures.at(members.front()).region;
    a.allAssigned &=
        std::all_of(members.begin(), members.end(), [&](unsigned slot) {
          return c.world.creatures.at(slot).region == region;
        });
  }
  std::vector<unsigned> occupants;
  const auto d = c.world.dimensions;
  for (auto p : danger.inputs.observationPoints) {
    if (p.x < 0 || p.y < 0 || p.z < 0 || p.x >= d.x || p.y >= d.y || p.z >= d.z)
      throw std::invalid_argument("owner special cell out of bounds");
    const auto token =
        c.native.cells.at((std::size_t(p.z) * d.y + p.y) * d.x + p.x).occupant;
    if (token != 0xffff)
      occupants.push_back(token - 1u);
  }
  a.allOffSpecialCells =
      std::none_of(members.begin(), members.end(), [&](unsigned slot) {
        return model::ownerTaskOnSpecialCell(slot, occupants);
      });
  // Current native planner admits ordinary type17 only. Its source has no
  // singleton wizard alternative; a separate logical commander mapping is
  // supplied by the native danger service, not a recovered wizard factory.
  a.singletonWizardReady = false;
  a.ownerAvailable = r->policy.ownerAvailable;
  a.ownerDistressed = false;
  if (a.ownerAvailable) {
    const auto &leader = c.world.creatures.at(unsigned(commander));
    a.ownerDistressed = model::ownerTaskDistressed(
        leader.ai.kind, leader.block7c != 0, leader.ai.health,
        leader.ai.maximumHealth, r->policy.ownerNumerator,
        r->policy.ownerDenominator);
  }
}
void creditNativeOwnerDamage(Continuation &c, unsigned victim, int source,
                             unsigned accounted) {
  auto credit = [&](NativeOwnerPlanner &p) {
    if (!p.grouped || !p.grouped->replanning ||
        !p.grouped->replanning->eventInputs || !p.grouped->replanning->previous)
      return;
    const auto members = nativeOwnerSourceActors(p);
    auto has = [&](int slot) {
      return slot >= 0 && std::find(members.begin(), members.end(),
                                    unsigned(slot)) != members.end();
    };
    auto &previous = *p.grouped->replanning->previous;
    model::addOwnerTaskDamage(has(source) ? &previous : nullptr,
                              has(int(victim)) ? &previous : nullptr,
                              c.world.creatures.at(victim).ai.kind, accounted);
  };
  if (c.native.ownerSources) {
    for (auto &row : c.native.ownerSources->planners)
      if (row.planner)
        credit(*row.planner);
  } else if (c.native.ownerPlanner)
    credit(*c.native.ownerPlanner);
}
void creditNativeOwnerMovement(Continuation &c, unsigned actor,
                               unsigned accounted) {
  auto credit = [&](NativeOwnerPlanner &p) {
    const auto members = nativeOwnerSourceActors(p);
    if (p.grouped && p.grouped->replanning &&
        p.grouped->replanning->eventInputs && p.grouped->replanning->previous &&
        std::find(members.begin(), members.end(), actor) != members.end())
      model::addOwnerTaskCost(*p.grouped->replanning->previous, accounted);
  };
  if (c.native.ownerSources) {
    for (auto &row : c.native.ownerSources->planners)
      if (row.planner)
        credit(*row.planner);
  } else if (c.native.ownerPlanner)
    credit(*c.native.ownerPlanner);
}
} // namespace mnm::command_session
