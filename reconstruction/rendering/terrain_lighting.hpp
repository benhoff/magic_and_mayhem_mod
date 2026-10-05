#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <vector>
namespace mnm::reconstruction {
struct TerrainLightingConfig { int ambient=-108,ramp=-16; };
// GLOBAL_OPTIONS: AmbientLight is clamped then negated; LightRamp is clamped.
TerrainLightingConfig applyTerrainLighting(TerrainLightingConfig,std::optional<int> ambient,std::optional<int> ramp);
struct TerrainLightSource {unsigned column=0,row=0,layer=0,size=17;};
class TerrainLightField {
public:
 TerrainLightField(unsigned width,unsigned height,unsigned mapLayers,TerrainLightingConfig = {});
 // Selected 004f2140 stamp: wrapped XY, clipped paired Z planes, signed maxima.
 // Sizes 2..17; dimensions must cover the kernel's extent. No entity admission.
 void stamp(TerrainLightSource);
 // Explicit immediate publication; original smoothing/tick branches are separate.
 void publish();
 std::int8_t at(unsigned column,unsigned row,unsigned mapLayer) const;
 const std::array<std::vector<std::int8_t>,5>& buffers() const{return buffers_;}
 const std::array<std::vector<std::int8_t>,18>& kernels() const{return kernels_;}
private:
 unsigned width_,height_,mapLayers_;
 TerrainLightingConfig config_;
 // Original +7cc,+7d0,+7d4,+7d8,+7dc: published, prior, target, work, base.
 std::array<std::vector<std::int8_t>,5> buffers_;
 std::array<std::vector<std::int8_t>,18> kernels_;
};
}
