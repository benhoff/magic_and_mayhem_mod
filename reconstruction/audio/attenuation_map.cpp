#include "attenuation_map.hpp"
#include <stdexcept>
#include <utility>
namespace mnm::reconstruction::audio {
AttenuationMap::AttenuationMap(std::int32_t width,std::vector<std::uint32_t> rows,
    std::vector<std::uint32_t> layers,std::vector<std::uint8_t> bytes):
    width_(width),rows_(std::move(rows)),layers_(std::move(layers)),bytes_(std::move(bytes)) {
    if(width<=0)throw std::invalid_argument("Map width must be positive");
}
void AttenuationMap::clear(){rows_.clear();layers_.clear();bytes_.clear();}
std::optional<std::int8_t> AttenuationMap::read(std::int32_t x,std::int32_t y,std::uint32_t z) const {
    if(!enabled)return std::nullopt;
    const auto layer=std::size_t(z>>1);
    if(x<0 || x>=width_ || y<0 || std::size_t(y)>=rows_.size() || layer>=layers_.size())
        throw std::out_of_range("Source outside captured map tables");
    const auto index=std::uint64_t(rows_[std::size_t(y)])+layers_[layer]+std::uint32_t(x);
    if(index>=bytes_.size())throw std::out_of_range("Source outside captured byte storage");
    const int byte=bytes_[std::size_t(index)];
    return std::int8_t(byte<128?byte:byte-256);
}
}
