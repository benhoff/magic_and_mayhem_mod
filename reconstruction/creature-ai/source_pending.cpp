#include "source_pending.hpp"
#include "ownership_helpers.hpp"
#include <algorithm>
namespace mnm::reconstruction::combat {
namespace {
void frontOnce(std::deque<unsigned> &queue, unsigned root) {
  if (std::find(queue.begin(), queue.end(), root) == queue.end())
    queue.push_front(root);
}
} // namespace
void registerSourceRoot(AutonomousWorld &world, SourceControllerState &state,
                        SourcePendingQueues &queues, unsigned root) {
  if (!state.information.at(root).recipient)
    frontOnce(queues.general, root);
  unsigned cursor = beginSourceTraversal(state.graph, root);
  bool wizard = false;
  while (auto id = nextSourceActor(state.graph, root, cursor, state.mode)) {
    const auto &actor = world.creatures.at(*id);
    if (isWizardKind(actor.ai.kind) && !actor.block7c) {
      wizard = true;
      while (nextSourceActor(state.graph, root, cursor, state.mode)) {
      }
      break;
    }
  }
  frontOnce(wizard ? queues.wizard : queues.other, root);
}
} // namespace mnm::reconstruction::combat
