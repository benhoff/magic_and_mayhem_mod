#pragma once
#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace mnm::reconstruction::casting {
struct ProjectileActor {
  std::uint32_t health = 0, recentAttacker = 0, recentTime = 0;
  std::array<std::uint32_t, 8> visible{};
};
struct ProjectileEffect {
  std::uint32_t type = 3, spell = 73, source = UINT32_MAX,
                target = UINT32_MAX, attacker = UINT32_MAX, attribution = UINT32_MAX,
                remaining = 100, audio = 0;
  std::array<std::uint32_t, 2> contributions{};
  std::optional<std::uint32_t> cachedActor;
};
// Lower calls are explicit boundaries. A motion result zero must publish a valid
// target index, as the original immediately dereferences the lookup result.
// Callers stage state; callbacks cannot resize actors or mutate cache out of band.
struct ProjectileServices {
  std::function<std::uint32_t(ProjectileEffect &)> motion;
  std::function<void(ProjectileActor &, std::int32_t, std::uint32_t,
                     const std::array<std::uint32_t, 2> &)> damage;
  std::function<void(ProjectileEffect &, bool)> bounce;
  std::function<void(ProjectileEffect &)> release;
  std::function<std::int32_t()> animationTick;
  std::function<void()> animationRestart;
  std::function<void(ProjectileActor &)> visibleDamage;
  std::function<void(ProjectileEffect &)> startAudio;
  std::function<void(ProjectileEffect &)> followAudio;
};
// Selected whole 0048ecf0 actor-target path; terrain/area damage is excluded.
void impactProjectile(ProjectileEffect &, std::vector<ProjectileActor> &,
                      std::int32_t magnitude, const ProjectileServices &);
// Whole 004970e0 type3 arm, including shared release/audio tail (active effect,
// auxiliary +222 absent). Motion/ricochet/animation/audio are supplied services.
void updateProjectile(ProjectileEffect &, std::vector<ProjectileActor> &,
                      std::int32_t magnitude, std::int32_t localOwner,
                      const ProjectileServices &);
} // namespace mnm::reconstruction::casting
