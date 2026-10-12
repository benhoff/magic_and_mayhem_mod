#include "native_frame_recipe.hpp"
#include "command_state.hpp"
#include "../protocols/include/mnm/render_stream_v4.h"
#include <stdexcept>
namespace mnm::render {
NativeFrameRecipe::NativeFrameRecipe(GlBlitter& renderer,unsigned world,std::uint64_t generation,const std::map<unsigned,SurfaceId>& handles):renderer_(renderer),world_(world),generation_(generation){
 if(!world || !generation)throw std::invalid_argument("Prepared native frame identity");
 ensure(world,handles);
}
NativeFrameRecipe::~NativeFrameRecipe(){
 for(const auto& pair:planes_){renderer_.destroy(pair.second.target);renderer_.destroy(pair.second.baseline);}
 for(const auto& pair:frozen_)renderer_.destroy(pair.second);
}
void NativeFrameRecipe::budget(SurfaceId source,unsigned copies) const {
 const auto size=renderer_.dimensions(source);const auto count=std::size_t(size.width())*size.height();
 if(surfaces()+copies>32 || count*copies>16*1024*1024-pixels_)throw std::runtime_error("Prepared native frame plane budget");
}
void NativeFrameRecipe::ensure(unsigned id,const std::map<unsigned,SurfaceId>& handles){
 if(planes_.count(id))return;
 const auto source=handles.at(id);budget(source,2);const auto size=renderer_.dimensions(source);
 const auto baseline=renderer_.cloneSurface(source);SurfaceId work=0;
 try{work=renderer_.cloneSurface(baseline);planes_.emplace(id,Plane{baseline,work});}
 catch(...){if(work)renderer_.destroy(work);renderer_.destroy(baseline);throw;}
 pixels_+=std::size_t(size.width())*size.height()*2;
}
SurfaceId NativeFrameRecipe::freeze(unsigned id,const std::map<unsigned,SurfaceId>& handles){
 const auto key=std::make_pair(id,revisions_[id]);const auto found=frozen_.find(key);
 if(found!=frozen_.end())return found->second;
 const auto source=handles.at(id);budget(source,1);const auto size=renderer_.dimensions(source);
 const auto owned=renderer_.cloneSurface(source);
 try{frozen_.emplace(key,owned);}catch(...){renderer_.destroy(owned);throw;}
 pixels_+=std::size_t(size.width())*size.height();return owned;
}
void NativeFrameRecipe::add(const SurfaceCommand& command,SurfaceId source){
 const auto bytes=command.image.pixels.size()*4+command.coverage.size()+command.colors.size()*sizeof(Rgb);
 if(bytes>16*1024*1024-bytes_)throw std::runtime_error("Prepared native frame command budget");
 bytes_+=bytes;steps_.push_back({command,source});
}
void NativeFrameRecipe::record(const SurfaceCommand& c,const std::map<unsigned,SurfaceId>& handles){
 const auto& w=c.words;
 switch(c.operation){
 case 5:case 8:case 10:case 12:case 15:case MNM_RENDER_STREAM_V3_OPERATION_SURFACE_RESULT_CHECK:return;
 case 7: // Reused logical identities must never reuse an older frozen version.
  ++revisions_[w[0]];return;
 default:break;
 }
 if(++recorded_>8192)throw std::runtime_error("Prepared native frame command budget");
 dirty_=true;
 switch(c.operation){
 case 1:++revisions_[w[0]];return;
 case 2:case 4:case MNM_RENDER_STREAM_V3_OPERATION_CLIPPER_SET:case MNM_RENDER_STREAM_V4_OPERATION_MASKED_UPDATE:
  if(planes_.count(w[0]))add(c);
  ++revisions_[w[0]];return;
 case 3:case MNM_RENDER_STREAM_V3_OPERATION_SURFACE_COPY:{
  // Capture a static source at its read boundary, after preceding real writes.
  // A destination's initial static history is folded into its owned baseline.
  if(planes_.count(w[0]) || planes_.count(w[1])){
   ensure(w[1],handles);
   const auto source=planes_.count(w[0])?0:freeze(w[0],handles);
   add(c,source);
  }
  ++revisions_[w[1]];return;
 }
 case 11:
  if(planes_.count(w[0]) || planes_.count(w[1])){ensure(w[0],handles);ensure(w[1],handles);add(c);}
  ++revisions_[w[0]];++revisions_[w[1]];return;
 default:throw std::runtime_error("Unsupported prepared native frame operation");
 }
}
void NativeFrameRecipe::palette(unsigned id,const std::vector<Rgb>& colors,unsigned first,const std::map<unsigned,SurfaceId>& handles){
 SurfaceCommand c;c.operation=4;c.words={id,first,0,0,0,0,0,0,0,0};c.colors=colors;record(c,handles);
}
void NativeFrameRecipe::seal(unsigned id,const std::map<unsigned,SurfaceId>& handles){
 frozenFinal_=planes_.count(id)?0:freeze(id,handles);final_=id;dirty_=false;
}
GpuFrame NativeFrameRecipe::render(const GpuFrame& world){
 if(!ready() || !world.valid())throw std::logic_error("Incomplete prepared native frame");
 renderer_.batch([&]{
  for(const auto& pair:planes_)renderer_.restoreSurface(pair.second.baseline,pair.second.target);
  renderer_.importGpu565(target(world_),world);
  for(const auto& step:steps_){const auto& c=step.command;const auto& w=c.words;
   switch(c.operation){
   case 2:renderer_.update(target(w[0]),int(w[1]),int(w[2]),c.image);break;
   case 3:renderer_.copy(step.frozenSource?step.frozenSource:target(w[0]),target(w[1]),{int(w[2]),int(w[3]),int(w[4]),int(w[5])},int(w[6]),int(w[7]),w[8]?std::optional<std::uint32_t>(w[9]):std::nullopt);break;
   case 4:renderer_.setPalette(target(w[0]),w[1],c.colors);break;
   case 11:renderer_.swapContents(target(w[0]),target(w[1]));break;
   case MNM_RENDER_STREAM_V3_OPERATION_CLIPPER_SET:renderer_.setClipper(target(w[0]),detail::commandClipper(c));break;
   case MNM_RENDER_STREAM_V3_OPERATION_SURFACE_COPY:renderer_.surfaceCopy(step.frozenSource?step.frozenSource:target(w[0]),target(w[1]),detail::commandSurfaceCopy(c));break;
   case MNM_RENDER_STREAM_V4_OPERATION_MASKED_UPDATE:renderer_.updateMasked(target(w[0]),int(w[1]),int(w[2]),c.image,c.coverage);break;
   default:throw std::logic_error("Corrupt prepared native frame recipe");
   }
  }
 });return renderer_.presentGpu(frozenFinal_?frozenFinal_:target(final_));
}
}
