#include "../renderer/scenes/scene_renderer.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
using namespace mnm;
using namespace resource_test;
// Independent authored SPR with transparent holes, opaque zero and signed origin.
static Bytes fixture(){
    constexpr unsigned w=63,h=31,header=40+8*h;
    Bytes frame(header,0);word(frame,4,w);word(frame,8,h);word(frame,12,3);word(frame,16,unsigned(-2));word(frame,28,0xffffffff);
    std::array<Bytes,h> pixelRows;
    for(unsigned y=0;y<h;++y){
        Bytes runs;auto& pixels=pixelRows[y];unsigned x=0;
        while(x<w){unsigned skip=0,count=0;
            while(x<w&&(x+y)%11<3){++x;++skip;}
            while(x<w&&(x+y)%11>=3){const auto c=(x*997+y*863)&65535;pixels.push_back(c&255);pixels.push_back(c>>8);++x;++count;}
            runs.push_back(skip);if(count)runs.push_back(count);
        }
        word(frame,40+8*y,frame.size());frame.insert(frame.end(),runs.begin(),runs.end());
    }
    for(unsigned y=0;y<h;++y){word(frame,44+8*y,frame.size());frame.insert(frame.end(),pixelRows[y].begin(),pixelRows[y].end());}
    word(frame,0,frame.size());Bytes all(28,0);word(all,0,0x00525053);word(all,4,28+frame.size());word(all,8,4);word(all,12,1);all.insert(all.end(),frame.begin(),frame.end());return all;
}
static render::Image scalar(render::Image image,const assets::SpriteFrame& f,const std::vector<render::SceneDraw>& draws){
    for(const auto& d:draws){if(!d.visible)continue;auto before=image;
        const auto clip=d.viewport.value_or(render::Rect{0,0,image.width,image.height});
        const auto left=d.anchorX-f.originX,top=d.anchorY-f.originY;
        for(unsigned y=0;y<f.height;++y)for(unsigned x=0;x<f.width;++x){
            const int dx=left+int(x),dy=top+int(y);
            if(!f.opaqueMask[y*f.width+x]||dx<clip.left||dx>=clip.right||dy<clip.top||dy>=clip.bottom)continue;
            const auto dest=std::size_t(dy)*image.width+dx;
            if(d.composite.mode==render::CompositeMode::copy)image.pixels[dest]=std::get<std::vector<std::uint16_t>>(f.pixels)[y*f.width+x];
            else {const auto src=dest+d.composite.rowOffsets[unsigned(dy)%d.composite.rowPeriod];if(src<image.pixels.size())image.pixels[dest]=before.pixels[src];}
        }
    }
    return image;
}
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);QTemporaryDir directory;const auto root=std::filesystem::path(directory.path().toStdString());
    write(root,"wave.spr",fixture());assets::ResourceManager resources(store(root));const assets::ResourceId id{assets::ResourceKind::ui,"wave"};resources.bind(id,spr("wave.spr"));
    const auto& f=std::get<assets::Sprite>(resources.load(id).image).frames[0];
    render::GlBlitter renderer;render::Image base{80,48,{}};for(unsigned i=0;i<80*48;++i)base.pixels.push_back((i*1031+0x1234)&65535);
    unsigned comparisons=0;
    for(bool prepared:{false,true}){
        render::SceneLimits limits;limits.cache.preparedOnly=prepared;limits.cache.residentOnly=true;
        auto baseline=limits;baseline.batchDisplacements=false;
        render::SceneRenderer fast(renderer,resources,base,limits),old(renderer,resources,base,baseline);
        std::vector<render::SceneDraw> draws;
        for(int i=0;i<140;++i){
            render::SceneDraw d{id,0,(i*7)%41-8,(i*13)%37-8};d.composite.mode=i%47==0?render::CompositeMode::copy:render::CompositeMode::displace;
            d.composite.rowPeriod=1+unsigned(i%16);for(unsigned r=0;r<d.composite.rowPeriod;++r)d.composite.rowOffsets[r]=(i+int(r)*3)%17;
            d.composite.retainOutsideDisplacement=true;
            if(i%13==0)d.viewport=render::Rect{4,6,76,44};
            if(i%23==0)d.visible=false;
            draws.push_back(d);
        }
        auto run=[&](render::SceneRenderer& scene,std::size_t budget){scene.beginFrame(draws);while(!scene.drawNext(budget))if(const auto need=scene.uploadNeed())scene.supplyUpload(*need,render::prepareSpriteFrame(std::get<assets::Sprite>(resources.resident(need->request.id).image),need->request.frame));};
        for(auto budget:{1u,2u,7u,32u,12320u}){
            const auto before=renderer.stats();run(fast,budget);run(old,budget);const auto after=renderer.stats();
            require(before.nativeReadbacks==after.nativeReadbacks&&before.rgbaReadbacks==after.rgbaReadbacks,"Displacements introduced ordinary readback");
            const auto expected=scalar(base,f,draws);require(fast.read().pixels==expected.pixels&&old.read().pixels==expected.pixels,"Ordered displacement scalar/serial mismatch");++comparisons;
        }
        // Whole encoded-run clipping stays on the established serial path.
        for(auto& d:draws)if(d.composite.mode==render::CompositeMode::displace)d.wholeOpaqueRunClipping=true;
        run(fast,32);run(old,32);require(fast.read().pixels==old.read().pixels,"Whole-run clipping fallback differs");++comparisons;
    }
    if(!render::GlBlitter::isolatedRecoveryEnabled()){
        render::Image sampleBase{128,80,{}};for(unsigned i=0;i<128*80;++i)sampleBase.pixels.push_back((i*863)&65535);
        const auto target=renderer.create(sampleBase,render::spriteFormat);std::vector<render::DestinationSample> spans;
        auto expected=sampleBase;
        for(int y=0;y<80;++y)for(int x=0;x<128;x+=2){spans.push_back({x,y,(x+11)%128,(y+7)%80,1});expected.pixels[y*128+x]=sampleBase.pixels[((y+7)%80)*128+(x+11)%128];}
        renderer.sampleDestination(target,spans);require(renderer.read(target).pixels==expected.pixels,"Multiple instance flushes differ");++comparisons;
        for(const auto& invalid:std::vector<std::vector<render::DestinationSample>>{{{0,0,0,0,2},{1,0,5,0,1}},{{0,0,0,0,1},{128,0,0,0,1}},{{0,0,0,0,1},{2,1,127,0,2}}}){
            rejected([&]{renderer.sampleDestination(target,invalid);});require(renderer.read(target).pixels==expected.pixels,"Rejected samples changed pixels");
        }
        const auto unknown=renderer.allocate(80,48,render::spriteFormat);rejected([&]{renderer.sampleDestination(unknown,{{0,0,1,0,1}});});renderer.destroy(unknown);renderer.destroy(target);
    }
    require(renderer.stats().surfaces==0,"Displacement surface leak");
    std::cout<<QJsonDocument(QJsonObject{{"success",true},{"comparisons",int(comparisons)},{"driver",QString::fromStdString(renderer.driver().renderer)}}).toJson(QJsonDocument::Compact).constData()<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
