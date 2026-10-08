#include "scene_history.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <iostream>
#include <limits>
using namespace mnm;
using namespace resource_test;
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);QTemporaryDir directory;require(directory.isValid(),"Fixture root failed");
    const auto root=std::filesystem::path(directory.path().toStdString());populate(root);
    assets::ResourceManager resources(store(root));
    const assets::ResourceId body{assets::ResourceKind::creature,"body"},bitmap{assets::ResourceKind::ui,"bitmap"};
    auto recipe=spr();recipe.animationBytes=animation();resources.bind(body,recipe);
    resources.bind(bitmap,{assets::ResourceImageFormat::bmp,"ui.bmp",{},{},{}});
    render::GlBlitter renderer;const render::Image background{5,3,std::vector<std::uint32_t>(15,0x1234)};
    {
        render::SceneHistory history(renderer,resources,background,{4,{2,12}});
        rejected([&]{history.beginFrame({1,1},{{body,0,2,0}},false);});
        rejected([&]{history.beginFrame({0,1},{},true);});rejected([&]{history.beginFrame({1,0},{},true);});
        history.beginFrame({1,10},{{body,0,2,0}},true);require(history.drawNext(1),"Initial native frame incomplete");
        auto expected=background;expected.pixels[6]=0;expected.pixels[7]=0xf800;
        require(history.read().pixels==expected.pixels,"Initial native background differs");
        const auto before=renderer.stats();
        history.beginFrame({1,11},{{body,0,0,0},{body,0,4,0}},false);
        rejected([&]{history.beginFrame({1,12},{},false);});rejected([&]{history.read();});rejected([&]{history.drawNext(0);});
        require(!history.drawNext(1),"Partial native history reported complete");
        require(history.completed().sequence==10,"Partial history advanced completion");
        rejected([&]{history.presentGpu();});require(history.drawNext(1),"Native history did not complete");
        expected.pixels[5]=0xf800;expected.pixels[8]=0;expected.pixels[9]=0xf800;
        require(history.read().pixels==expected.pixels,"Masked holes lost previous native pixels");
        const auto after=renderer.stats();require(after.copies==before.copies+2&&after.uploads==before.uploads&&after.nativeReadbacks==before.nativeReadbacks+1,"Continuation reset/uploaded/read back during drawing");
        for(const auto stamp:std::vector<render::CanvasStamp>{{1,10},{1,11},{1,13},{2,12}}){
            rejected([&]{history.beginFrame(stamp,{},false);});require(history.read().pixels==expected.pixels,"Rejected history changed completed pixels");}
        rejected([&]{history.beginFrame({1,12},{{body,99,0,0}},false);});
        history.beginFrame({1,12},{},false);require(history.drawNext(1)&&history.read().pixels==expected.pixels,"Empty contiguous frame lost canvas");
        history.beginFrame({2,1},{},true);require(history.drawNext(1)&&history.read().pixels==background.pixels,"New native canvas reset retained pixels");
        // Force a real upload failure after admission; retained continuation must
        // remain poisoned until a successful explicit native background reset.
        std::vector<render::SurfaceId> external;for(unsigned i=0;i<60;++i)external.push_back(renderer.create({1,1,{0}},render::spriteFormat));
        render::SceneDraw shadow{body,0,2,0};shadow.composite.mode=render::CompositeMode::projectedShadow;
        history.beginFrame({2,2},{shadow},false);rejected([&]{history.drawNext(1);});
        rejected([&]{history.read();});rejected([&]{history.beginFrame({2,3},{},false);});
        for(const auto id:external)renderer.destroy(id);
        history.beginFrame({2,3},{},true);require(history.drawNext(1)&&history.read().pixels==background.pixels,"Explicit reset did not recover poisoned history");
        history.beginFrame({3,std::numeric_limits<std::uint64_t>::max()},{},true);require(history.drawNext(1),"Terminal sequence frame incomplete");
        rejected([&]{history.beginFrame({3,1},{},false);});
    }
    require(renderer.stats().surfaces==0,"Native history leaked surfaces");
    std::cout<<"Native canvas retention, contiguous identity/sequence admission, batches, poisoned-history reset and cleanup pass\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
