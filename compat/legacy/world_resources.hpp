#pragma once
#include "world_frame.hpp"
#include <set>
namespace mnm::legacy {
// Bounded native catalogue; decode and bind only files actually requested.
class WorldResources final {
public:
    WorldResources(const assets::AssetStore&,assets::ResourceManager&);
    std::vector<render::SceneDraw> display(const WorldFrame&);
private:
    struct File {std::string path;QByteArray sha;};
    std::vector<File> files_;
    std::map<QByteArray,std::size_t> index_;
    std::set<std::size_t> bound_;
    SnapshotResources bindings_;
};
}
