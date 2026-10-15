#pragma once
#include "../../reconstruction/creature-ai/regional_graph.hpp"
#include "native_geometry.hpp"
namespace mnm::command_session {
model::RegionalGraphServices
nativeRegionalGraphServices(Continuation &, const NativeCreatureProfile &,
                            NativeCreatureGeometry &);
// Build privately, then publish atomically. Existing routes/searches must be
// idle.
void prepareNativeRegionalGraph(
    Continuation &, const NativeCreatureProfile &,
    const std::array<model::RegionalCreatureShape, 28> &,
    unsigned capacity = 512);
} // namespace mnm::command_session
