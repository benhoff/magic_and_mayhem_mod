#pragma once
#include "native_providers.hpp"
namespace mnm::command_session {
// Explicit native observation policy: retain the last visible hostile snapshot;
// this is separate from the original owner's knowledge/task machinery.
void prepareNativeRegionalDanger(Continuation &, const NativeCreatureProfile &,
                                 model::RegionalDangerInputs);
void updateNativeRegionalDanger(Continuation &, const NativeCreatureProfile &);
} // namespace mnm::command_session
