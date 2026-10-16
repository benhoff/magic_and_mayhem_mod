#include "owner_knowledge.hpp"
#include <algorithm>
#include <cstring>
#include <limits>
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
int signedWord(unsigned v) { int out; std::memcpy(&out, &v, 4); return out; }
int difference(int a, int b) { return signedWord(unsigned(a) - unsigned(b)); }
int distance(Cell a, Cell b, Extent d) {
  auto delta = [](int a, int b, int span) {
    int n = difference(a, b) % span;
    if (n < 0) n += span;
    if (n > span / 2) n -= span;
    return unsigned(n);
  };
  const unsigned x = delta(a.x, b.x, d.x * 32), y = delta(a.y, b.y, d.y * 32),
                 z = unsigned(a.z) - unsigned(b.z);
  return signedWord(x*x + y*y + z*z);
}
int allowance(int tick, int seen, int speed) {
  const auto rate = signedWord((unsigned(tick) - unsigned(seen)) * unsigned(speed) * 3u);
  // Exact original float at 5c53f0, promoted for x87's unrounded product.
  const long double v = static_cast<long double>(rate) * rate *
                        static_cast<long double>(0.00019290123600512743f);
  return signedWord(unsigned(static_cast<std::uint64_t>(v)));
}
auto lower(std::vector<OwnerMemory> &v, int key) {
  return std::lower_bound(v.begin(), v.end(), key,
      [](const auto &m, int k) { return m.memory.key < k; });
}
} // namespace
void markOwnerKnowledge(OwnerKnowledge &s, int tick) {
  for (auto &owner : s.records)
    for (auto &r : owner)
      if (r.memory.observedTick >= s.previousSweep) r.markedTick = tick;
}
void updateOwnerKnowledge(OwnerKnowledge &s, int tick, Extent dimensions,
                         const std::array<bool, 8> &enabled,
                         const std::array<std::array<std::uint8_t, 8>, 8> &allied,
                         const std::vector<OwnerObservation> &actors) {
  if (dimensions.x <= 0 || dimensions.y <= 0 || dimensions.x > 32767 || dimensions.y > 32767)
    throw std::invalid_argument("owner knowledge dimensions");
  validateOwnerKnowledge(s, 125);
  int lastSlot = -1;
  for (const auto &a : actors) {
    if (a.actor.slot <= lastSlot || a.actor.slot >= 125 || a.actor.owner < 0 || a.actor.owner >= 8)
      throw std::invalid_argument("owner knowledge actor order/domain");
    lastSlot = a.actor.slot;
  }
  unsigned advances = 0;
  for (unsigned work = 0; work < 8; ++work) {
    if (s.reset || !enabled.at(unsigned(s.owner)) || s.cursor == -1) {
      if (s.reset) { s.reset = false; s.owner = -1; }
      do {
        if (advances == 8) return;
        ++advances;
        if (++s.owner == 8) {
          s.previousSweep = s.currentSweep; s.currentSweep = tick; s.owner = 0;
        }
        auto &records = s.records[unsigned(s.owner)];
        s.cursor = records.empty() ? -1 : records.front().memory.key;
      } while (!enabled[unsigned(s.owner)]);
      s.matched.fill(false);
    }
    const auto owner = unsigned(s.owner);
    auto &records = s.records[owner];
    if (s.cursor != -1) {
      auto it = lower(records, s.cursor);
      const OwnerObservation *best = nullptr;
      int nearest = std::numeric_limits<int>::max();
      for (const auto &a : actors) {
        if (!a.actor.active || s.matched[unsigned(a.actor.slot)] || !a.visible[owner] ||
            a.actor.owner != it->memory.actor.owner || a.actor.type != it->memory.actor.type) continue;
        const int dist = distance(a.finePosition, it->finePosition, dimensions);
        if (dist <= allowance(tick, it->memory.observedTick, a.speed) && dist < nearest) {
          nearest = dist; best = &a;
        }
      }
      if (best) {
        it->memory.observedTick = tick; it->memory.actor = best->actor;
        it->finePosition = best->finePosition; s.matched[unsigned(best->actor.slot)] = true;
      }
      const bool expired = difference(tick, it->memory.observedTick) > 1024;
      auto next = it + 1;
      s.cursor = next == records.end() ? -1 : next->memory.key;
      if (expired) records.erase(it);
    }
    if (s.cursor == -1) {
      for (const auto &a : actors) {
        if (!a.actor.active || !a.actor.health || !a.visible[owner] ||
            allied[owner][unsigned(a.actor.owner)] || s.matched[unsigned(a.actor.slot)]) continue;
        if (records.size() >= 125 || s.nextKey[owner] == std::numeric_limits<int>::max())
          throw std::invalid_argument("owner knowledge admitted memory/key bound");
        records.push_back({{s.nextKey[owner]++, tick, a.actor}, tick, a.finePosition});
      }
    }
  }
}
void validateOwnerKnowledge(const OwnerKnowledge &s, unsigned actors) {
  if (actors > 125 || s.owner < -1 || s.owner >= 8 || (!s.reset && s.owner < 0) || s.cursor < -1)
    throw std::invalid_argument("owner knowledge cursor domain");
  for (unsigned owner = 0; owner < 8; ++owner) {
    int key = -1;
    if (s.nextKey[owner] < 0 || s.records[owner].size() > 125)
      throw std::invalid_argument("owner knowledge memory bound");
    for (const auto &r : s.records[owner]) {
      const auto &m = r.memory;
      if (m.key <= key || m.key >= s.nextKey[owner] || m.actor.slot < 0 || unsigned(m.actor.slot) >= actors)
        throw std::invalid_argument("owner knowledge memory identity");
      key = m.key;
    }
  }
  if (s.cursor != -1) {
    if (s.owner < 0) throw std::invalid_argument("owner knowledge absent owner");
    const auto &records = s.records[unsigned(s.owner)];
    if (std::none_of(records.begin(), records.end(), [&](const auto &r) {return r.memory.key == s.cursor;}))
      throw std::invalid_argument("owner knowledge missing cursor");
  }
}
} // namespace mnm::reconstruction::combat
