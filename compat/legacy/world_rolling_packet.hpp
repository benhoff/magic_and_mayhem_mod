#pragma once
#include "canvas_producers.hpp"
#include "../../protocols/include/mnm/world_rolling_v1.h"
namespace mnm::legacy {
// One complete queue, including the native producer operations preceding it.
// Payload references use a bounded packet-local owned source table, discarded
// after each queue; no indefinite capture/source journal.
// Local V3 record/queue numbering is normalized; the rolling envelope owns the
// monotonic uint64 sequence. This is a native policy, not a V3 continuation.
std::vector<std::uint8_t> encodeRollingQueue(const std::vector<CanvasProducer>&);
CanvasProducerStream decodeRollingQueue(const std::vector<std::uint8_t>&);
}
