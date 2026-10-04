#include "sft.hpp"
#include "sprite_frame_decoder.hpp"
#include <limits>
#include <new>
#include <stdexcept>
namespace mnm::assets {
namespace {
[[noreturn]] void fail(SpriteErrorCode code,std::size_t offset,const char* detail) {
    throw SftError{code,offset,{},detail,{}};
}
std::uint32_t word(const std::vector<std::uint8_t>& b,std::size_t p) {
    return std::uint32_t(b[p]) | std::uint32_t(b[p+1])<<8 | std::uint32_t(b[p+2])<<16 | std::uint32_t(b[p+3])<<24;
}
std::int32_t signedWord(const std::vector<std::uint8_t>& b,std::size_t p) {
    const auto w=word(b,p);return static_cast<std::int32_t>(w<0x80000000U ? std::int64_t(w) : std::int64_t(w)-0x100000000LL);
}
}
std::optional<std::uint32_t> sftGlyphIndex(const SftFont& font,std::uint8_t byte) {
    if(byte<33 || std::size_t(byte-33)>=font.glyphs.frames.size()) return std::nullopt;
    return std::uint32_t(byte-33);
}
SftResult decodeSft(const std::vector<std::uint8_t>& bytes,const SftLimits& limits) try {
    if(bytes.size()>limits.glyphs.inputBytes) fail(SpriteErrorCode::limitExceeded,0,"SFT input limit exceeded");
    if(bytes.size()<40) fail(SpriteErrorCode::malformedData,bytes.size(),"Truncated SFT header");
    if(word(bytes,0)!=0x00544653) fail(SpriteErrorCode::invalidFormat,0,"Invalid SFT signature");
    if(word(bytes,4)!=bytes.size()) fail(SpriteErrorCode::malformedData,4,"SFT declared size disagrees with input");
    if(word(bytes,8)!=3) fail(SpriteErrorCode::unsupportedVersion,8,"Only SFT version 3 is supported");
    const auto count=word(bytes,12),rows=word(bytes,16),metricGlyphs=word(bytes,36),paletteFlag=word(bytes,32);
    if(count>limits.glyphs.frames || metricGlyphs>limits.glyphs.frames || rows>limits.glyphs.height)
        fail(SpriteErrorCode::limitExceeded,12,"SFT glyph or metric row limit exceeded");
    const std::uint64_t metricCount=std::uint64_t(rows)*metricGlyphs;
    if(metricCount>limits.metricsBytes/sizeof(SftRowMetric) || metricCount>limits.glyphs.decodedBytes/sizeof(SftRowMetric))
        fail(SpriteErrorCode::limitExceeded,36,"SFT row metric allocation limit exceeded");
    const auto metricBytes=metricCount*8;
    const std::uint64_t metrics=40+(paletteFlag ? 768 : 0),table=metrics+metricBytes,base=table+std::uint64_t(count)*4;
    if(base>bytes.size()) fail(SpriteErrorCode::malformedData,36,"Truncated SFT palette, metrics or frame table");
    auto frameLimits=limits.glyphs;frameLimits.decodedBytes-=metricCount*sizeof(SftRowMetric);
    auto decoded=detail::decodeSpriteFrames(bytes,{3,count,paletteFlag ? 1U : 0U,word(bytes,28),40,table,base},frameLimits);
    if(const auto* error=std::get_if<SpriteError>(&decoded)) return *error;
    SftFont font;font.glyphs=std::get<Sprite>(std::move(decoded));font.rowCount=rows;font.metricGlyphCount=metricGlyphs;
    font.paletteFlag=paletteFlag;font.headerWord28=word(bytes,28);font.ascent=signedWord(bytes,20);font.descent=signedWord(bytes,24);
    if(metricCount>font.rowMetrics.max_size()) fail(SpriteErrorCode::limitExceeded,36,"SFT metric array too large");
    font.rowMetrics.resize(static_cast<std::size_t>(metricCount));
    for(std::size_t i=0;i<metricCount;++i) font.rowMetrics[i]={signedWord(bytes,metrics+i*8),signedWord(bytes,metrics+i*8+4)};
    return font;
} catch(const SftError& error) {return error;}
  catch(const std::bad_alloc&) {return SftError{SpriteErrorCode::limitExceeded,0,{},"SFT allocation failed",{}};}
  catch(const std::length_error&) {return SftError{SpriteErrorCode::limitExceeded,0,{},"SFT allocation too large",{}};}
SftResult loadSft(AssetFile& file,const SftLimits& limits) {
    if(limits.glyphs.inputBytes>std::uint64_t(std::numeric_limits<std::int64_t>::max()))
        return SftError{SpriteErrorCode::invalidArgument,0,{},"SFT input limit exceeds file API range",{}};
    auto bytes=readWhole(file,static_cast<std::int64_t>(limits.glyphs.inputBytes));
    if(const auto* error=std::get_if<Error>(&bytes)) return SftError{error->code==ErrorCode::limitExceeded ? SpriteErrorCode::limitExceeded : SpriteErrorCode::assetInput,0,{},error->detail,*error};
    return decodeSft(std::get<std::vector<std::uint8_t>>(bytes),limits);
}
}
