#include "profile_source.hpp"
#include <stdexcept>
namespace mnm::audio {
mnm::assets::ProfileSnapshot ProfileSource::load(const mnm::assets::AssetStore& assets){
    auto input=assets.open("Sounds.ini");
    if(const auto* error=std::get_if<mnm::assets::Error>(&input))throw std::runtime_error(error->detail);
    auto parsed=mnm::assets::loadProfile(*std::get<std::unique_ptr<mnm::assets::AssetFile>>(input));
    if(const auto* error=std::get_if<mnm::assets::Error>(&parsed))throw std::runtime_error(error->detail);
    return std::get<mnm::assets::ProfileSnapshot>(std::move(parsed));
}
ProfileSource::ProfileSource(mnm::assets::AssetStore assets):assets_(std::move(assets)),profile_(load(assets_)){}
}
