#pragma once
#include "sprite_loader.hpp"

namespace mnm::assets {
struct SftRowMetric {std::int32_t leading=0,trailing=0;};
struct SftFont {
    std::uint32_t version=3,rowCount=0,metricGlyphCount=0,paletteFlag=0,headerWord28=0;
    std::int32_t ascent=0,descent=0;
    std::vector<SftRowMetric> rowMetrics; // Glyph-major, rowCount pairs per metric glyph.
    Sprite glyphs; // Owned palettes, frame metadata, masks, pixels and auxiliary planes.
};
struct SftLimits {
    SpriteLimits glyphs;
    std::uint64_t metricsBytes=8*1024*1024;
};
using SftError=SpriteError;
using SftResult=std::variant<SftFont,SftError>;
SftResult decodeSft(const std::vector<std::uint8_t>&,const SftLimits& limits={});
SftResult loadSft(AssetFile&,const SftLimits& limits={});
// Raw byte -> frame ordinal, first frame is byte 33. Not Unicode/text layout;
// the original handles space, underscore, tab and newline separately.
std::optional<std::uint32_t> sftGlyphIndex(const SftFont&,std::uint8_t);
}
