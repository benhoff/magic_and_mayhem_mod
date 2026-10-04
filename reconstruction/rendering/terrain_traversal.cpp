#include "terrain_traversal.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>
namespace mnm::reconstruction {
namespace {
std::int32_t narrow(std::int64_t n){
    if(n<std::numeric_limits<std::int32_t>::min() || n>std::numeric_limits<std::int32_t>::max())
        throw std::out_of_range("Terrain traversal arithmetic overflow");
    return std::int32_t(n);
}
}
std::vector<TerrainVisit> traverseTerrain(std::uint32_t width,std::uint32_t height,
 std::uint32_t layers,const TerrainCamera& c,TerrainViewport v,
 const std::function<std::array<std::uint16_t,2>(std::uint32_t,std::uint32_t,std::uint32_t)>& flags){
    if(c.view!=0 || !width || !height || !layers || width>128 || height>128 || layers>32 ||
       !c.span || c.span>std::min(width,height) || c.diagonal>std::min(width,height) ||
       c.column<0 || c.column>=int(width) || c.row<0 || c.row>=int(height) ||
       !c.cutLevel || c.cutLevel>layers || c.mode>1 || v.left>=v.right || v.top>=v.bottom || !flags)
        throw std::invalid_argument("Unsupported orientation-zero terrain camera/grid domain");
    const auto sum=narrow(std::int64_t(c.f45)+c.f49);
    const auto originX=narrow(std::int64_t(c.f11)+c.f51/2-c.f45+c.f49);
    const auto originY=narrow(std::int64_t(c.f15)+16*(std::int64_t(c.f41)-2*c.diagonal)+c.f55/2+c.f4d-(std::uint32_t(sum)>>1));
    const auto marginX=narrow(std::int64_t(v.left)-64),marginY=narrow(std::int64_t(v.top)-48);
    int startX=c.column-int(c.diagonal),startY=c.row-int(c.diagonal);
    if(startX<0)startX+=width;
    if(startY<0)startY+=height;
    std::vector<TerrainVisit> result;
    for(unsigned z=0;z<layers;++z){
        if(c.mode==0 && z>c.cutLevel)continue; // Original inclusive cut in this orientation.
        for(unsigned r=0;r<c.span;++r){
            auto x=narrow(std::int64_t(originX)-32*r),y=narrow(std::int64_t(originY)+16*r-16*z);
            std::int64_t skip=0;
            if(x<=marginX){const auto n=(std::int64_t(marginX)-x)/32;skip+=n;x=narrow(x+32*n);y=narrow(y+16*n);}
            if(y<=marginY){const auto n=(std::int64_t(marginY)-y)/16;skip+=n;x=narrow(x+32*n);y=narrow(y+16*n);}
            if(skip>=c.span || x>=v.right || y>=v.bottom)continue;
            const auto end=std::min<std::int64_t>({c.span,skip+(std::int64_t(v.right)-x+32)/32,skip+(std::int64_t(v.bottom)-y+16)/16});
            for(auto col=skip;col<end;++col,x=narrow(std::int64_t(x)+32),y=narrow(std::int64_t(y)+16)){
                const std::int64_t unwrappedX=startX+col,unwrappedY=startY+r;
                const auto mx=unsigned(unwrappedX%width),my=unsigned(unwrappedY%height);
                const auto f=flags(mx,my,z);
                if((f[1]&0x4000) || (z!=c.specialLayer && (f[0]&0x80)))continue;
                const auto priority=narrow((unwrappedX/width)*c.wrapColumnPriority+std::int64_t(unwrappedY/height)*c.wrapRowPriority);
                result.push_back({mx,my,z,x,y,priority});
            }
        }
    }
    return result;
}
}
