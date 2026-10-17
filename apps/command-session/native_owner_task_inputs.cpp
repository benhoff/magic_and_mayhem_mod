#include "native_owner_task_inputs.hpp"
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
  const auto &actor = c.world.creatures.at(s.actor);
  const auto &danger = *c.native.regionalDanger;
  const auto owner = s.owner;
  const int commander = danger.inputs.commander.at(owner);
  r->policy.ownerAvailable = danger.inputs.enabled.at(owner) && commander >= 0;
  r->policy.ownerNumerator = r->policy.ownerAvailable ? danger.state.dangerTotals.at(owner) : 0;
  r->policy.ownerDenominator = r->policy.ownerAvailable ? danger.state.deductionTotals.at(owner) : 0;
  model::validateOwnerReplacementPolicy(r->policy);
  auto &a = r->admission;
  a.enabled = s.percent != 0;
  a.brain = 0;
  a.parent = false;
  a.allAssigned = actor.region != -1;
  std::vector<unsigned> occupants;
  const auto d = c.world.dimensions;
  for (auto p : danger.inputs.observationPoints) {
    if (p.x < 0 || p.y < 0 || p.z < 0 || p.x >= d.x || p.y >= d.y || p.z >= d.z)
      throw std::invalid_argument("owner special cell out of bounds");
    const auto token = c.native.cells.at((std::size_t(p.z)*d.y+p.y)*d.x+p.x).occupant;
    if (token != 0xffff)
      occupants.push_back(token - 1u);
  }
  a.allOffSpecialCells = !model::ownerTaskOnSpecialCell(s.actor, occupants);
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
        leader.ai.maximumHealth, r->policy.ownerNumerator, r->policy.ownerDenominator);
  }
}
void creditNativeOwnerDamage(Continuation &c, unsigned victim, int source,
                              unsigned accounted) {
  auto *r = inputs(c);
  if (!r || !r->previous)
    return;
  const auto slot = c.native.ownerPlanner->actor;
  model::addOwnerTaskDamage(source == int(slot) ? &*r->previous : nullptr,
                            victim == slot ? &*r->previous : nullptr,
                            c.world.creatures.at(victim).ai.kind, accounted);
}
void creditNativeOwnerMovement(Continuation &c, unsigned actor, unsigned accounted) {
  auto *r = inputs(c);
  if (r && r->previous && actor == c.native.ownerPlanner->actor)
    model::addOwnerTaskCost(*r->previous, accounted);
}
} // namespace mnm::command_session
