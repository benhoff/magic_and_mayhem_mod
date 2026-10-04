#pragma once
#include "../../assets/map.hpp"
#include "../../assets/terrain_catalog.hpp"
namespace mnm::reconstruction {
// Selected 004ef6b0 ordinary-cell branch. Copies a square from layer zero;
// source assets remain immutable. Rejects the object rotation branch (flags10&8).
// Validates every selected cell and extent before changing destination.
void copyTerrainSection(assets::MapAsset& destination,const assets::MapAsset& source,
 const assets::TerrainCatalog&,unsigned side,unsigned sourceX,unsigned sourceY,
 unsigned destinationX,unsigned destinationY,unsigned rotation);
struct TerrainSection {
 const assets::MapAsset* source=nullptr;
 unsigned sourceX=0,sourceY=0,column=0,row=0,rotation=0;
};
// Explicit complete grid policy; no random selection, overlap, holes or entities.
assets::MapAsset assembleTerrainRegion(unsigned columns,unsigned rows,unsigned side,
 unsigned layers,const std::vector<TerrainSection>&,const assets::TerrainCatalog&);
}
