#include "native_frame_recipe.hpp"
#include "command_state.hpp"
#include "../protocols/include/mnm/render_stream_v5.h"
#include "../protocols/include/mnm/resident_frame_budget.h"
#include <stdexcept>
namespace mnm::render {
namespace {const PixelFormat tags{32,{0xff0000,0xff00,0xff}};}
NativeFrameRecipe::NativeFrameRecipe(GlBlitter& renderer,unsigned world,std::uint64_t generation,const std::map<unsigned,SurfaceId>& handles):renderer_(renderer),world_(world),generation_(generation),worldSize_(renderer.dimensions(handles.at(world))){
 if(!world || !generation)throw std::invalid_argument("Prepared native frame identity");
 worldIdentity_=handles.at(world);ensure(world,handles,true);
}
NativeFrameRecipe::~NativeFrameRecipe(){for(auto& pair:maps_)renderer_.destroy(pair.second);drop(work_);drop(coordinates_);drop(ui_);}
void NativeFrameRecipe::drop(SurfaceId& surface){if(surface){const auto s=renderer_.dimensions(surface);pixels_-=std::size_t(s.width())*s.height();renderer_.destroy(surface);surface=0;}}
void NativeFrameRecipe::budget(std::size_t pixels,unsigned count) const {
 if(surfaces()+count>MNM_RESIDENT_PREPARED_HANDLES || pixels>MNM_RESIDENT_PREPARED_PIXELS-pixels_)throw std::runtime_error("Compiled native frame budget");
}
SurfaceId NativeFrameRecipe::ensure(unsigned id,const std::map<unsigned,SurfaceId>& handles,bool world){
 const auto found=maps_.find(id);if(found!=maps_.end())return found->second;
 const auto size=renderer_.dimensions(handles.at(id));const auto count=std::size_t(size.width())*size.height();budget(count);
 Image image{size.width(),size.height(),std::vector<std::uint32_t>(count,UINT32_MAX)};
 if(world)for(std::size_t i=0;i<count;++i)image.pixels[i]=std::uint32_t(i);
 const auto surface=renderer_.create(image,tags);
 try{maps_.emplace(id,surface);}catch(...){renderer_.destroy(surface);throw;}
 pixels_+=count;return surface;
}
void NativeFrameRecipe::copy(const SurfaceCommand& c,const std::map<unsigned,SurfaceId>& handles,Rect r,int x,int y,std::optional<std::uint32_t> key){
 const auto& w=c.words;const auto src=maps_.find(w[0]);
 if(src==maps_.end() && !maps_.count(w[1]))return;
 const std::optional<SurfaceId> source=src==maps_.end()?std::nullopt:std::optional<SurfaceId>(src->second);
 const auto destination=ensure(w[1],handles);
 renderer_.copyCoordinates(handles.at(w[0]),source,destination,r,x,y,key);
}
void NativeFrameRecipe::record(const SurfaceCommand& c,const std::map<unsigned,SurfaceId>& handles){
 const auto& w=c.words;
 switch(c.operation){case MNM_RENDER_STREAM_V5_OPERATION_CPU_READ:case 5:case 8:case 10:case 12:case 15:case MNM_RENDER_STREAM_V3_OPERATION_SURFACE_RESULT_CHECK:return;
 case 7:{auto it=maps_.find(w[0]);if(it!=maps_.end()){auto id=it->second;drop(id);maps_.erase(it);}return;}
 default:break;}
 if(++recorded_>8192)throw std::runtime_error("Compiled native frame command budget");
 dirty_=true;
 switch(c.operation){
 case 1:return;
 case 2:case MNM_RENDER_STREAM_V4_OPERATION_MASKED_UPDATE:
  if(maps_.count(w[0])){
   Image flags{c.image.width,c.image.height,std::vector<std::uint32_t>(c.image.pixels.size(),UINT32_MAX)};
   if(c.operation==2)renderer_.update(maps_.at(w[0]),int(w[1]),int(w[2]),flags);
   else renderer_.updateCoveredWords(maps_.at(w[0]),int(w[1]),int(w[2]),flags,c.coverage);
  }return;
 case 3:copy(c,handles,{int(w[2]),int(w[3]),int(w[4]),int(w[5])},int(w[6]),int(w[7]),w[8]?std::optional<std::uint32_t>(w[9]):std::nullopt);return;
 case 4:return;
 case 11:
  if(maps_.count(w[0])||maps_.count(w[1])){auto first=ensure(w[0],handles),second=ensure(w[1],handles);renderer_.swapContents(first,second);}return;
 case MNM_RENDER_STREAM_V3_OPERATION_CLIPPER_SET:return;
 case MNM_RENDER_STREAM_V3_OPERATION_SURFACE_COPY:{
  const auto source=renderer_.dimensions(handles.at(w[0])),destination=renderer_.dimensions(handles.at(w[1]));
  const auto plan=planSurfaceCopy(source.width(),source.height(),destination.width(),destination.height(),renderer_.surfaceClipper(handles.at(w[1])),detail::commandSurfaceCopy(c));
  for(const auto& piece:plan.pieces)copy(c,handles,piece.source,piece.x,piece.y,std::nullopt);
  return;
 }
 default:throw std::runtime_error("Unsupported compiled native frame operation");
 }
}
void NativeFrameRecipe::palette(unsigned,const std::vector<Rgb>&,unsigned,const std::map<unsigned,SurfaceId>&){dirty_=true;}
void NativeFrameRecipe::seal(unsigned id,const std::map<unsigned,SurfaceId>& handles){
 const auto original=handles.at(id);const auto size=renderer_.dimensions(original);const auto count=std::size_t(size.width())*size.height();
 if(final_!=id){drop(work_);drop(coordinates_);drop(ui_);}
 if(!ui_){budget(count);ui_=renderer_.cloneSurface(original);pixels_+=count;}else renderer_.restoreSurface(original,ui_);
 const auto found=maps_.find(id);
 if(found!=maps_.end()){
  if(!coordinates_){budget(count);coordinates_=renderer_.cloneSurface(found->second);pixels_+=count;}else renderer_.restoreSurface(found->second,coordinates_);
  if(!work_){budget(count);work_=renderer_.allocate(size.width(),size.height(),{16,{0xf800,0x7e0,31}});pixels_+=count;}
 }else{drop(work_);drop(coordinates_);}
 final_=id;finalIdentity_=original;sealedSourceGeneration_=generation_;dirty_=false;recorded_=0;
}
bool NativeFrameRecipe::restart(unsigned world,std::uint64_t generation,const std::map<unsigned,SurfaceId>& handles){
 const auto source=handles.find(world),final=handles.find(final_);
 if(!sealedReady() || !hasWorldOutput() || world!=world_ || generation_==UINT64_MAX || generation!=generation_+1 ||
    source==handles.end() || source->second!=worldIdentity_ || final==handles.end() || final->second!=finalIdentity_ ||
    renderer_.dimensions(source->second)!=worldSize_ || renderer_.dimensions(final->second)!=renderer_.dimensions(ui_))return false;
 for(auto& pair:maps_){auto id=pair.second;drop(id);}maps_.clear();
 generation_=generation;dirty_=true;recorded_=0;ensure(world,handles,true);return true;
}
GpuFrame NativeFrameRecipe::render(const GpuFrame& world){
 if(!ready())throw std::logic_error("Incomplete compiled native frame");
 return renderSealed(world);
}
GpuFrame NativeFrameRecipe::renderSealed(const GpuFrame& world){
 if(!sealedReady() || !world.valid() || world.size()!=worldSize_)throw std::logic_error("No sealed compiled native frame");
 if(!coordinates_)return renderer_.presentGpu(ui_);
 renderer_.assembleMappedWorld(work_,ui_,coordinates_,world);return renderer_.presentGpu(work_);
}
}
