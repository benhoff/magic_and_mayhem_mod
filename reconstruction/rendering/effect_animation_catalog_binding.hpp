#pragma once
#include "effect_animation_binding.hpp"
#include "effect_animation_catalog.hpp"
namespace mnm::reconstruction {
// The asset service owns native recipes; this adapter assigns their words to
// the recovered binding contract. No reverse dependency on reconstruction.
EffectAnimationBinding bindEffectAnimationCatalog(std::uint32_t ordinal,
    const assets::EffectAnimationCatalog& catalog);
}
