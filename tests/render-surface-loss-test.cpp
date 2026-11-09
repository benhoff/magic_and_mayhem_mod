#include "../renderer/blit.hpp"
#include "../renderer/native_frame_recipe.hpp"
#include <QGuiApplication>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLVersionFunctionsFactory>
#include <iostream>
#include <stdexcept>
using namespace mnm::render;
static void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
template<class F>static void lost(F f){bool caught=false;try{f();}catch(const GraphicsFailure& e){caught=std::string(e.what())=="OpenGL offscreen target is unavailable";}check(caught,"Lost target did not produce typed native graphics failure");}
int main(int argc,char** argv)try{
 QGuiApplication app(argc,argv);QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);
 QOpenGLContext sharing;sharing.setFormat(format);check(sharing.create(),"Sharing context");
 QOffscreenSurface target;target.setFormat(sharing.format());target.create();check(target.isValid(),"Target create");
 QOffscreenSurface inspection;inspection.setFormat(sharing.format());inspection.create();check(inspection.isValid(),"Inspection target");
 unsigned refusals=0,oldTexture=0;
 {
  GlBlitter renderer(&sharing,&target);const PixelFormat rgb{16,{0xf800,0x7e0,31}};
  Image pixels{3,2,{0x1234,0x5678,31,0xf800,0x7e0,0xffff}};
  const auto source=renderer.create(pixels,rgb),destination=renderer.create({3,2,std::vector<std::uint32_t>(6,0)},rgb);
  check(renderer.read(source).pixels==pixels.pixels,"Independent pre-loss native pixels");
  {
   std::map<unsigned,SurfaceId> handles{{1,source},{2,destination}};
   auto recipe=std::make_unique<NativeFrameRecipe>(renderer,1,1,handles);
   SurfaceCommand copy;copy.operation=3;copy.words={1,2,0,0,3,2,0,0,0,0};recipe->record(copy,handles);renderer.copy(source,destination,{0,0,3,2},0,0);recipe->seal(2,handles);
   auto frame=recipe->render(renderer.presentGpu(source));check(frame.valid(),"Pre-loss completed frame");
   check(sharing.makeCurrent(&inspection),"Independent inspection current");oldTexture=frame.textureForCurrentContext();frame.samplingComplete();sharing.doneCurrent();
   target.destroy();check(!target.isValid(),"Actual platform target destroy");
   lost([&]{renderer.read(source);});++refusals;
   lost([&]{renderer.presentGpu(source);});++refusals;
   lost([&]{renderer.copy(source,destination,{0,0,3,2},0,0);});++refusals;
   lost([&]{renderer.checkpointRecovery();});++refusals;
   check(!renderer.hasRecoveryCheckpoint(),"Failed context acquisition fabricated checkpoint");
   // A destructor reached during a graphics exception must not terminate.
   recipe.reset();frame={};
  }
  target.create();check(target.isValid(),"Target recreated for terminal deletion");
  renderer.destroy(source);renderer.destroy(destination);
  // Prepared allocations may remain owned by the renderer after lost cleanup;
  // its destructor releases them using the recreated deletion target.
 }
 check(sharing.makeCurrent(&inspection),"Retirement inspection current");
 auto* functions=QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_3_3_Core>(&sharing);check(functions,"Inspection GL functions");
 check(!functions->glIsTexture(oldTexture),"Lost frame texture leaked into surviving sharing group");sharing.doneCurrent();
 {
  GlBlitter fresh(&sharing,&target);auto id=fresh.create({2,1,{31,0x7e0}},{16,{0xf800,0x7e0,31}});
  check(fresh.read(id).pixels==std::vector<std::uint32_t>({31,0x7e0}),"Independent fresh-owner pixels");fresh.destroy(id);check(!fresh.stats().surfaces,"Fresh owner leaked storage");
 }
 std::cout<<"{\"success\":true,\"actual_target_destructions\":1,\"graphics_refusals\":"<<refusals<<",\"destructor_survived\":true,\"fresh_owner_pixels\":true,\"original_game_runs\":0}\n";
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
