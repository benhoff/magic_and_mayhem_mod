#pragma once
#include "asset_file.hpp"
#include <QTemporaryDir>
#include <fstream>
#include <stdexcept>
namespace audioFixture {
inline void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
inline void write(const std::filesystem::path& p,const std::vector<std::uint8_t>& data){
    std::ofstream file(p,std::ios::binary);file.write(reinterpret_cast<const char*>(data.data()),std::streamsize(data.size()));check(bool(file),"write fixture");
}
inline void profile(const std::filesystem::path& root,const std::string& text){write(root/"Sounds.ini",{text.begin(),text.end()});}
inline std::vector<std::uint8_t> wave(int sample){
    std::vector<std::uint8_t> v(44+256,0);
    const auto tag=[&](unsigned at,const char* s){for(unsigned i=0;i<4;++i)v[at+i]=std::uint8_t(s[i]);};
    const auto word=[&](unsigned at,unsigned n,unsigned count){for(unsigned i=0;i<count;++i)v[at+i]=std::uint8_t(n>>(i*8));};
    tag(0,"RIFF");word(4,292,4);tag(8,"WAVE");tag(12,"fmt ");word(16,16,4);word(20,1,2);word(22,1,2);word(24,48000,4);word(28,96000,4);word(32,2,2);word(34,16,2);tag(36,"data");word(40,256,4);
    for(unsigned at=44;at<v.size();at+=2)word(at,unsigned(sample),2);
    return v;
}
inline mnm::assets::AssetStore store(const std::filesystem::path& root){
    auto made=mnm::assets::AssetStore::create(root);check(std::holds_alternative<mnm::assets::AssetStore>(made),"fixture root");return std::get<mnm::assets::AssetStore>(std::move(made));
}
}
