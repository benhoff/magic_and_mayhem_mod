#include "../compat/legacy/font_text.hpp"
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace mnm;
using namespace reconstruction::rendering;
void require(bool v) {if(!v)throw std::runtime_error("Font admission assertion");}
int main() try {
  assets::SftFont font;font.rowCount=1;font.metricGlyphCount=65;
  font.rowMetrics.resize(65,{0,4});font.glyphs.frames.resize(65);
  for(auto &g:font.glyphs.frames) {
    g.width=2;g.height=1;g.pixels=std::vector<std::uint8_t>{1,1};g.opaqueMask={1,1};
  }
  compat::FontText text(std::move(font));
  render::CanvasSequence canvas;canvas.create(1,8,4);canvas.fill(1,{0,0,8,4},0x1234);
  FontCursor state{{3},0,1,0,7};std::array<float,64> coverage{};coverage.fill(1);
  auto refusal=[&](std::uint8_t b,std::array<float,64> table,std::array<unsigned,3> tint) {
    const auto before=canvas.read(1).pixels;const auto saved=state;
    bool failed=false;
    try{text.draw(canvas,1,state,b,1,{0,0,8,4},tint,table);}catch(const std::invalid_argument &){failed=true;}
    require(failed && state.x==saved.x && state.y==saved.y && state.trailing==saved.trailing &&
            canvas.read(1).pixels==before);
  };
  refusal(0,coverage,{255,255,255});refusal(255,coverage,{255,255,255});
  auto bad=coverage;bad[1]=std::numeric_limits<float>::quiet_NaN();refusal('A',bad,{255,255,255});
  refusal('A',coverage,{256,255,255});
  state.trailing.clear();refusal('A',coverage,{255,255,255});state.trailing={3};
  // Undefined late destination reads must preserve both contour/cursor and pixels.
  render::CanvasSequence incomplete;incomplete.create(2,8,4);incomplete.fill(2,{3,1,4,2},0x1234);
  const auto saved=state;bool failed=false;
  try{text.draw(incomplete,2,state,'A',1,{0,0,8,4},{255,255,255},coverage);}
  catch(const std::runtime_error &){failed=true;}
  require(failed && state.x==saved.x && state.trailing==saved.trailing);
  incomplete.fill(2,{0,0,3,4},0x1234);incomplete.fill(2,{4,0,8,4},0x1234);
  incomplete.fill(2,{3,0,4,1},0x1234);incomplete.fill(2,{3,2,4,4},0x1234);
  for(auto p:incomplete.read(2).pixels)require(p==0x1234);
  // Suppressed tab/newline advance without requiring a drawable canvas.
  text.draw(canvas,99,state,9,1,{},{},bad);require(state.x==7 && state.trailing==saved.trailing);
  text.draw(canvas,99,state,10,1,{},{},bad);require(state.x==7 && state.y==1);
  FontCursor probe{{3},0,0,0,7};
  require(fontByteAdvance(text.contours(),probe,'A',false,0)==3 && probe.trailing[0]==3);
  require(fontByteAdvance(text.contours(),probe,'A',true,0)==3 && probe.trailing[0]==4);
  std::cout<<"Six atomic text refusals and suppressed-byte state pass\n";return 0;
} catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
