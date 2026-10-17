#pragma once
#include "native_owner_planner.hpp"
namespace mnm::command_session {
std::vector<unsigned> nativeOwnerSourceActors(const NativeOwnerPlanner &);
void prepareNativeOwnerSources(Continuation &);
void enableAutomaticOwnerSources(Continuation &);
void validateNativeOwnerSources(const Continuation &);
void updateNativeOwnerSources(Continuation &, const NativeCreatureProfile &,
                              model::OwnedCreatureCommands &);
// Explicit topology commits for a previously selected proposal. No authored
// score can silently stand in for original423590 proposal generation.
void splitNativeOwnerSource(Continuation &, unsigned source,
                            const std::vector<unsigned> &movedLeaves);
void mergeNativeOwnerSources(Continuation &, unsigned left, unsigned right);
} // namespace mnm::command_session
