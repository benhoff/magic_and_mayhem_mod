#pragma once
#include "../../assets/sft.hpp"
#include "../../renderer/fonts/font_pixels.hpp"
#include "../../reconstruction/rendering/font_layout.hpp"
namespace mnm::text {
using Cursor=reconstruction::rendering::FontCursor;
using LayoutInput=reconstruction::rendering::FontLayoutInput;
using LayoutResult=reconstruction::rendering::FontLayoutResult;
class FontResource {
 std::shared_ptr<const assets::SftFont> source_;
 reconstruction::rendering::ByteFont metrics_;
public:
 explicit FontResource(assets::SftFont);
 static std::shared_ptr<const FontResource> load(assets::AssetFile&);
 const auto& metrics() const{return metrics_;}
 const auto& source() const{return *source_;}
 std::shared_ptr<const assets::Sprite> sprite() const{return {source_,&source_->glyphs};}
};
struct TextResult {render::FontDrawPlan drawing;LayoutResult layout;};
// Thread-owned application orchestration over immutable font and recovered
// layout suppliers. Byte text has one final NUL and at most65536 bytes.
class TextSession {
 std::shared_ptr<const FontResource> font_;Cursor cursor_;
 TextResult paragraph(LayoutInput,reconstruction::rendering::FontLayoutKind,bool);
 render::FontDrawPlan plan(const std::vector<reconstruction::rendering::ByteGlyph>&) const;
public:
 explicit TextSession(std::shared_ptr<const FontResource>);
 const Cursor& cursor() const{return cursor_;}
 void restore(Cursor);
 std::int32_t advance(std::uint8_t,bool update=false,std::int32_t punctuation=0);
 render::FontDrawPlan byte(std::uint8_t,std::int32_t punctuation=0);
 render::FontDrawPlan string(const std::vector<std::uint8_t>&,std::int32_t punctuation=0);
 TextResult rectangle(LayoutInput);
 TextResult heightLimited(LayoutInput);
 // Measurement returns supplier state in result, preserving the session pen.
 TextResult measure(LayoutInput) const;
};
}
