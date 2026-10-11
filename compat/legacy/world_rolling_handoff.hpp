#pragma once
#include "world_raster_batch.hpp"
#include "world_rolling_channel.hpp"
#include "world_rolling_packet.hpp"
namespace mnm::legacy {
// Finite original V3 adapter. It cannot infer a returned queue from
// publication.
struct RollingHandoff {
  WorldRasterBatch batch;
  CanvasProducerStream stream;
  std::vector<std::uint8_t> packet;
  unsigned checkpointSequence = 0, checkpointOracle = 0;
};
RollingHandoff prepareRollingHandoff(
    const CanvasProducerStream &, std::size_t cursor,
    const std::vector<std::uint8_t> &request, unsigned ordinal,
    unsigned priorRasters,
    WorldBatchSourcePolicy policy = WorldBatchSourcePolicy::canonicalV3,
    bool encode = true);
std::vector<std::uint8_t> rollingHandoffReply(const RollingHandoff &,
                                              const RollingCompletion &);
void verifyRollingHandoffReturn(const RollingHandoff &,
                                const CanvasProducer &checkpoint,
                                const CanvasProducer &returned, bool resident = false);
} // namespace mnm::legacy
