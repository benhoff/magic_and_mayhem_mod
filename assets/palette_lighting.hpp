#pragma once
#include "persistence.hpp"
namespace mnm::assets {
// Selected global palette controls, owned and optional. No rendering policy.
struct PaletteLightingFields {
 std::optional<int> lightCurve,colourFactor;
 std::optional<double> lightPower,colourPower;
};
PersistenceResult<PaletteLightingFields> decodePaletteLightingFields(const Config&);
PersistenceResult<PaletteLightingFields> loadPaletteLightingFields(AssetFile&,bool packed=true,const PersistenceLimits& = {});
}
