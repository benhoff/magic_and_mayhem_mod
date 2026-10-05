#pragma once
#include "persistence.hpp"
namespace mnm::assets {
// Selected global palette controls, owned and optional. No rendering policy.
struct PaletteLightingFields {
 std::optional<int> lightCurve,colourFactor;
 std::optional<double> lightPower,colourPower;
};
struct TerrainLightingFields { std::optional<int> ambientLight,lightRamp; };
PersistenceResult<TerrainLightingFields> decodeTerrainLightingFields(const Config&);
PersistenceResult<TerrainLightingFields> loadTerrainLightingFields(AssetFile&,const PersistenceLimits& = {});
// Selected VIDEO preference; parsing does not clamp or choose rendering policy.
struct TerrainPalettePreference { std::optional<int> lightLevels; };
PersistenceResult<TerrainPalettePreference> decodeTerrainPalettePreference(const Config&);
PersistenceResult<TerrainPalettePreference> loadTerrainPalettePreference(AssetFile&,const PersistenceLimits& = {});
PersistenceResult<PaletteLightingFields> decodePaletteLightingFields(const Config&);
PersistenceResult<PaletteLightingFields> loadPaletteLightingFields(AssetFile&,bool packed=true,const PersistenceLimits& = {});
}
