#include "gl_viewport.hpp"
#include "commands.hpp"
#include <QApplication>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QSurfaceFormat>
#include <QtEndian>
#include <cstdio>
#include <stdexcept>
using namespace mnm::render;
namespace {
unsigned comparisons=0;
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
QImage colors(const Image& image,PixelFormat format,const std::vector<Rgb>& palette){
    QImage result(image.width,image.height,QImage::Format_RGBA8888);
    for(int y=0;y<image.height;++y)for(int x=0;x<image.width;++x){
        const auto value=image.pixels[std::size_t(y)*image.width+x];
        unsigned rgb[3];
        if(format.bits==8){const auto c=palette.at(value);rgb[0]=c.red;rgb[1]=c.green;rgb[2]=c.blue;}
        else for(unsigned i=0;i<3;++i){const auto mask=format.masks[i],low=mask&(~mask+1);rgb[i]=((value&mask)/low)*255/(mask/low);}
        if(format.bits==16 && format.masks==std::array<std::uint32_t,3>{0xf800,0x7e0,0x1f}){
            const auto r=(value>>11)&31,g=(value>>5)&63,b=value&31;
            rgb[0]=(r<<3)|(r>>2);rgb[1]=(g<<2)|(g>>4);rgb[2]=(b<<3)|(b>>2);
        }
        result.setPixelColor(x,y,QColor(rgb[0],rgb[1],rgb[2]));
    }
    return result;
}
void compare(GlViewport& viewport,const QImage& expected){
    QApplication::processEvents();viewport.repaint();const auto actual=viewport.grabFramebuffer();
    require(viewport.ready() && viewport.error().isEmpty() && !actual.isNull(),"Viewport presentation failed");
    const auto scale=qMin(double(actual.width())/expected.width(),double(actual.height())/expected.height());
    const int w=qRound(expected.width()*scale),h=qRound(expected.height()*scale);
    const int left=(actual.width()-w)/2,top=actual.height()-h-(actual.height()-h)/2;
    for(int y=0;y<actual.height();++y)for(int x=0;x<actual.width();++x){
        QColor color(Qt::black);
        if(x>=left && x<left+w && y>=top && y<top+h){
            const int ix=qMin(expected.width()-1,int((x-left+0.5)*expected.width()/w));
            const int iy=qMin(expected.height()-1,int((y-top+0.5)*expected.height()/h));
            color=expected.pixelColor(ix,iy);
        }
        if(actual.pixelColor(x,y)!=color){
            std::fprintf(stderr,"Mismatch %d,%d actual %08x expected %08x\n",x,y,actual.pixel(x,y),color.rgba());
            throw std::runtime_error("Complete framebuffer disagrees with independent pixel oracle");
        }
    }
    ++comparisons;
}
QByteArray words(std::initializer_list<unsigned> values){QByteArray out;for(auto v:values){char bytes[4];qToLittleEndian(v,bytes);out.append(bytes,4);}return out;}
QByteArray fixture(){
    QByteArray out="MNMCMD01";out+=words({1,16});unsigned sequence=0;
    const auto appendRecord=[&](unsigned op,const QByteArray& payload){out+=words({op,++sequence,unsigned(payload.size())});out+=payload;};
    appendRecord(1,words({1,2,2,8,0,0,0})+QByteArray::fromHex("00010203"));
    appendRecord(4,words({1,0,4})+QByteArray::fromHex("ff000000ff000000ffffffff"));
    appendRecord(6,words({1}));appendRecord(2,words({1,0,0,2,1})+QByteArray::fromHex("0302"));appendRecord(6,words({1}));
    appendRecord(7,words({1}));appendRecord(8,{});return out;
}
}
int main(int argc,char** argv){
    QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);
    QApplication app(argc,argv);
    try {
        GlViewport viewport;viewport.resize(320,240);viewport.show();app.processEvents();
        require(viewport.ready(),"Viewport did not initialize");
        auto renderer=std::make_unique<GlBlitter>(viewport.context());const auto driver=renderer->driver();
        std::vector<Rgb> palette;for(unsigned i=0;i<256;++i)palette.push_back({std::uint8_t(i),std::uint8_t(255-i),std::uint8_t(i*7)});
        const PixelFormat formats[]={{8,{}},{16,{0xf800,0x07e0,0x1f}},{24,{0xff0000,0xff00,0xff}},{32,{0xff0000,0xff00,0xff}}};
        // Every canonical RGB565 value must survive shared-texture presentation.
        Image exhaustive565{256,256,{}};
        for(unsigned value=0;value<65536;++value)exhaustive565.pixels.push_back(value);
        const auto exhaustiveSurface=renderer->create(exhaustive565,formats[1]);
        viewport.resize(512,512);viewport.setGpuFrame(renderer->presentGpu(exhaustiveSurface));
        compare(viewport,colors(exhaustive565,formats[1],{}));
        renderer->destroy(exhaustiveSurface);viewport.resize(320,240);
        for(auto pixelFormat:formats){
            Image cpu{4,3,{}};for(unsigned i=0;i<12;++i)cpu.pixels.push_back((i*23457u+123u)&(pixelFormat.bits==8?255u:pixelFormat.bits==16?65535u:0xffffffu));
            Image otherCpu=cpu;
            const auto surface=renderer->create(cpu,pixelFormat),other=renderer->create(cpu,pixelFormat);
            if(pixelFormat.bits==8){renderer->setPalette(surface,0,palette);renderer->setPalette(other,0,palette);}
            GpuFrame retained;
            unsigned texture=0;
            for(unsigned i=0;i<24;++i){
                const auto value=(i*891u+51)&(pixelFormat.bits==8?255u:pixelFormat.bits==16?65535u:0xffffffu);
                renderer->update(other,i%4,i%3,{1,1,{value}});otherCpu.pixels[(i%3)*4+i%4]=value;
                renderer->copy(other,surface,{int(i%4),int(i%3),int(i%4+1),int(i%3+1)},int(i%4),int(i%3),i%2?std::optional<std::uint32_t>(value):std::nullopt);
                if(!(i%2))cpu.pixels[(i%3)*4+i%4]=value;
                if(i%6==5){renderer->swapContents(surface,other);std::swap(cpu,otherCpu);}
                if(pixelFormat.bits==8){palette[i]={std::uint8_t(i*3),std::uint8_t(i*5),std::uint8_t(i*9)};renderer->setPalette(surface,i,{palette[i]});}
                const auto before=renderer->stats();auto frame=renderer->presentGpu(surface);const auto after=renderer->stats();
                require(before.nativeReadbacks==after.nativeReadbacks && before.rgbaReadbacks==after.rgbaReadbacks,"GPU presentation read pixels back");
                viewport.makeCurrent();const auto id=frame.textureForCurrentContext();viewport.doneCurrent();
                if(texture)require(id==texture,"Retained presentation texture was not reused");
                texture=id;
                retained=frame;viewport.setGpuFrame(frame);if(i%8==7)viewport.resize(i%16==7?337:320,i%16==7?252:240);
                compare(viewport,colors(cpu,pixelFormat,palette));
                require(viewport.imageUploads()==0,"GPU viewport uploaded CPU pixels");
            }
            renderer->destroy(surface);renderer->destroy(other);compare(viewport,colors(cpu,pixelFormat,palette));
        }
        // Use decoded complete commands and paint every PRESENT before END destroys its source.
        const auto commands=decodeCommands(fixture());unsigned presentations=0;
        const auto replay=replayCommandsGpu(commands,*renderer,[&](GpuFrame frame){
            viewport.setGpuFrame(frame);
            QImage expected(2,2,QImage::Format_RGBA8888);expected.setPixelColor(0,0,presentations?Qt::white:Qt::red);
            expected.setPixelColor(1,0,presentations?Qt::blue:Qt::green);expected.setPixelColor(0,1,Qt::blue);expected.setPixelColor(1,1,Qt::white);
            compare(viewport,expected);++presentations;
        });
        require(presentations==2 && replay.stats.surfaces==0,"Replay lifetime/presentation count failed");
        bool callbackRefused=false;
        try {replayCommandsGpu(commands,*renderer,[](GpuFrame){throw std::runtime_error("Synthetic callback failure");});}
        catch(const std::runtime_error&){callbackRefused=true;}
        require(callbackRefused && renderer->stats().surfaces==0,"Failed replay leaked source surfaces");
        bool emptyRefused=false;try {replayCommandsGpu(commands,*renderer,{});}catch(const std::runtime_error&){emptyRefused=true;}
        require(emptyRefused,"Empty presentation callback accepted");
        const auto before=renderer->stats();
        // A standalone producer survives its owner and source being released.
        Image final{2,2,{0xff0000,0x00ff00,0x0000ff,0xffffff}};const auto surface=renderer->create(final,formats[2]);
        auto lease=renderer->presentGpu(surface);renderer->destroy(surface);
        // Renderer teardown also releases still-live native surfaces.
        renderer->create(final,formats[2]);renderer.reset();
        viewport.setGpuFrame(lease);compare(viewport,colors(final,formats[2],{}));
        // Refuse unrelated contexts rather than interpreting an unrelated texture name.
        QOpenGLContext isolated;isolated.setFormat(format);require(isolated.create(),"Cannot create isolation context");
        QOffscreenSurface offscreen;offscreen.setFormat(isolated.format());offscreen.create();require(isolated.makeCurrent(&offscreen),"Cannot activate isolation context");
        bool refused=false;try {lease.textureForCurrentContext();}catch(const std::runtime_error&){refused=true;}
        require(refused,"Unshared context accepted GPU frame");isolated.doneCurrent();
        QOpenGLContext second;second.setFormat(format);second.setShareContext(viewport.context());
        require(second.create(),"Cannot create second sharing context");
        QOffscreenSurface secondSurface;secondSurface.setFormat(second.format());secondSurface.create();
        require(second.makeCurrent(&secondSurface),"Cannot activate second sharing context");
        refused=false;try {lease.textureForCurrentContext();}catch(const std::runtime_error&){refused=true;}
        require(refused,"Multiple consumers accepted without ordering contract");second.doneCurrent();
        refused=false;try {lease.textureForCurrentContext();}catch(const std::runtime_error&){refused=true;}
        require(refused,"No-current-context access accepted");
        // Switching back to CPU frames and empty frames releases the viewport lease.
        viewport.resize(336,240);QImage cpuImage(7,5,QImage::Format_RGBA8888);cpuImage.fill(Qt::cyan);viewport.setFrame(cpuImage);compare(viewport,cpuImage);
        require(viewport.imageUploads()==1,"CPU compatibility path did not upload exactly once");
        viewport.setGpuFrame({});app.processEvents();require(viewport.frameSize().isEmpty(),"Empty GPU frame retained dimensions");
        viewport.makeCurrent();const auto releasedTexture=lease.textureForCurrentContext();viewport.doneCurrent();
        lease={};viewport.makeCurrent();
        require(!viewport.context()->functions()->glIsTexture(releasedTexture),"Final lease did not release GPU texture");
        viewport.doneCurrent();
        require(replay.stats.nativeReadbacks==before.nativeReadbacks && replay.stats.rgbaReadbacks==0,"PRESENT commands added readbacks");
        std::printf("{\"success\":true,\"full_frame_comparisons\":%u,\"command_presentations\":%u,\"gpu_presentations\":%llu,\"rgba_readbacks\":%llu,\"gpu_viewport_uploads\":0,\"unshared_context_refused\":true,\"owner_release_survived\":true,\"driver\":\"%s\"}\n",comparisons,presentations,static_cast<unsigned long long>(before.gpuPresentations),static_cast<unsigned long long>(before.rgbaReadbacks),driver.renderer.c_str());
        return 0;
    }catch(const std::exception& e){std::fprintf(stderr,"GPU presentation test failed: %s\n",e.what());return 1;}
}
