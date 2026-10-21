#pragma once
#include "entity_lighting.hpp"
#include "palette_binding.hpp"
#include "source_snapshot.hpp"
namespace mnm::scene {
struct SceneAssetBinding {
  assets::ResourceId id;
  assets::ResourceRecipe recipe;
};
struct SceneManifest {
  std::optional<std::string> map, mps, terrainAnimation;
  assets::ResourceId terrain{assets::ResourceKind::terrain, "terrain"};
  std::vector<SceneAssetBinding> assets;
  std::vector<SceneTypeProfile> types;
  std::vector<ObjectLightProfile> objectLights;
  assets::EffectLightingFields effectLights;
  reconstruction::TerrainLightingConfig lighting;
  reconstruction::PaletteShadingConfig shading;
  reconstruction::PaletteEffectProfile effects;
  std::vector<unsigned> paletteModes{0};
  std::optional<std::size_t> paletteOrdinal;
  reconstruction::TerrainCamera camera = [] {
    reconstruction::TerrainCamera value;
    value.span = 0;
    value.cutLevel = 0;
    return value;
  }();
  reconstruction::TerrainViewport viewport;
  reconstruction::TerrainAdmission admission;
  int lightPlayer = 0;
  bool creatureLights = true, objectLightsEnabled = true, visibility = true;
  bool useLighting = false, autoFocus = true;
  // Explicit animation registry order for native command snapshots.
  std::vector<assets::ResourceId> nativeAnimations;
};
// Strict version1 owned declaration. Asset paths are resolved by AssetStore;
// no executable offsets, pointer identities or implicit gameplay defaults.
SceneManifest decodeSceneManifest(const assets::Bytes &,
                                  const assets::AssetStore &);
} // namespace mnm::scene
