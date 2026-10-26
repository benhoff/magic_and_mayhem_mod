#include "sprite_batch.hpp"
#include <algorithm>
#include <stdexcept>

namespace mnm::render {
namespace {
void viewport(Rect r,int w,int h) {
    if(w<1 || h<1 || w>2048 || h>2048 || r.left<0 || r.top<0 ||
       r.right<=r.left || r.bottom<=r.top || r.right>w || r.bottom>h)
        throw std::invalid_argument("Sprite viewport outside destination");
}
std::vector<PreparedSpriteFrame> prepare(const SpriteDrawPlan& plan) {
    if(!plan.source || plan.placements.size()>4096)
        throw std::invalid_argument("Missing or oversized sprite batch");
    // Bound aggregate work BEFORE allocating any prepared plane.
    std::uint64_t pixels=0;
    for(const auto& p:plan.placements) {
        const auto& f=plan.source->frames.at(p.frame);
        if(f.width>2048 || f.height>2048) throw std::length_error("Sprite batch frame dimensions");
        pixels+=std::uint64_t(f.width)*f.height;
        if(pixels>1024*1024)
            throw std::length_error("Sprite batch pixel budget");
    }
    std::vector<PreparedSpriteFrame> result;result.reserve(plan.placements.size());
    for(const auto& p:plan.placements)
        result.push_back(prepareSpriteFrame(*plan.source,p.frame,p.colours?&*p.colours:nullptr));
    return result;
}
}
void drawSpritePixels(Image& image,const SpriteDrawPlan& plan,Rect clip) {
    viewport(clip,image.width,image.height);
    if(image.pixels.size()!=std::size_t(image.width)*image.height ||
       std::any_of(image.pixels.begin(),image.pixels.end(),[](auto p){return p>65535;}))
        throw std::invalid_argument("Sprite batch destination requires complete RGB565 words");
    auto frames=prepare(plan);auto candidate=image;
    for(std::size_t i=0;i<frames.size();++i) {
        const auto& f=frames[i];const auto& p=plan.placements[i];
        const auto x=std::int64_t(p.x)-f.originX,y=std::int64_t(p.y)-f.originY;
        const auto left=std::max<std::int64_t>(x,clip.left),top=std::max<std::int64_t>(y,clip.top);
        const auto right=std::min<std::int64_t>(x+f.pixels.width,clip.right),bottom=std::min<std::int64_t>(y+f.pixels.height,clip.bottom);
        for(auto row=top;row<bottom;++row) for(auto col=left;col<right;++col) {
            const auto from=std::size_t(row-y)*f.pixels.width+std::size_t(col-x);
            if(f.mask.pixels[from]) candidate.pixels[std::size_t(row)*candidate.width+std::size_t(col)]=f.pixels.pixels[from];
        }
    }
    image=std::move(candidate);
}
void drawSpriteBatch(GlBlitter& renderer,SurfaceId destination,const SpriteDrawPlan& plan,Rect clip) {
    const auto size=renderer.dimensions(destination);viewport(clip,size.width(),size.height());
    auto frames=prepare(plan);
    std::vector<std::unique_ptr<UploadedSpriteFrame>> uploads;uploads.reserve(frames.size());
    for(auto& f:frames) {
        auto upload=std::make_unique<UploadedSpriteFrame>(renderer,std::move(f));
        auto bytes=std::size_t(-1);while(!upload->advanceUpload(bytes)) {}
        uploads.push_back(std::move(upload));
    }
    for(std::size_t i=0;i<uploads.size();++i)
        uploads[i]->drawClipped(destination,plan.placements[i].x,plan.placements[i].y,clip);
}
}
