#pragma once
#include "native_owner_planner.hpp"
#include "../../reconstruction/creature-ai/owner_task_inputs.hpp"
namespace mnm::command_session {
void updateNativeOwnerTaskInputs(Continuation &);
// A source slot is a retained actor identity, never inferred from owner alone.
void creditNativeOwnerDamage(Continuation &, unsigned victim, int source,
                              unsigned accounted);
void creditNativeOwnerMovement(Continuation &, unsigned actor, unsigned accounted);
} // namespace mnm::command_session
