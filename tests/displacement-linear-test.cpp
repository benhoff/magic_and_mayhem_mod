#include "../renderer/scenes/scene_renderer.hpp"
#include "../renderer/canvas_sequence.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <QFile>
#include <QtEndian>
#include <iostream>
using namespace mnm;
using namespace resource_test;
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);require(argc==3,"Expected owned frame+offsets and new output");
    QFile file(argv[1]);require(file.open(QIODevice::ReadOnly),"Open captured input");const auto bytes=file.readAll();
    require(bytes.size()>=104,"Captured header extent");
    const auto n=qFromLittleEndian<quint32>(bytes.constData());require(n<=1048576&&bytes.size()==n+64,"Captured extent");
    Bytes encoded(28+768+n,0);word(encoded,0,0x00525053);word(encoded,4,encoded.size());word(encoded,8,4);word(encoded,12,1);word(encoded,16,1);
    std::copy(bytes.begin(),bytes.begin()+n,encoded.begin()+28+768);
    QTemporaryDir temp;require(temp.isValid(),"Fixture root");const auto root=std::filesystem::path(temp.path().toStdString());write(root,"captured.spr",encoded);
    assets::ResourceManager resources(store(root));const assets::ResourceId id{assets::ResourceKind::ui,"linear-distortion"};resources.bind(id,spr("captured.spr"));
    const auto& frame=std::get<assets::Sprite>(resources.load(id).image).frames.at(0);
    render::Image base{800,600,std::vector<std::uint32_t>(480000)};for(unsigned i=0;i<base.pixels.size();++i)base.pixels[i]=(i*97+13)&65535;
    render::SpriteComposite composite;composite.mode=render::CompositeMode::displace;composite.rowPeriod=16;
    for(unsigned i=0;i<16;++i)composite.rowOffsets[i]=qFromLittleEndian<quint32>(bytes.constData()+n+i*4);
    render::CanvasSequence cpu;cpu.create(1,800,600);cpu.update(1,0,0,base);cpu.raster(1,frame,764,456,{0,0,800,600},3,9,{},composite.rowOffsets,16);const auto expected=cpu.read(1);
    cpu.update(1,0,0,base);cpu.raster(1,frame,764,584,{0,0,800,600},3,9,{},composite.rowOffsets,16);const auto nearEdge=cpu.read(1);
    render::GlBlitter renderer;
    for(const bool atlas:{false,true}){
        render::SceneLimits limits;limits.cache.indexedAtlas=atlas;render::SceneRenderer scene(renderer,resources,base,limits);
        render::SceneDraw draw{id,0,764,456,true,true,{},render::Rect{0,0,800,600},composite};
        const auto before=renderer.stats();scene.draw({draw});const auto after=renderer.stats();
        require(before.nativeReadbacks==after.nativeReadbacks&&before.rgbaReadbacks==after.rgbaReadbacks,"Linear distortion delivery readback");
        require(scene.read().pixels==expected.pixels,"Native/CPU linear canvas samples differ");
        // At584 only the conservative nominal envelope crosses the allocation;
        // every covered sample fits. At600 an actual covered tap is word480000.
        draw.anchorY=584;scene.draw({draw});require(scene.read().pixels==nearEdge.pixels,"Covered in-range edge sample was refused or changed");
        draw.anchorY=600;rejected([&]{scene.draw({draw});});require(scene.read().pixels==nearEdge.pixels,"Past-allocation sample changed completed scene");
    }
    // A late genuine allocation/validity failure must preserve earlier rows.
    assets::SpriteFrame solid;solid.width=4;solid.height=2;solid.opaqueMask.assign(8,1);solid.pixels=std::vector<std::uint16_t>(8,0);
    const render::Image small{5,2,{0,1,2,3,4,5,6,7,8,9}};std::array<int,16> shifts{};shifts.fill(2);
    render::CanvasSequence bounded;bounded.create(1,5,2);bounded.update(1,0,0,small);
    rejected([&]{bounded.raster(1,solid,0,0,{0,0,5,2},3,1,{},shifts,16);});require(bounded.read(1).pixels==small.pixels,"Late linear sample refusal mutated CPU canvas");
    bounded.raster(1,solid,0,0,{0,0,5,1},3,1,{},shifts,16);auto shifted=small;for(unsigned i=0;i<4;++i)shifted.pixels[i]=small.pixels[i+2];require(bounded.read(1).pixels==shifted.pixels,"Right-edge CPU linear sample differs");
    require(!renderer.stats().surfaces,"Linear fixture leaked surfaces");
    QFile output(argv[2]);require(output.open(QIODevice::WriteOnly|QIODevice::NewOnly),"New native output");
    for(auto value:expected.pixels){const auto word=qToLittleEndian<quint16>(value);require(output.write(reinterpret_cast<const char*>(&word),2)==2,"Native output write");}
    std::cout<<"{\"success\":true,\"native_backends\":2,\"canvas_words_compared\":960000,\"covered_edge_admissions\":2,\"atomic_scene_refusals\":2,\"atomic_cpu_refusals\":1,\"delivery_readbacks\":0,\"remaining_surfaces\":0}\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
