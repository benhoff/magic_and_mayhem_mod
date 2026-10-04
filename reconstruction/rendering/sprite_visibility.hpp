#pragma once
#include <cstdint>
#include <optional>
#include <vector>
namespace mnm::reconstruction {
struct SpriteVisibilityShape {
    std::int32_t originX=0,originY=0;
    std::vector<std::uint32_t> testRows,coverRows;
};
// Inputs are retained opaque SPR +32/+36 planes, not pixel transparency.
std::optional<SpriteVisibilityShape> decodeSpriteVisibility(const std::vector<std::uint8_t>& headerAndCover,
    const std::vector<std::uint8_t>& tests,std::int32_t originX,std::int32_t originY);
struct VisibilityOwner {
    std::uint32_t body=0,first=0,second=0;
    std::uint16_t flags8=0,flags10=0;
};
struct SpriteVisibilityEntry {
    std::optional<SpriteVisibilityShape> shape;
    std::int32_t x=0,y=0,kind=0;
    std::uint32_t spriteIdentity=0;
    std::optional<std::size_t> owner;
};
class SpriteVisibilityGrid {
public:
    static constexpr std::size_t stride=43,rows=339,bytes=stride*rows;
    explicit SpriteVisibilityGrid(bool expanded=false):expanded_(expanded),data_(bytes,0){}
    void clear();
    bool testAndCover(const SpriteVisibilityEntry& entry);
    const std::vector<std::uint8_t>& data() const{return data_;}
    // Owned seed for fixtures; validates exact recovered allocation size.
    void seed(std::vector<std::uint8_t> bytes);
private:
    bool expanded_;std::vector<std::uint8_t> data_;
};
// Mutates draw kinds/owner flags. Original deliberately skips queue entry zero.
void applySpriteVisibility(std::vector<SpriteVisibilityEntry>& entries,SpriteVisibilityGrid& grid,
    std::vector<VisibilityOwner>& owners);
}
