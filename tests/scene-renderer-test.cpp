#include "scene_renderer.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <iostream>
using namespace mnm;
using namespace resource_test;
Bytes shadowSprite(bool overflow){
    constexpr unsigned width=68,height=38,header=40+height*8;
    const unsigned frameBytes=header+height*3+height*2;
    Bytes b(28+frameBytes,0);word(b,0,0x00525053);word(b,4,b.size());word(b,8,4);word(b,12,1);
    word(b,28,frameBytes);word(b,32,width);word(b,36,height);word(b,40,unsigned(-18));word(b,44,unsigned(-23));word(b,56,0xffffffff);
    for(unsigned y=0;y<height;++y){
        const auto delta=header+y*3,pixel=header+height*3+y*2;
        word(b,28+40+y*8,delta);word(b,28+44+y*8,pixel);
        const unsigned col=overflow&&y==0?67:49;
        b[28+delta]=col;b[28+delta+1]=1;b[28+delta+2]=width-col-1;
    }
    return b;
}
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);QTemporaryDir directory;require(directory.isValid(),"Fixture root failed");
    const auto root=std::filesystem::path(directory.path().toStdString());populate(root);
    assets::ResourceManager resources(store(root));
    const assets::ResourceId body{assets::ResourceKind::creature,"body"},indexed{assets::ResourceKind::effect,"child"},bitmap{assets::ResourceKind::ui,"bitmap"};
    auto recipe=spr();recipe.animationBytes=animation();resources.bind(body,recipe);
    recipe=spr("indexed.spr");recipe.animation="body.ani";resources.bind(indexed,recipe);
    resources.bind(bitmap,{assets::ResourceImageFormat::bmp,"ui.bmp",{},{},{}});
    render::GlBlitter renderer;const render::Image background{5,3,std::vector<std::uint32_t>(15,0x1234)};
    // Original shadow refusal uses the complete projected envelope, even when
    // only transparent padding exceeds the horizontal clip edge.
    for(const bool atlas:{false,true}){
        const assets::ResourceId shadow{assets::ResourceKind::ui,"edge-shadow"},bad{assets::ResourceKind::ui,"overflow-shadow"};
        write(root,"edge-shadow.spr",shadowSprite(false));write(root,"overflow-shadow.spr",shadowSprite(true));
        resources.bind(shadow,spr("edge-shadow.spr"));resources.bind(bad,spr("overflow-shadow.spr"));
        const render::Image base{800,600,std::vector<std::uint32_t>(480000,0xffff)};
        render::SceneLimits limits;limits.cache.indexedAtlas=atlas;
        render::SceneRenderer scene(renderer,resources,base,limits);
        render::SceneDraw draw{shadow,0,700,179};draw.viewport=render::Rect{0,0,800,312};draw.composite.mode=render::CompositeMode::projectedShadow;
        const auto before=renderer.stats();scene.draw({draw});const auto after=renderer.stats();
        require(before.nativeReadbacks==after.nativeReadbacks&&before.rgbaReadbacks==after.rgbaReadbacks,"Edge shadow delivery readback");
        require(scene.read().pixels==base.pixels,"Projected transparent envelope refusal differs");
        draw.resource=bad;scene.draw({draw});require(scene.read().pixels==base.pixels,"Covered projected envelope refusal differs");
        draw.resource=shadow;draw.anchorX=690;scene.draw({draw});
        auto expected=base;for(int row=0;row<19;++row)expected.pixels[(221+row)*800+776-row]=0x7bef;
        require(scene.read().pixels==expected.pixels,"Admitted projected shadow differs");
        draw.viewport=render::Rect{0,230,800,258};scene.draw({draw});
        expected=base;for(int row=0;row<14;++row)expected.pixels[(230+row)*800+775-row]=0x7bef;
        require(scene.read().pixels==expected.pixels,"Odd cropped shadow source rows differ");
        draw.anchorX=720;scene.draw({draw});require(scene.read().pixels==base.pixels,"Whole horizontal shadow refusal changed");
        const auto surface=renderer.allocate(3,2,render::spriteFormat);const auto stats=renderer.stats();
        require(renderer.dimensions(surface)==QSize(3,2),"Undefined surface metadata unavailable");
        require(renderer.stats().nativeReadbacks==stats.nativeReadbacks&&renderer.stats().rgbaReadbacks==stats.rgbaReadbacks,"Metadata read pixels");renderer.destroy(surface);
    }
    {
        render::SceneRenderer scene(renderer,resources,background,{4,{2,12}});
        rejected([&]{scene.read();});rejected([&]{scene.present();});
        scene.draw({{body,0,2,0},{indexed,1,3,0}});
        auto expected=background;expected.pixels[6]=0;expected.pixels[7]=0;expected.pixels[8]=0x07e0;
        require(scene.read().pixels==expected.pixels,"Ordered mixed scene coverage/opaque zero differs");
        const auto before=renderer.stats();scene.draw({{body,0,2,0},{indexed,1,3,0}});const auto after=renderer.stats();
        require(before.uploads==after.uploads && before.nativeReadbacks==after.nativeReadbacks &&
                before.rgbaReadbacks==after.rgbaReadbacks && after.copies==before.copies+3,"Resident scene drew with upload/readback");
        require(scene.cacheStats().hits>=2,"Scene did not reuse frame cache");
        scene.beginFrame({{body,0,2,0},{indexed,1,3,0}});
        rejected([&]{scene.beginFrame({});});rejected([&]{scene.drawNext(0);});
        require(!scene.drawNext(1),"Partial scene reported complete");
        rejected([&]{scene.read();});rejected([&]{scene.presentGpu();});
        require(scene.drawNext(1)&&scene.read().pixels==expected.pixels,"Batched scene differs from atomic draw");
        rejected([&]{scene.drawNext(1);});
        // Explicit caller order is retained, including equal-depth permutations.
        scene.draw({{indexed,1,3,0},{body,0,2,0}});expected.pixels[7]=0xf800;
        require(scene.read().pixels==expected.pixels,"Scene reordered caller queue");
        const auto previous=scene.read().pixels;
        rejected([&]{scene.draw({{body,0,2,0},{body,99,2,0}});});require(scene.read().pixels==previous,"Bad later record changed completed frame");
        rejected([&]{scene.draw({{body,0,-1,0,true,false}});});require(scene.read().pixels==previous,"Unclipped admission changed completed frame");
        rejected([&]{scene.draw(std::vector<render::SceneDraw>(5,{body,0,2,0}));});require(scene.read().pixels==previous,"Draw budget failure changed frame");
        scene.draw({{body,0,0,0},{assets::ResourceId{assets::ResourceKind::ui,"unknown"},99,0,0,false}});
        expected=background;expected.pixels[5]=0xf800;require(scene.read().pixels==expected.pixels,"Clipped/hidden scene policy differs");
        render::SpriteColourTable table{};table[0]=0x001f;table[1]=0xffff;
        scene.draw({{indexed,0,2,0,true,true,table}});expected=background;expected.pixels[6]=0x001f;expected.pixels[7]=0xffff;
        require(scene.read().pixels==expected.pixels,"Owned scene colour table differs");
        auto gpu=scene.presentGpu();require(gpu.valid() && gpu.size()==QSize(5,3),"GPU scene presentation unavailable");
        scene.release(body);scene.release(indexed);require(scene.cacheStats().frames==0,"Scene release retained uploads");
        std::vector<render::SurfaceId> external;for(unsigned i=0;i<62;++i)external.push_back(renderer.create({1,1,{0}},render::spriteFormat));
        rejected([&]{scene.draw({{bitmap,0,0,0}});});
        rejected([&]{scene.read();});rejected([&]{scene.present();});rejected([&]{scene.presentGpu();});
        scene.draw({});require(scene.read().pixels==background.pixels,"Failed execution did not recover with complete background");
        for(const auto id:external)renderer.destroy(id);
        scene.draw({{bitmap,0,0,0}});expected=background;expected.pixels[0]=0;expected.pixels[1]=0x07e0;
        require(scene.read().pixels==expected.pixels,"Execution retry changed bitmap pixels");
    }
    require(renderer.stats().surfaces==0 && renderer.stats().pixels==0,"Scene destruction leaked resources");
    {render::SceneLimits limits;limits.cache.residentOnly=true;render::SceneRenderer scene(renderer,resources,background,limits);
        scene.draw({});resources.unload(body);const auto loads=resources.stats().loads;
        rejected([&]{scene.beginFrame({{body,0,2,0}});});
        require(resources.stats().loads==loads&&scene.read().pixels==background.pixels,"Resident-only scene reloaded missing CPU data or lost its completed frame");
        resources.adopt(assets::prepareResource(resources.request(body)));scene.draw({{body,0,2,0}});
        require(scene.read().pixels[7]==0xf800,"Prepared scene pixels differ");}
    // Second-surface construction failure frees the successfully created first.
    std::vector<render::SurfaceId> external;for(unsigned i=0;i<63;++i)external.push_back(renderer.create({1,1,{0}},render::spriteFormat));
    rejected([&]{render::SceneRenderer scene(renderer,resources,background);});require(renderer.stats().surfaces==63,"Scene constructor leaked first surface");
    for(const auto id:external)renderer.destroy(id);
    require(renderer.stats().surfaces==0,"Terminal surfaces nonzero");
    std::cout<<"Complete ordered scene pixels, resident drawing, hidden/clipped/color admission, failed-frame refusal/retry and ownership pass\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
