#pragma once
#include "scene_renderer.hpp"
#include <QByteArray>
#include <map>

namespace mnm::legacy {
struct SnapshotRecord {
    std::uint32_t token=0;
    std::int32_t depth=0,x=0,y=0,kind=0;
    std::uint32_t parameters=0,sentinels=0,flags=0;
};
struct SnapshotFrame { bool indexed=false; QByteArray encoded; };
struct SceneSnapshot {
    std::uint32_t sequence=0,view=0,mode=0,width=0,height=0;
    std::vector<SnapshotRecord> records;
    std::vector<SnapshotFrame> frames;
};
SceneSnapshot decodeSceneSnapshot(const QByteArray&);
QByteArray frameIdentity(const QByteArray&,bool indexed);
QByteArray spriteVisualIdentity(const assets::Sprite&,const assets::SpriteFrame&);
struct BoundFrame { assets::ResourceId resource; std::size_t frame=0; };
struct SnapshotBindingStats { std::uint64_t visualChecks=0,visualReuses=0; };
// Explicit pinned candidate files. Original pointers never become ResourceIds.
class SnapshotResources final {
public:
    SnapshotResources(const assets::AssetStore&,assets::ResourceManager&);
    void add(const assets::ResourceId&,const std::string& sprite,const QByteArray& expectedSha256);
    BoundFrame resolve(const SnapshotFrame&,bool ownedColours=false) const;
    SnapshotBindingStats stats() const { return stats_; }
private:
    struct Candidate {BoundFrame binding;QByteArray visual;mutable std::uint64_t checkedRevision=0;};
    const assets::AssetStore& store_;
    assets::ResourceManager& resources_;
    std::map<QByteArray,std::vector<Candidate>> index_;
    std::size_t files_=0,frames_=0;
    mutable SnapshotBindingStats stats_;
};
struct SnapshotDisplay {
    struct Gap {std::size_t record;std::int32_t kind;std::uint32_t token;std::string reason;};
    std::vector<render::SceneDraw> draws;
    std::vector<Gap> gaps;
    std::size_t hidden=0,unsupported=0,unmapped=0,shaded=0;
    bool complete() const {return !unsupported&&!unmapped;}
};
// Explicit unshaded diagnostic policy. Strict mapping refuses any visible gap;
// supportedOnly permits a labeled partial preview, preserving relative order.
SnapshotDisplay snapshotDisplay(const SceneSnapshot&,const SnapshotResources&,bool supportedOnly=false);
}
