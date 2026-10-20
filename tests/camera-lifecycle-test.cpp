#include "camera_lifecycle.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::reconstruction;
static void check(bool ok){if(!ok)throw std::runtime_error("Camera lifecycle unit mismatch");}
template<class F> static void refused(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}check(caught);}
int main()try{
 TerrainCameraLifecycle state;state.camera.column=-3;state.camera.f45=17;state.camera.f41=5;state.clockwise=true;state.counterclockwise=true;state.requestedView=3;state.redrawFrames=9;
 check(!initializeTerrainCamera(state,32,48,4,{-20,-10,200,150},11,12,false));check(!state.clockwise&&!state.counterclockwise&&state.camera.f41==5&&state.requestedView==3&&state.redrawFrames==9);
 check(initializeTerrainCamera(state,32,48,4,{-20,-10,200,150},11,12,true));check(state.camera.column==-3&&state.camera.f45==17&&state.camera.f41==0&&state.camera.span==32&&state.camera.diagonal==16&&state.camera.cutLevel==4&&state.camera.f51==220&&state.camera.f55==160&&state.parameter29==11&&state.parameter2d==12);
 state.clockwise=state.counterclockwise=true;beginTerrainCameraFrame(state);check(state.redrawFrames==8);
 auto requests=finishTerrainCameraFrame(state,{false,true,true},{true,false});check(requests.size()==4&&requests[0].kind==CameraRefresh::Kind::creature&&requests[0].ordinal==1&&requests[1].ordinal==2&&requests[2].kind==CameraRefresh::Kind::effect&&requests[3].kind==CameraRefresh::Kind::minimap);
 check(state.camera.view==1&&state.requestedView==1&&state.rendered==0&&state.redrawFrames==3&&!state.clockwise&&state.counterclockwise);
 finishTerrainCameraFrame(state,{},{});check(state.camera.view==0&&!state.counterclockwise&&state.redrawFrames==3);finishTerrainCameraFrame(state,{},{});check(state.rendered==1);
 requests=rotateTerrainCamera(state,CameraTurn::counterclockwise,{true},{true},2);check(state.camera.view==3&&requests[0].view==2&&requests[2].view==2);
 const auto saved=state.camera.view;refused([&]{rotateTerrainCamera(state,CameraTurn::clockwise,{}, {},4);});check(state.camera.view==saved);
 state.clockwise=true;refused([&]{initializeTerrainCamera(state,0,1,1,{},0,0,true);});check(state.clockwise&&state.camera.view==saved);
 TerrainViewportRegistry registry;check(!registry.hit(0,0));registry.append({1,{0,0,10,10},1});registry.append({2,{1,1,5,5},1});registry.append({3,{0,0,10,10},0});check(registry.hit(1,1)==1&&registry.hit(5,5)==0&&!registry.hit(10,0));
 registry.select(2);registry.remove(0);check(registry.selected()==1&&registry.entries()[0].identity==2);registry.remove(1);check(!registry.selected()&&registry.entries().size()==1);registry.remove({});refused([&]{registry.remove(8);});refused([&]{registry.append({2,{},1});});refused([&]{registry.append({0,{},1});});
 registry.clear();for(unsigned i=0;i<400;++i)check(registry.append({i+1,{},1}));check(!registry.append({401,{},1})&&registry.entries().size()==400);registry.clear();check(registry.entries().empty()&&!registry.selected());
 std::cout<<"Camera lifecycle, rotation ordering and owned viewport limits passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
