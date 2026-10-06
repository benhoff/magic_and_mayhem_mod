#pragma once
#include "../../renderer/dib.hpp"
#include <algorithm>
#include <optional>
#include <stdexcept>
#include <string_view>

namespace mnm::reconstruction {
// Original sequential-read BMP subset; safe bounds are native refusals, not
// newly inferred original failure returns. No native service depends on this.
inline std::optional<render::DibInput> parseSurfaceBitmap(const std::vector<std::uint8_t>& bytes){
    if(bytes.size()>2*1024*1024)throw std::runtime_error("Surface bitmap input budget exceeded");
    if(bytes.size()<14 || bytes[0]!='B' || bytes[1]!='M' || bytes.size()<54)return std::nullopt;
    const auto u32=[&](unsigned at){return std::uint32_t(bytes[at])|(std::uint32_t(bytes[at+1])<<8)|
        (std::uint32_t(bytes[at+2])<<16)|(std::uint32_t(bytes[at+3])<<24);};
    const auto bits=unsigned(bytes[28])|(unsigned(bytes[29])<<8);
    if(bits!=1 && bits!=4 && bits!=8 && bits!=24)throw std::runtime_error("Unsupported surface bitmap depth");
    const unsigned paletteBytes=bits==24?0:(1u<<bits)*4;
    if(bytes.size()<54+paletteBytes)return std::nullopt;
    const auto size=u32(2),offset=u32(10);
    if(size<=offset || size-offset>2*1024*1024)throw std::runtime_error("Surface bitmap pixel budget exceeded");
    const auto count=size-offset,start=54+paletteBytes;
    if(count>bytes.size()-start)return std::nullopt;
    render::DibInput out;std::copy_n(bytes.begin()+14,40,out.header.begin());
    out.palette.assign(bytes.begin()+54,bytes.begin()+start);
    out.pixels.assign(bytes.begin()+start,bytes.begin()+start+count);
    out.usage=bits==24?1:0;return out;
}
// Reader resolves a fixed asset path; draw targets the application's global
// cursor binding. Loader returns file-load success, independent of draw HRESULT.
// Unsupported native layouts/errors throw instead of manufacturing success.
template<class Reader,class Draw>
unsigned loadSurfaceBitmap(Reader&& reader,Draw&& draw,bool hasSurface){
    const auto bytes=reader(std::string_view("bitmaps\\cursors.bmp"));
    if(!bytes)return 0;
    const auto dib=parseSurfaceBitmap(*bytes);
    if(!dib)return 0;
    if(hasSurface)draw(*dib);
    return 1;
}
template<class Reader,class Draw>
unsigned reloadCursorBitmap(Reader&& reader,Draw&& draw,bool hasSurface=true){
    (void)loadSurfaceBitmap(reader,draw,hasSurface);return 0;
}
}
