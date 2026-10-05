#pragma once
#include "persistence.hpp"
#include "profile.hpp"
namespace mnm::assets {
// Owned results of the three 256-byte profile reads per FXA_0..FXA_88.
// Empty/missing values are equivalent at this loader boundary.
struct EffectLightingFields {
    std::array<std::array<std::string,3>,89> values{};
};
EffectLightingFields readEffectLightingFields(const ProfileSnapshot&);
PersistenceResult<EffectLightingFields> loadEffectLightingFields(AssetFile&,const PersistenceLimits& = {});
}
