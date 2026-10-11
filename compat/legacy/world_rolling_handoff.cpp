#include "world_rolling_handoff.hpp"
#include <algorithm>
namespace mnm::legacy {
RollingHandoff prepareRollingHandoff(const CanvasProducerStream &stream,
                                     std::size_t cursor,
                                     const std::vector<std::uint8_t> &request,
                                     unsigned ordinal, unsigned priorRasters,
                                     WorldBatchSourcePolicy policy,
                                     bool encode) {
  if (stream.version != 3 || ordinal < 1 || ordinal > 32 ||
      cursor >= stream.operations.size())
    throw std::invalid_argument(
        "Rolling handoff requires an unreturned finite V3 queue");
  std::vector<CanvasProducer> operations;
  std::vector<const CanvasProducer *> rasters;
  const CanvasProducer *entry = nullptr;
  unsigned lastOracle = 0;
  for (const auto &c : stream.operations)
    if (c.fields[2] == 10)
      lastOracle = std::max(lastOracle, c.fields[14]);
  for (std::size_t i = cursor; i < stream.operations.size(); ++i) {
    const auto &c = stream.operations[i];
    const auto &r = c.fields;
    if (r[2] == 11) {
      if (entry || r[14] != ordinal)
        throw std::invalid_argument("Rolling handoff queue identity");
      entry = &c;
    } else if (r[2] == 12 || r[2] == 13)
      throw std::invalid_argument(
          "Rolling handoff already returned or refused");
    else if (entry) {
      if (r[2] == 9)
        rasters.push_back(&c);
      else if (r[2] != 4 && r[2] != 25)
        throw std::invalid_argument("Rolling handoff unexpected World writer");
    }
    operations.push_back(c);
    // Preserve original checkpoint identity independently of packet-local
    // order.
    if (r[2] == 10)
      operations.back().fields[16] = r[1];
  }
  if (!entry || lastOracle >= 9999 || stream.operations.size() > 131070)
    throw std::invalid_argument(
        "Rolling handoff missing entry or exhausted closing identities");
  RollingHandoff result;
  const auto &e = entry->fields;
  result.batch = validateWorldRasterBatch(
      request, ordinal, ordinal, unsigned(stream.operations.size()), e[3], e[5],
      e[6], priorRasters, rasters, policy);
  CanvasProducer checkpoint{};
  checkpoint.fields[0] = 96;
  checkpoint.fields[1] = unsigned(stream.operations.size() + 1);
  checkpoint.fields[2] = 10;
  checkpoint.fields[3] = e[3];
  checkpoint.fields[5] = e[5];
  checkpoint.fields[6] = e[6];
  checkpoint.fields[7] = e[7];
  checkpoint.fields[14] = lastOracle + 1;
  checkpoint.fields[15] = 4;
  checkpoint.fields[16] = checkpoint.fields[1];
  result.checkpointSequence = checkpoint.fields[1];
  result.checkpointOracle = checkpoint.fields[14];
  operations.push_back(checkpoint);
  CanvasProducer returned{};
  returned.fields = checkpoint.fields;
  returned.fields[1]++;
  returned.fields[2] = 12;
  returned.fields[14] = 1;
  returned.fields[15] = result.batch.header[15];
  returned.fields[16] = 0;
  returned.fields[17] = ordinal;
  returned.fields[19] = rasters.size();
  returned.fields[20] = 1;
  operations.push_back(returned);
  if (encode)
    result.packet = encodeRollingQueue(operations);
  else {
    result.stream.version = 3;
    result.stream.queues = 1;
    result.stream.operations = std::move(operations);
    validateRollingQueue(result.stream);
  }
  return result;
}
std::vector<std::uint8_t>
rollingHandoffReply(const RollingHandoff &handoff,
                    const RollingCompletion &completion) {
  const auto &h = handoff.batch.header;
  const auto &image = completion.image;
  if (completion.sequence != h[4] || image.width != int(h[8]) ||
      image.height != int(h[9]) ||
      image.pixels.size() != std::size_t(h[8]) * h[9] ||
      std::any_of(image.pixels.begin(), image.pixels.end(),
                  [](auto p) { return p > 65535; }))
    throw std::invalid_argument(
        "Rolling handoff stale reply or requested extent changed");
  return worldRasterBatchReply(handoff.batch,
                               {image.pixels.begin(), image.pixels.end()});
}
void verifyRollingHandoffReturn(const RollingHandoff &handoff,
                                const CanvasProducer &checkpoint,
                                const CanvasProducer &returned) {
  const auto &h = handoff.batch.header;
  const auto &c = checkpoint.fields;
  const auto &r = returned.fields;
  if (c[2] != 10 || c[1] != handoff.checkpointSequence ||
      c[14] != handoff.checkpointOracle || c[3] != h[7] || c[5] != h[8] ||
      c[6] != h[9] || c[7] != h[10] || c[15] != 4 || c[16] != h[5] ||
      r[2] != 12 || r[1] != c[1] + 1 || r[3] != h[7] || r[5] != h[8] ||
      r[6] != h[9] || r[7] != h[10] || r[14] != h[5] || r[15] != h[15] ||
      r[17] != h[4] || r[19] != h[14] || r[20] != 1)
    throw std::invalid_argument(
        "Original guarded World did not acknowledge the rolling handoff");
}
} // namespace mnm::legacy
