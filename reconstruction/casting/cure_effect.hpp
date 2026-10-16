#pragma once
#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace mnm::reconstruction::casting {
// Owned logical projection of spell 41. These are not packed engine layouts.
struct CurePosition { std::int32_t x = 0, y = 0, z = 0; };
struct CureActor {
  std::uint32_t health = 0, blocked = 0, status = 0, ongoingDamage = 0,
                resource = 0;
  std::array<std::uint32_t, 8> visible{};
};
struct CureEffect {
  std::uint32_t spell = 41, type = 0, source = UINT32_MAX,
                target = UINT32_MAX, deliveryMode = 0, attribution = UINT32_MAX;
  CurePosition position;
  std::array<std::uint32_t, 2> contributions{};
  std::optional<std::uint32_t> cachedActor;
};
struct CureConfiguration {
  std::uint32_t cost = 0;
  std::int32_t magnitude = 0, localOwner = -1;
};
struct CureServices {
  std::function<void(CureActor &, std::int32_t, std::uint32_t,
                     const std::array<std::uint32_t, 2> &, std::uint32_t)> health;
  std::function<void(CureActor &)> cleanse;
  std::function<void(std::uint32_t, CurePosition)> cue;
  std::function<void(CureEffect &, CureActor &)> feedback;
  std::function<void(CureEffect &, std::uint32_t)> transition;
  std::function<bool(const CureActor &)> chargeable;
  std::function<void(CureActor &, std::uint32_t)> resource;
  std::function<void(CureEffect &)> release;
};
// Caller stages state/events for rollback. Services may change the passed actor,
// but cannot resize actors or change the effect cache out of band. No invented
// Boolean cast outcome; lifecycle consequences belong to explicit services.
void applyCureEffect(CureEffect &, std::vector<CureActor> &,
                     const CureConfiguration &, const CureServices &);
} // namespace mnm::reconstruction::casting
