#include "graphics_initializer.hpp"
#include "canvas_initializer.hpp"
#include <limits>
#include <stdexcept>
namespace mnm::reconstruction {
namespace {
// Adapt the existing recovered initializer's successful provider boundaries to
// owned requests. Numeric original identities stay behind this reconstruction.
struct PlanRecorder {
 GraphicsInitializerPlan& plan;
 unsigned count=0,created=0;
 void record(GraphicsInitializerStep step){plan.steps.at(count++)=step;}
 void window(std::uint32_t){record(GraphicsInitializerStep::setupWindow);}
 void mode(std::uint32_t,std::uint32_t,std::uint32_t,std::uint32_t){record(GraphicsInitializerStep::setupMode);}
 void create(std::uint32_t receiver,const std::array<std::uint32_t,27>& descriptor,
             std::uint32_t file,std::uint32_t line){
  if(file!=0x5e3328 || created>1 || receiver!=(created?0x6a49e0u:0x6a4fe0u) || line!=(created?0x184u:0x171u))
   throw std::logic_error("Initializer request identity");
  (created?plan.offscreen:plan.primary)=descriptor;
  record(created++?GraphicsInitializerStep::createOffscreen:GraphicsInitializerStep::createPrimary);
 }
 void fill(std::uint32_t receiver,std::uint32_t color){
  if(receiver!=0x6a49e0 || color)throw std::logic_error("Initializer clear request");
  plan.clearEffects[0]=100;record(GraphicsInitializerStep::clearOffscreen);
 }
 std::array<std::uint32_t,8> format(std::uint32_t receiver){
  if(receiver!=0x6a4fe0)throw std::logic_error("Initializer format request");
  record(GraphicsInitializerStep::queryPixelFormat);
  // Planning records the query, not a real provider response or global masks.
  return {};
 }
};
}
GraphicsInitializerPlan planGraphicsInitialization(GraphicsInitializerInput input){
 GraphicsInitializerPlan result;
 result.width=input.largeVideo?800:640;result.height=input.largeVideo?600:480;
 const auto paddingX=input.configuration?96:input.paddingX;
 const auto paddingY=input.configuration?0:input.paddingY;
 if(paddingX>std::numeric_limits<std::uint32_t>::max()-result.width ||
    paddingY>std::numeric_limits<std::uint32_t>::max()-result.height)
  throw std::invalid_argument("Graphics initializer padded extent overflow");
 OriginalCanvasStartup state;state.window=input.windowed;state.video=input.largeVideo;state.depth=result.depth;
 state.put(8,input.paddingX);state.put(12,input.paddingY);state.put(16,input.configuration);
 PlanRecorder provider{result};
 if(initializeOriginalCanvas(state,0,provider)!=1 || provider.count!=result.steps.size() || provider.created!=2)
  throw std::logic_error("Initializer request sequence");
 result.paddingX=state.get(8);result.paddingY=state.get(12);
 return result;
}
}
