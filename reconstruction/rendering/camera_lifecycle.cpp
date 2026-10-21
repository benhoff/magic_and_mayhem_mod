#include "camera_lifecycle.hpp"
#include <stdexcept>
namespace mnm::reconstruction {
namespace {
void inputs(unsigned w,unsigned h,unsigned l,TerrainViewport viewport){
 TerrainCamera checked;bindTerrainCamera(checked,w,h,l);setTerrainCameraViewport(checked,viewport);
}
void rotationInputs(const TerrainCameraLifecycle& state,std::optional<unsigned> primary){
 if(state.camera.view>3||(primary&&*primary>3))throw std::invalid_argument("Camera rotation view");
}
}
bool initializeTerrainCamera(TerrainCameraLifecycle& state,unsigned w,unsigned h,unsigned l,
 TerrainViewport viewport,std::uint32_t a,std::uint32_t b,bool admitted){
 inputs(w,h,l,viewport);state.clockwise=false;state.counterclockwise=false;
 if(!admitted)return false;
 auto next=state;bindTerrainCamera(next.camera,w,h,l);setTerrainCameraViewport(next.camera,viewport);
 next.camera.f41=0;next.width=w;next.height=h;next.layers=l;next.parameter29=a;next.parameter2d=b;
 next.requestedView=0;next.rendered=0;state=next;return true;
}
std::vector<CameraRefresh> rotateTerrainCamera(TerrainCameraLifecycle& state,CameraTurn turn,
 const std::vector<bool>& creatures,const std::vector<bool>& effects,std::optional<unsigned> primary){
 rotationInputs(state,primary);
 if(turn!=CameraTurn::clockwise&&turn!=CameraTurn::counterclockwise)throw std::invalid_argument("Camera turn");
 auto next=state;next.camera.view=(next.camera.view+(turn==CameraTurn::clockwise?1:3))&3;
 next.requestedView=next.camera.view;next.rendered=0;next.redrawFrames=3;
 const auto view=primary.value_or(next.camera.view);std::vector<CameraRefresh> requests;
 if(creatures.size()>65536||effects.size()>65536)throw std::length_error("Camera consumer pool bounds");
 requests.reserve(creatures.size()+effects.size()+1);
 for(std::size_t i=0;i<creatures.size();++i)if(creatures[i])requests.push_back({CameraRefresh::Kind::creature,i,view});
 for(std::size_t i=0;i<effects.size();++i)if(effects[i])requests.push_back({CameraRefresh::Kind::effect,i,view});
 requests.push_back({CameraRefresh::Kind::minimap,0,view});state=next;return requests;
}
void beginTerrainCameraFrame(TerrainCameraLifecycle& state){if(state.redrawFrames)--state.redrawFrames;}
std::vector<CameraRefresh> finishTerrainCameraFrame(TerrainCameraLifecycle& state,
 const std::vector<bool>& creatures,const std::vector<bool>& effects,std::optional<unsigned> primary){
 auto next=state;next.rendered=1;std::vector<CameraRefresh> requests;
 if(next.clockwise){requests=rotateTerrainCamera(next,CameraTurn::clockwise,creatures,effects,primary);next.clockwise=false;}
 else if(next.counterclockwise){requests=rotateTerrainCamera(next,CameraTurn::counterclockwise,creatures,effects,primary);next.counterclockwise=false;}
 state=next;return requests;
}
bool TerrainViewportRegistry::append(ViewportRegistration entry){
 if(entries_.size()==400)return false;
 // Original receivers have identity independent of the dense slot. Native
 // service refuses duplicate ownership, rather than retaining aliased receivers.
 if(!entry.identity)throw std::invalid_argument("Viewport identity");
 for(const auto& old:entries_)if(old.identity==entry.identity)throw std::invalid_argument("Duplicate viewport identity");
 entries_.push_back(entry);return true;
}
void TerrainViewportRegistry::remove(std::optional<std::size_t> index){
 if(!index)return;
 if(*index>=entries_.size())throw std::out_of_range("Viewport retirement index");
 if(selected_){if(*selected_==*index)selected_.reset();else if(*selected_>*index)--*selected_;}
 entries_.erase(entries_.begin()+*index);
}
void TerrainViewportRegistry::clear(){entries_.clear();selected_.reset();}
void TerrainViewportRegistry::select(std::optional<std::size_t> index){if(index&&*index>=entries_.size())throw std::out_of_range("Viewport selected index");selected_=index;}
std::optional<std::size_t> TerrainViewportRegistry::hit(std::int32_t x,std::int32_t y) const {
 for(auto i=entries_.size();i;--i){const auto& e=entries_[i-1];if(e.flags&&x>=e.rectangle.left&&x<e.rectangle.right&&y>=e.rectangle.top&&y<e.rectangle.bottom)return i-1;}
 return {};
}
}
