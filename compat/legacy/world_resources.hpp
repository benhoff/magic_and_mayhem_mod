#pragma once
#include "world_frame.hpp"
#include <set>
#include <list>
namespace mnm::legacy {
struct WorldAssetFile {assets::ResourceId id;std::string path;QByteArray sha;};
struct WorldIdentityLimits {std::size_t frames=4096,bytes=16*1024*1024;};
struct WorldIdentityStats {
    std::uint64_t hashes=0,hits=0,evictions=0,bypasses=0;
    std::size_t frames=0,bytes=0;
};
// Exact indexed tag + owned input bytes key this bounded LRU. No legacy pointer
// or caller-supplied digest is a cache key. Tokens remain valid after eviction.
class WorldIdentityCache final {
public:
    explicit WorldIdentityCache(WorldIdentityLimits limits={});
    WorldIdentityCache(const WorldIdentityCache&)=delete;
    WorldIdentityCache& operator=(const WorldIdentityCache&)=delete;
    SnapshotFrameIdentity identity(const SnapshotFrame&);
    WorldIdentityStats stats() const { return stats_; }
private:
    using Key=std::pair<bool,QByteArray>;
    struct Entry {Key key;SnapshotFrameIdentity identity;std::size_t bytes;};
    WorldIdentityLimits limits_;
    WorldIdentityStats stats_;
    std::list<Entry> lru_;
    std::map<Key,std::list<Entry>::iterator> index_;
};
// CPU-only immutable catalogue; construction/lookup may run on a worker.
class WorldCatalogue final {
public:
    explicit WorldCatalogue(const assets::AssetStore&,const std::function<void()>& checkpoint={});
    std::vector<WorldAssetFile> needed(const WorldFrame&) const;
    std::vector<WorldAssetFile> needed(const std::vector<SnapshotFrameIdentity>&) const;
private:
    std::vector<WorldAssetFile> files_;
    std::map<QByteArray,std::size_t> index_;
};
// Bounded native catalogue; decode and bind only files actually requested.
class WorldResources final {
public:
    WorldResources(const assets::AssetStore&,assets::ResourceManager&);
    std::vector<render::SceneDraw> display(const WorldFrame&);
    SnapshotBindingStats bindingStats() const { return bindings_.stats(); }
    WorldIdentityStats identityStats() const { return identities_.stats(); }
private:
    WorldCatalogue catalogue_;
    assets::ResourceManager& resources_;
    std::set<assets::ResourceId> bound_;
    SnapshotResources bindings_;
    WorldIdentityCache identities_;
};
}
