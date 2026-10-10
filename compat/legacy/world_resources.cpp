#include "world_resources.hpp"
#include <QCryptographicHash>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include "../../protocols/include/mnm/world_frame_v1.h"
namespace mnm::legacy {
namespace {
quint32 get(const QByteArray& b,qsizetype at){if(at<0||at>b.size()-4)throw std::runtime_error("Native SPR catalogue extent");const auto* p=reinterpret_cast<const unsigned char*>(b.constData()+at);return p[0]|quint32(p[1])<<8|quint32(p[2])<<16|quint32(p[3])<<24;}
}
WorldIdentityCache::WorldIdentityCache(WorldIdentityLimits limits):limits_(limits){
    if(!limits.frames||limits.frames>4096||!limits.bytes||limits.bytes>16*1024*1024)
        throw std::invalid_argument("World identity cache limits outside bounds");
}
SnapshotFrameIdentity WorldIdentityCache::identity(const SnapshotFrame& frame){
    const auto found=index_.find({frame.indexed,frame.encoded});
    if(found!=index_.end()){
        ++stats_.hits;lru_.splice(lru_.end(),lru_,found->second);return found->second->identity;
    }
    SnapshotFrameIdentity identity(frame);++stats_.hashes;
    // Always copy fromRawData/borrowed storage too; QByteArray assignment alone
    // may retain a caller's external buffer after that caller changes/frees it.
    QByteArray bytes(frame.encoded.constData(),frame.encoded.size());
    const auto size=std::size_t(bytes.capacity());
    if(size>limits_.bytes){++stats_.bypasses;return identity;}
    while(stats_.frames>=limits_.frames||size>limits_.bytes-stats_.bytes){
        auto oldest=lru_.begin();index_.erase(oldest->key);stats_.bytes-=oldest->bytes;
        --stats_.frames;++stats_.evictions;lru_.erase(oldest);
    }
    lru_.push_back({{frame.indexed,std::move(bytes)},identity,size});
    try{index_.emplace(lru_.back().key,std::prev(lru_.end()));}
    catch(...){lru_.pop_back();throw;}
    ++stats_.frames;stats_.bytes+=size;return identity;
}
WorldCatalogue::WorldCatalogue(const assets::AssetStore& store,const std::function<void()>& checkpoint){
    std::vector<std::filesystem::path> paths;
    for(const auto& entry:std::filesystem::recursive_directory_iterator(store.root())){
        if(checkpoint)checkpoint();
        if(!entry.is_regular_file())continue;
        auto extension=entry.path().extension().string();
        std::transform(extension.begin(),extension.end(),extension.begin(),[](unsigned char c){return char(std::tolower(c));});
        if(extension==".spr")paths.push_back(entry.path());}
    std::sort(paths.begin(),paths.end());if(paths.size()>256)throw std::runtime_error("Native SPR catalogue file limit");
    std::size_t frames=0,bytes=0;
    for(const auto& path:paths){const auto relative=path.lexically_relative(store.root()).generic_string();auto opened=store.open(relative);
        if(checkpoint)checkpoint();
        if(auto* e=std::get_if<assets::Error>(&opened))throw std::runtime_error(e->detail);
        auto fileHandle=std::get<std::unique_ptr<assets::AssetFile>>(std::move(opened));
        auto loaded=assets::readWhole(*fileHandle,32*1024*1024);
        if(auto* e=std::get_if<assets::Error>(&loaded))throw std::runtime_error(e->detail);
        const auto& v=std::get<std::vector<std::uint8_t>>(loaded);QByteArray raw(reinterpret_cast<const char*>(v.data()),qsizetype(v.size()));
        bytes+=v.size();if(bytes>512*1024*1024)throw std::runtime_error("Native SPR catalogue byte limit");
        // Other installed sprite formats are outside this catalogue. Any
        // request for them remains unmapped and refuses the whole live frame.
        if(raw.size()<24||get(raw,8)!=4)continue;
        const auto n=get(raw,12),palettes=get(raw,16);frames+=n;
        const quint64 table=24+quint64(palettes)*768,base=table+quint64(n)*4;
        if(frames>524288||base>quint64(raw.size()))throw std::runtime_error("Native SPR catalogue table limit");
        const auto file=files_.size();files_.push_back({{assets::ResourceKind::ui,"world/"+std::to_string(file)},relative,QCryptographicHash::hash(raw,QCryptographicHash::Sha256).toHex()});
        for(quint32 i=0;i<n;++i){const quint64 at=base+get(raw,qsizetype(table+i*4));
            if(checkpoint)checkpoint();
            if(at>quint64(raw.size()-40))throw std::runtime_error("Native SPR catalogue frame header");
            const auto size=get(raw,qsizetype(at));if(size<40||size>MNM_WORLD_MAX_FRAME||at+size>quint64(raw.size()))throw std::runtime_error("Native SPR catalogue frame extent");
            index_.emplace(frameIdentity(raw.mid(qsizetype(at),size),palettes!=0),file);}
    }
}
std::vector<WorldAssetFile> WorldCatalogue::needed(const WorldFrame& frame) const{
    std::vector<SnapshotFrameIdentity> identities;identities.reserve(frame.draws.size());
    for(const auto& draw:frame.draws)if(!draw.additive&&!draw.colourRectangle)identities.emplace_back(draw.frame);
    return needed(identities);
}
std::vector<WorldAssetFile> WorldCatalogue::needed(const std::vector<SnapshotFrameIdentity>& identities) const{
    std::set<std::size_t> needed;
    for(const auto& identity:identities){const auto it=index_.find(identity.digest());
        if(it==index_.end())throw std::runtime_error("Unmapped complete native World frame");
        needed.insert(it->second);}
    std::vector<WorldAssetFile> result;for(auto i:needed)result.push_back(files_[i]);return result;
}
WorldResources::WorldResources(const assets::AssetStore& store,assets::ResourceManager& resources):catalogue_(store),resources_(resources),bindings_(store,resources){}
std::vector<render::SceneDraw> WorldResources::display(const WorldFrame& frame){
    std::vector<SnapshotFrameIdentity> identities;identities.reserve(frame.draws.size());
    for(const auto& draw:frame.draws)if(!draw.additive&&!draw.colourRectangle)identities.push_back(identities_.identity(draw.frame));
    std::vector<WorldAssetFile> needed;
    for(const auto& f:catalogue_.needed(identities))if(!bound_.count(f.id))needed.push_back(f);
    for(const auto& f:needed){resources_.bind(f.id,{assets::ResourceImageFormat::sprite,f.path,{},{},{}});
        bindings_.addPrepared(assets::prepareResource(resources_.request(f.id),{},f.sha.toStdString()),f.sha);bound_.insert(f.id);}
    std::vector<render::SceneDraw> draws;draws.reserve(frame.draws.size());std::size_t i=0;
    for(const auto& draw:frame.draws){
        if(draw.additive||draw.colourRectangle){draws.push_back(worldPrimitiveDraw(draw));continue;}
        const auto bound=bindings_.resolve(identities.at(i++),true);
        draws.push_back({bound.resource,bound.frame,draw.x,draw.y,true,true,draw.colours,draw.clip,draw.composite});
    }
    return draws;
}
}
