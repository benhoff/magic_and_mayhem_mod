#include "scene_renderer.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <future>
#include <iostream>
using namespace mnm;
using namespace resource_test;
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);QTemporaryDir directory;const auto root=std::filesystem::path(directory.path().toStdString());populate(root);
    const assets::ResourceId bitmap{assets::ResourceKind::ui,"large"},indexed{assets::ResourceKind::ui,"indexed"};
    QImage image(512,256,QImage::Format_RGB888);image.fill(QColor(255,0,0));QByteArray bytes;QBuffer buffer(&bytes);require(buffer.open(QIODevice::WriteOnly)&&image.save(&buffer,"BMP"),"Large BMP fixture");
    write(root,"large.bmp",Bytes(bytes.begin(),bytes.end()));
    assets::ResourceManager manager(store(root));manager.bind(bitmap,{assets::ResourceImageFormat::bmp,"large.bmp",{},{},{}});manager.bind(indexed,spr("indexed.spr"));manager.load(bitmap);manager.load(indexed);
    render::GlBlitter renderer;render::Image background{512,256,std::vector<std::uint32_t>(512*256,0x1234)};
    render::SceneLimits limits;limits.cache.frames=1;limits.cache.residentOnly=true;limits.cache.preparedOnly=true;
    const render::SceneBatchBudget budget{32,8192,std::chrono::milliseconds(20)};
    {
        render::SceneRenderer scene(renderer,manager,background,limits);scene.beginFrame({});require(scene.drawNext(budget),"Empty scene incomplete");auto previous=scene.presentGpu();require(previous.valid(),"Previous complete lease absent");
        scene.beginFrame({{bitmap,0,0,0}});require(!scene.drawNext(budget),"Cold frame completed without upload");auto need=*scene.uploadNeed();
        // Only an owned worker snapshot crosses threads, never a manager reference.
        auto owned=std::make_shared<const assets::VisualResource>(manager.resident(bitmap));const auto gui=std::this_thread::get_id();
        auto task=std::async(std::launch::async,[owned,need,gui]{unsigned rows=0;
            auto result=render::prepareResourceUpload(*owned,need.request,[&]{require(std::this_thread::get_id()!=gui,"Expansion ran on GUI");++rows;});
            require(rows==256,"Preparation row checkpoints differ");return result;});
        auto wrong=need;++wrong.draw;rejected([&]{scene.supplyUpload(wrong,task.get());});
        auto planes=render::prepareResourceUpload(*owned,need.request);scene.supplyUpload(need,std::move(planes));
        const auto presentations=renderer.stats().gpuPresentations;unsigned ticks=0;
        while(!scene.drawNext(budget)){++ticks;rejected([&]{scene.presentGpu();});require(renderer.stats().gpuPresentations==presentations&&previous.valid(),"Partial frame published");require(!scene.uploadNeed(),"In-progress GPU upload requested preparation twice");}
        ++ticks;require(ticks==128&&scene.uploadStats().bytes==1048576&&scene.uploadStats().maxTickBytes==8192,"Upload byte bound or resumable row accounting differs");
        require(renderer.stats().nativeReadbacks==0,"Upload path read back pixels");require(scene.read().pixels==std::vector<std::uint32_t>(512*256,0xf800),"Bounded upload pixels differ");
        const auto before=renderer.stats().uploads;scene.beginFrame({{bitmap,0,0,0}});require(scene.drawNext(budget)&&renderer.stats().uploads==before,"Warm frame uploaded again");
        scene.beginFrame({{bitmap,0,0,0}});require(scene.drawNext(render::SceneBatchBudget{32,8192,std::chrono::microseconds(1)}),"Context/time overhead stranded warm drawing");
        // A frame working set larger than the LRU capacity must progress in order.
        render::SpriteColourTable colours{};colours[0]=0x001f;colours[1]=0x07e0;
        const std::vector<render::SceneDraw> draws{{indexed,0,2,0,true,true,colours},{bitmap,0,0,0},{indexed,0,2,0,true,true,colours}};
        scene.beginFrame(draws);unsigned misses=0;
        while(!scene.drawNext(budget)){if(auto n=scene.uploadNeed()){++misses;scene.supplyUpload(*n,render::prepareResourceUpload(manager.resident(n->request.id),n->request));}}
        require(misses==3&&scene.read().pixels[513]==0x001f&&scene.read().pixels[514]==0x07e0,"LRU pressure reordered or stranded draws");
        // Equal palette values share an upload; changes create another variant.
        scene.beginFrame({draws.back()});require(scene.drawNext(budget),"Equal palette value cache miss");auto changed=draws.back();changed.colours->at(1)=0xffff;
        scene.beginFrame({changed});require(!scene.drawNext(budget),"Changed palette values reused old upload");auto colourNeed=*scene.uploadNeed();
        scene.supplyUpload(colourNeed,render::prepareResourceUpload(manager.resident(indexed),colourNeed.request));const auto colourDone=scene.drawNext(budget);require(colourDone,"Palette variant upload incomplete");require(scene.read().pixels[514]==0xffff,"Palette variant pixels differ");
        // Projected shape includes vertical clipping, and reuses equal shape at a new anchor.
        render::SceneDraw shadow{indexed,0,2,2};shadow.composite.mode=render::CompositeMode::projectedShadow;
        scene.beginFrame({shadow});require(!scene.drawNext(budget),"Cold shadow reused colour upload");auto shadowNeed=*scene.uploadNeed();require(shadowNeed.request.shadow.has_value(),"Shadow has no shape key");
        scene.supplyUpload(shadowNeed,render::prepareResourceUpload(manager.resident(indexed),shadowNeed.request));require(scene.drawNext(budget),"Shadow upload incomplete");
        const auto shadowUploads=renderer.stats().uploads;shadow.anchorX=4;scene.beginFrame({shadow});require(scene.drawNext(budget)&&renderer.stats().uploads==shadowUploads,"Equal projected shape rebuilt");
        // Retired resource revision and scene cursor both reject stale completions.
        scene.release(indexed);scene.beginFrame({draws.back()});require(!scene.drawNext(budget),"Released cache hit");auto stale=*scene.uploadNeed();auto stalePlanes=render::prepareResourceUpload(manager.resident(indexed),stale.request);
        manager.unload(indexed);manager.load(indexed);rejected([&]{scene.supplyUpload(stale,std::move(stalePlanes));});
    }
    require(renderer.stats().surfaces==0,"Scene cleanup leaked surfaces");
    {render::SceneRenderer scene(renderer,manager,background,limits);scene.beginFrame({{bitmap,0,0,0}});require(!scene.drawNext(budget),"Cold cleanup frame completed");
        auto need=*scene.uploadNeed();scene.supplyUpload(need,render::prepareResourceUpload(manager.resident(bitmap),need.request));require(!scene.drawNext(render::SceneBatchBudget{32,8192,std::chrono::microseconds(1)}),"Large cleanup upload completed in one tick");require(scene.uploadStats().bytes>0,"Context/time overhead stranded a pending upload");}
    require(renderer.stats().surfaces==0,"Pending upload cleanup leaked surfaces");
    // Undefined allocation never invents pixels; bounded row writes validate format.
    const auto surface=renderer.allocate(2,2,render::spriteFormat);rejected([&]{renderer.read(surface);});
    const render::Image plane{2,2,{1,2,3,4}};renderer.updateRows(surface,plane,0,1);rejected([&]{renderer.read(surface);});renderer.updateRows(surface,plane,1,1);require(renderer.read(surface).pixels==plane.pixels,"Row validity differs");renderer.destroy(surface);
    std::cout<<"Worker-owned planes; 1MiB upload in 128 ticks <=8192 bytes; exact pixels, no partial presentation/readback, warm palette/shadow reuse, LRU pressure, stale revision and pending cleanup pass\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
