#pragma once
#include "resource_manager.hpp"
#include <QTemporaryDir>
#include <algorithm>
#include <fstream>
#include <stdexcept>

namespace mnm::test {
using Bytes=std::vector<std::uint8_t>;
inline void word(Bytes& b,std::size_t at,std::uint32_t n){for(unsigned i=0;i<4;++i)b.at(at+i)=std::uint8_t(n>>(8*i));}
inline Bytes ani(const assets::Animation& a){
    Bytes b(44+a.starts.size()*4+a.records.size()*44,0);
    word(b,0,0x00494e41);word(b,4,b.size());word(b,8,a.records.size());word(b,12,5);word(b,16,a.opaqueHeader);word(b,20,a.starts.size());
    std::copy(a.spriteName.begin(),a.spriteName.end(),b.begin()+24);
    for(std::size_t i=0;i<a.starts.size();++i)word(b,44+i*4,a.starts[i]);
    for(std::size_t i=0;i<a.records.size();++i){const auto at=44+a.starts.size()*4+i*44;
        word(b,at,a.records[i].opcode);word(b,at+4,std::uint32_t(a.records[i].argument));
        for(unsigned j=0;j<9;++j)word(b,at+8+j*4,a.records[i].metadata[j]);}
    return b;
}
inline Bytes spr(const assets::Sprite& s){
    const auto base=24+s.palettes.size()*768+s.frames.size()*4;
    Bytes b(base,0);word(b,0,0x00525053);word(b,8,4);word(b,12,s.frames.size());word(b,16,s.palettes.size());
    for(std::size_t p=0;p<s.palettes.size();++p)for(std::size_t i=0;i<256;++i){const auto c=s.palettes[p][i];
        b[24+p*768+i*3]=c.red;b[25+p*768+i*3]=c.green;b[26+p*768+i*3]=c.blue;}
    for(std::size_t f=0;f<s.frames.size();++f){const auto& frame=s.frames[f];const auto start=b.size();
        word(b,24+s.palettes.size()*768+f*4,start-base);
        Bytes out(40+frame.height*8,0),deltas,pixels;word(out,4,frame.width);word(out,8,frame.height);
        word(out,12,std::uint32_t(frame.originX));word(out,16,std::uint32_t(frame.originY));word(out,28,frame.paletteIndex.value_or(0xffffffff));
        std::copy(frame.name.begin(),frame.name.end(),out.begin()+20);
        std::vector<std::size_t> rowPixels;
        for(unsigned y=0;y<frame.height;++y){word(out,40+y*8,out.size()+deltas.size());rowPixels.push_back(pixels.size());
            unsigned x=0;bool opaque=false;
            while(x<frame.width){unsigned run=0;
                while(run<255 && x+run<frame.width && bool(frame.opaqueMask.at(std::size_t(y)*frame.width+x+run))==opaque)++run;
                deltas.push_back(std::uint8_t(run));
                if(opaque)for(unsigned k=0;k<run;++k){const auto slot=std::size_t(y)*frame.width+x+k;
                    if(s.storage==assets::SpriteStorage::indexed8)pixels.push_back(std::get<std::vector<std::uint8_t>>(frame.pixels).at(slot));
                    else{const auto p=std::get<std::vector<std::uint16_t>>(frame.pixels).at(slot);pixels.push_back(std::uint8_t(p));pixels.push_back(std::uint8_t(p>>8));}}
                x+=run;opaque=!opaque;
            }
        }
        const auto first=out.size()+deltas.size();for(unsigned y=0;y<frame.height;++y)word(out,44+y*8,first+rowPixels[y]);
        out.insert(out.end(),deltas.begin(),deltas.end());out.insert(out.end(),pixels.begin(),pixels.end());
        for(unsigned plane=0;plane<2;++plane)if(!frame.auxiliaryData[plane].empty()){
            word(out,32+plane*4,out.size());out.insert(out.end(),frame.auxiliaryData[plane].begin(),frame.auxiliaryData[plane].end());}
        word(out,0,out.size());b.insert(b.end(),out.begin(),out.end());
    }
    word(b,4,b.size());return b;
}
class SceneResources {
    QTemporaryDir directory_;
    static assets::AssetStore store(const QTemporaryDir& d){
        auto configured=assets::AssetStore::create(d.path().toStdString());
        if(const auto* e=std::get_if<assets::Error>(&configured))throw std::runtime_error(e->detail);
        return std::get<assets::AssetStore>(std::move(configured));
    }
    void write(const std::string& path,const Bytes& bytes){
        std::ofstream out(std::filesystem::path(directory_.path().toStdString())/path,std::ios::binary);
        out.write(reinterpret_cast<const char*>(bytes.data()),std::streamsize(bytes.size()));if(!out)throw std::runtime_error("Scene fixture write failed");
    }
public:
    assets::ResourceManager manager;
    SceneResources():manager(store(directory_)){}
    assets::ResourceId add(const std::string& name,const assets::Sprite& sprite){
        const assets::ResourceId id{assets::ResourceKind::ui,name};write(name+".spr",spr(sprite));
        manager.bind(id,{assets::ResourceImageFormat::sprite,name+".spr",{},{},{}});return id;
    }
    assets::ResourceId add(const std::string& name,const assets::Sprite& sprite,const assets::Animation& animation){
        const assets::ResourceId id{assets::ResourceKind::creature,name};write(name+".spr",spr(sprite));write(name+".ani",ani(animation));
        manager.bind(id,{assets::ResourceImageFormat::sprite,name+".spr",name+".ani",{},{}});return id;
    }
};
}
