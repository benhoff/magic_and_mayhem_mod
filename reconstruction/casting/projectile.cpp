#include "projectile.hpp"
#include <stdexcept>
namespace mnm::reconstruction::casting {
namespace {
void validate(const ProjectileEffect &e, const std::vector<ProjectileActor> &a) {
  if (e.type != 3 || a.size() > 65536 ||
      (e.cachedActor && *e.cachedActor >= a.size()))
    throw std::invalid_argument("projectile outside type3 actor domain");
}
std::int32_t signedBits(std::uint32_t n) {
  return n <= INT32_MAX ? std::int32_t(n) : -1 - std::int32_t(~n);
}
}
void impactProjectile(ProjectileEffect &e, std::vector<ProjectileActor> &actors,
                      std::int32_t magnitude, const ProjectileServices &s) {
  validate(e, actors);
  if (e.target == UINT32_MAX)
    throw std::invalid_argument("terrain projectile impact remains separate");
  if (e.cachedActor)
    s.damage(actors.at(*e.cachedActor), magnitude, e.attribution, e.contributions);
}
void updateProjectile(ProjectileEffect &e, std::vector<ProjectileActor> &actors,
                      std::int32_t magnitude, std::int32_t localOwner,
                      const ProjectileServices &s) {
  validate(e, actors);
  const auto result = s.motion(e);
  --e.remaining; // DWORD wrapping, then signed comparison in the timeout branch.
  if (result == 1) {
    e.source = UINT32_MAX;
    s.bounce(e, true);
  } else if (result == 0) {
    if (e.target >= actors.size())
      throw std::invalid_argument("projectile motion produced invalid actor");
    e.cachedActor = e.target;
    const auto old = actors[e.target].health;
    impactProjectile(e, actors, magnitude, s);
    auto &a = actors.at(*e.cachedActor);
    if (e.target != UINT32_MAX) {
      a.recentAttacker = e.attacker + 1;
      a.recentTime = 100;
    }
    e.source = e.target;
    if (old > a.health && localOwner >= 0 && localOwner < 8 &&
        a.visible[unsigned(localOwner)])
      s.visibleDamage(a);
  } else if (result == 2 || signedBits(e.remaining) <= 0) {
    s.release(e);
  } else {
    while (s.animationTick() == 1)
      s.animationRestart();
    if (e.spell == 73) {
      if (e.audio)
        s.followAudio(e);
      else
        s.startAudio(e);
    }
  }
}
} // namespace mnm::reconstruction::casting
