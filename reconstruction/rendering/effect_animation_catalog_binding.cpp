#include "effect_animation_catalog_binding.hpp"
#include <stdexcept>
namespace mnm::reconstruction {
EffectAnimationBinding bindEffectAnimationCatalog(std::uint32_t ordinal,const assets::EffectAnimationCatalog& catalog){
    if(ordinal>=catalog.recipes.entries.size())throw std::invalid_argument("Effect catalog ordinal outside entries");
    const auto& r=catalog.recipes.entries[ordinal];
    auto binding=bindEffectAnimation(0,{{r.sequence,r.assetIndex,r.printer,r.data1}},catalog.animations);
    binding.animationOrdinal=ordinal;return binding;
}
}
