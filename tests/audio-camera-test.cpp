#include "camera_projection.hpp"
#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>
namespace r=mnm::reconstruction::audio;
void check(bool b,const char* message){if(!b)throw std::runtime_error(message);}
using I=std::int64_t;
I trunc(I n,I d){return n<0?-((-n)/d):n/d;}
int wrap(I n,int extent){while(n<0)n+=extent;while(n>=extent)n-=extent;return int(n);}
r::CameraPoint originOracle(const r::CameraState& c){
    const I baseX=c.f11+trunc(c.f51,2),baseY=c.f15+16*(I(c.f41)-2*c.f5d)+trunc(c.f55,2)+c.f4d;
    const I sum=I(c.f45)+c.f49,difference=I(c.f45)-c.f49;
    const std::array<I,4> x={-difference,-sum,difference,sum};
    const I unsignedHalf=std::uint32_t(sum)/2;
    const std::array<I,4> y={-unsignedHalf,trunc(difference,2),unsignedHalf,-trunc(difference,2)};
    return {int(baseX+x[c.orientation]),int(baseY+y[c.orientation])};
}
r::CameraPoint mapOracle(const r::CameraState& c,r::CameraPoint screen){
    const auto origin=originOracle(c);const I horizontal=I(screen.x)-origin.x;
    const I vertical=2*(I(screen.y)-origin.y-24+16*c.f61)+56;
    const I u=trunc(vertical+horizontal,64),v=trunc(vertical-horizontal,64);
    const std::array<I,4> dx={u,-v,-u,v},dy={v,u,-v,-u};
    const int signsX[]={-1,1,1,-1},signsY[]={-1,-1,1,1};
    return {wrap(c.f39+dx[c.orientation]+signsX[c.orientation]*I(c.f5d),c.mapWidth),
            wrap(c.f3d+dy[c.orientation]+signsY[c.orientation]*I(c.f5d),c.mapHeight)};
}
void same(r::CameraPoint a,r::CameraPoint b,const char* message){check(a.x==b.x && a.y==b.y,message);}
void matrix(){
    for(unsigned rotation=0;rotation<4;++rotation)for(bool smallMap:{false,true})for(int offset:{-2,0,3})for(int step:{-1,0,2})for(bool large:{false,true}){
        r::CameraState c;c.orientation=rotation;c.mapWidth=smallMap?7:128;c.mapHeight=smallMap?9:256;
        c.f11=-7;c.f15=9;c.f39=-129;c.f3d=513;c.f41=5;c.f45=17;c.f49=20;c.f4d=-11;
        c.f51=large?800:639;c.f55=large?600:479;c.f5d=offset;c.f61=step;
        same(r::cameraScreenOrigin(c),originOracle(c),"origin offsets, odd spans and signed half truncation");
        for(auto point:{r::CameraPoint{0,0},{320,240},{400,300},{-129,-65},{-1,-1},{639,479},{800,600},{1200,-300}})
            same(r::screenToMap(c,point),mapOracle(c,point),"independent rotated u/v projection and iterative wrap oracle");
        same(r::audioListener(c,large),mapOracle(c,large?r::CameraPoint{400,300}:r::CameraPoint{320,240}),"audio resolution flag center selection");
    }
}
void truncationBoundaries(){
    r::CameraState c;c.mapWidth=1024;c.mapHeight=1024;
    for(unsigned rotation=0;rotation<4;++rotation){c.orientation=rotation;
        for(int x=-65;x<=65;++x)for(int y=-65;y<=65;++y)
            same(r::screenToMap(c,{x,y}),mapOracle(c,{x,y}),"negative and positive 64-pixel division boundaries");}
    c.orientation=0;c.f45=-1;c.f49=0;
    same(r::cameraScreenOrigin(c),originOracle(c),"origin SHR keeps logical semantics for negative sum");
    c={};c.orientation=4;bool invalid=false;try{r::cameraScreenOrigin(c);}catch(const std::invalid_argument&){invalid=true;}check(invalid,"unsupported rotation rejected");
    c.orientation=0;invalid=false;try{r::screenToMap(c,{});}catch(const std::invalid_argument&){invalid=true;}check(invalid,"zero map dimensions rejected");
    c.mapWidth=128;c.mapHeight=128;c.f41=std::numeric_limits<int>::max();bool overflow=false;
    try{r::cameraScreenOrigin(c);}catch(const std::out_of_range&){overflow=true;}check(overflow,"unvalidated overflow domain explicitly rejected");
}
void positionalIntegration(){
    r::CameraState c;c.mapWidth=128;c.mapHeight=256;c.f51=640;c.f55=480;c.f39=50;c.f3d=60;
    const int bx[]={-6,6,6,-6},by[]={-6,-6,6,6};
    for(unsigned rotation=0;rotation<4;++rotation){c.orientation=rotation;const auto listener=r::audioListener(c,false);
        r::PositionalInput input;input.sourceX=listener.x+bx[rotation];input.sourceY=listener.y+by[rotation];input.range=10;input.panWidth=5;
        const auto controls=r::cameraPositionalControls(c,false,input);check(controls.volume==0 && controls.panWritten && controls.pan==0,"camera listener feeds biased positional center for every rotation");
        input.sourceX+=40;check(r::cameraPositionalControls(c,false,input).volume==-5000,"camera-to-positional cutoff");}
}
int main(){try{matrix();truncationBoundaries();positionalIntegration();std::cout<<"Camera projection passed: 1152 screen/map cases, 68644 division-boundary cases and four-orientation positional integration\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
