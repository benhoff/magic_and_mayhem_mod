#include "cure_services.hpp"
#include "../audio/voice_lifetime.hpp"
#include <stdexcept>

namespace mnm::reconstruction::casting {
bool chargeableKind(std::uint32_t kind) {
  return kind == 0 || kind == 24 || kind == 25 || kind == 26;
}
void changeCastResource(CastResource &actor, std::uint32_t delta,
                        std::uint32_t localOwner,
                        const std::function<void(std::uint32_t)> &first,
                        const std::function<void(std::uint32_t)> &second) {
  const auto old = actor.value;
  const auto sum = old + delta;
  const auto nonnegative = (sum & 0x80000000u) ? 0 : sum;
  const auto cap = actor.maximum << 8;
  actor.value = nonnegative < cap ? nonnegative : cap;
  if (chargeableKind(actor.kind) && !actor.blocked &&
      actor.owner == localOwner && actor.value != old) {
    first(actor.value >> 8);
    second(actor.value >> 8);
  }
  actor.dirty = 1;
}

void clearCureStatus(CureStatus &actor, CureStatusAudio &state,
                     audio::VoiceBackend &backend) {
  const auto size = state.schedules.size();
  if (size > 65536 || state.buffers.size() > 65536 ||
      state.outputs.size() > 65536 ||
      (actor.voice && *actor.voice >= state.buffers.size()) ||
      (size && state.head >= size))
    throw std::invalid_argument("Cure status projection");
  std::vector<bool> visited(size, false);
  auto cursor = state.head;
  std::optional<std::uint32_t> found;
  for (std::size_t count = 0; count < size; ++count) {
    if (cursor >= size || visited[cursor])
      throw std::invalid_argument("Cure schedule ring");
    visited[cursor] = true;
    const auto &node = state.schedules[cursor];
    if (node.next >= size || node.previous >= size ||
        state.schedules[node.next].previous != cursor ||
        state.schedules[node.previous].next != cursor ||
        node.voiceAddress > state.buffers.size() ||
        node.outputAddress > state.outputs.size())
      throw std::invalid_argument("Cure schedule links");
    if (!found && actor.voice && node.voiceAddress == *actor.voice + 1)
      found = cursor;
    cursor = node.next;
  }
  if (size && cursor != state.head)
    throw std::invalid_argument("Cure schedule cycle");
  const bool active = actor.voice && state.enabled && state.initialized &&
                      state.buffers[*actor.voice];
  if (active && !found)
    throw std::invalid_argument("Cure voice has no schedule");
  // Allocate annotations before changing owned state. Reuse the already
  // recovered scheduler retirement instead of inventing a second ring policy.
  std::vector<audio::ScheduleNode> nodes(size);
  for (std::size_t i = 0; i < size; ++i) {
    nodes[i].record = state.schedules[i];
    nodes[i].next = &nodes[state.schedules[i].next];
    nodes[i].previous = &nodes[state.schedules[i].previous];
    const auto output = state.schedules[i].outputAddress;
    nodes[i].output = output ? &state.outputs[output - 1] : nullptr;
  }
  actor.status = 0;
  if (active) {
    const auto buffer = state.buffers[*actor.voice];
    state.statusScratch = 0;
    if (backend.getStatus(buffer, state.statusScratch) != 0 ||
        (state.statusScratch & 1)) {
      state.resultScratch = std::uint32_t(backend.stop(buffer));
      auto *head = &nodes[state.head];
      audio::retireSchedule(head, nodes[*found]);
      state.head = std::uint32_t(head - nodes.data());
      for (std::size_t i = 0; i < size; ++i) {
        state.schedules[i] = nodes[i].record;
        state.schedules[i].next = std::uint32_t(nodes[i].next - nodes.data());
        state.schedules[i].previous = std::uint32_t(nodes[i].previous - nodes.data());
      }
      if (!state.resultScratch)
        state.resultScratch = std::uint32_t(backend.position(buffer, 0));
    }
  }
  actor.voice.reset();
}
} // namespace mnm::reconstruction::casting
