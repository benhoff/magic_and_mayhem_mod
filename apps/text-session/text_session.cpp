#include "text_session.hpp"
#include <algorithm>
#include <stdexcept>
namespace mnm::text {
namespace rr=mnm::reconstruction::rendering;
FontResource::FontResource(assets::SftFont source){
 const auto count=source.glyphs.frames.size();
 if(source.version!=3||!count||count>223||source.rowCount>128||source.metricGlyphCount<count||source.metricGlyphCount>223||
    source.rowMetrics.size()!=std::size_t(source.metricGlyphCount)*source.rowCount||source.ascent<0||source.descent<0||
    std::int64_t(source.ascent)+source.descent<1||std::int64_t(source.ascent)+source.descent>128)
  throw std::invalid_argument("Font resource requires complete bounded SFT metrics");
 std::uint64_t pixels=0;
 for(std::size_t k=0;k<count;++k){const auto& f=source.glyphs.frames[k];pixels+=std::uint64_t(f.width)*f.height;
  if(f.width>128||f.height>128||pixels>1024*1024)throw std::length_error("Font glyph pixel budget");
  render::prepareSpriteFrame(source.glyphs,k);
 }
 metrics_.glyphCount=unsigned(count);metrics_.rows=source.rowCount;
 for(std::size_t k=0;k<count*source.rowCount;++k){auto m=source.rowMetrics[k];metrics_.contours.push_back({m.leading,m.trailing});}
 source_=std::make_shared<const assets::SftFont>(std::move(source));
}
std::shared_ptr<const FontResource> FontResource::load(assets::AssetFile& file){
 assets::SftLimits limits;limits.glyphs.inputBytes=2*1024*1024;limits.glyphs.frames=223;limits.glyphs.width=128;limits.glyphs.height=128;
 limits.glyphs.pixels=1024*1024;limits.glyphs.decodedBytes=8*1024*1024;limits.metricsBytes=223*128*8;
 auto result=assets::loadSft(file,limits);
 if(auto* e=std::get_if<assets::SftError>(&result))throw std::invalid_argument(e->detail);
 return std::make_shared<const FontResource>(std::get<assets::SftFont>(std::move(result)));
}
TextSession::TextSession(std::shared_ptr<const FontResource> font):font_(std::move(font)){
 if(!font_)throw std::invalid_argument("Missing owned font");
 cursor_.trailing.resize(font_->metrics().rows);cursor_.tracking=1;cursor_.tabAdvance=20;
}
void TextSession::restore(Cursor value){if(value.trailing.size()!=font_->metrics().rows)throw std::invalid_argument("Font cursor row count");cursor_=std::move(value);}
std::int32_t TextSession::advance(std::uint8_t b,bool update,std::int32_t punctuation){auto c=cursor_;const auto n=rr::fontByteAdvance(font_->metrics(),c,b,update,punctuation);if(update)cursor_=std::move(c);return n;}
render::FontDrawPlan TextSession::plan(const std::vector<rr::ByteGlyph>& glyphs) const {
 render::FontDrawPlan out{font_->sprite(),{}};
 for(auto g:glyphs)if(g.frame)out.glyphs.push_back({*g.frame,g.x,g.y});
 return out;
}
render::FontDrawPlan TextSession::byte(std::uint8_t b,std::int32_t punctuation){auto c=cursor_;const auto g=rr::fontByteGlyph(font_->metrics(),c,b,punctuation);auto out=plan({g});cursor_=std::move(c);return out;}
render::FontDrawPlan TextSession::string(const std::vector<std::uint8_t>& bytes,std::int32_t punctuation){
 if(bytes.empty()||bytes.size()>65536||bytes.back()!=0||std::find(bytes.begin(),bytes.end()-1,0)!=bytes.end()-1)throw std::invalid_argument("Expected bounded NUL byte string");
 auto c=cursor_;std::vector<rr::ByteGlyph> glyphs;
 for(auto b:bytes){if(!b)break;glyphs.push_back(rr::fontByteGlyph(font_->metrics(),c,b,punctuation));}
 auto out=plan(glyphs);cursor_=std::move(c);return out;
}
TextResult TextSession::paragraph(LayoutInput in,rr::FontLayoutKind kind,bool commit){
 in.kind=kind;in.cursor=cursor_;in.ascent=font_->source().ascent;in.descent=font_->source().descent;
 if(!in.lineHeight)in.lineHeight=in.ascent+in.descent;
 if(in.lineHeight<1||in.lineHeight>256)throw std::invalid_argument("Font line height domain");
 auto layout=rr::fontLineLayout(font_->metrics(),in);auto drawing=plan(layout.glyphs);
 if(commit)cursor_=layout.cursor;
 return {std::move(drawing),std::move(layout)};
}
TextResult TextSession::rectangle(LayoutInput in){return paragraph(std::move(in),rr::FontLayoutKind::rectangle,true);}
TextResult TextSession::heightLimited(LayoutInput in){return paragraph(std::move(in),rr::FontLayoutKind::heightLimited,true);}
TextResult TextSession::measure(LayoutInput in) const {auto candidate=*this;return candidate.paragraph(std::move(in),rr::FontLayoutKind::measure,false);}
}
