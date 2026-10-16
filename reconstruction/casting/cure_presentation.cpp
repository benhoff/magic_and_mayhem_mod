#include "cure_presentation.hpp"
#include "../rendering/effect_animation_selection.hpp"
#include <algorithm>
#include <stdexcept>

namespace mnm::reconstruction::casting {
namespace {
void require(bool yes, const char *why) {
  if (!yes)
    throw std::invalid_argument(why);
}
unsigned wrap(unsigned word, unsigned extent) {
  auto n = word <= 0x7fffffff ? std::int64_t(word)
                              : std::int64_t(word) - 0x100000000ll;
  require(n >= -std::int64_t(extent) && n < 2 * std::int64_t(extent),
          "Cure one-period coordinate");
  if (n < 0)
    n += extent;
  if (n >= extent)
    n -= extent;
  return unsigned(n);
}
void insert(CurePresentationState &s, unsigned slot) {
  auto &r = s.records.at(slot).placement;
  auto &cell = s.cells.at(r.cell);
  if (cell.flags & 0x40000000u)
    return;
  if (cell.head == NoEffect)
    cell.head = std::uint16_t(slot);
  else {
    auto at = unsigned(cell.head);
    for (unsigned visited = 0;; ++visited) {
      require(at < s.records.size() && at != slot && visited < s.records.size(),
              "Cure cell chain");
      auto &tail = s.records[at].placement;
      if (tail.next == NoEffect) {
        tail.next = std::uint16_t(slot);
        r.next = NoEffect;
        break;
      }
      at = tail.next;
    }
  }
  cell.flags &= ~0x80u;
}
void unlink(CurePresentationState &s, unsigned slot,
            const std::function<bool(unsigned)> &query) {
  auto &r = s.records.at(slot).placement;
  auto &c = s.cells.at(r.cell);
  if (c.head != NoEffect) {
    if (c.head == slot)
      c.head = r.next;
    else {
      auto at = unsigned(c.head);
      for (unsigned visited = 0;; ++visited) {
        require(at < s.records.size() && visited < s.records.size(),
                "Cure unlink chain");
        auto &p = s.records[at].placement;
        if (p.next == slot) {
          p.next = r.next;
          break;
        }
        if (p.next == NoEffect)
          return; // Original early return skips recycle.
        at = p.next;
      }
    }
    r.next = NoEffect;
  }
  if (!c.terrain && c.head == NoEffect && c.otherA == NoEffect &&
      c.otherB == NoEffect && !(c.flags & 0x20000000u) && query(r.cell))
    c.flags |= 0x80;
}
} // namespace
void validateCurePresentation(const CurePresentationState &s) {
  require(s.width && s.width <= 128 && s.height && s.height <= 128 &&
              s.layers && s.layers <= 32 && s.records.size() <= 65535 &&
              s.cells.size() == std::size_t(s.width) * s.height * s.layers &&
              s.scan <= s.records.size(),
          "Cure presentation dimensions");
  std::vector<bool> linked(s.records.size(), false);
  for (unsigned c = 0; c < s.cells.size(); ++c) {
    auto at = unsigned(s.cells[c].head);
    while (at != NoEffect) {
      require(at < s.records.size() && !linked[at], "Cure repeated cell link");
      linked[at] = true;
      const auto &r = s.records[at].placement;
      require(r.active && r.cell == c, "Cure foreign cell link");
      at = r.next;
    }
  }
  for (unsigned i = 0; i < s.records.size(); ++i) {
    const auto &r = s.records[i].placement;
    if (r.active) {
      require((r.type == 11 || r.type == 77) && r.parameters[8] == 41 &&
                  r.position[0] < s.width && r.position[1] < s.height &&
                  r.position[2] < s.layers &&
                  r.cell ==
                      (r.position[2] * s.height + r.position[1]) * s.width +
                          r.position[0] &&
                  (linked[i] || (s.cells[r.cell].flags & 0x40000000u)) &&
                  i < s.scan,
              "Cure active spatial record");
    }
  }
}
CurePresentationState makeCurePresentation(unsigned w, unsigned h, unsigned z,
                                           unsigned capacity) {
  require(w && w <= 128 && h && h <= 128 && z && z <= 32 && capacity <= 65535,
          "Cure spatial capacity");
  CurePresentationState s;
  s.width = w;
  s.height = h;
  s.layers = z;
  s.cells.resize(std::size_t(w) * h * z);
  s.records.resize(capacity);
  return s;
}
void createCurePresentation(
    CurePresentationState &state, unsigned slot,
    const CastEffectDescriptor &parameters,
    const std::vector<CureCreator> &creators,
    const std::vector<CureContribution> &contributions) {
  validateCurePresentation(state);
  require(
      slot < state.records.size() && !state.records[slot].placement.active &&
          state.records[slot].placement.next == NoEffect && parameters[8] == 41,
      "Cure creation slot/spell");
  auto s = state;
  auto &record = s.records[slot];
  auto &r = record.placement;
  r.parameters = parameters;
  r.sentinels.fill(UINT32_MAX);
  const auto maxZ = s.layers * 16 - 1;
  for (unsigned base : {0u, 3u}) {
    r.parameters[base] = wrap(parameters[base], s.width * 32);
    r.parameters[base + 1] = wrap(parameters[base + 1], s.height * 32);
    require(parameters[base + 2] <= 0x7fffffff, "Cure negative height");
    r.parameters[base + 2] = std::min(parameters[base + 2], maxZ);
  }
  if (parameters[6] != UINT32_MAX) {
    require(parameters[6] < creators.size() && contributions.size() <= 65536,
            "Cure creator identity");
    const auto &creator = creators[parameters[6]];
    r.sentinels[9] = creator.owner;
    for (unsigned i = 0; i < 2; ++i) {
      auto at = creator.contributions[i];
      std::vector<bool> seen(contributions.size(), false);
      while (at) {
        require(*at < contributions.size() && !seen[*at],
                "Cure contributor chain");
        seen[*at] = true;
        const auto &node = contributions[*at];
        if (!node.next) {
          r.sentinels[10 + i] = node.value.value_or(UINT32_MAX);
          break;
        }
        at = node.next;
      }
    }
  }
  r.units = {r.parameters[3], r.parameters[4], r.parameters[5]};
  r.position = {r.units[0] >> 5, r.units[1] >> 5, r.units[2] >> 4};
  r.initialPosition = r.position;
  r.cell = (r.position[2] * s.height + r.position[1]) * s.width + r.position[0];
  r.active = true;
  r.initialized = true;
  r.type = 11;
  record.reservation = record.property = 0;
  record.animationReset = true;
  if (record.animation)
    record.animation->player.stop();
  insert(s, slot);
  s.scan = std::max(s.scan, slot + 1);
  validateCurePresentation(s);
  state = std::move(s);
}
void bindCurePresentation(CurePresentationState &s, unsigned slot,
                          const CureAnimationDefinition &definition) {
  require(slot < s.records.size() && s.records[slot].placement.active,
          "Cure bind slot");
  assets::Animation a;
  a.starts = {0, unsigned(definition.sequence.size())};
  a.records = definition.sequence;
  auto binding = bindEffectAnimation(
      0, {{0, 0, definition.entry.property, definition.entry.opaque}}, {a});
  binding.animationOrdinal = selectEffectAnimation(77, 41, {}, UINT32_MAX, {});
  binding.entry = definition.entry;
  auto &r = s.records[slot];
  r.animation = std::move(binding);
  r.animationReset = false;
  r.property = definition.entry.property;
  r.placement.type = 77;
}
std::int32_t tickCurePresentation(CurePresentationState &s, unsigned slot) {
  auto &r = s.records.at(slot);
  require(r.placement.active && r.placement.type == 77 && r.animation &&
              !r.animationReset,
          "Cure tick binding");
  auto candidate = *r.animation;
  const auto result = candidate.player.tick();
  r.animation = std::move(candidate);
  return result;
}
void releaseCurePresentation(CurePresentationState &state, unsigned slot,
                             const std::function<bool(unsigned)> &query) {
  validateCurePresentation(state);
  require(slot < state.records.size() &&
              state.records[slot].placement.initialized,
          "Cure release slot");
  auto s = state;
  auto &r = s.records[slot];
  if (r.placement.active) {
    unlink(s, slot, query);
    r.placement.active = false;
  }
  if (slot + 1 == s.scan) {
    s.scan = 0;
    for (unsigned i = 0; i < s.records.size(); ++i)
      if (s.records[i].placement.active)
        s.scan = i + 1;
  }
  r.reservation = 0;
  r.placement.sentinels[9] = 255;
  r.placement.sentinels[10] = r.placement.sentinels[11] = UINT32_MAX;
  if (r.animation)
    r.animation->player.stop();
  r.animationReset = true;
  validateCurePresentation(s);
  state = std::move(s);
}
void moveCurePresentation(CurePresentationState &state, unsigned slot,
                          std::array<unsigned, 3> p) {
  validateCurePresentation(state);
  require(slot < state.records.size() && state.records[slot].placement.active &&
              p[0] < state.width && p[1] < state.height && p[2] < state.layers,
          "Cure movement domain");
  auto s = state;
  auto &r = s.records[slot].placement;
  const auto cell = (p[2] * s.height + p[1]) * s.width + p[0];
  if (cell != r.cell) {
    unlink(s, slot, [](unsigned) { return false; });
    r.cell = cell;
    insert(s, slot);
  }
  r.position = p;
  r.units = {p[0] * 32 + 16, p[1] * 32 + 16, p[2] * 16 + 8};
  validateCurePresentation(s);
  state = std::move(s);
}
} // namespace mnm::reconstruction::casting
