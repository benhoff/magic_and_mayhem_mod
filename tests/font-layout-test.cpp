#include "../compat/legacy/font_layout.hpp"
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace mnm;
using namespace reconstruction::rendering;
void require(bool v){if(!v)throw std::runtime_error("Font layout admission assertion");}
int main() try {
  assets::SftFont font;font.rowCount=1;font.metricGlyphCount=65;font.ascent=1;
  font.rowMetrics.resize(65,{0,2});font.glyphs.frames.resize(65);
  for(auto &g:font.glyphs.frames){g.width=1;g.height=1;g.pixels=std::vector<std::uint8_t>{1};g.opaqueMask={1};}
  // First byte can paint successfully; second has an invalid coverage index.
  font.glyphs.frames['!'-33].pixels=std::vector<std::uint8_t>{64};
  compat::FontLayout producer(font);
  render::CanvasSequence canvas;canvas.create(1,32,8);canvas.fill(1,{0,0,32,8},0x1234);
  FontLayoutInput in;in.bounds={0,0,32,8};in.cursor.trailing={7};in.lineHeight=2;
  std::array<float,64> table{};table.fill(1);
  unsigned refusals=0;
  auto refusal=[&](FontLayoutInput input) {
    const auto before=canvas.read(1).pixels;const auto cursorBefore=in.cursor;bool failed=false;
    try{producer.draw(canvas,1,input,{0,0,32,8},{255,255,255},table);}
    catch(const std::exception &){failed=true;}
    require(failed && canvas.read(1).pixels==before && in.cursor.trailing==cursorBefore.trailing);++refusals;
  };
  in.text={'A','!',0};refusal(in);
  in.text={'A'};refusal(in);
  in.text={'A',0,'B',0};refusal(in);
  in.text={0};in.kind=FontLayoutKind::measure;refusal(in);
  in.kind=FontLayoutKind::rectangle;in.text={'A',0};in.nullSource=true;refusal(in);in.nullSource=false;
  in.text={'A','A','A',0};in.flags=16;in.bounds.right=1;refusal(in);
  in.bounds.right=100000;in.text.assign(514,'A');in.text.back()=0;refusal(in);
  in.kind=FontLayoutKind::measure;in.text.assign(102,'A');in.text.back()=0;refusal(in);
  in.flags=0;in.text={'A',0};in.cursor.trailing.clear();refusal(in);in.cursor.trailing={7};
  // Measurement never requires canvas/table admission, and null resets line count
  // while retaining cursor state. Unknown bytes are skipped during drawing.
  in.nullSource=true;auto result=producer.draw(canvas,99,in,{}, {},table);require(result.lines.empty() && result.cursor.trailing==in.cursor.trailing);
  in.nullSource=false;in.kind=FontLayoutKind::rectangle;in.text={1,255,0};
  result=producer.draw(canvas,99,in,{}, {},table);require(result.glyphs.empty() && result.consumed==2);
  require(refusals==9);
  std::cout<<"Nine whole-string atomic refusals and suppressed/measurement paths pass\n";return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
