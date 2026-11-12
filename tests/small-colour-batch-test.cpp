#include "../renderer/scenes/scene_renderer.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
using namespace mnm;
using namespace resource_test;
static std::uint32_t blend(std::uint32_t s,std::uint32_t d,render::CompositeMode mode,std::uint32_t m){
    if(mode==render::CompositeMode::copy)return s;
    if(mode==render::CompositeMode::half)return ((s>>1)&m)+((d>>1)&m);
    if(mode==render::CompositeMode::quarterSource){const auto h=(d>>1)&m;return h+((h>>1)&m)+((s>>2)&(m>>1)&m);}
    const auto h=(s>>1)&m;return h+((h>>1)&m)+((d>>2)&(m>>1)&m);
}
static render::Image scalar(render::Image image,const std::vector<render::SceneDraw>& draws,std::uint32_t m){
    for(const auto& d:draws){if(!d.visible)continue;const auto& src=d.colourRectangle->pixels;const auto c=d.viewport.value_or(render::Rect{0,0,image.width,image.height});
        for(int y=std::max(d.anchorY,c.top);y<std::min(d.anchorY+src.height,c.bottom);++y)
            for(int x=std::max(d.anchorX,c.left);x<std::min(d.anchorX+src.width,c.right);++x){auto& v=image.pixels[std::size_t(y)*image.width+x];v=blend(src.pixels[std::size_t(y-d.anchorY)*src.width+x-d.anchorX],v,d.composite.mode,m);}
    }
    return image;
}
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);QTemporaryDir temp;auto assetStore=store(temp.path().toStdString());assets::ResourceManager resources(assetStore);render::GlBlitter renderer;
    unsigned comparisons=0;std::uint64_t sparseCalls=0;
    for(bool rgb555:{false,true}){
        const auto maximum=rgb555?0x7fffu:0xffffu;render::Image base{64,48,{}};
        for(unsigned i=0;i<64*48;++i)base.pixels.push_back((i*1031u)&maximum);
        render::SceneLimits optimized;optimized.cacheColourRectangles=true;optimized.colourRgb555=rgb555;
        auto baseline=optimized;baseline.batchSmallColourRectangles=false;
        render::SceneRenderer fast(renderer,resources,base,optimized),old(renderer,resources,base,baseline);
        std::vector<render::SceneDraw> draws;
        for(unsigned i=0;i<620;++i){
            render::SceneDraw d;const int w=1+int(i%2),h=1+int((i/2)%2);
            render::Image source{w,h,{}};for(int p=0;p<w*h;++p)source.pixels.push_back((i*863+p*997)&maximum);
            d.colourRectangle=render::ColourRectangle{source};d.anchorX=int(i*13%67)-2;d.anchorY=int(i*7%51)-2;
            d.composite.mode=std::array<render::CompositeMode,4>{render::CompositeMode::copy,render::CompositeMode::half,render::CompositeMode::quarterSource,render::CompositeMode::quarterDestination}[i%4];
            if(i%5==0)d.viewport=render::Rect{3,4,61,45};
            if(i%17==0)d.anchorX=d.anchorY=8; // Dependencies must split the frozen snapshot.
            if(i%19==0)d.visible=false;
            draws.push_back(d);
            if(i%31==0){d.colourRectangle=render::ColourRectangle{{3,2,std::vector<std::uint32_t>(6,0x1234)}};d.visible=true;draws.push_back(d);} // Ordering barrier.
        }
        for(auto budget:{std::size_t(1),std::size_t(3),std::size_t(32),std::size_t(12320)}){
            fast.beginFrame(draws);const auto before=renderer.stats().nativeReadbacks;
            while(!fast.drawNext(budget)){}
            require(renderer.stats().nativeReadbacks==before,"Small colour drawing introduced a readback");
            old.draw(draws);const auto oracle=scalar(base,draws,rgb555?0x3def:0x7bef);
            require(fast.read().pixels==oracle.pixels&&old.read().pixels==oracle.pixels,"Small colours differ from ordered scalar/serial reference");++comparisons;
        }
        std::vector<render::SceneDraw> sparse;
        for(unsigned i=0;i<256;++i){auto d=draws[i];d.visible=true;d.viewport.reset();d.anchorX=int(i%32)*2;d.anchorY=int(i/32)*2;sparse.push_back(d);}
        // Restore genuinely small dimensions after borrowing varied fixtures.
        for(auto& d:sparse)d.colourRectangle=render::ColourRectangle{{2,2,{0xffffu&maximum,0x1234,0,0x3def}}};
        fast.beginFrame(sparse);const auto before=renderer.stats();require(fast.drawNext(512),"Sparse batch did not finish");const auto after=renderer.stats();
        require(after.textureUploadCalls==before.textureUploadCalls,"Small colours uploaded source textures");
        require(after.drawCalls-before.drawCalls==1,"Disjoint small colours did not share a single draw");sparseCalls+=after.drawCalls-before.drawCalls;
        require(fast.read().pixels==scalar(base,sparse,rgb555?0x3def:0x7bef).pixels,"Sparse small pixels differ");++comparisons;
        fast.releaseCachedImages();fast.draw(sparse);require(fast.read().pixels==scalar(base,sparse,rgb555?0x3def:0x7bef).pixels,"Cache release lost small-colour state");++comparisons;
    }
    const render::Image base{4,4,std::vector<std::uint32_t>(16,0x1234)};const auto target=renderer.create(base,{16,{0xf800,0x7e0,31}});
    const render::SmallColourDraw valid{{0,0,2,2},{1,2,3,4},render::CompositeMode::half};
    auto invalid=valid;invalid.destination.right=5;rejected([&]{renderer.smallColourRects(target,{valid,invalid});});
    invalid=valid;invalid.pixels[3]=65536;rejected([&]{renderer.smallColourRects(target,{valid,invalid});});
    invalid=valid;invalid.mode=render::CompositeMode::displace;rejected([&]{renderer.smallColourRects(target,{invalid});});
    rejected([&]{renderer.smallColourRects(target,std::vector<render::SmallColourDraw>(513,valid));});
    require(renderer.read(target).pixels==base.pixels,"Invalid batch mutated earlier pixels");
    auto unknown=renderer.allocate(4,4,{16,{0xf800,0x7e0,31}});rejected([&]{renderer.smallColourRects(unknown,{valid});});renderer.destroy(unknown);renderer.destroy(target);
    require(renderer.stats().surfaces==0,"Small colour controls leaked surfaces");
    std::cout<<QJsonDocument(QJsonObject{{"success",true},{"full_frame_comparisons",int(comparisons)},{"sparse_draw_calls",qint64(sparseCalls)},{"driver",QString::fromStdString(renderer.driver().renderer)}}).toJson(QJsonDocument::Compact).constData()<<'\n';return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
