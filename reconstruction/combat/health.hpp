#pragma once
#include <array>
#include <cstdint>
#include <vector>

namespace mnm::reconstruction::combat {
enum class Entry { maximum, signedHealth, defendedDamage, directDamage };
// Typed projection of one ordinary class-1/type-17 creature. No host pointers.
struct State {
  std::uint32_t health = 24, bar = 13, dirty = 0, cooldown = 1;
  std::uint32_t target = 3, gate9c = 7, expiry = 7, timer = 7, cacheA = 7,
                cacheB = 7, marker = 7, flag104 = 7, rank = 0;
  std::array<std::uint32_t, 8> incoming{{5, 5, 5, 5, 5, 5, 5, 5}},
      ages{{9, 9, 9, 9, 9, 9, 9, 9}};
  std::array<std::uint32_t, 2> credit{{11, 13}};
};
struct Inputs {
  std::uint32_t baseMaximum = 24, bonusOne = 25, bonusTwo = 50;
  std::uint32_t status = 0, blocked = 0, mitigation = 0, mitigationPercent = 25,
                quarter = 0, absorption = 0;
  std::uint32_t response = 0, globalGate = 0, notify = 1, visibleMask = 255,
                owner = 1, selectedOwner = 0;
  std::int32_t attributionOwner = -1;
  bool contributions = false;
  std::array<std::uint32_t, 2> contributionIds{{41, 43}};
  // Explicit lookup endpoints may return null for absent identities.
  std::array<bool, 2> creditAvailable{{true, true}};
  // Unrecovered helper returns are explicit supplied boundaries, not models.
  std::uint32_t feedbackValue = 7, slot = 0;
};
enum class Effect : std::uint32_t {
  status = 1,
  mitigated,
  response,
  damageNotice,
  scenarioQuery,
  killTracker,
  definitionQuery,
  spatialCleanup,
  detachActor,
  behavior,
  repair,
  feedback,
  creditA,
  creditB,
  attribution,
  selectedQuery,
  barSet,
  barNotify,
  pain
};
struct Event {
  Effect kind;
  std::uint32_t a = 0, b = 0, c = 0;
  bool operator==(const Event &o) const {
    return kind == o.kind && a == o.a && b == o.b && c == o.c;
  }
};
struct Result {
  State state;
  std::vector<Event> events;
  std::uint32_t returned = 0;
};
std::uint32_t maximum(const State &, const Inputs &);
Result apply(State, const Inputs &, Entry, std::int32_t amount);
} // namespace mnm::reconstruction::combat
