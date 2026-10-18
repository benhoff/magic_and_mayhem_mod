#include "transfer_idle_owned.hpp"
#include <stdexcept>
namespace mnm::reconstruction::combat {
void transferCreatureOwnerIdleOwned(
    unsigned actor, AutonomousWorld &world,
    std::vector<CreatureExecution> &execution,
    OwnershipBehaviorBindings &bindings, CreatureTransferState &state,
    CreatureTransferWorld &selection, const OwnerVisionSharing &sharing,
    CreatureOwnerVisibility &visibility, SourceGraph &graph,
    const std::array<AssociatedOwnerRecord, 8> &owners,
    std::vector<CreatureControlLimit> &limits,
    const OwnerCountServices &countUI, OwnerTargetRepairState &repair,
    const OwnerTargetRepairServices &repairServices,
    const std::function<ExecutionServices(unsigned)> &perActor,
    const std::function<BehaviorExecutionServices(unsigned)> &perIdle,
    int owner, const CreatureTransferServices &lower) {
  auto idle = [&](auto &a) { idleBehavior(a, perIdle(a.ai.slot)); };
  auto resetConsequences = [&](unsigned slot) {
    auto result = perActor(slot);
    result.behavior = [&](auto &a, unsigned kind) {
      if (kind != 1)
        throw std::invalid_argument("ownership reset behavior selector");
      bindings.at(a.ai.slot) = OwnershipBehaviorBinding::idle;
      idle(a);
    };
    return result;
  };
  auto repairConsequences = repairServices;
  repairConsequences.behavior = [&](auto &a) {
    switch (bindings.at(a.ai.slot)) {
    case OwnershipBehaviorBinding::idle:
      idle(a);
      break;
    case OwnershipBehaviorBinding::supplied:
      repairServices.behavior(a);
      break;
    default:
      throw std::invalid_argument("ownership behavior binding");
    }
  };
  transferCreatureOwnerResetOwned(actor, world, execution, state, selection,
                                  sharing, visibility, graph, owners, limits,
                                  countUI, repair, repairConsequences,
                                  resetConsequences, owner, lower);
}
} // namespace mnm::reconstruction::combat
