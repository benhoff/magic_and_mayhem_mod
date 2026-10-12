#include "wave_source.hpp"

namespace mnm::audio {
std::unique_ptr<mnm::assets::AssetFile> WaveSource::open(std::string_view leaf) const{
    if(leaf.empty()||leaf.size()>259||leaf.find_first_of("/\\:")!=std::string_view::npos||
       leaf.size()<5||leaf.substr(leaf.size()-4)!=".wav")
        throw std::runtime_error("Native wave request requires a bounded leaf .wav name");
    auto input=assets_.open(leaf);
    if(const auto* error=std::get_if<mnm::assets::Error>(&input)){
        const auto stem=leaf.substr(0,leaf.size()-4);
        if(dequoteMissingLeaf_&&error->code==mnm::assets::ErrorCode::notFound&&
           stem.size()>2&&stem.front()=='\''&&stem.back()=='\'')
            input=assets_.open(std::string(stem.substr(1,stem.size()-2))+".wav");
    }
    if(const auto* error=std::get_if<mnm::assets::Error>(&input))throw AssetInputError(*error);
    return std::get<std::unique_ptr<mnm::assets::AssetFile>>(std::move(input));
}
std::uint32_t WaveSource::fileBytes(std::string_view leaf) const{
    auto input=open(leaf);const auto size=input->size();
    if(const auto* error=std::get_if<mnm::assets::Error>(&size))throw AssetInputError(*error);
    const auto bytes=std::get<std::int64_t>(size);
    if(bytes<0 || std::uint64_t(bytes)>0xffffffffu)throw std::runtime_error("Native audio file size exceeds DWORD domain");
    return std::uint32_t(bytes);
}
SourceWave WaveSource::load(std::string_view leaf) const{
    auto input=open(leaf);auto pcm=loadWave(*input);
    if(pcm.samples.size()>16u*1024u*1024u)
        throw std::runtime_error("Native wave exceeds the static voice payload limit");
    // Recovered helper multiplies in DWORD arithmetic before division.
    const auto duration=std::uint32_t(std::uint32_t(pcm.samples.size())*1000u)/pcm.format.bytesPerSecond;
    return {std::move(pcm),duration};
}
}
