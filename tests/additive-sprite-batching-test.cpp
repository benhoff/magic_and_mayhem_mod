#include "scene_renderer.hpp"
#include "scene-resource-fixtures.hpp"
#include <QGuiApplication>
#include <iostream>
#include <stdexcept>
using namespace mnm;
namespace {
void require(bool okay,const char* why){if(!okay)throw std::runtime_error(why);}
assets::Sprite sprite(unsigned width,unsigned height,bool indexed=true){
    assets::Sprite s;s.storage=indexed?assets::SpriteStorage::indexed8:assets::SpriteStorage::rgb565;
    if(indexed)s.palettes.resize(1);
    assets::SpriteFrame f;f.width=width;f.height=height;f.originX=3;f.originY=-2;
    f.opaqueMask.resize(std::size_t(width)*height);
    std::vector<std::uint8_t> indices;std::vector<std::uint16_t> words;
    for(unsigned y=0;y<height;++y)for(unsigned x=0;x<width;++x){
        const bool opaque=(x+2*y)%7<4;f.opaqueMask[std::size_t(y)*width+x]=opaque;
        indices.push_back(opaque?std::uint8_t(x*23+y*29):0);words.push_back(opaque?std::uint16_t(x*1241+y*3781):0);
    }
    if(indexed){f.paletteIndex=0;f.pixels=std::move(indices);}else f.pixels=std::move(words);
    s.frames.push_back(std::move(f));return s;
}
render::SpriteColourTable palette(unsigned seed){
    render::SpriteColourTable p;for(unsigned i=0;i<256;++i)p[i]=std::uint16_t((i+seed)*1733+seed*8761);
    p[0]=0x0fe1;return p; // Opaque index zero must contribute; holes must not.
}
void scalar(render::Image& image,const assets::Sprite& sprite,const render::SceneDraw& d){
    if(!d.visible)return;
    const auto& f=sprite.frames.at(d.frame);const auto clip=d.viewport.value_or(render::Rect{0,0,image.width,image.height});
    for(unsigned y=0;y<f.height;++y)for(unsigned x=0;x<f.width;++x){
        const int dx=d.anchorX-f.originX+int(x),dy=d.anchorY-f.originY+int(y);const auto at=std::size_t(y)*f.width+x;
        if(!f.opaqueMask[at]||dx<clip.left||dy<clip.top||dx>=clip.right||dy>=clip.bottom)continue;
        const unsigned source=d.colours?(*d.colours)[std::get<std::vector<std::uint8_t>>(f.pixels)[at]]:std::get<std::vector<std::uint16_t>>(f.pixels)[at];
        auto& dest=image.pixels[std::size_t(dy)*image.width+dx];
        if(d.composite.mode==render::CompositeMode::copy)dest=source;
        else dest=(std::min((dest>>11)+(source>>11),31u)<<11)|(std::min(((dest>>5)&63u)+((source>>5)&63u),63u)<<5)|std::min((dest&31u)+(source&31u),31u);
    }
}
}
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);test::SceneResources resources;render::GlBlitter renderer;
    const auto small=sprite(17,9),large=sprite(129,17),words=sprite(17,9,false);
    const auto id=resources.add("small",small),largeId=resources.add("large",large),wordId=resources.add("words",words);
    render::Image base{256,256,{}};for(unsigned i=0;i<65536;++i)base.pixels.push_back(i);
    unsigned cases=0;std::uint64_t saved=0;
    for(unsigned variant=0;variant<8;++variant){
        std::vector<render::SceneDraw> draws;
        for(unsigned i=0;i<96;++i){
            render::SceneDraw d{id,0,int(i%13)*5-8,int(i%11)*3-6};d.colours=palette(i%5);d.composite.mode=render::CompositeMode::additive;
            d.viewport=render::Rect{2,1,250,254};
            if(variant==1&&i%7==0)d.composite.mode=render::CompositeMode::copy;
            if(variant==2&&i%5==0)d.visible=false;
            if(variant==3)d.anchorX=int(i%2)*180;
            if(variant==4)d.resource=largeId;
            if(variant==5){d.resource=wordId;d.colours.reset();}
            if(variant==6&&i%3==0){d.anchorX=-100;d.anchorY=-100;}
            draws.push_back(std::move(d));
        }
        auto expected=base;
        for(const auto& d:draws)scalar(expected,d.resource==largeId?large:d.resource==wordId?words:small,d);
        std::uint64_t serialCalls=0;
        for(const bool batched:{false,true}){
            const auto surfaces=renderer.stats().surfaces;render::SceneLimits limits;limits.batchAdditiveSprites=batched;limits.cache.indexedAtlas=true;
            {render::SceneRenderer scene(renderer,resources.manager,base,limits);
                auto before=renderer.stats();scene.beginFrame(draws);
                const std::size_t drawBudget=variant==7?1:32;
                const auto byteBudget=variant==7?8192:256*1024;
                while(!scene.drawNext(render::SceneBatchBudget{drawBudget,std::size_t(byteBudget),std::chrono::hours(1)})){}
                const auto after=renderer.stats();
                require(after.nativeReadbacks==before.nativeReadbacks&&after.rgbaReadbacks==before.rgbaReadbacks,"Batch delivery read back pixels");
                require(scene.read().pixels==expected.pixels,"Batch/serial pixels differ from independent scalar composition");
                const auto calls=after.drawCalls-before.drawCalls;
                if(!batched)serialCalls=calls;else if(variant==0){require(calls<serialCalls/2,"Consecutive additive sprites were not batched");saved=serialCalls-calls;}
                // Retained canvas uses the same grouping without accumulating an
                // extra application of any old source contribution.
                auto twice=expected;for(const auto& d:draws)scalar(twice,d.resource==largeId?large:d.resource==wordId?words:small,d);
                scene.beginFrame(draws,render::SceneStart::retainedCanvas);while(!scene.drawNext(drawBudget)){}
                require(scene.read().pixels==twice.pixels,"Retained-canvas aggregation differs");
                scene.releaseCachedImages();scene.beginFrame(draws);while(!scene.drawNext(drawBudget)){}
                require(scene.read().pixels==expected.pixels,"Cache retirement changed batch pixels");
            }
            require(renderer.stats().surfaces==surfaces,"Additive scratch surfaces leaked");++cases;
        }
    }
    std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"draws_saved\":"<<saved<<",\"ordinary_readbacks\":0}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
