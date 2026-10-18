#include "cure_effect.hpp"
#include <stdexcept>

namespace mnm::reconstruction::casting {
void applyCureEffect(CureEffect &effect, std::vector<CureActor> &actors,
                     const CureConfiguration &configuration,
                     const CureServices &services) {
  if (effect.spell != 41 || actors.size() > 65536 ||
      (effect.source != UINT32_MAX && effect.source >= actors.size()) ||
      (effect.cachedActor && *effect.cachedActor >= actors.size()))
    throw std::invalid_argument("Cure projection domain");

  auto fail = [&] {
    services.cue(0x3f3, effect.position);
    if (effect.source != UINT32_MAX && effect.deliveryMode != 4) {
      effect.cachedActor = effect.source;
      auto &source = actors.at(effect.source);
      if (services.chargeable(source))
        services.resource(source, configuration.cost << 8);
    }
    services.release(effect);
  };
  if (effect.target == UINT32_MAX) {
    fail();
    return;
  }
  if (effect.type == 6 || effect.type == 7 || effect.type == 11 ||
      effect.type == 30) {
    services.transition(effect, 77);
    return;
  }
  effect.cachedActor = effect.target < actors.size()
                           ? std::optional<std::uint32_t>(effect.target)
                           : std::nullopt;
  if (!effect.cachedActor || !actors[*effect.cachedActor].health) {
    fail();
    return;
  }
  auto &target = actors[*effect.cachedActor];
  if (target.blocked)
    return;
  const auto oldHealth = target.health, oldStatus = target.status,
             oldOngoing = target.ongoingDamage;
  services.health(target, configuration.magnitude, effect.attribution,
                  effect.contributions, 1);
  services.cleanse(target);
  target.ongoingDamage = 0;
  services.cue(0x2dc, effect.position);
  if (configuration.localOwner >= 0 && configuration.localOwner < 8 &&
      target.visible[unsigned(configuration.localOwner)] &&
      (oldHealth < target.health || oldStatus || oldOngoing))
    services.feedback(effect, target);
}
} // namespace mnm::reconstruction::casting
