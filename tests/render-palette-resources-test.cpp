#include "commands.hpp"
#include "gl_viewport.hpp"
#include <QApplication>
#include <QSurfaceFormat>
#include <QtEndian>
#include <cstdio>
#include <stdexcept>
using namespace mnm::render;
namespace {
void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
void word(QByteArray& b,unsigned v){auto at=b.size();b.resize(at+4);qToLittleEndian<quint32>(v,b.data()+at);}
struct Stream {
    QByteArray bytes{"MNMCMD02"};unsigned seq=0;
    Stream(){word(bytes,2);word(bytes,16);}
    void add(unsigned op,std::initializer_list<unsigned> fields,QByteArray data={}){
        word(bytes,op);word(bytes,++seq);word(bytes,fields.size()*4+data.size());for(auto v:fields)word(bytes,v);bytes+=data;
    }
    void palette(unsigned id,unsigned gen,unsigned red){QByteArray colors(768,0);for(unsigned i=0;i<256;++i)colors[i*3]=char(red);add(12,{id,gen},colors);}
};
unsigned frames=0,failures=0;
void valid(GlViewport& viewport,unsigned fragment){
    Stream s;s.add(1,{1,2,2,8,0,0,0},QByteArray(4,5));s.add(1,{2,2,2,8,0,0,0},QByteArray(4,5));
    s.palette(1,7,30);s.add(14,{1,1,7});s.add(14,{2,1,7});s.add(6,{1});s.add(6,{2});
    s.add(13,{1,7,5,1},QByteArray::fromHex("006400"));s.add(6,{1});s.add(6,{2});
    s.palette(2,9,90);s.add(14,{1,2,9});s.add(13,{1,7,5,1},QByteArray::fromHex("0000c8"));s.add(6,{1});s.add(6,{2});
    s.add(14,{2,0,0});s.add(15,{1,7});s.palette(3,10,120);s.add(14,{2,3,10});s.add(6,{2});
    s.add(7,{1});s.add(7,{2});s.add(15,{2,9});s.add(15,{3,10});s.add(8,{});
    const QRgb expected[]={qRgb(30,0,0),qRgb(30,0,0),qRgb(0,100,0),qRgb(0,100,0),qRgb(90,0,0),qRgb(0,0,200),qRgb(120,0,0)};
    viewport.setGpuFrame({});GlBlitter renderer(viewport.context());unsigned displayed=0;
    CommandConsumer consumer(renderer,[&](GpuFrame frame){viewport.setGpuFrame(frame);const auto image=viewport.grabFramebuffer();const auto rect=viewport.imageRect();for(int y=0;y<2;++y)for(int x=0;x<2;++x)check(image.pixel(int(rect.left()+(x+0.5)*rect.width()/2),int(rect.top()+(y+0.5)*rect.height()/2))==expected[displayed],"Independent shared palette frame");++displayed;++frames;},{CommandDiagnostics::Skip,false,CommandStreamMode::Streaming});
    CommandDecoder decoder(CommandStreamMode::Streaming);
    for(qsizetype at=0;at<s.bytes.size();at+=fragment){auto commands=decoder.append(s.bytes.mid(at,fragment));consumer.submit(commands.data(),commands.size());}
    decoder.finish();consumer.finish();const auto& r=consumer.result();check(displayed==7 && !r.liveSurfaces && !r.livePalettes && !r.stats.nativeReadbacks && !r.stats.rgbaReadbacks && !viewport.imageUploads(),"Palette cleanup and ordinary readbacks");
}
void invalid(GlViewport& viewport){
    for(unsigned mode=0;mode<14;++mode){
        Stream s;s.add(1,{1,1,1,8,0,0,0},QByteArray(1,5));s.palette(1,7,30);s.add(14,{1,1,7});
        switch(mode){
        case 0:s.add(13,{1,6,5,1},QByteArray(3,0));break;
        case 1:s.add(14,{1,1,6});break;
        case 2:s.add(15,{1,7});break;
        case 3:s.add(14,{1,2,7});break;
        case 4:s.add(13,{2,7,5,1},QByteArray(3,0));break;
        case 5:s.add(15,{2,7});break;
        case 6:s.add(14,{1,0,1});break;
        case 7:s.add(14,{1,0,0});s.add(15,{1,7});s.palette(1,8,40);break;
        case 8:s.add(13,{1,7,255,2},QByteArray(6,0));break;
        case 9:s.add(13,{1,7,5,1},QByteArray(2,0));break;
        case 10:s.bytes.replace(0,8,"MNMCMD01");qToLittleEndian<quint32>(1,s.bytes.data()+8);break;
        case 11:for(unsigned id=2;id<=33;++id)s.palette(id,7,20);break;
        case 12:s.palette(2,0,20);break;
        case 13:s.add(14,{1,0,0});s.add(6,{1});break;
        }
        viewport.setGpuFrame({});GlBlitter renderer(viewport.context());CommandConsumer consumer(renderer,[](GpuFrame){},{CommandDiagnostics::Skip,false,CommandStreamMode::Streaming});CommandDecoder decoder(CommandStreamMode::Streaming);bool refused=false;
        try{auto commands=decoder.append(s.bytes);consumer.submit(commands.data(),commands.size());decoder.finish();consumer.finish();}catch(const std::exception&){refused=true;consumer.abort();}
        check(refused && !consumer.result().liveSurfaces && !consumer.result().livePalettes && !renderer.stats().surfaces,"Invalid palette identity leaked resources");++failures;
    }
    // A decoded command cannot switch protocol versions within one session.
    GlBlitter renderer(viewport.context());CommandConsumer consumer(renderer,[](GpuFrame){});SurfaceCommand c;c.version=2;c.sequence=1;c.operation=12;c.words[0]=1;c.words[1]=1;c.colors.resize(256);consumer.submit(c);c.version=1;c.sequence=2;bool refused=false;try{consumer.submit(c);}catch(...){refused=true;}check(refused && !consumer.result().livePalettes,"Mixed command version");++failures;
}
}
int main(int argc,char** argv){QSurfaceFormat f;f.setVersion(3,3);f.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(f);QApplication app(argc,argv);try{GlViewport v;v.resize(64,64);v.show();app.processEvents();check(v.ready(),"GPU ready");for(unsigned batch:{1u,7u,65536u})valid(v,batch);invalid(v);std::printf("{\"success\":true,\"full_frames\":%u,\"rejections\":%u,\"ordinary_readbacks\":0}\n",frames,failures);return 0;}catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}}
