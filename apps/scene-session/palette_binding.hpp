#pragma once
#include "../../assets/profile.hpp"
#include "../../assets/resources/resource_manager.hpp"
#include "../../renderer/scenes/scene_renderer.hpp"
#include "../../reconstruction/rendering/palette_effect_chain.hpp"
namespace mnm::scene {
// The existing owned ASCII profile service supplies 255-byte string values.
// Numeric conversion is locale independent, with the original decimal-prefix
// behavior; nonfinite/out-of-domain values are rejected before use.
reconstruction::PaletteEffectProfile paletteEffectProfile(const assets::ProfileSnapshot&);
std::vector<reconstruction::PaletteRgb> paletteSources(const assets::Sprite&);
struct PaletteBindingLimits {
 std::size_t resources=128;
 std::uint64_t bytes=128ULL*1024*1024;
 reconstruction::PaletteEffectChainLimits perResource{};
};
// Application orchestration. The manager outlives this service. No widgets,
// original pointers or original pixel snapshots enter the owned palette chains.
class ScenePaletteBindings {
 struct Binding {std::uint64_t revision;reconstruction::PaletteEffectChain chain;};
 assets::ResourceManager& resources_;
 PaletteBindingLimits limits_;
 std::map<assets::ResourceId,Binding> bindings_;
 std::uint64_t bytes_=0;
 std::thread::id thread_;
 void checkThread() const;
 Binding& current(const assets::ResourceId&);
 const Binding& current(const assets::ResourceId&) const;
public:
 explicit ScenePaletteBindings(assets::ResourceManager&,PaletteBindingLimits={});
 ScenePaletteBindings(const ScenePaletteBindings&)=delete;
 ScenePaletteBindings& operator=(const ScenePaletteBindings&)=delete;
 void bind(const assets::ResourceId&);
 void retire(const assets::ResourceId&);
 void clear();
 std::optional<std::size_t> append(const assets::ResourceId&,unsigned,
     reconstruction::PaletteShadingConfig={},const reconstruction::PaletteEffectProfile& = {});
 void apply(render::SceneDraw&,std::size_t ordinal,std::int32_t shade) const;
 std::uint64_t retainedBytes() const {checkThread();return bytes_;}
};
}
