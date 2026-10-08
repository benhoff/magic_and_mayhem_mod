#include "scene_snapshot.hpp"
#include "../../protocols/include/mnm/scene_snapshot_v1.h"
#include <QCryptographicHash>
#include <cstring>
#include <stdexcept>

namespace mnm::legacy {
namespace {
std::uint32_t word(const QByteArray& b,qsizetype at){
    if(at<0||at>b.size()||b.size()-at<4)throw std::invalid_argument("Truncated scene snapshot");
    const auto* p=reinterpret_cast<const unsigned char*>(b.constData()+at);
    return p[0]|std::uint32_t(p[1])<<8|std::uint32_t(p[2])<<16|std::uint32_t(p[3])<<24;
}
std::int32_t signedWord(std::uint32_t n){std::int32_t v;std::memcpy(&v,&n,4);return v;}
QByteArray hash(const QByteArray& b){return QCryptographicHash::hash(b,QCryptographicHash::Sha256);}
QByteArray visual(const assets::Sprite& sprite,const assets::SpriteFrame& f){
    QByteArray out;out.reserve(qsizetype(f.opaqueMask.size()*3));
    for(auto word:{f.width,f.height,std::uint32_t(f.originX),std::uint32_t(f.originY)})
        for(unsigned i=0;i<4;++i)out.append(char(word>>(i*8)));
    for(std::size_t i=0;i<f.opaqueMask.size();++i){std::uint16_t pixel=0;
        if(f.opaqueMask[i]&&sprite.storage==assets::SpriteStorage::rgb565)pixel=std::get<std::vector<std::uint16_t>>(f.pixels)[i];
        else if(f.opaqueMask[i]){const auto c=sprite.palettes.at(*f.paletteIndex)[std::get<std::vector<std::uint8_t>>(f.pixels)[i]];
            pixel=std::uint16_t((c.red>>3)<<11|(c.green>>2)<<5|(c.blue>>3));}
        out.append(char(f.opaqueMask[i]));out.append(char(pixel));out.append(char(pixel>>8));
    }
    return hash(out);
}
}
QByteArray frameIdentity(const QByteArray& raw,bool indexed){
    if(raw.size()<40||raw.size()>MNM_SCENE_V1_MAX_FRAME||word(raw,0)!=std::uint32_t(raw.size()))
        throw std::invalid_argument("Invalid normalized frame extent");
    auto bytes=raw;bytes.replace(28,4,QByteArray(4,0));bytes.prepend(indexed?'\1':'\0');return hash(bytes);
}
SceneSnapshot decodeSceneSnapshot(const QByteArray& b){
    if(b.size()<64||b.size()>MNM_SCENE_V1_MAX_BYTES||b.first(8)!=MNM_SCENE_V1_MAGIC||word(b,8)!=1||word(b,12)!=64||word(b,16)!=std::uint32_t(b.size()))
        throw std::invalid_argument("Unsupported scene snapshot envelope");
    const auto count=word(b,24),blobs=word(b,28);
    if(!word(b,20)||count>MNM_SCENE_V1_MAX_DRAWS||blobs>MNM_SCENE_V1_MAX_BLOBS||word(b,32)>3||word(b,36)>1||word(b,40)!=MNM_SCENE_V1_BUILD||word(b,44)||word(b,48)!=64||word(b,52)!=64+count*32||word(b,52)>std::uint32_t(b.size()))
        throw std::invalid_argument("Invalid scene snapshot bounds/build/view");
    SceneSnapshot result;result.sequence=word(b,20);result.view=word(b,32);result.mode=word(b,36);result.width=word(b,56);result.height=word(b,60);
    if(result.width>2048||result.height>2048)throw std::invalid_argument("Invalid observed viewport extent");
    for(std::uint32_t i=0;i<count;++i){const qsizetype at=64+i*32;
        SnapshotRecord r{word(b,at),signedWord(word(b,at+4)),signedWord(word(b,at+8)),signedWord(word(b,at+12)),signedWord(word(b,at+16)),word(b,at+20),word(b,at+24),word(b,at+28)};
        if(r.flags>2u||r.token>blobs||(r.kind==-2)!=(r.flags==MNM_SCENE_V1_HIDDEN)||
           (r.flags==MNM_SCENE_V1_HIDDEN&&r.token)||
           (r.kind!=-2&&((r.flags==MNM_SCENE_V1_NO_FRAME)!=(r.token==0))))
            throw std::invalid_argument("Invalid snapshot record token/flags");
        result.records.push_back(r);
    }
    qsizetype at=word(b,52);
    for(std::uint32_t i=0;i<blobs;++i){const auto token=word(b,at),size=word(b,at+4),flags=word(b,at+8);
        if(token!=i+1||size<40||size>MNM_SCENE_V1_MAX_FRAME||flags>1||word(b,at+12)||b.size()-at-16<size)
            throw std::invalid_argument("Invalid snapshot blob admission");
        SnapshotFrame frame{bool(flags),b.mid(at+16,size)};
        if(word(frame.encoded,28))throw std::invalid_argument("Unnormalized legacy palette pointer");
        frameIdentity(frame.encoded,frame.indexed);result.frames.push_back(std::move(frame));at+=16+size;
    }
    if(at!=b.size())throw std::invalid_argument("Trailing snapshot bytes");
    return result;
}
SnapshotResources::SnapshotResources(const assets::AssetStore& store,assets::ResourceManager& resources):store_(store),resources_(resources){}
void SnapshotResources::add(const assets::ResourceId& id,const std::string& path,const QByteArray& expected){
    if(files_>=256)throw std::runtime_error("Snapshot candidate file budget exceeded");
    id.text();if(id.kind!=assets::ResourceKind::ui)throw std::invalid_argument("Snapshot bitmap bindings use the explicit UI observation namespace");
    auto opened=store_.open(path);if(const auto* e=std::get_if<assets::Error>(&opened))throw std::runtime_error(e->detail);
    auto file=std::get<std::unique_ptr<assets::AssetFile>>(std::move(opened));auto read=assets::readWhole(*file,32*1024*1024);file.reset();
    if(const auto* e=std::get_if<assets::Error>(&read))throw std::runtime_error(e->detail);
    const auto input=std::get<std::vector<std::uint8_t>>(std::move(read));
    const QByteArray bytes(reinterpret_cast<const char*>(input.data()),qsizetype(input.size()));
    if(expected.size()!=64||hash(bytes).toHex()!=expected)throw std::invalid_argument("Pinned snapshot SPR hash mismatch");
    auto decoded=assets::decodeSprite(input);if(const auto* e=std::get_if<assets::SpriteError>(&decoded))throw std::runtime_error(e->detail);
    const auto& sprite=std::get<assets::Sprite>(decoded);
    if(sprite.version!=4||frames_+sprite.frames.size()>65536)throw std::invalid_argument("Snapshot SPR version/frame budget exceeded");
    std::vector<std::pair<QByteArray,Candidate>> additions;
    for(std::size_t i=0;i<sprite.frames.size();++i){const auto& f=sprite.frames[i];
        additions.push_back({frameIdentity(bytes.mid(f.sourceOffset,f.encodedSize),sprite.storage==assets::SpriteStorage::indexed8),{{id,i},visual(sprite,f)}});
    }
    resources_.bind(id,{assets::ResourceImageFormat::sprite,path,{},{},{}});
    for(auto& addition:additions)index_[addition.first].push_back(std::move(addition.second));
    ++files_;frames_+=sprite.frames.size();
}
BoundFrame SnapshotResources::resolve(const SnapshotFrame& frame,bool ownedColours) const{
    const auto it=index_.find(frameIdentity(frame.encoded,frame.indexed));if(it==index_.end())throw std::out_of_range("Unmapped observed frame content");
    const auto& candidates=it->second;
    for(const auto& c:candidates)if(c.visual!=candidates.front().visual&&!(ownedColours&&frame.indexed))
        throw std::out_of_range("Ambiguous observed frame with different native palette pixels");
    const auto& choice=candidates.front();const auto& owned=resources_.load(choice.binding.resource);
    const auto& sprite=std::get<assets::Sprite>(owned.image);
    if(choice.binding.frame>=sprite.frames.size()||visual(sprite,sprite.frames[choice.binding.frame])!=choice.visual)
        throw std::out_of_range("Native resource changed since pinned identity indexing");
    return choice.binding; // Exact visual aliases use manifest order/lowest frame.
}
SnapshotDisplay snapshotDisplay(const SceneSnapshot& snapshot,const SnapshotResources& resources,bool supportedOnly){
    SnapshotDisplay result;
    for(std::size_t i=0;i<snapshot.records.size();++i){const auto& r=snapshot.records[i];
        if(r.kind==-2){++result.hidden;continue;}
        // Other kinds can blend, clip against depth sentinels or use special effects.
        if(!r.token||(r.kind!=0&&r.kind!=33)||r.sentinels!=0x8ad08ad0){++result.unsupported;
            result.gaps.push_back({i,r.kind,r.token,!r.token?"No readable frame":r.sentinels!=0x8ad08ad0?"Original depth-sentinel adjustment pending":"Original draw mode pending"});continue;}
        try{const auto bound=resources.resolve(snapshot.frames.at(r.token-1));
            result.draws.push_back({bound.resource,bound.frame,r.x,r.y,true,true,{}});
            if(std::int16_t(r.parameters&0xffff)!=0)++result.shaded;
        }catch(const std::out_of_range& e){++result.unmapped;result.gaps.push_back({i,r.kind,r.token,e.what()});}
    }
    if(!supportedOnly&&!result.complete())throw std::invalid_argument("Incomplete snapshot resource/mode mapping; explicit partial preview required");
    return result;
}
}
