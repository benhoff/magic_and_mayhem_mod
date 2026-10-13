#include "contribution_owned.hpp"
#include <stdexcept>
namespace mnm::reconstruction::combat {
void contributeCreatureResourceOwned(
    AutonomousCreature &source, AutonomousWorld &world,
    std::vector<CreatureExecution> &execution, ResourceContributionState &state,
    const ContributionOwnership &ownership, unsigned grant, int localOwner,
    const ContributionDisplayServices &display) {
  if (world.creatures.size() > UINT32_MAX)
    throw std::invalid_argument("owned contribution actor capacity");
  ResourceContributionServices services;
  services.subject = [&](int owner) {
    std::array<AssociatedOwnerRecord, 8> records;
    for (unsigned i = 0; i < 8; ++i)
      records[i] = {ownership.ownerEnabled[i], unsigned(state.controllers[i])};
    return associatedCreature(owner, records, unsigned(world.creatures.size()));
  };
  services.change = [&](auto &actor, unsigned delta) {
    const unsigned slot = actor.ai.slot;
    CreatureResourceState resource;
    resource.current = state.resource.at(slot);
    resource.maximum = ownership.maximum.at(slot);
    CreatureResourceServices callbacks;
    auto invoke = [&](const auto &callback, unsigned quantity) {
      state.resource.at(slot) = resource.current;
      callback(actor, quantity);
      resource.current = state.resource.at(slot);
    };
    callbacks.primaryDisplay = [&](unsigned quantity) {
      invoke(display.primary, quantity);
    };
    callbacks.secondaryDisplay = [&](unsigned quantity) {
      invoke(display.secondary, quantity);
    };
    changeCreatureResource(actor, resource, execution.at(slot).dirtyValue,
                           delta, localOwner, callbacks);
    state.resource.at(slot) = resource.current;
  };
  services.account = [&](unsigned type, float amount, float delta) {
    return contributionAccount(ownership.profiles, type, 0, 0, amount, delta,
                               0);
  };
  contributeCreatureResource(source, world, state, grant, services);
}
} // namespace mnm::reconstruction::combat
