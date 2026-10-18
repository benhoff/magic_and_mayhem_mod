#include "source_owned.hpp"
namespace mnm::reconstruction::combat {
void updateCreatureSourceOwned(
    unsigned actor, AutonomousWorld &world, SourceControllerState &state,
    SourceManager &manager, SourcePendingQueues &queues,
    const SourceControllerConfiguration &configuration,
    const std::function<bool(unsigned)> &allocate) {
  updateCreatureSourceController(
      actor, world, state, manager, configuration,
      {allocate,
       [&](unsigned root) { registerSourceRoot(world, state, queues, root); }});
}
} // namespace mnm::reconstruction::combat
