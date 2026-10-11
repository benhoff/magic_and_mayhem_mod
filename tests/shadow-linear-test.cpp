#include "../renderer/scenes/scene_renderer.hpp"
#include "../renderer/canvas_sequence.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <QFile>
#include <QtEndian>
#include <iostream>
using namespace mnm;
using namespace resource_test;
void saveImage(const QString& path,const render::Image& image){
    QFile output(path);require(output.open(QIODevice::WriteOnly|QIODevice::NewOnly),"New native output");
    for(auto value:image.pixels){const auto word=qToLittleEndian<quint16>(value);require(output.write(reinterpret_cast<const char*>(&word),2)==2,"Native output write");}
}
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);require(argc==3,"Expected owned shadow frame and new output");
    QFile file(argv[1]);require(file.open(QIODevice::ReadOnly),"Open captured input");const auto bytes=file.readAll();require(bytes.size()>=40,"Captured header extent");
    const auto n=qFromLittleEndian<quint32>(bytes.constData());require(n<=1048576&&bytes.size()==n,"Captured extent");
    Bytes encoded(28+768+n,0);word(encoded,0,0x00525053);word(encoded,4,encoded.size());word(encoded,8,4);word(encoded,12,1);word(encoded,16,1);std::copy(bytes.begin(),bytes.end(),encoded.begin()+28+768);
    QTemporaryDir temp;require(temp.isValid(),"Fixture root");const auto root=std::filesystem::path(temp.path().toStdString());write(root,"captured.spr",encoded);
    assets::ResourceManager resources(store(root));const assets::ResourceId id{assets::ResourceKind::ui,"projected-envelope"};resources.bind(id,spr("captured.spr"));
    const auto& frame=std::get<assets::Sprite>(resources.load(id).image).frames.at(0);const bool covered=frame.width==48&&frame.height==29;
    require(covered||(frame.width==68&&frame.height==38),"Captured shadow geometry");
    const int x=covered?723:700,y=covered?242:179,bottom=covered?329:312;
    const int boundaryX=800-int(frame.width)-int(frame.height/2)+frame.originX;
    const int top=y-frame.originY+int(frame.height/2);
    render::Image base{800,600,std::vector<std::uint32_t>(480000)};for(unsigned i=0;i<base.pixels.size();++i)base.pixels[i]=(i*97+13)&65535;
    render::CanvasSequence cpu;cpu.create(1,800,600);cpu.update(1,0,0,base);cpu.raster(1,frame,x,y,{0,0,800,bottom},5,10,{});
    require(cpu.read(1).pixels==base.pixels,"CPU projected refusal changed pixels");
    cpu.raster(1,frame,x,y,{0,0,800,top+4},5,10,{});require(cpu.read(1).pixels==base.pixels,"CPU vertical crop relaxed full envelope");
    cpu.raster(1,frame,boundaryX,y,{0,0,800,bottom},5,10,{});const auto boundary=cpu.read(1);
    require(boundary.pixels!=base.pixels,"CPU exact edge incorrectly refused");
    render::GlBlitter renderer;
    for(const bool atlas:{false,true}){
        render::SceneLimits limits;limits.cache.indexedAtlas=atlas;render::SceneRenderer scene(renderer,resources,base,limits);
        render::SpriteComposite composite;composite.mode=render::CompositeMode::projectedShadow;
        render::SceneDraw draw{id,0,x,y,true,true,{},render::Rect{0,0,800,bottom},composite};
        auto before=renderer.stats();scene.draw({draw});auto after=renderer.stats();require(before.nativeReadbacks==after.nativeReadbacks&&before.rgbaReadbacks==after.rgbaReadbacks,"Shadow refusal delivery readback");require(scene.read().pixels==base.pixels,"GPU whole projected refusal differs");
        draw.viewport=render::Rect{0,0,800,top+4};scene.draw({draw});require(scene.read().pixels==base.pixels,"GPU crop relaxed full source envelope");
        draw.viewport=render::Rect{0,0,800,bottom};draw.anchorX=boundaryX;before=renderer.stats();scene.draw({draw});after=renderer.stats();require(before.nativeReadbacks==after.nativeReadbacks&&before.rgbaReadbacks==after.rgbaReadbacks,"Admitted shadow delivery readback");require(scene.read().pixels==boundary.pixels,"GPU exact projected edge differs");
    }
    require(!renderer.stats().surfaces,"Shadow fixture leaked surfaces");saveImage(argv[2],base);saveImage(QString(argv[2])+".boundary.565",boundary);
    std::cout<<"{\"success\":true,\"native_backends\":2,\"cases_per_backend\":3,\"canvas_words_compared\":2880000,\"delivery_readbacks\":0,\"remaining_surfaces\":0}\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
