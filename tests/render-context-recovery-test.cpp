#include "../renderer/blit.hpp"
#include "../renderer/surface_copy.hpp"
#include <QGuiApplication>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLVersionFunctionsFactory>
#include <iostream>
#include <stdexcept>
using namespace mnm::render;
static void require(bool condition,const char* message){if(!condition)throw std::runtime_error(message);}
template<class F> static void refuses(F f){bool refused=false;try{f();}catch(const std::exception&){refused=true;}require(refused,"Expected refusal");}
static std::vector<std::uint8_t> frameBytes(const GpuFrame& frame,QOpenGLContext& context,QOffscreenSurface& surface){
    require(context.makeCurrent(&surface),"Consumer current");
    auto* gl=QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_3_3_Core>(&context);require(gl,"Consumer functions");
    std::vector<std::uint8_t> bytes(std::size_t(frame.size().width())*frame.size().height()*4);
    gl->glBindTexture(GL_TEXTURE_2D,frame.textureForCurrentContext());gl->glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA_INTEGER,GL_UNSIGNED_BYTE,bytes.data());frame.samplingComplete();context.doneCurrent();return bytes;
}
int main(int argc,char** argv){QGuiApplication app(argc,argv);try{
    QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);
    QOpenGLContext before,after;before.setFormat(format);after.setFormat(format);
    require(before.create() && after.create(),"Two independent contexts");
    QOffscreenSurface first,second;first.setFormat(before.format());first.create();second.setFormat(after.format());second.create();
    require(first.isValid() && second.isValid(),"Platform targets");
    GlBlitter renderer(&before);const PixelFormat rgb{16,{0xf800,0x7e0,31}},indexed{8,{}};
    Image image{7,5,{}};for(unsigned i=0;i<35;++i)image.pixels.push_back((i*1777)&65535);
    const auto id=renderer.create(image,rgb),partial=renderer.allocate(7,5,rgb),pal=renderer.create({7,5,std::vector<std::uint32_t>(35,2)},indexed);
    std::vector<Rgb> colours(256);colours[2]={32,64,96};renderer.setPalette(pal,0,colours);
    renderer.update(partial,1,2,{2,1,{0x1234,0x5678}});renderer.setClipper(id,{true,std::vector<Rect>{{1,1,6,4}}});
    auto live=renderer.presentGpu(id),frozen=live.snapshot(),paletteFrame=renderer.presentGpu(pal);
    const auto expectedLive=frameBytes(live,before,first),expectedFrozen=frameBytes(frozen,before,first),expectedPalette=frameBytes(paletteFrame,before,first);
    require(!renderer.hasRecoveryCheckpoint(),"No fabricated initial checkpoint");refuses([&]{renderer.recoverContext(&after);});
    renderer.checkpointRecovery();require(renderer.hasRecoveryCheckpoint(),"Current checkpoint");
    renderer.recoverContext(&after);require(renderer.read(id).pixels==image.pixels,"Stable handles and native pixels");
    require(frameBytes(live,after,second)==expectedLive && frameBytes(frozen,after,second)==expectedFrozen && frameBytes(paletteFrame,after,second)==expectedPalette,"Outstanding mutable/immutable/palette frame leases");
    refuses([&]{renderer.read(partial);});
    const auto defined=renderer.allocate(2,1,rgb);require(!renderer.surfaceCopy(partial,defined,{SurfaceCopyApi::BltFast,{1,2,3,3},{0,0,2,1},0}).hresult,"Partial validity survived");
    require(renderer.read(defined).pixels==std::vector<std::uint32_t>({0x1234,0x5678}),"Recovered defined subset");
    require(!renderer.hasRecoveryCheckpoint(),"Changed state invalidates checkpoint");refuses([&]{renderer.recoverContext(&before);});
    renderer.checkpointRecovery();renderer.recoverContext(&before);
    require(renderer.read(id).pixels==image.pixels,"Repeated context rebuild");
    auto request=SurfaceCopyRequest{SurfaceCopyApi::Blt,{0,0,7,5},{0,0,7,5},0};
    const auto blank=renderer.create({7,5,std::vector<std::uint32_t>(35,0)},rgb);
    require(!renderer.surfaceCopy(blank,id,request).hresult,"Clipped copy after recovery");
    const auto clipped=renderer.read(id);for(unsigned y=0;y<5;++y)for(unsigned x=0;x<7;++x)require(clipped.pixels[y*7+x]==(x>=1 && x<6 && y>=1 && y<4?0:image.pixels[y*7+x]),"Clip state retained");
    const auto stats=renderer.stats();require(stats.recoveryRebuilds==2 && stats.recoveryCheckpoints==2 && stats.recoveryReadbacks>0,"Recovery accounting");
    std::cout<<"Native contexts rebuilt twice; pixels, palettes, clipping, partial validity and outstanding frame leases preserved\n";
    live={};frozen={};paletteFrame={};renderer.destroy(blank);renderer.destroy(defined);renderer.destroy(partial);renderer.destroy(pal);renderer.destroy(id);
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
