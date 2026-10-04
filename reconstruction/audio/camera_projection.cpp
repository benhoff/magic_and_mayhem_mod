#include "camera_projection.hpp"
#include <limits>
#include <stdexcept>
namespace mnm::reconstruction::audio {
namespace {
std::int32_t narrow(std::int64_t value){
    if(value<std::numeric_limits<std::int32_t>::min() || value>std::numeric_limits<std::int32_t>::max())
        throw std::out_of_range("Outside inspected nonoverflow camera arithmetic domain");
    return std::int32_t(value);
}
void orientation(const CameraState& c){if(c.orientation>3)throw std::invalid_argument("Expected camera orientation 0..3");}
std::int32_t add(std::int32_t a,std::int32_t b){return narrow(std::int64_t(a)+b);}
std::int32_t sub(std::int32_t a,std::int32_t b){return narrow(std::int64_t(a)-b);}
std::int32_t wrap(std::int32_t value,std::int32_t extent){
    if(extent<=0)throw std::invalid_argument("Positive map extent required");
    auto result=value%extent;return result<0?result+extent:result;
}
}
CameraPoint cameraScreenOrigin(const CameraState& c){
    orientation(c);
    auto x=add(c.f11,c.f51/2);
    auto y=add(c.f15,add(add(narrow(std::int64_t(sub(c.f41,narrow(2ll*c.f5d)))*16),c.f55/2),c.f4d));
    const auto sum=add(c.f45,c.f49),difference=sub(c.f45,c.f49);
    const auto logicalHalf=std::int32_t(std::uint32_t(sum)>>1); // SHR, not SAR/division.
    switch(c.orientation){
    case 0:x=sub(x,difference);y=sub(y,logicalHalf);break;
    case 1:x=sub(x,sum);y=add(y,difference/2);break;
    case 2:x=add(x,difference);y=add(y,logicalHalf);break;
    case 3:x=add(x,sum);y=sub(y,difference/2);break;
    }
    return {x,y};
}
CameraPoint screenToMap(const CameraState& c,CameraPoint screen){
    const auto origin=cameraScreenOrigin(c);
    const auto rx=sub(screen.x,origin.x);
    const auto ry=sub(screen.y,add(origin.y,sub(24,narrow(16ll*c.f61))));
    const auto b=add(narrow(2ll*ry),56);
    std::int32_t x=0,y=0;
    switch(c.orientation){
    case 0:x=add(sub(c.f39,c.f5d),add(b,rx)/64);y=add(sub(c.f3d,c.f5d),sub(b,rx)/64);break;
    case 1:x=add(add(c.f39,c.f5d),sub(rx,b)/64);y=add(sub(c.f3d,c.f5d),add(b,rx)/64);break;
    case 2:x=sub(add(c.f39,c.f5d),add(b,rx)/64);y=add(add(c.f3d,c.f5d),sub(rx,b)/64);break;
    case 3:x=add(sub(c.f39,c.f5d),sub(b,rx)/64);y=sub(add(c.f3d,c.f5d),add(b,rx)/64);break;
    }
    return {wrap(x,c.mapWidth),wrap(y,c.mapHeight)};
}
CameraPoint audioListener(const CameraState& c,bool large){return screenToMap(c,large?CameraPoint{400,300}:CameraPoint{320,240});}
PositionalControls cameraPositionalControls(const CameraState& c,bool large,PositionalInput source){
    const auto listener=audioListener(c,large);source.listenerX=listener.x;source.listenerY=listener.y;
    source.mapWidth=c.mapWidth;source.mapHeight=c.mapHeight;source.orientation=c.orientation;
    return positionalControls(source);
}
}
