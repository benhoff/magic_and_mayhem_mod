#include "text_session.hpp"
#include <QGuiApplication>
#include <QTemporaryDir>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
using namespace mnm;
namespace {
unsigned checks=0,refusals=0;
void require(bool ok,const char* m){++checks;if(!ok)throw std::runtime_error(m);}
template<class F> void rejects(F f){bool refused=false;try{f();}catch(const std::exception&){refused=true;}require(refused,"Expected refusal");++refusals;}
bool same(const text::Cursor& a,const text::Cursor& b){return a.x==b.x&&a.y==b.y&&a.tracking==b.tracking&&a.tabAdvance==b.tabAdvance&&a.trailing==b.trailing;}
assets::SftFont fixture(){
 assets::SftFont f;f.rowCount=2;f.metricGlyphCount=65;f.ascent=2;f.descent=1;
 f.rowMetrics.resize(130,{0,2});f.glyphs.storage=assets::SpriteStorage::indexed8;f.glyphs.palettes.resize(1);
 f.glyphs.palettes[0][0]={0,0,0};f.glyphs.palettes[0][1]={255,0,0};f.glyphs.palettes[0][2]={0,255,0};
 for(unsigned i=0;i<65;++i){assets::SpriteFrame g;g.width=3;g.height=2;g.originX=1;g.originY=1;g.paletteIndex=0;
  g.pixels=std::vector<std::uint8_t>{1,0,2,0,1,0};g.opaqueMask={1,0,1,1,1,0};f.glyphs.frames.push_back(g);}
 return f;
}
void put(std::vector<std::uint8_t>& b,std::size_t at,std::uint32_t v){for(unsigned i=0;i<4;++i)b.at(at+i)=std::uint8_t(v>>(8*i));}
std::vector<std::uint8_t> encoded(){
 // One indexed glyph with complete row contours and opaque black pixel.
 const unsigned table=40+768+16,base=table+4;std::vector<std::uint8_t> b(base+52,0);
 put(b,0,0x00544653);put(b,4,b.size());put(b,8,3);put(b,12,1);put(b,16,2);put(b,20,2);put(b,24,1);put(b,32,1);put(b,36,1);
 put(b,40+768+4,2);put(b,40+768+12,2);put(b,table,0);
 put(b,base,52);put(b,base+4,1);put(b,base+8,1);put(b,base+28,0);put(b,base+40,48);put(b,base+44,50);b[base+48]=0;b[base+49]=1;return b;
}
}
int main(int argc,char** argv)try{
 QGuiApplication app(argc,argv);QTemporaryDir directory;require(directory.isValid(),"Fixture directory failed");
 const auto raw=encoded();const auto path=std::filesystem::path(directory.path().toStdString());
 {std::ofstream file(path/"font.sft",std::ios::binary);file.write(reinterpret_cast<const char*>(raw.data()),raw.size());}
 auto store=std::get<assets::AssetStore>(assets::AssetStore::create(path));auto opened=store.open("FONT.SFT");
 auto file=std::get<std::unique_ptr<assets::AssetFile>>(std::move(opened));auto loaded=text::FontResource::load(*file);file.reset();std::filesystem::remove(path/"font.sft");
 require(loaded->source().glyphs.frames.size()==1&&loaded->metrics().contours.size()==2,"Owned SFT file load differs");
 auto source=fixture();auto font=std::make_shared<const text::FontResource>(source);source.rowMetrics.clear();source.glyphs.frames.clear();
 text::TextSession pen(font);require(pen.cursor().tracking==1&&pen.cursor().tabAdvance==20,"Default pen differs");
 auto cursor=pen.cursor();cursor.x=4;cursor.y=3;cursor.trailing={3,4};pen.restore(cursor);
 require(pen.advance('A')==5&&same(pen.cursor(),cursor),"Nonmutating byte advance differs");
 require(pen.advance('A',true)==5&&pen.cursor().trailing==std::vector<std::int32_t>({2,2}),"Committed contour advance differs");
 pen.restore(cursor);auto byte=pen.byte('A');require(byte.glyphs.size()==1&&byte.glyphs[0].x==9&&pen.cursor().x==9,"Byte glyph placement differs");
 pen.restore(cursor);auto string=pen.string({'A',' ','A',9,'_',10,0});require(string.glyphs.size()==2&&pen.cursor().y==3,"Suppressed byte string differs");
 auto before=pen.cursor();rejects([&]{pen.string({'A',1,0});});require(same(before,pen.cursor()),"Late string refusal changed pen");
 for(auto bytes:{std::vector<std::uint8_t>{},std::vector<std::uint8_t>{'A'},std::vector<std::uint8_t>{'A',0,'A',0}})rejects([&]{pen.string(bytes);});
 rejects([&]{auto c=cursor;c.trailing.clear();pen.restore(c);});require(same(before,pen.cursor()),"Bad restored state changed pen");
 text::LayoutInput input;input.text={'A','A','A',0};input.bounds={0,0,32,20};
 for(unsigned flags:{0u,2u,4u,8u,16u,20u}){
  pen.restore(cursor);input.flags=flags;auto a=pen.rectangle(input);
  require(a.drawing.glyphs.size()==3&&a.layout.consumed==3,"Rectangle text differs");
  auto expected=input;expected.kind=reconstruction::rendering::FontLayoutKind::rectangle;expected.cursor=cursor;expected.ascent=2;expected.descent=1;expected.lineHeight=3;
  const auto supplied=reconstruction::rendering::fontLineLayout(font->metrics(),expected);
  require(a.layout.width==supplied.width&&same(a.layout.cursor,supplied.cursor),"Native rectangle composition changed supplier");
  pen.restore(cursor);auto measured=pen.measure(input);require(same(pen.cursor(),cursor)&&measured.drawing.glyphs.empty(),"Measure changed pen or produced glyphs");
  pen.restore(cursor);auto h=pen.heightLimited(input);require(h.layout.consumed.has_value(),"Height-limited source consumption missing");
 }
 before=pen.cursor();input.text={0};rejects([&]{pen.heightLimited(input);});require(same(before,pen.cursor()),"Layout refusal changed pen");
 input.nullSource=true;require(pen.measure(input).layout.lines.empty(),"Null measurement differs");
 input.nullSource=false;input.text={'A',0};input.lineHeight=-1;rejects([&]{pen.rectangle(input);});
 for(unsigned failure=0;failure<6;++failure){auto bad=fixture();if(failure==0)bad.metricGlyphCount=1;if(failure==1)bad.ascent=-1;if(failure==2)bad.glyphs.frames[2].opaqueMask[0]=4;if(failure==3)bad.glyphs.frames[2].paletteIndex=3;if(failure==4)bad.rowCount=129;if(failure==5)bad.glyphs.frames[2].width=129;rejects([&]{text::FontResource rejected(std::move(bad));});}
 render::GlBlitter renderer;const auto baseline=renderer.stats();unsigned cases=0;
 for(bool overridePalette:{false,true})for(int x:{-4,0,3,10,std::numeric_limits<int>::min(),std::numeric_limits<int>::max()})for(int y:{-2,1,4}){
  render::SpriteColourTable table{};table[0]=0x001f;table[1]=0xffff;table[2]=0xf81f;
  auto plan=render::FontDrawPlan{font->sprite(),{{0,x,y},{1,3,2}}};render::Image image{12,7,std::vector<std::uint32_t>(84,0x1234)};
  const render::Rect clip{1,1,11,6};auto expected=image;
  // Independent fixture raster: signed origins, sparse coverage, opaque zero.
  for(const auto& g:plan.glyphs){const unsigned indices[]={1,0,2,0,1,0};const bool mask[]={true,false,true,true,true,false};
   for(int row=0;row<2;++row)for(int col=0;col<3;++col){const auto dx=std::int64_t(g.x)-1+col,dy=std::int64_t(g.y)-1+row;
    if(mask[row*3+col]&&dx>=1&&dx<11&&dy>=1&&dy<6)expected.pixels[std::size_t(dy)*12+std::size_t(dx)]=overridePalette?table[indices[row*3+col]]:indices[row*3+col]==1?0xf800:indices[row*3+col]==2?0x7e0:0;
   }
  }
  render::drawFontPixels(image,plan,clip,overridePalette?&table:nullptr);require(image.pixels==expected.pixels,"CPU font pixels differ");
  const auto destination=renderer.create({12,7,std::vector<std::uint32_t>(84,0x1234)},render::spriteFormat);
  {render::FontGpuCache cache(renderer,font->sprite(),{2,24});cache.draw(destination,plan,clip,overridePalette?&table:nullptr);require(renderer.read(destination).pixels==expected.pixels,"GPU font pixels differ");
   const auto s=renderer.stats();cache.draw(destination,plan,clip,overridePalette?&table:nullptr);const auto e=renderer.stats();require(s.uploads==e.uploads&&s.nativeReadbacks==e.nativeReadbacks&&s.rgbaReadbacks==e.rgbaReadbacks,"Resident font draw read back or uploaded");
   auto bad=plan;bad.glyphs.push_back({1000,0,0});const auto prior=renderer.read(destination).pixels;rejects([&]{cache.draw(destination,bad,clip);});require(renderer.read(destination).pixels==prior,"Late glyph failure changed GPU output");
   auto unchanged=image;rejects([&]{render::drawFontPixels(image,bad,clip);});require(image.pixels==unchanged.pixels,"Late glyph failure changed CPU output");
   bad=plan;bad.font=loaded->sprite();rejects([&]{cache.draw(destination,bad,clip);});
   cache.clear();require(cache.stats().frames==0&&cache.stats().pixels==0,"GPU cache clear retained storage");
  }
  renderer.destroy(destination);require(renderer.stats().surfaces==baseline.surfaces,"Font GPU surface leaked");++cases;
 }
 {const auto dest=renderer.create({12,7,std::vector<std::uint32_t>(84,0)},render::spriteFormat);render::FontGpuCache cache(renderer,font->sprite(),{1,12});
  cache.draw(dest,{font->sprite(),{{0,3,3},{1,3,3},{0,3,3}}},{0,0,12,7});require(cache.stats().evictions==2&&cache.stats().frames==1,"LRU eviction differs");
  render::SpriteColourTable table{};table[1]=65535;cache.draw(dest,{font->sprite(),{{0,3,3}}},{0,0,12,7},&table);require(cache.stats().evictions==3,"Palette values reused wrong upload");
  cache.clear();renderer.destroy(dest);
 }
 {render::FontGpuCache cache(renderer,font->sprite(),{1,11});const auto dest=renderer.create({12,7,std::vector<std::uint32_t>(84,0)},render::spriteFormat);
  rejects([&]{cache.draw(dest,{font->sprite(),{{0,3,3}}},{0,0,12,7});});require(cache.stats().uploads==0,"Budget refusal uploaded glyph");renderer.destroy(dest);}
 require(renderer.stats().surfaces==baseline.surfaces,"Cache lifetime leak");
 std::cout<<"{\"success\":true,\"checks\":"<<checks<<",\"refusals\":"<<refusals<<",\"cpu_gpu_cases\":"<<cases<<",\"scopes\":10,\"surface_leaks\":0}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
