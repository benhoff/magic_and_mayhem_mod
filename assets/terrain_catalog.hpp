#pragma once
#include "asset_file.hpp"
#include <array>
namespace mnm::assets {
// TTD version 4 records remain owned opaque bytes. Rendering field meanings
// belong to the build-specific reconstruction, not the asset service.
struct TerrainCatalog {std::vector<std::array<std::uint8_t,356>> records;};
struct TerrainCatalogError {std::string detail;std::optional<Error> input;};
using TerrainCatalogResult=std::variant<TerrainCatalog,TerrainCatalogError>;
TerrainCatalogResult decodeTerrainCatalog(const std::vector<std::uint8_t>&);
TerrainCatalogResult loadTerrainCatalog(AssetFile&);
}
