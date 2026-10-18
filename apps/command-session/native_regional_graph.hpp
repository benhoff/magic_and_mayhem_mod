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
struct NativeTerrainEdit {
  model::Cell position;
  std::uint16_t kind = 0, flagsMask = 0, flagsValue = 0;
};
// Transactional native input policy: kind/terrain flags only; preserve actor
// reservations. Uses recovered before/after signatures and queued repair.
void editNativeTerrain(Continuation &, const NativeCreatureProfile &,
                       const std::vector<NativeTerrainEdit> &);
void maintainNativeRegionalGraph(Continuation &, const NativeCreatureProfile &,
                                 bool blocked = false);
} // namespace mnm::command_session
