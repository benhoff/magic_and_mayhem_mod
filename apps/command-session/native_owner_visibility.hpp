#pragma once
#include "native_providers.hpp"
namespace mnm::command_session {
void prepareNativeOwnerVisibility(Continuation &,
                                  std::vector<model::SightDefinition>,
                                  model::VisionRelations sharing = {});
void updateNativeOwnerVisibility(Continuation &, const NativeCreatureProfile &);
bool nativePointSight(const Continuation &, const NativeCreatureProfile &,
                      unsigned observer, model::Cell target);
} // namespace mnm::command_session
