#include "../renderer/native_frame_recipe.hpp"
#include <stdexcept>
#include "../protocols/include/mnm/render_stream_v4.h"
#include <QGuiApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>
#include <iostream>
using namespace mnm::render;
void require(bool value,const char* reason){if(!value)throw std::runtime_error(reason);}
template<class F> void rejected(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}require(caught,"Invalid frame operation accepted");}
const PixelFormat rgb{16,{0xf800,0x7e0,31}};
SurfaceCommand copy(unsigned from,unsigned to,int width,int height,int x=0,int y=0){SurfaceCommand c;c.operation=3;c.words={from,to,0,0,unsigned(width),unsigned(height),unsigned(x),unsigned(y),0,0};return c;}
int main(int argc,char** argv)try{
 QGuiApplication app(argc,argv);QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QOpenGLContext context;context.setFormat(format);require(context.create(),"Fixture context creation");QOffscreenSurface surface;surface.setFormat(context.format());surface.create();GlBlitter g(&context);
 const auto world=g.create({8,8,std::vector<std::uint32_t>(64,0x07e0)},rgb),front=g.create({8,8,std::vector<std::uint32_t>(64,0)},rgb),hud=g.create({2,2,std::vector<std::uint32_t>(4,31)},rgb),output=g.allocate(8,8,rgb);
 std::map<unsigned,SurfaceId> handles{{1,world},{2,front},{3,hud}};
 {
  NativeFrameRecipe recipe(g,1,7,handles);recipe.record(copy(1,2,8,8),handles);g.copy(world,front,{0,0,8,8},0,0);recipe.record(copy(3,2,2,2,5,6),handles);g.copy(hud,front,{0,0,2,2},5,6);recipe.seal(2,handles);
  auto expected=std::vector<std::uint32_t>(64,0x07e0);for(auto at:{53,54,61,62})expected[at]=31;
  g.update(hud,0,0,{2,2,std::vector<std::uint32_t>(4,0xf800)});g.destroy(hud);handles.erase(3);
  for(unsigned i=0;i<30;++i){auto reads=g.stats().nativeReadbacks,rgba=g.stats().rgbaReadbacks;auto frame=recipe.render(g.presentGpu(world));g.importGpu565(output,frame);require(g.stats().nativeReadbacks==reads&&g.stats().rgbaReadbacks==rgba,"Prepared HUD exported pixels");require(g.read(output).pixels==expected,"Prepared native World/HUD source changed after source mutation/release");}
  // Native UI can complete several presentations within one engine update.
  SurfaceCommand update;update.operation=2;update.words={2,0,0};update.image={1,1,{0xf800}};recipe.record(update,handles);require(!recipe.ready(),"Partial updated HUD was replayable");rejected([&]{recipe.render(g.presentGpu(world));});recipe.seal(2,handles);expected[0]=0xf800;g.importGpu565(output,recipe.render(g.presentGpu(world)));require(g.read(output).pixels==expected,"Updated native HUD recipe lost commands");
  require(recipe.generation()==7,"Extra native frame changed generation");
  g.update(world,0,0,{8,8,std::vector<std::uint32_t>(64,0xffff)});auto changed=std::vector<std::uint32_t>(64,0xffff);for(auto at:{53,54,61,62})changed[at]=31;changed[0]=0xf800;g.importGpu565(output,recipe.render(g.presentGpu(world)));require(g.read(output).pixels==changed,"Prepared composition did not use supplied native World");
 }
 // Static upload history is folded once; two reads separated by mutation
 // retain two distinct GPU versions, even after the source is deleted.
 const auto bitmap=g.create({2,2,{0,0,0,0}},rgb);handles[4]=bitmap;
 {
  NativeFrameRecipe recipe(g,1,8,handles);
  for(unsigned i=0;i<100;++i){SurfaceCommand u;u.operation=2;u.words={4,0,0};u.image={2,2,{31,31,31,31}};recipe.record(u,handles);g.update(bitmap,0,0,u.image);}
  recipe.record(copy(1,2,8,8),handles);g.copy(world,front,{0,0,8,8},0,0);
  recipe.record(copy(4,2,2,2),handles);g.copy(bitmap,front,{0,0,2,2},0,0);
  SurfaceCommand masked;masked.operation=MNM_RENDER_STREAM_V4_OPERATION_MASKED_UPDATE;masked.words={4,0,0};masked.image={2,2,{0xf800,0xf800,0xf800,0xf800}};masked.coverage={1,0,0,1};
  recipe.record(masked,handles);g.updateMasked(bitmap,0,0,masked.image,masked.coverage);
  recipe.record(copy(4,2,2,2,6,6),handles);g.copy(bitmap,front,{0,0,2,2},6,6);
  recipe.seal(2,handles);require(recipe.replayCommands()==3,"Static uploads were replayed");require(recipe.surfaces()==6,"Static assets allocated reset targets");
  g.destroy(bitmap);handles.erase(4);
  auto expected=std::vector<std::uint32_t>(64,0xffff);for(auto at:{0,1,8,9})expected[at]=31;expected[54]=expected[63]=0xf800;expected[55]=expected[62]=31;
  for(unsigned i=0;i<30;++i){auto uploads=g.stats().uploads;auto reads=g.stats().nativeReadbacks;g.importGpu565(output,recipe.render(g.presentGpu(world)));require(g.stats().uploads==uploads,"Static patch was uploaded per native frame");require(g.stats().nativeReadbacks==reads,"Static folding exported pixels");require(g.read(output).pixels==expected,"Read-version or masked static patch changed");}
 }
 // Swap propagates World dependence to both identities. Replaying it must
 // restore both baselines and retain subsequent writes in command order.
 {
  NativeFrameRecipe recipe(g,1,9,handles);SurfaceCommand swap;swap.operation=11;swap.words={1,2};recipe.record(swap,handles);g.swapContents(world,front);
  SurfaceCommand u;u.operation=2;u.words={2,0,0};u.image={1,1,{0xf800}};recipe.record(u,handles);g.update(front,0,0,u.image);recipe.seal(2,handles);
  for(unsigned i=0;i<10;++i){g.importGpu565(output,recipe.render(g.presentGpu(world)));auto expected=g.read(world).pixels;expected[0]=0xf800;require(g.read(output).pixels==expected,"World-dependent swap accumulated or reordered");}
 }
 // Static indexed palettes are owned at the actual PRESENT read boundary.
 const PixelFormat indexed{8,{0,0,0}};const auto indices=g.create({8,8,std::vector<std::uint32_t>(64,1)},indexed);handles[5]=indices;
 {
  NativeFrameRecipe recipe(g,1,10,handles);std::vector<Rgb> colors(256);colors[1]={255,0,0};recipe.palette(5,colors,0,handles);g.setPalette(indices,0,colors);recipe.seal(5,handles);
  colors[1]={0,255,0};g.setPalette(indices,0,colors);g.destroy(indices);handles.erase(5);
  auto frame=recipe.render(g.presentGpu(world));require(context.makeCurrent(&surface),"Fixture sampling context");QOpenGLFunctions_3_3_Core gl;require(gl.initializeOpenGLFunctions(),"Fixture GL functions");gl.glBindTexture(GL_TEXTURE_2D,frame.textureForCurrentContext());std::vector<unsigned char> bytes(64*4);gl.glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA_INTEGER,GL_UNSIGNED_BYTE,bytes.data());frame.samplingComplete();require(gl.glGetError()==GL_NO_ERROR,"Diagnostic texture read failed");context.doneCurrent();for(unsigned i=0;i<64;++i)require(bytes[i*4]==255&&bytes[i*4+1]==0&&bytes[i*4+2]==0&&bytes[i*4+3]==255,"Frozen indexed palette used mutable source");
 }
 for(auto id:{world,front,output})g.destroy(id);
 require(g.stats().surfaces==0,"Prepared native UI leaked surfaces");std::cout<<"{\"world_hud_repeats\":30,\"source_mutation_release\":true,\"partial_frame_refusal\":true,\"same_update_hud_changes\":true,\"fresh_world_used\":true,\"ordinary_readbacks\":0,\"static_uploads_folded\":100,\"versioned_masked_reads\":true,\"swap_repeats\":10,\"indexed_palette_owned\":true,\"teardown_surfaces\":0}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
