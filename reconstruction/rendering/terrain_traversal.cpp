#include "terrain_traversal.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>
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
    if(c.view>3 || !width || !height || !layers || width>128 || height>128 || layers>32 ||
       !c.span || c.span>std::min(width,height) || c.diagonal>std::min(width,height) ||
       c.column<0 || c.column>=int(width) || c.row<0 || c.row>=int(height) ||
       !c.cutLevel || c.cutLevel>layers || c.mode>1 || v.left>=v.right || v.top>=v.bottom || !flags)
        throw std::invalid_argument("Unsupported terrain camera/grid domain");
    const auto sum=narrow(std::int64_t(c.f45)+c.f49);
    const auto difference=narrow(std::int64_t(c.f45)-c.f49);
    auto originX=narrow(std::int64_t(c.f11)+c.f51/2);
    auto originY=narrow(std::int64_t(c.f15)+16*(std::int64_t(c.f41)-2*c.diagonal)+c.f55/2+c.f4d);
    switch(c.view){
    case 0:originX=narrow(std::int64_t(originX)-difference);originY=narrow(std::int64_t(originY)-(std::uint32_t(sum)>>1));break;
    case 1:originX=narrow(std::int64_t(originX)-sum);originY=narrow(std::int64_t(originY)+difference/2);break;
    case 2:originX=narrow(std::int64_t(originX)+difference);originY=narrow(std::int64_t(originY)+(std::uint32_t(sum)>>1));break;
    case 3:originX=narrow(std::int64_t(originX)+sum);originY=narrow(std::int64_t(originY)-difference/2);break;
    }
    const auto marginX=narrow(std::int64_t(v.left)-64),marginY=narrow(std::int64_t(v.top)-48);
    int startX=c.column+((c.view==1 || c.view==2)?1:-1)*int(c.diagonal),startY=c.row+((c.view>=2)?1:-1)*int(c.diagonal);
    if(startX<0)startX+=width;
    if(startY<0)startY+=height;
    if(startX>=int(width))startX-=width;
    if(startY>=int(height))startY-=height;
    std::vector<TerrainVisit> result;
    for(unsigned z=0;z<(c.view==0?layers:c.cutLevel);++z){
        if(c.mode==0 && z>c.cutLevel)continue; // Original inclusive cut in this orientation.
        if(c.view!=0){
            // The rotated routines retain a physical pointer independently of
            // depth coordinates. Keep that distinction at their boundary carries.
            std::int64_t cell=(std::int64_t(z)*height+startY)*width+startX;
            std::int64_t mx=startX,my=startY,priority=0;
            const auto plane=std::int64_t(width)*height;
            const bool vertical=c.view!=2;const int direction=c.view==1?1:-1;
            const auto step=[&](std::int64_t n,bool reverse,bool clipped){
                const int sign=reverse?-direction:direction;
                auto& coord=vertical?my:mx;
                const std::int64_t extent=vertical?height:width;
                if(vertical && reverse && n>=height)throw std::out_of_range("Rotated row lookup exceeds recovered table");
                const auto stride=vertical?((clipped && n>=height)?height:width):1;
                coord+=sign*n;cell+=sign*n*stride;
                if((sign>0 && coord>=extent) || (sign<0 && coord<0)){
                    coord-=sign*extent;cell-=sign*(vertical?plane:width);
                    priority+=(reverse?-1:1)*std::int64_t(vertical?c.wrapRowPriority:c.wrapColumnPriority);
                }
            };
            for(unsigned r=0;r<c.span;++r){
                auto x=narrow(std::int64_t(originX)-32*r),y=narrow(std::int64_t(originY)+16*r-16*z);
                std::int64_t col=0;
                const auto skip=[&](std::int64_t n){step(n,false,true);col+=n;x=narrow(std::int64_t(x)+32*n);y=narrow(std::int64_t(y)+16*n);};
                if(x<=marginX)skip((std::int64_t(marginX)-x)/32+1);
                if(y<=marginY)skip((std::int64_t(marginY)-y)/16+1);
                if(col>=c.span){step(col-c.span,true,false);col=c.span;}
                else if(x<v.right && y<v.bottom){
                    // Unlike view zero, once a row starts it runs to span even
                    // beyond the right/bottom viewport edges.
                    for(;col<c.span;++col,x=narrow(std::int64_t(x)+32),y=narrow(std::int64_t(y)+16)){
                        if(cell<0 || cell>=plane*layers || mx<0 || mx>width || my<0 || my>=height)
                            throw std::out_of_range("Rotated traversal exceeds owned grid/coordinate domain: view="+std::to_string(c.view)+" layer="+std::to_string(z)+" row="+std::to_string(r)+" col="+std::to_string(col)+" cell="+std::to_string(cell)+" mx="+std::to_string(mx)+" my="+std::to_string(my));
                        const auto f=flags(unsigned(cell%width),unsigned((cell/width)%height),unsigned(cell/plane));
                        if(!(f[1]&0x4000) && (z==c.specialLayer || !(f[0]&0x80)))
                            result.push_back({unsigned(mx),unsigned(my),z,x,y,narrow(priority),unsigned(cell)});
                        step(1,false,false);
                    }
                }
                if(col<c.span)step(c.span-col,false,false);
                // Secondary row advance + primary reset. View two uses a
                // strict >width comparison, preserving column==width carry.
                if(c.view==1){
                    if(--mx<0){mx+=width;cell+=width;priority+=c.wrapColumnPriority;}
                    my-=c.span;cell-=1+std::int64_t(width)*c.span;
                    if(my<0){my+=height;cell+=plane;priority-=c.wrapRowPriority;}
                }else if(c.view==2){
                    if(--my<0){my+=height;cell+=plane;priority+=c.wrapRowPriority;}
                    mx+=c.span;cell+=std::int64_t(c.span)-width;
                    if(mx>width){mx-=width;cell-=width;priority-=c.wrapColumnPriority;}
                }else{
                    if(++mx>=width){mx-=width;cell-=width;priority+=c.wrapColumnPriority;}
                    my+=c.span;cell+=1+std::int64_t(width)*c.span;
                    if(my>=height){my-=height;cell-=plane;priority-=c.wrapRowPriority;}
                }
            }
            continue;
        }
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
                result.push_back({mx,my,z,x,y,priority,unsigned((std::uint64_t(z)*height+my)*width+mx)});
            }
        }
    }
    return result;
}
}
