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
void validateNativeOwnerGroupPlanner(const Continuation &,
                                     const NativeOwnerPlanner * = nullptr);
struct NativeOwnerProposalEvaluation {
  int *budget = nullptr;
  float bias = 0;
  const model::OwnerPreviousTask *old = nullptr;
  bool root = true, rejectOne = false, changed = false;
};
void updateNativeOwnerGroupPlanner(Continuation &,
                                   const NativeCreatureProfile &,
                                   model::OwnedCreatureCommands &,
                                   NativeOwnerProposalEvaluation * = nullptr,
                                   bool executeOnly = false);
void updateNativeOwnerPlanner(Continuation &, const NativeCreatureProfile &,
                              model::OwnedCreatureCommands &);
} // namespace mnm::command_session
