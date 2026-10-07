#include "scene_renderer.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <iostream>
using namespace mnm;
using namespace resource_test;
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);QTemporaryDir directory;require(directory.isValid(),"Fixture root failed");
    const auto root=std::filesystem::path(directory.path().toStdString());populate(root);
    assets::ResourceManager resources(store(root));
    const assets::ResourceId body{assets::ResourceKind::creature,"body"},indexed{assets::ResourceKind::effect,"child"},bitmap{assets::ResourceKind::ui,"bitmap"};
    auto recipe=spr();recipe.animationBytes=animation();resources.bind(body,recipe);
    recipe=spr("indexed.spr");recipe.animation="body.ani";resources.bind(indexed,recipe);
    resources.bind(bitmap,{assets::ResourceImageFormat::bmp,"ui.bmp",{},{},{}});
    render::GlBlitter renderer;const render::Image background{5,3,std::vector<std::uint32_t>(15,0x1234)};
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
    // Second-surface construction failure frees the successfully created first.
    std::vector<render::SurfaceId> external;for(unsigned i=0;i<63;++i)external.push_back(renderer.create({1,1,{0}},render::spriteFormat));
    rejected([&]{render::SceneRenderer scene(renderer,resources,background);});require(renderer.stats().surfaces==63,"Scene constructor leaked first surface");
    for(const auto id:external)renderer.destroy(id);
    require(renderer.stats().surfaces==0,"Terminal surfaces nonzero");
    std::cout<<"Complete ordered scene pixels, resident drawing, hidden/clipped/color admission, failed-frame refusal/retry and ownership pass\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
