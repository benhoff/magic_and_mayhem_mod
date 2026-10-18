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
void validateNativeOwnerPlanner(const Continuation &, bool advancing = false);
void validateNativeOwnerGroupPlanner(const Continuation &,
                                     const NativeOwnerPlanner * = nullptr);
struct NativeOwnerProposalEvaluation {
  int *budget = nullptr;
  float bias = 0;
  const model::OwnerPreviousTask *old = nullptr;
  bool root = true, rejectOne = false, changed = false;
};
struct NativeOwnerTaskExecution {
  const model::OwnerGroupTask &before, &after;
  const model::OwnerKnowledge &knowledge;
  const model::AutonomousWorld &world;
  const std::vector<unsigned> &members;
  const std::vector<int> &raw, &net, &threat;
  int tick = 0;
  unsigned owner = 0;
  int percent = 0, initialBudget = 0, finalBudget = 0;
  const model::OwnerSourceGroupResult &result;
};
struct NativeOwnerTaskVisit {
  bool cleanup = false, execute = false, continuing = false;
  int *budget = nullptr;
  const NativeOwnerTaskVerifier *verifier = nullptr;
};
void updateNativeOwnerGroupPlanner(Continuation &,
                                   const NativeCreatureProfile &,
                                   model::OwnedCreatureCommands &,
                                   NativeOwnerProposalEvaluation * = nullptr,
                                   bool executeOnly = false,
                                   NativeOwnerTaskVisit * = nullptr);
void updateNativeOwnerPlanner(Continuation &, const NativeCreatureProfile &,
                              model::OwnedCreatureCommands &,
                              const NativeOwnerTaskVerifier & = {});
} // namespace mnm::command_session
