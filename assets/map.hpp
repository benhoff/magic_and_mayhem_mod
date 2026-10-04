#pragma once
#include "persistence.hpp"
namespace mnm::assets {
struct MapCell {
    std::uint16_t definition=0;
    std::array<std::uint16_t,3> references{};
    std::uint16_t flags8=0,flags10=0;
};
struct MapAsset {
    std::uint32_t width=0,height=0,layers=0;
    // Header DWORDs +24..+75; retained without assigning speculative meanings.
    std::array<std::uint32_t,13> metadata{};
    std::vector<MapCell> cells;
    const MapCell& cell(std::uint32_t x,std::uint32_t y,std::uint32_t z) const;
};
struct MapLimits {
    std::uint32_t inputBytes=16*1024*1024,decodedBytes=16*1024*1024;
    std::uint32_t dimension=128,layers=32,cells=128*128*32;
};
using MapResult=PersistenceResult<MapAsset>;
// Version-6 payload only; older versions require separate conversion evidence.
MapResult decodeMapPayload(const Bytes&,const MapLimits& = {});
MapResult decodeMap(const Bytes&,const MapLimits& = {});
MapResult loadMap(AssetFile&,const MapLimits& = {});
}
