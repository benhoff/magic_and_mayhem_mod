#include "terrain_camera.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>
namespace mnm::reconstruction {
namespace {
void dimensions(std::uint32_t w,std::uint32_t h,std::uint32_t l=1){
 if(!w || !h || !l || w>128 || h>128 || l>32)throw std::invalid_argument("Camera dimensions");
}
std::int32_t checked(std::int64_t n){
 if(n<std::numeric_limits<std::int32_t>::min() || n>std::numeric_limits<std::int32_t>::max())throw std::overflow_error("Camera arithmetic");
 return static_cast<std::int32_t>(n);
}
void axis(std::int32_t& cell,std::int32_t& fraction,std::uint32_t extent,std::int32_t delta){
 const auto total=checked(std::int64_t(cell)*32+fraction+delta);
 // 4f7b50 uses floor division and normalizes exact negative multiples to zero remainder.
 const auto whole=total<0 ? (std::int64_t(total)-31)/32 : total/32;
 auto rem=total<0 ? 32-((-std::int64_t(total))%32) : total%32;
 if(rem==32)rem=0;
 cell=static_cast<std::int32_t>((whole%extent+extent)%extent);fraction=static_cast<std::int32_t>(rem);
}
}
void bindTerrainCamera(TerrainCamera& c,std::uint32_t w,std::uint32_t h,std::uint32_t l){
 dimensions(w,h,l);c.view=0;c.cutLevel=l;c.span=std::min(w,h);c.diagonal=c.span/2;
 // 4f7930 rebind preserves position, viewport and fractional offsets.
}
void setTerrainCameraViewport(TerrainCamera& c,TerrainViewport r){
 const auto w=checked(std::int64_t(r.right)-r.left),h=checked(std::int64_t(r.bottom)-r.top);
 if(w<=0 || h<=0)throw std::invalid_argument("Camera viewport");
 c.f11=r.left;c.f15=r.top;c.f51=w;c.f55=h;
}
void setTerrainCameraPosition(TerrainCamera& c,std::uint32_t w,std::uint32_t h,std::uint32_t l,
 std::uint32_t x,std::uint32_t y,std::uint32_t z){
 dimensions(w,h,l);c.column=(x>>5)%w;c.row=(y>>5)%h;c.f41=std::min(z>>4,l-1);
 c.f45=x&31;c.f49=y&31;c.f4d=z&15;
}
void scrollTerrainCamera(TerrainCamera& c,std::uint32_t w,std::uint32_t h,std::int32_t x,std::int32_t y){
 dimensions(w,h);if(c.view>3)throw std::invalid_argument("Camera orientation");
 if(!x && !y)return;
 std::int64_t a=0,b=0;
 switch(c.view){case 0:a=std::int64_t(x)+y;b=std::int64_t(y)-x;break;
 case 1:a=std::int64_t(x)-y;b=std::int64_t(x)+y;break;
 case 2:a=-std::int64_t(x)-y;b=std::int64_t(x)-y;break;
 case 3:a=std::int64_t(y)-x;b=-std::int64_t(x)-y;break;}
 auto next=c;axis(next.column,next.f45,w,checked(a));axis(next.row,next.f49,h,checked(b));c=next;
}
}
