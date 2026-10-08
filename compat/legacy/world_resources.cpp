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
WorldResources::WorldResources(const assets::AssetStore& store,assets::ResourceManager& resources):bindings_(store,resources){
    std::vector<std::filesystem::path> paths;
    for(const auto& entry:std::filesystem::recursive_directory_iterator(store.root())){
        if(!entry.is_regular_file())continue;
        auto extension=entry.path().extension().string();
        std::transform(extension.begin(),extension.end(),extension.begin(),[](unsigned char c){return char(std::tolower(c));});
        if(extension==".spr")paths.push_back(entry.path());}
    std::sort(paths.begin(),paths.end());if(paths.size()>256)throw std::runtime_error("Native SPR catalogue file limit");
    std::size_t frames=0,bytes=0;
    for(const auto& path:paths){const auto relative=path.lexically_relative(store.root()).generic_string();auto opened=store.open(relative);
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
        const auto file=files_.size();files_.push_back({relative,QCryptographicHash::hash(raw,QCryptographicHash::Sha256).toHex()});
        for(quint32 i=0;i<n;++i){const quint64 at=base+get(raw,qsizetype(table+i*4));
            if(at>quint64(raw.size()-40))throw std::runtime_error("Native SPR catalogue frame header");
            const auto size=get(raw,qsizetype(at));if(size<40||size>MNM_WORLD_MAX_FRAME||at+size>quint64(raw.size()))throw std::runtime_error("Native SPR catalogue frame extent");
            index_.emplace(frameIdentity(raw.mid(qsizetype(at),size),palettes!=0),file);}
    }
}
std::vector<render::SceneDraw> WorldResources::display(const WorldFrame& frame){
    std::set<std::size_t> needed;
    for(const auto& draw:frame.draws){const auto it=index_.find(frameIdentity(draw.frame.encoded,draw.frame.indexed));
        if(it==index_.end())throw std::runtime_error("Unmapped complete native World frame");
        needed.insert(it->second);}
    for(auto i:needed)if(!bound_.count(i)){const auto& f=files_[i];bindings_.add({assets::ResourceKind::ui,"world/"+std::to_string(i)},f.path,f.sha);bound_.insert(i);}
    return worldDisplay(frame,bindings_);
}
}
