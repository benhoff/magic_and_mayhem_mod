#pragma once
#include "../audio/voice_contract.hpp"
#include <functional>
#include <optional>
#include <vector>

namespace mnm::reconstruction::casting {
struct CastResource {
  std::uint32_t kind = 0, value = 0, maximum = 0, blocked = 0, owner = 0,
                dirty = 0;
};
bool chargeableKind(std::uint32_t kind);
// Delta and maximum shift wrap as DWORDs. UI callbacks can change value; the
// second notification observes the first callback's write, as the original does.
void changeCastResource(CastResource &, std::uint32_t delta,
                        std::uint32_t localOwner,
                        const std::function<void(std::uint32_t)> &first,
                        const std::function<void(std::uint32_t)> &second);

struct CureStatus {
  std::uint32_t status = 0;
  std::optional<std::uint32_t> voice;
};
// Owned logical IDs: next/previous are zero-based node indices. voiceAddress
// and outputAddress are one-based vector indices (zero means absent), never
// process addresses. No original wire layout is shared with native consumers.
struct CureStatusAudio {
  std::uint32_t enabled = 0, initialized = 0, head = 0;
  std::uint32_t statusScratch = 0, resultScratch = 0;
  std::vector<std::uint32_t> buffers, outputs;
  std::vector<audio::Schedule32> schedules;
};
// Valid nonempty ring containing the associated voice is required whenever
// enabled+initialized+buffer can reach Stop. Invalid projections are refused
// before writes/backend calls; the original dereferences absent schedule nodes.
// Backend calls must not resize/mutate the audio graph or throw.
void clearCureStatus(CureStatus &, CureStatusAudio &, audio::VoiceBackend &);
} // namespace mnm::reconstruction::casting
