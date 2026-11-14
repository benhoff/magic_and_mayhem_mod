#include "scene_renderer.hpp"
#include "scene-resource-fixtures.hpp"
#include <QGuiApplication>
#include <iostream>
#include <iomanip>
#include <limits>
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
    if(d.additive){
        const auto& a=*d.additive;const auto clip=d.viewport.value_or(render::Rect{0,0,image.width,image.height});
        for(int y=clip.top;y<clip.bottom;++y)for(int x=clip.left;x<clip.right;++x){
            if(x<std::int64_t(d.anchorX)||y<std::int64_t(d.anchorY)||x>=std::int64_t(d.anchorX)+a.width||y>=std::int64_t(d.anchorY)+a.height)continue;
            auto& word=image.pixels[std::size_t(y)*image.width+x];
            // Independent recovered rectangle oracle: wrap BEFORE saturation.
            const auto red=std::uint16_t((word>>11)+a.channels[0]);
            const auto green=std::uint16_t(((word>>5)&63)+a.channels[1]);
            const auto blue=std::uint16_t((word&31)+a.channels[2]);
            word=(std::min(unsigned(red),31u)<<11)|(std::min(unsigned(green),63u)<<5)|std::min(unsigned(blue),31u);
        }
        return;
    }
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
    QGuiApplication app(argc,argv);test::SceneResources resources;
    // Isolate source aggregation from the lower-level ordered blend pass. Its
    // serial control must remain serial for the draw-reduction assertion.
    const auto previousBlendBatch=qgetenv("MNM_NATIVE_COMPOSITE_BATCH_BASELINE");
    qputenv("MNM_NATIVE_COMPOSITE_BATCH_BASELINE","1");render::GlBlitter renderer;
    if(previousBlendBatch.isNull())qunsetenv("MNM_NATIVE_COMPOSITE_BATCH_BASELINE");else qputenv("MNM_NATIVE_COMPOSITE_BATCH_BASELINE",previousBlendBatch);
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
    const auto tiny=sprite(1,1);const auto tinyId=resources.add("tiny",tiny);
    const bool profileWasSet=qEnvironmentVariableIsSet("MNM_WORLD_OWNER_PROFILE");const auto oldProfile=qgetenv("MNM_WORLD_OWNER_PROFILE");qputenv("MNM_WORLD_OWNER_PROFILE","1");
    for(const bool barriers:{false,true}){
        std::vector<render::SceneDraw> draws;auto expected=base;
        for(unsigned i=0;i<768;++i){
            render::SceneDraw d{tinyId,0,32,32};d.colours=palette(i%5);(*d.colours)[0]=std::uint16_t(1u<<(i%3==0?0:i%3==1?5:11));d.composite.mode=render::CompositeMode::additive;
            if(barriers&&i%127==0)d.composite.mode=render::CompositeMode::copy;
            scalar(expected,tiny,d);draws.push_back(std::move(d));
        }
        for(const unsigned cap:{32u,512u}){
            render::SceneLimits limits;limits.batchAdditiveSprites=true;limits.maxAdditiveSprites=cap;limits.cache.indexedAtlas=true;
            render::SceneRenderer scene(renderer,resources.manager,base,limits);scene.beginFrame(draws);
            const auto before=renderer.stats();while(!scene.drawNext(512)){}const auto after=renderer.stats();
            require(after.nativeReadbacks==before.nativeReadbacks&&after.rgbaReadbacks==before.rgbaReadbacks,"Wide additive delivery readback");
            require(scene.read().pixels==expected.pixels,"Wide overlapping source aggregation differs from sequential scalar");
            const auto profile=scene.additiveProfile();require(profile.enabled&&profile.sprites>0,"Additive stage profile absent");
            require(profile.combinationMs>=0&&profile.uploadMs>=0&&profile.compositeMs>=0,"Invalid additive stage timing");
            if(!barriers)require(profile.batches==(cap==32?24:2)&&profile.sprites==768&&profile.visitedPixels==768,"Wide additive count/bound differs");
            ++cases;
        }
    }
    if(profileWasSet)qputenv("MNM_WORLD_OWNER_PROFILE",oldProfile);else qunsetenv("MNM_WORLD_OWNER_PROFILE");
    for(const unsigned cap:{0u,513u}){
        render::SceneLimits limits;limits.maxAdditiveSprites=cap;const auto before=renderer.stats().surfaces;bool refused=false;
        try{render::SceneRenderer invalid(renderer,resources.manager,base,limits);}catch(const std::invalid_argument&){refused=true;}
        require(refused&&renderer.stats().surfaces==before,"Invalid additive limit changed storage");
    }
    qputenv("MNM_WORLD_OWNER_PROFILE","1");
    std::uint64_t rectangleSaved=0;
    for(unsigned variant=0;variant<12;++variant){
        std::vector<render::SceneDraw> draws;auto expected=base;
        for(unsigned i=0;i<(variant==8?12u:768u);++i){
            render::SceneDraw d;d.anchorX=16+int(i%23)*3;d.anchorY=20+int(i%19)*3;
            d.additive=render::AdditiveRectangle{2,2,{std::uint16_t(i%11),std::uint16_t(i%17),std::uint16_t(i%5)}};
            d.viewport=render::Rect{1,2,254,253};
            if(variant==1&&i%7==0)d.visible=false;
            if(variant==2&&i%17==0){d.additive.reset();d.resource=id;d.colours=palette(i);d.composite.mode=render::CompositeMode::copy;}
            if(variant==3)d.anchorX=int(i%2)*190;
            if(variant==4)d.additive->width=129;
            if(variant==5){d.anchorX=i%3==0?std::numeric_limits<int>::min():i%3==1?std::numeric_limits<int>::max():-1;d.anchorY=-1;}
            if(variant==8){d.anchorX=8;d.anchorY=9;d.additive->width=d.additive->height=128;}
            if(variant==9&&i%13==0)d.additive->channels={65505,65473,65535};
            if(variant==10){d.anchorX=80;d.anchorY=80;d.additive->channels={65504,65472,65504};}
            if(variant==11&&i%17==0){d.additive.reset();d.resource=tinyId;d.colours=palette(i);d.composite.mode=render::CompositeMode::additive;}
            scalar(expected,d.resource==tinyId?tiny:small,d);draws.push_back(std::move(d));
        }
        std::uint64_t serial=0;
        for(const bool batched:{false,true}){
            const auto surfaces=renderer.stats().surfaces;render::SceneLimits limits;limits.batchAdditiveRectangles=batched;limits.maxAdditiveRectangles=512;
            limits.batchAdditiveSprites=true;
            {render::SceneRenderer scene(renderer,resources.manager,base,limits);scene.beginFrame(draws);
                const auto before=renderer.stats();
                const std::size_t budget=variant==7?1:512;
                while(!scene.drawNext(render::SceneBatchBudget{budget,std::size_t(variant==6?8192:256*1024),std::chrono::hours(1)})){}
                const auto after=renderer.stats();
                require(after.nativeReadbacks==before.nativeReadbacks&&after.rgbaReadbacks==before.rgbaReadbacks,"Rectangle batch readback");
                require(scene.read().pixels==expected.pixels,"Rectangle/serial pixels differ from independent wrap/clamp oracle");
                const auto calls=after.drawCalls-before.drawCalls;
                if(!batched)serial=calls;
                else if(variant==0){require(calls<serial/2,"Consecutive rectangles were not batched");rectangleSaved=serial-calls;require(scene.additiveProfile().rectangles==768,"Rectangle profile absent");}
                else if(variant==8)require(scene.additiveProfile().batches==3&&scene.additiveProfile().visitedPixels==12*128*128,"Rectangle visited-pixel limit differs");
                auto twice=expected;for(const auto& d:draws)scalar(twice,d.resource==tinyId?tiny:small,d);
                scene.beginFrame(draws,render::SceneStart::retainedCanvas);while(!scene.drawNext(budget)){}
                require(scene.read().pixels==twice.pixels,"Retained rectangle batch differs");
                scene.releaseCachedImages();scene.beginFrame(draws);while(!scene.drawNext(budget)){}
                require(scene.read().pixels==expected.pixels,"Rectangle scratch lifetime differs");
            }
            require(renderer.stats().surfaces==surfaces,"Rectangle scratch surfaces leaked");++cases;
        }
    }
    // Every destination WORD, including the first wrapping increment and the
    // maximum safe increment in each channel. Each quadrant fits one union.
    for(const auto channels:std::vector<std::array<std::uint16_t,3>>{{0,0,0},{1,1,1},{31,63,31},{65504,65472,65504},{65505,65473,65505},{65535,65535,65535}}){
        std::vector<render::SceneDraw> draws;auto expected=base;
        for(int y:{0,128})for(int x:{0,128})for(unsigned repeat=0;repeat<2;++repeat){
            render::SceneDraw d;d.anchorX=x;d.anchorY=y;d.additive=render::AdditiveRectangle{128,128,channels};
            scalar(expected,small,d);draws.push_back(d);
        }
        render::SceneLimits limits;limits.batchAdditiveRectangles=true;render::SceneRenderer scene(renderer,resources.manager,base,limits);scene.draw(draws);
        require(scene.read().pixels==expected.pixels,"Rectangle wrap boundary differs across all65536destination WORDs");++cases;
    }
    for(const unsigned cap:{0u,513u}){
        render::SceneLimits limits;limits.maxAdditiveRectangles=cap;const auto before=renderer.stats().surfaces;bool refused=false;
        try{render::SceneRenderer invalid(renderer,resources.manager,base,limits);}catch(const std::invalid_argument&){refused=true;}
        require(refused&&renderer.stats().surfaces==before,"Invalid rectangle limit changed storage");
    }
    for(const unsigned cap:{32u,512u}){
        render::SceneDraw d;d.anchorX=d.anchorY=5;d.additive=render::AdditiveRectangle{1,1,{1,1,1}};
        std::vector<render::SceneDraw> draws(768,d);auto expected=base;for(const auto& draw:draws)scalar(expected,small,draw);
        render::SceneLimits limits;limits.batchAdditiveRectangles=true;limits.maxAdditiveRectangles=cap;
        render::SceneRenderer scene(renderer,resources.manager,base,limits);scene.beginFrame(draws);while(!scene.drawNext(512)){}
        require(scene.read().pixels==expected.pixels,"Rectangle draw cap changed pixels");
        require(scene.additiveProfile().batches==(cap==32?24:2)&&scene.additiveProfile().rectangles==768,"Rectangle draw cap differs");++cases;
    }
    if(profileWasSet)qputenv("MNM_WORLD_OWNER_PROFILE",oldProfile);else qunsetenv("MNM_WORLD_OWNER_PROFILE");
    std::cout<<"{\"driver\":"<<std::quoted(renderer.driver().renderer)<<",\"success\":true,\"cases\":"<<cases<<",\"draws_saved\":"<<saved<<",\"rectangle_draws_saved\":"<<rectangleSaved<<",\"ordinary_readbacks\":0}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
