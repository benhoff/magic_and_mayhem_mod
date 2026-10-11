#include "commands.hpp"
#include "gl_viewport.hpp"
#include <QApplication>
#include <QElapsedTimer>
#include <QSurfaceFormat>
#include <QtEndian>
#include <cstdio>
#include <stdexcept>
using namespace mnm::render;
namespace {
void require(bool ok,const char* reason){if(!ok)throw std::runtime_error(reason);}
template<class F> void refused(F f){bool caught=false;try{f();}catch(const std::runtime_error&){caught=true;}require(caught,"Invalid resident reference accepted");}
const PixelFormat rgb565{16,{0xf800,0x7e0,31}};
void word(QByteArray& b,unsigned v){char a[4];qToLittleEndian<quint32>(v,a);b.append(a,4);}
void record(QByteArray& b,unsigned seq,unsigned op,const QByteArray& p){word(b,op);word(b,seq);word(b,unsigned(p.size()));b+=p;}
QByteArray fields(std::initializer_list<unsigned> values){QByteArray b;for(auto v:values)word(b,v);return b;}
QByteArray create(unsigned id){auto p=fields({id,800,600,16,0xf800,0x7e0,31});p.append(QByteArray(800*600*2,0));return p;}
void fixture(GlViewport& viewport){
    viewport.makeCurrent();GlBlitter world(viewport.context()),ui(viewport.context());
    Image values{256,256,{}};for(unsigned i=0;i<65536;++i)values.pixels.push_back(i);
    const auto source=world.create(values,rgb565),dest=ui.allocate(256,256,rgb565);
    auto frame=world.presentGpu(source);ui.importGpu565(dest,frame);
    require(world.stats().nativeReadbacks==0 && world.stats().rgbaReadbacks==0 && ui.stats().nativeReadbacks==0 && ui.stats().rgbaReadbacks==0,"World handoff read pixels back");
    require(ui.read(dest).pixels==values.pixels,"RGB565 import differs for exhaustive words/orientation");
    ui.invalidateContents(dest);ui.importGpu565(dest,frame);require(ui.read(dest).pixels==values.pixels,"Import did not establish complete validity");
    const auto wrongSize=ui.allocate(255,256,rgb565),wrongFormat=ui.allocate(256,256,{16,{0x7c00,0x3e0,31}});
    refused([&]{ui.importGpu565(wrongSize,frame);});refused([&]{ui.importGpu565(wrongFormat,frame);});refused([&]{ui.importGpu565(dest,{});});
    ui.destroy(dest);ui.destroy(wrongSize);ui.destroy(wrongFormat);world.destroy(source);
    Image canvas{800,600,std::vector<std::uint32_t>(800*600)};
    for(unsigned i=0;i<canvas.pixels.size();++i)canvas.pixels[i]=(i*17+311)&65535;
    const auto worldId=world.create(canvas,rgb565);auto canvasFrame=world.presentGpu(worldId);
    QByteArray stream("MNMCMD04",8);word(stream,4);word(stream,16);
    record(stream,1,1,create(1));record(stream,2,1,create(2));
    record(stream,3,19,fields({1,1,0,0x12345678,0x98765432}));
    auto patch=fields({1,19,37,1,1});patch.append(char(0xe0));patch.append(char(0x07));record(stream,4,2,patch);
    record(stream,5,3,fields({1,2,0,0,800,600,0,0,0,0}));record(stream,6,6,fields({2}));
    record(stream,7,7,fields({1}));record(stream,8,7,fields({2}));record(stream,9,8,{});
    auto commands=decodeCommands(stream);unsigned imports=0,presents=0;
    CommandConsumerOptions options{CommandDiagnostics::Verify,false,CommandStreamMode::Streaming};
    options.residentWorld=[&](std::uint64_t generation,std::uint64_t nonce){require(generation==1 && nonce==0x9876543212345678ULL,"Provider identity changed");++imports;return canvasFrame;};
    canvas.pixels[37*800+19]=0x7e0;
    CommandConsumer consumer(ui,[&](GpuFrame gpu){++presents;require(gpu.valid(),"Presentation lost lease");},options);
    consumer.submit(commands.data(),6);
    require(ui.stats().nativeReadbacks==2 && ui.stats().rgbaReadbacks==0 && world.stats().nativeReadbacks==0 && world.stats().rgbaReadbacks==0,"Command handoff performed a pixel readback");
    // CHECK is an explicit diagnostic after ordinary command execution. Both
    // source and copied output must contain the World plus the later HUD pixel.
    for(unsigned id:{1u,2u}){
        SurfaceCommand check;check.version=4;check.sequence=6+id;check.operation=5;check.words[0]=id;
        for(auto p:canvas.pixels){check.expected.append(char(p));check.expected.append(char(p>>8));}
        consumer.submit(check);
    }
    require(consumer.result().checks==2,"Independent native-word comparison absent");
    for(unsigned i=6;i<9;++i){auto command=commands[i];command.sequence+=2;consumer.submit(command);}consumer.finish();
    require(consumer.result().residentWorldImports==1 && presents==1 && consumer.result().liveSurfaces==0,"Resident composition ownership/counts");
    for(unsigned failure=0;failure<4;++failure){
        GlBlitter invalid(viewport.context());auto badOptions=options;if(failure==0)badOptions.residentWorld={};
        CommandConsumer bad(invalid,[](GpuFrame){},badOptions);bad.submit(commands[0]);auto ref=commands[2];ref.sequence=2;
        if(failure==1)ref.words[1]=2;
        if(failure==2)ref.words[3]=ref.words[4]=0;
        if(failure==3)ref.version=3;
        refused([&]{bad.submit(ref);});require(bad.result().liveSurfaces==0,"Failed import leaked resources");
    }
    world.destroy(worldId);canvasFrame={};frame={};
    require(world.stats().surfaces==0 && ui.stats().surfaces==0,"Fixture left native surfaces");
    std::printf("{\"rgb565_words\":65536,\"world_readbacks\":0,\"ordinary_import_readbacks\":0,\"hud_and_copy_checks\":2,\"rejections\":7,\"remaining_surfaces\":0,\"provider_calls\":%u}\n",imports);
}
}
int main(int argc,char** argv){
    QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);
    QApplication app(argc,argv);GlViewport viewport;viewport.resize(800,600);viewport.show();QElapsedTimer timer;timer.start();
    while(!viewport.ready() && timer.elapsed()<10000)QApplication::processEvents();
    try{require(viewport.ready(),"Viewport unavailable");fixture(viewport);return 0;}catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}
}
