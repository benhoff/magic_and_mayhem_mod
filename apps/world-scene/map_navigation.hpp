#pragma once
#include "map.hpp"
#include "creature_movement.hpp"
#include "animation.hpp"
#include "terrain_catalog.hpp"
#include <array>
#include <optional>
namespace mnm::scene {
struct MapCrop { unsigned x=0,y=0,width=8,height=8; };
struct MapNavigation {
    assets::MapAsset geometry;
    assets::Bytes frozen;
    std::vector<std::array<int,3>> standing;
    unsigned projectedObjects=0,projectedReferences=0,sealedCells=0;
};
// Native policy: ordinary full-source geometry, then crop all layers and seal
// its perimeter. Explicit synthetic one-cell creature profile; no entity load.
MapNavigation projectMapNavigation(const assets::MapAsset&,const assets::TerrainCatalog&,MapCrop);
MapNavigation projectCreatureMapNavigation(const assets::MapAsset&,const assets::TerrainCatalog&,MapCrop,const assets::CreatureMovementConfig&,const assets::Animation&);
assets::Bytes mapPayload(const assets::MapAsset&);
// Exact dimension, cell and TTD byte agreement with the owned frozen input.
// Refuse a mismatched visual map/catalog before rendering or stepping.
void validateMapGeometry(const assets::MapAsset&,const assets::TerrainCatalog&,const assets::Bytes& frozen,std::optional<std::uint64_t> fingerprint={});
}
