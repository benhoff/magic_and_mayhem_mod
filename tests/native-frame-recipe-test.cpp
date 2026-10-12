#include "../renderer/native_frame_recipe.hpp"
#include <stdexcept>
#include "../protocols/include/mnm/resident_frame_budget.h"
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
  NativeFrameRecipe recipe(g,1,7,handles);rejected([&]{recipe.renderSealed(g.presentGpu(world));});recipe.record(copy(1,2,8,8),handles);g.copy(world,front,{0,0,8,8},0,0);recipe.record(copy(3,2,2,2,5,6),handles);g.copy(hud,front,{0,0,2,2},5,6);recipe.seal(2,handles);
  auto expected=std::vector<std::uint32_t>(64,0x07e0);for(auto at:{53,54,61,62})expected[at]=31;
  g.update(hud,0,0,{2,2,std::vector<std::uint32_t>(4,0xf800)});g.destroy(hud);handles.erase(3);
  for(unsigned i=0;i<30;++i){auto reads=g.stats().nativeReadbacks,rgba=g.stats().rgbaReadbacks;auto frame=recipe.render(g.presentGpu(world));g.importGpu565(output,frame);require(g.stats().nativeReadbacks==reads&&g.stats().rgbaReadbacks==rgba,"Prepared HUD exported pixels");require(g.read(output).pixels==expected,"Prepared native World/HUD source changed after source mutation/release");}
  // Native UI can complete several presentations within one engine update.
  SurfaceCommand update;update.operation=2;update.words={2,0,0};update.image={1,1,{0xf800}};recipe.record(update,handles);g.update(front,0,0,update.image);require(!recipe.ready(),"Partial updated HUD was replayable");rejected([&]{recipe.render(g.presentGpu(world));});
  // Current front is partial, while the sealed UI snapshot remains immutable.
  // Freshly drawn World values must still replace every World-owned pixel.
  g.update(world,0,0,{8,8,std::vector<std::uint32_t>(64,0xffff)});
  auto sealedExpected=std::vector<std::uint32_t>(64,0xffff);for(auto at:{53,54,61,62})sealedExpected[at]=31;
  const auto nativeReads=g.stats().nativeReadbacks,rgbaReads=g.stats().rgbaReadbacks;
  g.importGpu565(output,recipe.renderSealed(g.presentGpu(world)));require(g.stats().nativeReadbacks==nativeReads&&g.stats().rgbaReadbacks==rgbaReads,"Sealed partial UI exported pixels");require(g.read(output).pixels==sealedExpected,"Partial UI leaked into sealed frame or World screenshot was reused");
  for(unsigned i=0;i<40;++i){
   const unsigned colour=i%2?0x07e0:0xffff;
   recipe.record(update,handles);g.update(front,0,0,update.image);
   g.update(world,0,0,{8,8,std::vector<std::uint32_t>(64,colour)});
   auto complete=std::vector<std::uint32_t>(64,colour);for(auto at:{53,54,61,62})complete[at]=31;
   g.importGpu565(output,recipe.renderSealed(g.presentGpu(world)));require(g.read(output).pixels==complete,"Continuous partial UI exposed new UI or stale World pixels");
  }
  g.update(world,0,0,{8,8,std::vector<std::uint32_t>(64,0x07e0)});
  recipe.seal(2,handles);expected[0]=0xf800;g.importGpu565(output,recipe.render(g.presentGpu(world)));require(g.read(output).pixels==expected,"Updated native HUD recipe lost commands");
  require(recipe.generation()==7,"Extra native frame changed generation");
  g.update(world,0,0,{8,8,std::vector<std::uint32_t>(64,0xffff)});auto changed=std::vector<std::uint32_t>(64,0xffff);for(auto at:{53,54,61,62})changed[at]=31;changed[0]=0xf800;g.importGpu565(output,recipe.render(g.presentGpu(world)));require(g.read(output).pixels==changed,"Prepared composition did not use supplied native World");
 }
 // Equal-color UI is still UI. Coordinate ownership must not infer coverage
 // by comparing the composed screenshot with the old World pixels.
 {
  NativeFrameRecipe recipe(g,1,14,handles);recipe.record(copy(1,2,8,8),handles);g.copy(world,front,{0,0,8,8},0,0);
  SurfaceCommand u;u.operation=2;u.words={2,2,2};u.image={1,1,{0xffff}};recipe.record(u,handles);g.update(front,2,2,u.image);recipe.seal(2,handles);
  g.update(world,0,0,{8,8,std::vector<std::uint32_t>(64,0x07e0)});g.importGpu565(output,recipe.render(g.presentGpu(world)));auto expected=std::vector<std::uint32_t>(64,0x07e0);expected[18]=0xffff;require(g.read(output).pixels==expected,"Equal World/UI color lost UI ownership");
 }
 // Translated source coordinates survive compilation; copy ownership follows
 // the source pixel, rather than merely tagging the destination as World.
 {
  std::vector<std::uint32_t> values(64);for(unsigned i=0;i<64;++i)values[i]=i+1;g.update(world,0,0,{8,8,values});
  NativeFrameRecipe recipe(g,1,15,handles);recipe.record(copy(1,2,8,8),handles);g.copy(world,front,{0,0,8,8},0,0);
  auto shifted=copy(1,2,2,2,4,4);shifted.words[2]=1;shifted.words[3]=1;shifted.words[4]=3;shifted.words[5]=3;recipe.record(shifted,handles);g.copy(world,front,{1,1,3,3},4,4);
  SurfaceCommand self;self.operation=MNM_RENDER_STREAM_V3_OPERATION_SURFACE_COPY;self.words={2,2,1,0,0,4,4,6,6,0,0,2,2};recipe.record(self,handles);g.surfaceCopy(front,front,{SurfaceCopyApi::BltFast,{4,4,6,6},{0,0,2,2},0});recipe.seal(2,handles);
  for(auto& value:values){value+=100;}
  g.update(world,0,0,{8,8,values});auto expected=values;for(int y=0;y<2;++y)for(int x=0;x<2;++x){expected[(y+4)*8+x+4]=values[(y+1)*8+x+1];expected[y*8+x]=values[(y+1)*8+x+1];}g.importGpu565(output,recipe.render(g.presentGpu(world)));require(g.read(output).pixels==expected,"Translated/self-copy World mapping changed");
 }
 // Destination clippers affect coordinate propagation exactly as native
 // pixel copying; retained regions remain literal preexisting UI history.
 {
  auto expected=g.read(front).pixels;NativeFrameRecipe recipe(g,1,16,handles);
  SurfaceCommand clip;clip.operation=MNM_RENDER_STREAM_V3_OPERATION_CLIPPER_SET;clip.words={2,1,1};clip.regions={{2,2,4,4}};recipe.record(clip,handles);g.setClipper(front,{true,clip.regions});
  SurfaceCommand c;c.operation=MNM_RENDER_STREAM_V3_OPERATION_SURFACE_COPY;c.words={1,2,0,0,0,0,0,8,8,0,0,8,8};recipe.record(c,handles);g.surfaceCopy(world,front,{SurfaceCopyApi::Blt,{0,0,8,8},{0,0,8,8},0});recipe.seal(2,handles);
  g.update(world,0,0,{8,8,std::vector<std::uint32_t>(64,31)});for(int y=2;y<4;++y)for(int x=2;x<4;++x)expected[y*8+x]=31;
  g.importGpu565(output,recipe.render(g.presentGpu(world)));require(g.read(output).pixels==expected,"Clipped World-coordinate propagation changed");g.setClipper(front,{});
 }
 // Static upload history is folded once; two reads separated by mutation
 // retain two distinct GPU versions, even after the source is deleted.
 g.update(world,0,0,{8,8,std::vector<std::uint32_t>(64,0xffff)});
 const auto bitmap=g.create({2,2,{0,0,0,0}},rgb);handles[4]=bitmap;
 {
  NativeFrameRecipe recipe(g,1,8,handles);
  for(unsigned i=0;i<100;++i){SurfaceCommand u;u.operation=2;u.words={4,0,0};u.image={2,2,{31,31,31,31}};recipe.record(u,handles);g.update(bitmap,0,0,u.image);}
  recipe.record(copy(1,2,8,8),handles);g.copy(world,front,{0,0,8,8},0,0);
  recipe.record(copy(4,2,2,2),handles);g.copy(bitmap,front,{0,0,2,2},0,0);
  SurfaceCommand masked;masked.operation=MNM_RENDER_STREAM_V4_OPERATION_MASKED_UPDATE;masked.words={4,0,0};masked.image={2,2,{0xf800,0xf800,0xf800,0xf800}};masked.coverage={1,0,0,1};
  recipe.record(masked,handles);g.updateMasked(bitmap,0,0,masked.image,masked.coverage);
  recipe.record(copy(4,2,2,2,6,6),handles);g.copy(bitmap,front,{0,0,2,2},6,6);
  recipe.seal(2,handles);require(recipe.replayCommands()==1,"Static uploads were replayed");require(recipe.surfaces()==5,"Static assets allocated reset targets");
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
 // Hundreds of changing glyph read versions occupy one bounded native atlas.
 const auto glyph=g.create({2,2,{0,0,0,0}},rgb);handles[6]=glyph;
 {
  NativeFrameRecipe recipe(g,1,11,handles);recipe.record(copy(1,2,8,8),handles);g.copy(world,front,{0,0,8,8},0,0);
  for(unsigned i=0;i<200;++i){SurfaceCommand u;u.operation=2;u.words={6,0,0};u.image={2,2,{i,i,i,i}};recipe.record(u,handles);g.update(glyph,0,0,u.image);recipe.record(copy(6,2,2,2),handles);g.copy(glyph,front,{0,0,2,2},0,0);}
  recipe.seal(2,handles);require(recipe.surfaces()==5,"Glyph read versions allocated per-version textures");g.destroy(glyph);handles.erase(6);
  for(unsigned i=0;i<10;++i){g.importGpu565(output,recipe.render(g.presentGpu(world)));auto expected=g.read(world).pixels;for(auto at:{0,1,8,9})expected[at]=199;require(g.read(output).pixels==expected,"Packed changing glyph order changed");}
 }
 // Fullscreen mutable sources must not retain one full image per read.
 {
  const auto largeWorld=g.create({800,600,std::vector<std::uint32_t>(480000,0x07e0)},rgb),largeFront=g.create({800,600,std::vector<std::uint32_t>(480000,0)},rgb),largeSource=g.create({800,600,std::vector<std::uint32_t>(480000,0)},rgb);
  std::map<unsigned,SurfaceId> large{{1,largeWorld},{2,largeFront},{3,largeSource}};
  {
   NativeFrameRecipe recipe(g,1,13,large);recipe.record(copy(1,2,800,600),large);g.copy(largeWorld,largeFront,{0,0,800,600},0,0);
   for(unsigned i=1;i<=40;++i){SurfaceCommand u;u.operation=2;u.words={3,0,0};u.image={1,1,{i}};recipe.record(u,large);g.update(largeSource,0,0,u.image);auto c=copy(3,2,800,600);c.words[8]=1;recipe.record(c,large);g.copy(largeSource,largeFront,{0,0,800,600},0,0,0);}
   recipe.seal(2,large);require(recipe.surfaces()==5,"Fullscreen mutable read versions copied full images");g.destroy(largeSource);
   const auto scratch=g.allocate(800,600,rgb);g.importGpu565(scratch,recipe.render(g.presentGpu(largeWorld)));auto expected=std::vector<std::uint32_t>(480000,0x07e0);expected[0]=40;require(g.read(scratch).pixels==expected,"Fullscreen mutable keyed history changed");g.destroy(scratch);
  }
  g.destroy(largeWorld);g.destroy(largeFront);
 }
 // Masked scratch is reused without promoting unknown words or leaking older
 // covered values into later patches. Opposite aspect growth stays bounded.
 {
  const auto unknown=g.allocate(8,8,rgb);Image patch{8,8,std::vector<std::uint32_t>(64,31)};std::vector<std::uint8_t> coverage(64);coverage[0]=1;
  auto allocations=g.stats().maskedScratchAllocations;g.updateMasked(unknown,0,0,patch,coverage);require(g.stats().maskedScratchAllocations==allocations+1,"Masked scratch did not grow");rejected([&]{g.read(unknown);});
  coverage.assign(64,1);coverage[0]=0;g.updateMasked(unknown,0,0,patch,coverage);require(g.read(unknown).pixels==patch.pixels,"Covered writes did not define partial storage exactly");
  allocations=g.stats().maskedScratchAllocations;
  for(unsigned i=0;i<100;++i){patch.pixels.assign(64,i);coverage.assign(64,0);coverage[i%64]=1;g.updateMasked(unknown,0,0,patch,coverage);}
  require(g.stats().maskedScratchAllocations==allocations,"Masked update allocated scratch per call");auto expected=std::vector<std::uint32_t>(64,31);for(unsigned i=0;i<100;++i)expected[i%64]=i;require(g.read(unknown).pixels==expected,"Shared masked scratch leaked coverage/value history");
  coverage.assign(64,0);g.updateMasked(unknown,0,0,patch,coverage);require(g.stats().maskedScratchAllocations==allocations,"Zero mask allocated scratch");
  coverage[0]=2;rejected([&]{g.updateMasked(unknown,0,0,patch,coverage);});g.destroy(unknown);
  const auto large=g.allocate(2048,2048,rgb);Image wide{2048,1,std::vector<std::uint32_t>(2048,1)},tall{1,2048,std::vector<std::uint32_t>(2048,2)};g.updateMasked(large,0,0,wide,std::vector<std::uint8_t>(2048,1));g.updateMasked(large,0,0,tall,std::vector<std::uint8_t>(2048,1));require(g.stats().maskedScratchPixels<=4*1024*1024,"Opposite aspect scratch exceeded16MiB");g.destroy(large);
 }
 // Static indexed palettes are owned at the actual PRESENT read boundary.
 const PixelFormat indexed{8,{0,0,0}};const auto indices=g.create({8,8,std::vector<std::uint32_t>(64,1)},indexed);handles[5]=indices;
 {
  NativeFrameRecipe recipe(g,1,10,handles);std::vector<Rgb> colors(256);colors[1]={255,0,0};recipe.palette(5,colors,0,handles);g.setPalette(indices,0,colors);recipe.seal(5,handles);
  colors[1]={0,255,0};g.setPalette(indices,0,colors);g.destroy(indices);handles.erase(5);
  auto frame=recipe.render(g.presentGpu(world));require(context.makeCurrent(&surface),"Fixture sampling context");QOpenGLFunctions_3_3_Core gl;require(gl.initializeOpenGLFunctions(),"Fixture GL functions");gl.glBindTexture(GL_TEXTURE_2D,frame.textureForCurrentContext());std::vector<unsigned char> bytes(64*4);gl.glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA_INTEGER,GL_UNSIGNED_BYTE,bytes.data());frame.samplingComplete();require(gl.glGetError()==GL_NO_ERROR,"Diagnostic texture read failed");context.doneCurrent();for(unsigned i=0;i<64;++i)require(bytes[i*4]==255&&bytes[i*4+1]==0&&bytes[i*4+2]==0&&bytes[i*4+3]==255,"Frozen indexed palette used mutable source");
 }
 // The source cache and prepared frames must coexist inside the old global
 // cap. Full source residency retains32logical handles, not extra wire slots.
 require(MNM_RESIDENT_SOURCE_HANDLES+MNM_RESIDENT_PREPARED_HANDLES==64,"Shared handle partition changed total");
 require(MNM_RESIDENT_SOURCE_PIXELS+MNM_RESIDENT_PREPARED_PIXELS==16777216,"Shared pixel partition changed total");
 require(mnm_resident_source_pixel_limit(0)==16777216&&mnm_resident_source_pixel_limit(1)==5242880,"Legacy/owned cache partition changed");
 std::vector<SurfaceId> sourceResidency;for(unsigned i=0;i<30;++i)sourceResidency.push_back(g.allocate(384,384,rgb));
 {NativeFrameRecipe recipe(g,1,12,handles);recipe.record(copy(1,2,8,8),handles);g.copy(world,front,{0,0,8,8},0,0);recipe.seal(2,handles);g.importGpu565(output,recipe.render(g.presentGpu(world)));require(g.stats().surfaces<=64&&g.stats().pixels<=16777216,"Prepared/source residency exceeded renderer cap");}
 for(auto id:sourceResidency)g.destroy(id);
 for(auto id:{world,front,output})g.destroy(id);
 require(g.stats().surfaces==0,"Prepared native UI leaked surfaces");std::cout<<"{\"world_hud_repeats\":30,\"source_mutation_release\":true,\"partial_frame_refusal\":true,\"sealed_ui_during_partial_transaction\":true,\"continuous_partial_ui_gpu_frames\":40,\"unsealed_first_frame_refused\":true,\"same_update_hud_changes\":true,\"fresh_world_used\":true,\"ordinary_readbacks\":0,\"static_uploads_folded\":100,\"versioned_masked_reads\":true,\"swap_repeats\":10,\"indexed_palette_owned\":true,\"compiled_glyph_versions\":200,\"masked_scratch_repeats\":100,\"unknown_coverage_preserved\":true,\"paired_scratch_bound\":true,\"shared_budget_coexistence\":true,\"fullscreen_mutable_versions\":40,\"single_assembly_pass\":true,\"translated_self_copy\":true,\"equal_color_ui_ownership\":true,\"clipped_world_mapping\":true,\"teardown_surfaces\":0}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
