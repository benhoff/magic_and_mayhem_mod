#pragma once
#include "native_geometry.hpp"
namespace mnm::command_session {
// 0x512800's independent, non-yielding request. The pinned build's immutable
// 0x5e174c budget is 250; it does not consume the scheduler's shared budget.
bool immediateNativeRoute(Continuation &, const NativeCreatureProfile &,
                          model::AutonomousCreature &, model::Cell, unsigned);
} // namespace mnm::command_session
