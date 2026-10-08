#include "resource_cache.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <iostream>
using namespace mnm;
using namespace resource_test;
using namespace assets;
using namespace render;
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);QTemporaryDir directory;require(directory.isValid(),"Fixture root failed");
    const auto root=std::filesystem::path(directory.path().toStdString());populate(root);
    ResourceManager resources(store(root));GlBlitter renderer;
    const ResourceId body{ResourceKind::creature,"10"},indexed{ResourceKind::effect,"36"},bmpId{ResourceKind::ui,"bmp"},pcxId{ResourceKind::ui,"pcx"},jpegId{ResourceKind::ui,"jpeg"};
    auto recipe=spr();recipe.animation="body.ani";resources.bind(body,recipe);
    recipe.image="indexed.spr";resources.bind(indexed,recipe);
    resources.bind(bmpId,{ResourceImageFormat::bmp,"ui.bmp",{},{},{}});
    resources.bind(pcxId,{ResourceImageFormat::pcx,"ui.pcx",{},{},{}});
    resources.bind(jpegId,{ResourceImageFormat::jpeg,"ui.jpg",{},{},{}});
    const Image background{5,3,std::vector<std::uint32_t>(15,0x1234)};
    const auto canvas=renderer.create(background,spriteFormat);
    {
        ResourceCache cache(renderer,resources,{2,12});
        auto expected=background;expected.pixels[6]=0;expected.pixels[7]=0xf800;
        cache.draw(body,0,canvas,2,0);
        require(renderer.read(canvas).pixels==expected.pixels,"Resource draw lost origins, opaque black or coverage");
        const auto stats=renderer.stats();cache.draw(body,0,canvas,2,0);
        require(renderer.stats().uploads==stats.uploads && renderer.stats().nativeReadbacks==stats.nativeReadbacks && cache.stats().hits==1,"Resident draw uploaded or read back");
        cache.draw(body,1,canvas,2,0);cache.draw(body,0,canvas,2,0);
        cache.draw(body,2,0,INT32_MIN,INT32_MAX); // empty frame needs no GPU surfaces
        require(cache.stats().frames==2 && cache.stats().surfaces==2 && cache.stats().pixels==6 && cache.stats().evictions==1,"LRU did not evict least recently drawn frame");
        const auto before=cache.stats();rejected([&]{cache.draw(body,3,canvas,2,0);});
        require(cache.stats().evictions==before.evictions && cache.stats().uploads==before.uploads,"Bad frame evicted valid cache state");
        cache.draw(body,0,canvas,2,0);require(cache.stats().hits==3,"Recently used frame was evicted");
        renderer.update(canvas,0,0,background);cache.draw(body,0,canvas,0,0,Rect{0,0,5,3});
        expected=background;expected.pixels[5]=0xf800;require(renderer.read(canvas).pixels==expected.pixels,"Clipped resource draw differs");
        cache.clear();require(renderer.stats().surfaces==1 && !cache.stats().pixels,"Clear leaked GPU storage");
        SpriteColourTable table{};table[0]=0x07e0;table[1]=0x001f;
        cache.draw(indexed,0,canvas,2,0,std::nullopt,&table);table[1]=0xffff;
        cache.draw(indexed,0,canvas,2,0,std::nullopt,&table);
        require(renderer.read(canvas).pixels[7]==0xffff && cache.stats().frames==2,"Palette pointer reused stale colours");
        auto same=table;const auto paletteUploads=cache.stats().uploads;
        cache.draw(indexed,0,canvas,2,0,std::nullopt,&same);
        require(cache.stats().uploads==paletteUploads,"Equal colour table values missed cache");
        rejected([&]{cache.draw(body,0,canvas,2,0,std::nullopt,&table);});
        require(cache.stats().uploads==paletteUploads,"Rejected direct palette override changed cache");
        cache.retire(indexed);require(!cache.stats().frames && resources.stats().residentResources==1,"Retire did not release indexed CPU/GPU resources");
        cache.draw(body,0,canvas,2,0);resources.unload(body);
        write(root,"body.spr",sprite(false,0x07e0));cache.draw(body,0,canvas,2,0);
        require(cache.stats().frames==1 && renderer.read(canvas).pixels[7]==0x07e0,"Reload reused stale GPU revision");
        cache.retire(body);require(renderer.stats().surfaces==1,"Resource retirement leaked surfaces");
        renderer.update(canvas,0,0,background);cache.draw(bmpId,0,canvas,1,0);
        expected=background;expected.pixels[1]=0;expected.pixels[2]=0x07e0;
        require(renderer.read(canvas).pixels==expected.pixels,"BMP upload did not quantize opaque RGB565");
        cache.draw(pcxId,0,canvas,0,1);
        expected.pixels[5]=0;expected.pixels[6]=0xf800;expected.pixels[7]=0x07e0;
        require(renderer.read(canvas).pixels==expected.pixels,"PCX indexed colours/opaque zero differ");
        cache.draw(jpegId,0,canvas,1,2);expected.pixels[11]=0;expected.pixels[12]=0;
        require(renderer.read(canvas).pixels==expected.pixels,"JPEG opaque RGB565 output differs");
        const auto colours=renderer.present(canvas);require(colours.pixelColor(1,1)==QColor(255,0,0),"Presentation colour differs");
        cache.clear();
        std::vector<SurfaceId> external;for(unsigned i=0;i<62;++i)external.push_back(renderer.create({1,1,{0}},spriteFormat));
        const auto refused=cache.stats();rejected([&]{cache.draw(body,0,canvas,2,0);});
        require(renderer.stats().surfaces==63 && cache.stats().uploads==refused.uploads,"External handle refusal leaked/committed upload");
        renderer.destroy(external.back());external.pop_back();cache.draw(body,0,canvas,2,0);
        require(renderer.stats().surfaces==64,"Exact available renderer handle budget refused");
        for(const auto id:external)renderer.destroy(id);
        cache.draw(indexed,0,canvas,2,0); // now cache has two frames
        external.clear();for(unsigned i=0;i<59;++i)external.push_back(renderer.create({1,1,{0}},spriteFormat));
        cache.draw(bmpId,0,canvas,1,0);require(renderer.stats().surfaces==64 && cache.stats().frames==2,"Global handle pressure was not satisfied by LRU eviction");
        for(const auto id:external)renderer.destroy(id);
    }
    require(renderer.stats().surfaces==1 && renderer.stats().pixels==15,"Cache destructor leaked GPU storage");
    {ResourceCache small(renderer,resources,{1,5});rejected([&]{small.draw(body,0,canvas,2,0);});require(!small.stats().frames,"Oversize upload partially published");}
    {ResourceCache exact(renderer,resources,{1,6});exact.draw(body,0,canvas,2,0);require(exact.stats().pixels==6,"Exact texel budget refused");}
    {ResourceCache resident(renderer,resources,{1,6,true});resources.unload(body);
        const auto loads=resources.stats().loads;const auto previous=renderer.read(canvas).pixels;
        rejected([&]{resident.draw(body,0,canvas,2,0);});
        require(resources.stats().loads==loads&&renderer.read(canvas).pixels==previous,"Resident-only cache performed a synchronous reload or changed pixels");
        resources.adopt(prepareResource(resources.request(body)));resident.draw(body,0,canvas,2,0);
        require(resident.stats().frames==1,"Resident-only cache refused an adopted resource");}
    rejected([&]{ResourceCache bad(renderer,resources,{0,6});});
    renderer.destroy(canvas);resources.unloadAll();
    require(!renderer.stats().surfaces && !renderer.stats().pixels && !resources.stats().decodedBytes,"Terminal resources nonzero");
    std::cout<<"Native recipe-to-GPU pixels, hit reuse, LRU, palette values, revision reload, clipping, external budgets and cleanup pass\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
