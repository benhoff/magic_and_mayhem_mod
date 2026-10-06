#pragma once
#include "animation.hpp"
#include "persistence.hpp"

namespace mnm::assets {
// Native complete-entry policy for the recovered effectani.cfg producer.
// Printer codes retain the original numeric values; Data1 remains opaque.
struct EffectAnimationRecipe {
    std::uint32_t sequence=0,assetIndex=0,printer=0,data1=0;
};
struct EffectAnimationRecipes {
    std::uint32_t fileCount=0;
    std::vector<EffectAnimationRecipe> entries;
};
struct EffectAnimationCatalog {
    EffectAnimationRecipes recipes;
    std::vector<Animation> animations;
    std::vector<std::string> spritePaths;
};
struct EffectAnimationCatalogLimits {
    std::uint32_t entries=4096,files=64;
    AnimationLimits animation;
};
// Throws invalid_argument on missing/unknown/malformed fields. No legacy
// uninitialized-word or permissive atoi behavior is emulated.
EffectAnimationRecipes readEffectAnimationRecipes(const Config&,
    const EffectAnimationCatalogLimits& = {});
// All-or-nothing owned load; handles close before return, errors retain path.
EffectAnimationCatalog loadEffectAnimationCatalog(const AssetStore&,
    const EffectAnimationCatalogLimits& = {});
}
