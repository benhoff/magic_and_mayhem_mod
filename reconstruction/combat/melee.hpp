#pragma once
#include <array>
#include <cstdint>
#include <functional>
#include <vector>

namespace mnm::reconstruction::combat {
// Owned projection of 54e170/54e200; checkpoint every word and both cursors.
struct RandomState {
  std::uint32_t left = 0, right = 103;
  std::array<std::uint32_t, 250> words{};
  bool operator==(const RandomState &o) const {
    return left == o.left && right == o.right && words == o.words;
  }
};
RandomState seedRandom(std::uint32_t);
std::uint32_t nextRandom(RandomState &);
struct MeleeInputs {
  std::int32_t base = 18, power = 0, height = 0, heightBonus = 0,
               responseModifier = 0, rankBonus = 0, stunThreshold = 9;
  bool attackerPhysical = true, targetPhysical = true, bloodLust = false,
       quarterFlag = false, targetResponse = false;
  std::uint32_t rank = 0;
  // Supplied helper 521160 result; effects of stun/response helpers excluded.
  bool stunHandled = true;
};
enum class MeleeEffect {
  responseQuery,
  damage,
  hostileCredit,
  stunQuery,
  stunHandled,
  stunFallback,
  attackSound,
  effectAdmission
};
struct MeleeEvent {
  MeleeEffect kind;
  std::int32_t value = 0;
  bool operator==(const MeleeEvent &o) const {
    return kind == o.kind && value == o.value;
  }
};
struct MeleeResult {
  std::int32_t amount = 0;
  std::vector<std::uint32_t> draws;
  std::vector<MeleeEvent> events;
};
// Type17 -> type17, rank0/1/2, no evasion/charge/cached contributions,
// hostile distinct slots, null optional effect admission, supplied response.
// Draw words are masked exactly at the original call sites.
MeleeResult resolveMelee(const MeleeInputs &,
                         const std::function<std::uint32_t()> &);
struct MeleeActionState {
  std::uint32_t engaged = 1, incoming = 1, recentAttacker = 0, cooldown = 0;
  bool ready = false;
};
struct MeleeActionResult {
  MeleeActionState state;
  bool hit = false, sound = false;
};
// Main action a, no attached secondary animation; event2 hits, event1 finishes.
MeleeActionResult meleeEvent(MeleeActionState, std::int32_t event,
                             bool targetPresent, bool targetLiving,
                             bool blocked, bool slow, std::uint32_t actor,
                             const std::function<std::uint32_t()> &);
} // namespace mnm::reconstruction::combat
