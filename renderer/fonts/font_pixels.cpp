#include "font_pixels.hpp"
#include <algorithm>
#include <stdexcept>
#include <set>
namespace mnm::render {
namespace {
void viewport(Rect r,int w,int h){
 if(w<1||h<1||w>2048||h>2048||r.left<0||r.top<0||r.right<=r.left||r.bottom<=r.top||r.right>w||r.bottom>h)
  throw std::invalid_argument("Font viewport outside native image");
}
void preflight(const FontDrawPlan& plan,const SpriteColourTable* colours){
 if(!plan.font||plan.glyphs.size()>65536)throw std::invalid_argument("Missing or oversized font draw plan");
 std::set<std::size_t> checked;std::uint64_t work=0;
 for(const auto& g:plan.glyphs){
  const auto& f=plan.font->frames.at(g.frame);work+=std::uint64_t(f.width)*f.height;
  if(work>16*1024*1024)throw std::length_error("Font draw pixel-work budget");
  if(checked.insert(g.frame).second)prepareSpriteFrame(*plan.font,g.frame,colours);
 }
}
}
void drawFontPixels(Image& destination,const FontDrawPlan& plan,Rect clip,const SpriteColourTable* colours){
 viewport(clip,destination.width,destination.height);
 if(destination.pixels.size()!=std::size_t(destination.width)*destination.height||
    std::any_of(destination.pixels.begin(),destination.pixels.end(),[](auto p){return p>65535;}))
  throw std::invalid_argument("Font destination requires complete RGB565 words");
 preflight(plan,colours);auto candidate=destination;
 for(const auto& g:plan.glyphs){
  const auto f=prepareSpriteFrame(*plan.font,g.frame,colours);
  const auto x=std::int64_t(g.x)-f.originX,y=std::int64_t(g.y)-f.originY;
  const auto l=std::max<std::int64_t>(x,clip.left),t=std::max<std::int64_t>(y,clip.top);
  const auto r=std::min<std::int64_t>(x+f.pixels.width,clip.right),b=std::min<std::int64_t>(y+f.pixels.height,clip.bottom);
  for(auto row=t;row<b;++row)for(auto col=l;col<r;++col){
   const auto from=std::size_t(row-y)*f.pixels.width+std::size_t(col-x);
   if(f.mask.pixels[from])candidate.pixels[std::size_t(row)*candidate.width+std::size_t(col)]=f.pixels.pixels[from];
  }
 }
 destination=std::move(candidate);
}
FontGpuCache::FontGpuCache(GlBlitter& renderer,std::shared_ptr<const assets::Sprite> font,FontCacheLimits limits)
 :renderer_(renderer),font_(std::move(font)),limits_(limits){
 renderer_.stats();if(!font_||!limits.frames||limits.frames>223||!limits.pixels||limits.pixels>8*1024*1024)
  throw std::invalid_argument("Font cache ownership/budget");
}
UploadedSpriteFrame& FontGpuCache::glyph(std::size_t frame,const SpriteColourTable* colours){
 const auto value=colours?std::make_optional(*colours):std::nullopt;
 for(auto it=entries_.begin();it!=entries_.end();++it)if(it->frame==frame&&it->colours==value){
  ++stats_.hits;entries_.splice(entries_.end(),entries_,it);return *entries_.back().upload;
 }
 const auto& f=font_->frames.at(frame);const auto pixels=std::uint64_t(f.width)*f.height*2;
 if(pixels>limits_.pixels)throw std::length_error("Glyph exceeds font cache pixel budget");
 while(!entries_.empty()&&(entries_.size()>=limits_.frames||stats_.pixels+pixels>limits_.pixels)){
  stats_.pixels-=entries_.front().pixels;entries_.pop_front();++stats_.evictions;
 }
 auto upload=std::make_unique<UploadedSpriteFrame>(renderer_,*font_,frame,colours);
 entries_.push_back({frame,value,pixels,std::move(upload)});stats_.pixels+=pixels;stats_.frames=entries_.size();++stats_.uploads;
 return *entries_.back().upload;
}
void FontGpuCache::draw(SurfaceId destination,const FontDrawPlan& plan,Rect clip,const SpriteColourTable* colours){
 const auto size=renderer_.dimensions(destination);viewport(clip,size.width(),size.height());
 if(plan.font!=font_)throw std::invalid_argument("Foreign font plan");
 preflight(plan,colours);
 for(const auto& g:plan.glyphs){const auto& f=font_->frames.at(g.frame);if(std::uint64_t(f.width)*f.height*2>limits_.pixels)throw std::length_error("Glyph exceeds cache budget");}
 for(const auto& g:plan.glyphs)glyph(g.frame,colours).drawClipped(destination,g.x,g.y,clip);
}
void FontGpuCache::clear(){renderer_.stats();entries_.clear();stats_.frames=0;stats_.pixels=0;}
FontCacheStats FontGpuCache::stats() const {renderer_.stats();auto out=stats_;out.frames=entries_.size();return out;}
}
