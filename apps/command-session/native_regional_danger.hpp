#pragma once
#include "native_providers.hpp"
namespace mnm::command_session {
// Explicit native observation policy: retain the last visible hostile snapshot;
// this is separate from the original owner's knowledge/task machinery.
void prepareNativeRegionalDanger(Continuation &, const NativeCreatureProfile &,
                                 model::RegionalDangerInputs);
void updateNativeRegionalDanger(Continuation &, const NativeCreatureProfile &);
// Opt into recovered 4181d0 creature knowledge. Task/notification lists absent;
// frame admission and visibility inputs remain explicit native boundaries.
void prepareNativeOwnerKnowledge(Continuation &);
} // namespace mnm::command_session
