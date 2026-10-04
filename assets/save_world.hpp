#pragma once
#include "persistence.hpp"
namespace mnm::assets {
// Byte ranges into owned raw input, never legacy host pointers. Groups may overlap
// their children; record extents are explicit and field semantics remain raw.
struct WorldBlock {
    std::string name;
    std::size_t offset=0,size=0;
    std::optional<std::uint32_t> parent;
};
struct SavedWorld {
    Bytes raw;
    std::string mapPath;
    std::array<std::uint32_t,4> globals{};
    std::uint32_t counter=0;
    std::vector<WorldBlock> blocks;
};
struct WorldLimits {
    std::uint64_t inputBytes=64*1024*1024,decodedBytes=96*1024*1024;
    std::uint32_t records=65536,blocks=200000;
};
PersistenceResult<SavedWorld> decodeWorldState(const Bytes&,const WorldLimits& = {});
PersistenceResult<SavedWorld> loadWorldState(AssetFile&,const WorldLimits& = {});
// Explicit structural decoding, independent of the permissive envelope reader.
PersistenceResult<SavedWorld> decodeSavedWorld(const SavedGame&,const WorldLimits& = {});
}
