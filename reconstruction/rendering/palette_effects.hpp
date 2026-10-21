#pragma once
#include "palette_shading.hpp"
namespace mnm::reconstruction {
// Mode numbers are the original dispatcher slots, not a wire enum.
enum class PaletteEffect : unsigned {
 normal=0,blueTint=1,redBoost=2,greenBoost=3,stone=4,metal=5,
 desaturated=6,redTint=7,normalDword=8,blueTintDword=9
};
// Already decoded finite profile values. Missing values use these original
// per-builder defaults. Every supplied scalar must be finite and in[-1000,1000].
// SpecalValue preserves the original misspelled key.
struct PaletteEffectProfile {
 double stoneReducer=.5,stoneSpecalValue=120,stoneDarkest=5;
 double metalReducer=.1,metalSpecalValue=80,metalDarkest=5,metalBlueTint=30,metalContrast=0;
 double redTintReducer=.25;
};
struct PaletteEffectResult {
 unsigned mode=0,originalEntryBytes=2;
 ShadedPalette palette;
 // Mode5 changes its caller's RGB bytes before building. Own that result so
 // a chain can pass it to later builders without changing shared asset inputs.
 PaletteRgb source;
};
// All original DWORD entries contain zero-extended packed16 values. Native
// storage owns those exact values as words and records the original stride.
// Bounds/nonfinite policies are explicit; no original heap overrun is copied.
PaletteEffectResult buildPaletteEffect(const PaletteRgb&,unsigned mode,
 PaletteShadingConfig={},const PaletteEffectProfile& = {},bool rgb555=false);
}
