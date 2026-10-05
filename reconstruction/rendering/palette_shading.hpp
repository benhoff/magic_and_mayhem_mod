#pragma once
#include <array>
#include <cstdint>
#include <vector>
#include <optional>
namespace mnm::reconstruction {
using PaletteRgb = std::array<std::array<std::uint8_t,3>,256>;
using PaletteWords = std::array<std::uint16_t,256>;
// Recovered mode 0 builder (00582440), explicit configuration, owned tables.
// This is separate from generating spatial terrain light values.
struct PaletteShadingConfig {
 unsigned count=16;
 int intensityLevel=1,saturationLevel=1;
 double intensityPower=2,saturationPower=2;
};
struct PaletteLightingOverrides {
 std::optional<int> lightCurve,colourFactor;
 std::optional<double> lightPower,colourPower;
};
// Selected GLOBAL_OPTIONS admission: levels 1..1000, powers 0..1000.
// Missing keys preserve supplied state. Nonfinite inputs are refused.
PaletteShadingConfig applyPaletteLighting(PaletteShadingConfig,const PaletteLightingOverrides&);
struct ShadedPalette {
 unsigned shift=0,neutral=0;
 std::vector<PaletteWords> tables;
 std::size_t tableIndex(std::int32_t shade) const;
};
ShadedPalette buildShadedPalette(const PaletteRgb&,PaletteShadingConfig={},bool rgb555=false);
}
