#pragma once
#include "native_providers.hpp"
namespace mnm::command_session {
model::OwnerPlannerActor nativeOwnerPlannerActor(const Continuation &,
                                                 const NativeCreatureProfile &,
                                                 unsigned slot);
void prepareNativeOwnerPlanner(Continuation &, model::BrainInferenceModel,
                               unsigned owner, unsigned actor, unsigned seed,
                               std::array<float, 3> weights = {{1, 0, 1}},
                               int percent = 100);
void validateNativeOwnerPlanner(const Continuation &);
void updateNativeOwnerPlanner(Continuation &, const NativeCreatureProfile &,
                              model::OwnedCreatureCommands &);
} // namespace mnm::command_session
