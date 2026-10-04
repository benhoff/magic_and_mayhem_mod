#pragma once
#include "positional_audio.hpp"
#include <vector>
namespace mnm::reconstruction::audio {
// Owned decoded snapshot, not the original world object or its five work arrays.
// Captured row/layer offsets are byte indices; preserve padding and layout.
class AttenuationMap final:public PositionalByteSource {
public:
    AttenuationMap(std::int32_t width,std::vector<std::uint32_t> rows,
                   std::vector<std::uint32_t> layers,std::vector<std::uint8_t> bytes);
    bool enabled=true; // Decoded 0x5e1404, independent of snapshot ownership.
    void clear();
    std::optional<std::int8_t> read(std::int32_t x,std::int32_t y,std::uint32_t z) const override;
private:
    std::int32_t width_;
    std::vector<std::uint32_t> rows_,layers_;
    std::vector<std::uint8_t> bytes_;
};
}
