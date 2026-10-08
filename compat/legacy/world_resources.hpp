#pragma once
#include "world_frame.hpp"
#include <set>
namespace mnm::legacy {
struct WorldAssetFile {assets::ResourceId id;std::string path;QByteArray sha;};
// CPU-only immutable catalogue; construction/lookup may run on a worker.
class WorldCatalogue final {
public:
    explicit WorldCatalogue(const assets::AssetStore&,const std::function<void()>& checkpoint={});
    std::vector<WorldAssetFile> needed(const WorldFrame&) const;
private:
    std::vector<WorldAssetFile> files_;
    std::map<QByteArray,std::size_t> index_;
};
// Bounded native catalogue; decode and bind only files actually requested.
class WorldResources final {
public:
    WorldResources(const assets::AssetStore&,assets::ResourceManager&);
    std::vector<render::SceneDraw> display(const WorldFrame&);
private:
    WorldCatalogue catalogue_;
    std::set<assets::ResourceId> bound_;
    SnapshotResources bindings_;
};
}
