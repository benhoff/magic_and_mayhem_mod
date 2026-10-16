#include "installed.hpp"
#include "../../assets/effect_animation_catalog.hpp"
#include "../../reconstruction/rendering/effect_animation_catalog_binding.hpp"
#include "../../reconstruction/rendering/effect_animation_selection.hpp"
#include <stdexcept>
namespace mnm::cure {
CureAnimationDefinition
loadInstalledCureAnimation(const assets::AssetStore &store) {
  const auto catalog = assets::loadEffectAnimationCatalog(store);
  const auto ordinal =
      reconstruction::selectEffectAnimation(77, 41, {}, UINT32_MAX, {});
  const auto binding =
      reconstruction::bindEffectAnimationCatalog(ordinal, catalog);
  const auto &a = catalog.animations.at(binding.entry.assetIndex);
  const auto first = a.starts.at(binding.entry.sequence),
             end = a.starts.at(binding.entry.sequence + 1);
  return {binding.entry, {a.records.begin() + first, a.records.begin() + end}};
}
} // namespace mnm::cure
