#include "surface_copy.hpp"
#include <algorithm>
#include <stdexcept>

namespace mnm::render {
namespace {
bool inside(Rect r,int w,int h){
    return r.left>=0 && r.top>=0 && r.left<r.right && r.top<r.bottom && r.right<=w && r.bottom<=h;
}
bool intersects(Rect a,Rect b){
    return std::max(a.left,b.left)<std::min(a.right,b.right) &&
           std::max(a.top,b.top)<std::min(a.bottom,b.bottom);
}
}
void validateClipper(const ClipperState& c,int w,int h){
    if(!c.attached && c.regions)throw std::runtime_error("Detached clipper has regions");
    if(!c.regions)return;
    if(c.regions->size()>maxClipRegions)throw std::runtime_error("Clip region budget exceeded");
    for(std::size_t i=0;i<c.regions->size();++i){
        if(!inside((*c.regions)[i],w,h))throw std::runtime_error("Clip region outside destination");
        for(std::size_t j=0;j<i;++j)if(intersects((*c.regions)[i],(*c.regions)[j]))
            throw std::runtime_error("Overlapping clip regions are unsupported");
    }
}
SurfaceCopyPlan planSurfaceCopy(int sw,int sh,int dw,int dh,const ClipperState& c,const SurfaceCopyRequest& r){
    if(sw<1 || sh<1 || dw<1 || dh<1 || sw>2048 || sh>2048 || dw>2048 || dh>2048)
        throw std::runtime_error("Invalid copy surface dimensions");
    validateClipper(c,dw,dh);
    if(r.api!=SurfaceCopyApi::Blt && r.api!=SurfaceCopyApi::BltFast)
        throw std::runtime_error("Unknown surface copy API");
    const auto width=std::int64_t(r.source.right)-r.source.left;
    const auto height=std::int64_t(r.source.bottom)-r.source.top;
    const bool fast=r.api==SurfaceCopyApi::BltFast;
    // Widen first: even INT_MIN/INT_MAX inputs never overflow host arithmetic.
    const auto right=fast?std::int64_t(r.destination.left)+width:r.destination.right;
    const auto bottom=fast?std::int64_t(r.destination.top)+height:r.destination.bottom;
    const auto failed=[](std::uint32_t h){return SurfaceCopyPlan{{},h};};
    if(fast){
        if(width<0 || height<0 || r.destination.left<0 || r.destination.top<0 || right>dw || bottom>dh)
            return failed(surfaceStatus::invalidRect);
        if(c.attached)return failed(surfaceStatus::fastCannotClip);
        if(!width || !height || !inside(r.source,sw,sh))return failed(surfaceStatus::invalidRect);
    }else{
        if(width<=0 || height<=0 || r.destination.left>=right || r.destination.top>=bottom)
            return failed(surfaceStatus::invalidRect);
        if(width!=right-r.destination.left || height!=bottom-r.destination.top)
            throw std::runtime_error("Stretched surface copy is unsupported");
        if(c.attached && !c.regions)return failed(surfaceStatus::noClipList);
        if(!c.attached && (!inside(r.source,sw,sh) || !inside(r.destination,dw,dh)))
            return failed(surfaceStatus::invalidRect);
    }
    if(r.sourceBusy || r.destinationBusy)return failed(surfaceStatus::busy);
    const auto wait=fast?0x10u:0x01000000u,key=fast?1u:0x8000u;
    if(r.flags!=0 && r.flags!=wait && r.flags!=key && r.flags!=0x80000000u)
        throw std::runtime_error("Unvalidated surface copy flags");
    if(!fast && r.flags==key)return failed(surfaceStatus::invalidArgument);
    if(!fast && r.flags==0x80000000u)return failed(surfaceStatus::notImplemented);
    // Fast ignores the two selected missing-key/unsupported-flag cases, as captured.
    const std::vector<Rect> whole{{0,0,dw,dh}};
    const auto& regions=c.attached?*c.regions:whole;
    SurfaceCopyPlan plan;
    for(auto region:regions){
        const auto left=std::max(std::int64_t(region.left),std::int64_t(r.destination.left));
        const auto top=std::max(std::int64_t(region.top),std::int64_t(r.destination.top));
        const auto endX=std::min(std::int64_t(region.right),right),endY=std::min(std::int64_t(region.bottom),bottom);
        if(left>=endX || top>=endY)continue;
        const auto sl=std::int64_t(r.source.left)+left-r.destination.left;
        const auto st=std::int64_t(r.source.top)+top-r.destination.top;
        const auto sr=sl+endX-left,sb=st+endY-top;
        if(sl<0 || st<0 || sr>sw || sb>sh){plan.hresult=surfaceStatus::invalidRect;break;}
        plan.pieces.push_back({{int(sl),int(st),int(sr),int(sb)},int(left),int(top)});
    }
    return plan;
}
}
