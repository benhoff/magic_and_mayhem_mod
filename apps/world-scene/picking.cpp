#include "picking.hpp"
#include <stdexcept>
namespace mnm::scene {
namespace {
bool inside(int x,int y) {return x>=0 && y>=0 && x<512 && y<256;}
bool opaque(const Draw& d,const assets::Sprite& sprite,int x,int y) {
    const auto& f=sprite.frames.at(d.frame);
    if(std::uint64_t(f.width)*f.height!=f.opaqueMask.size()) throw std::invalid_argument("Picking mask extent differs");
    const auto col=std::int64_t(x)-d.x+f.originX,row=std::int64_t(y)-d.y+f.originY;
    if(col<0 || row<0 || col>=f.width || row>=f.height) return false;
    const auto mask=f.opaqueMask.at(std::size_t(row)*f.width+col);
    if(mask>1) throw std::invalid_argument("Picking mask must be binary");
    return mask!=0;
}
void budget(const std::vector<Draw>& q) {if(q.size()>12288+32) throw std::invalid_argument("Picking draw budget exceeded");}
}
std::optional<game::Handle> pickActor(const std::vector<Draw>& q,const assets::Sprite& terrain,const assets::Sprite& creature,int x,int y) {
    budget(q);if(!inside(x,y)) return {};
    for(auto it=q.rbegin();it!=q.rend();++it) if(opaque(*it,it->creature?creature:terrain,x,y)) return it->creature?it->actor:std::nullopt;
    return {};
}
std::optional<game::Point> pickTerrain(const std::vector<Draw>& q,const assets::Sprite& terrain,int x,int y) {
    budget(q);if(!inside(x,y)) return {};
    for(auto it=q.rbegin();it!=q.rend();++it) if(!it->creature && opaque(*it,terrain,x,y)) return it->standing;
    return {};
}
}
