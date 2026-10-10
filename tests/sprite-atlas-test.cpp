#include "resource_cache.hpp"
#include "sprite_atlas.hpp"
#include "scene_renderer.hpp"
#include "resource-fixtures.hpp"
#include "../renderer/known_pixels.hpp"
#include <QGuiApplication>
#include <chrono>
#include <iostream>
using namespace mnm;
using namespace resource_test;
using namespace render;
using namespace assets;
int main(int argc,char** argv)try{
    {
        KnownPixels validity(1024,2);validity.invalidate();
        validity.define({0,0,8,1});validity.define({9,0,16,1});
        require(validity.known({1,0,7,1})&&!validity.known({0,0,16,1}),"Validity proofs admitted an undefined hole");
        // Exceed the bounded rectangle cache: forgotten proofs must fall back
        // to the authoritative map, and invalidation must retire every proof.
        for(int x=0;x<1024;x+=2)validity.define({x,1,x+1,2});
        require(validity.known({0,0,8,1})&&validity.known({1022,1,1023,2})&&!validity.known({0,1,2,2}),"Validity proof eviction changed known pixels");
        validity.invalidate();require(!validity.known({1,0,7,1}),"Invalidation retained a stale validity proof");
        validity.define({0,0,1024,2});require(validity.known({0,0,1024,2}),"Full definition lost known pixels");
    }
    QGuiApplication app(argc,argv);QTemporaryDir directory;const auto root=std::filesystem::path(directory.path().toStdString());populate(root);
    ResourceManager resources(store(root));const ResourceId indexed{ResourceKind::ui,"indexed"},wordId{ResourceKind::ui,"word"};
    resources.bind(indexed,spr("indexed.spr"));resources.bind(wordId,spr());resources.load(indexed);resources.load(wordId);
    GlBlitter renderer;const Image background{32,12,std::vector<std::uint32_t>(384,0xa35b)};
    const auto actual=renderer.create(background,spriteFormat),expected=renderer.create(background,spriteFormat);
    ResourceCacheLimits limits;limits.indexedAtlas=true;limits.pixels=8192;
    {
        ResourceCache atlas(renderer,resources,limits),expanded(renderer,resources);
        SpriteColourTable palette{};
        for(unsigned i=0;i<256;++i)palette[i]=std::uint16_t(i*251);
        // The frame's opaque index zero must remain distinguishable from holes.
        for(const auto mode:{CompositeMode::copy,CompositeMode::half,CompositeMode::quarterSource,CompositeMode::quarterDestination,CompositeMode::displace}){
            SpriteComposite operation;operation.mode=mode;for(unsigned i=0;i<16;++i)operation.rowOffsets[i]=i%3;
            for(const auto pos:{std::pair<int,int>{2,0},{0,-1},{-1,-1},{8,3}}){
                renderer.update(actual,0,0,background);renderer.update(expected,0,0,background);
                atlas.draw(indexed,0,actual,pos.first,pos.second,Rect{0,0,28,12},&palette,operation);
                expanded.draw(indexed,0,expected,pos.first,pos.second,Rect{0,0,28,12},&palette,operation);
                require(renderer.read(actual).pixels==renderer.read(expected).pixels,"Indexed atlas composite/clipping differs from expanded words");
            }
        }
        const auto uploads=atlas.stats().uploads;
        for(unsigned i=0;i<256;++i){palette[0]=std::uint16_t(i*257);palette[1]=std::uint16_t(65535-i*131);
            atlas.draw(indexed,0,actual,2,0,std::nullopt,&palette);
            require(renderer.read(actual).pixels[33]==palette[0]&&renderer.read(actual).pixels[34]==palette[1],"Palette values/opaque zero differ");}
        require(atlas.stats().uploads==uploads&&atlas.stats().frames==1,"Palette variant allocated or uploaded sprite data");
        const auto uniforms=renderer.stats().paletteUpdates;atlas.draw(indexed,0,actual,2,0,std::nullopt,&palette);
        require(renderer.stats().paletteUpdates==uniforms,"Equal palette values resent uniforms");
        renderer.update(actual,0,0,background);renderer.update(expected,0,0,background);
        atlas.draw(indexed,1,actual,5,0);expanded.draw(indexed,1,expected,5,0);
        require(renderer.read(actual).pixels==renderer.read(expected).pixels,"Embedded palette or nonzero atlas origin differs");
        atlas.draw(wordId,0,actual,8,2);expanded.draw(wordId,0,expected,8,2);
        require(renderer.read(actual).pixels==renderer.read(expected).pixels,"Word atlas draw differs");
        SpriteComposite shadow;shadow.mode=CompositeMode::projectedShadow;
        atlas.draw(indexed,0,actual,12,2,Rect{0,0,32,12},nullptr,shadow);expanded.draw(indexed,0,expected,12,2,Rect{0,0,32,12},nullptr,shadow);
        require(renderer.read(actual).pixels==renderer.read(expected).pixels,"Projected shadow atlas differs");
        require(atlas.stats().surfaces==2&&atlas.stats().pixels<=limits.pixels,"Packed page storage is not bounded");
        const auto before=atlas.request(indexed,0,&palette);resources.unload(indexed);resources.load(indexed);
        rejected([&]{atlas.supply(before,prepareResourceUpload(resources.resident(indexed),before));});
        atlas.draw(indexed,0,actual,2,0,std::nullopt,&palette);
        require(atlas.stats().frames==2,"Stale resource revision remained in atlas"); // word + indexed; old shadow retired
        atlas.release(indexed);atlas.release(wordId);require(!atlas.stats().surfaces&&!atlas.stats().frames,"Atlas release retained empty pages");
    }
    {
        // Small page packing and LRU pressure exercise holes, page reuse and release.
        SpriteAtlas atlas(renderer,128,128);std::vector<ResourceUploadRequest> requests;
        for(unsigned i=0;i<50;++i){ResourceUploadRequest request{{ResourceKind::ui,"item"+std::to_string(i)},1,0,{},{},false};requests.push_back(request);
            PreparedSpriteFrame p{{3,1,{i,i,i}},{3,1,{1,1,1}},0,0,{}};atlas.supply(request,std::move(p));
            std::size_t bytes=24;require(!atlas.advance(request,bytes)&&bytes==12,"One advance wrote more than one plane");require(atlas.advance(request,bytes)&&bytes==0,"Atlas upload byte accounting differs");
            atlas.draw(request,actual,0,0,{},{});require(renderer.read(actual).pixels[0]==i,"Evicted page reused old pixels");
            require(atlas.stats().surfaces<=2&&atlas.stats().pixels<=128&&atlas.stats().frames<=16,"Page or metadata bound exceeded");
        }
        require(atlas.stats().evictions>0&&!atlas.ready(requests.front()),"Full page did not evict");
        auto malformed=PreparedSpriteFrame{{2,1,{1,2}},{2,1,{1}},0,0,{}};auto bad=requests.back();bad.frame=1;
        rejected([&]{atlas.supply(bad,std::move(malformed));});
        auto oversized=PreparedSpriteFrame{{9,9,std::vector<std::uint32_t>(81)},{9,9,std::vector<std::uint32_t>(81)},0,0,{}};
        rejected([&]{atlas.supply(bad,std::move(oversized));});
        atlas.clear();require(!atlas.stats().pixels&&!atlas.stats().frames,"Atlas clear leaked pages");
        SpriteAtlas metadata(renderer,8192,1);
        for(unsigned i=0;i<2;++i){auto r=requests[i];metadata.supply(r,{{3,1,{1,2,3}},{3,1,{1,1,1}},0,0,{}});std::size_t bytes=24;while(!metadata.advance(r,bytes)){};}
        require(metadata.stats().frames==1&&!metadata.ready(requests.front()),"Metadata cap ignored");
    }
    {
        // Large cold atlas uploads keep the same row budget as standalone planes.
        SpriteAtlas atlas(renderer,1024*1024,4);ResourceUploadRequest r{indexed,1,12,{},{},true};
        SpriteColourTable palette{};palette[0]=0;
        PreparedSpriteFrame p{{512,256,std::vector<std::uint32_t>(512*256,0)},{512,256,std::vector<std::uint32_t>(512*256,1)},0,0,palette};
        atlas.supply(r,std::move(p));rejected([&]{atlas.draw(r,actual,0,0,Rect{0,0,32,12},{});});
        const auto reads=renderer.stats().nativeReadbacks;unsigned ticks=0;std::uint64_t transferred=0;
        while(!atlas.ready(r)){std::size_t bytes=8192;atlas.advance(r,bytes);transferred+=8192-bytes;++ticks;}
        require(ticks==128&&transferred==1048576&&renderer.stats().nativeReadbacks==reads,"Large atlas upload escaped byte/readback bounds");
        atlas.draw(r,actual,0,0,Rect{0,0,32,12},{});require(renderer.read(actual).pixels==std::vector<std::uint32_t>(384,0),"Opaque palette zero became transparent");
        auto bad=r;bad.frame=13;auto invalid=PreparedSpriteFrame{{1,1,{256}},{1,1,{1}},0,0,palette};atlas.supply(bad,std::move(invalid));
        std::size_t bytes=8192;rejected([&]{atlas.advance(bad,bytes);});require(!atlas.ready(bad),"Out-of-range indices became drawable");
        atlas.release(indexed);require(!atlas.stats().pixels,"Pending atlas release leaked storage");
        auto maskRequest=r;maskRequest.indexed=false;maskRequest.frame=14;atlas.supply(maskRequest,{{1,1,{0}},{1,1,{2}},0,0,{}});
        require(!atlas.advance(maskRequest,bytes),"Mask validation test completed too early");rejected([&]{atlas.advance(maskRequest,bytes);});
    }
    {
        // Row writes at atlas offsets define exactly their patch, with no full-page fill.
        auto surface=renderer.allocate(8,8,spriteFormat);Image plane{2,2,{1,2,3,4}};
        renderer.updateRegionRows(surface,3,4,plane,0,1);rejected([&]{renderer.read(surface);});
        rejected([&]{renderer.copy(surface,actual,{3,5,5,6},0,0);});
        renderer.updateRegionRows(surface,3,4,plane,1,1);renderer.copy(surface,actual,{3,4,5,6},0,0);
        require(renderer.read(actual).pixels[0]==1&&renderer.read(actual).pixels[33]==4,"Atlas region row offset differs");
        rejected([&]{renderer.updateRegionRows(surface,7,4,plane,0,1);});renderer.destroy(surface);
    }
    {
        // Scene upload admission uses one owned indexed completion for every palette.
        SceneLimits sceneLimits;sceneLimits.cache=limits;sceneLimits.cache.residentOnly=true;sceneLimits.cache.preparedOnly=true;
        SceneRenderer scene(renderer,resources,background,sceneLimits);SpriteColourTable palette{};palette[0]=0;palette[1]=0xffff;
        scene.beginFrame({{indexed,0,2,0,true,true,palette}});const SceneBatchBudget budget{32,8192,std::chrono::milliseconds(20)};
        require(!scene.drawNext(budget)&&scene.uploadNeed().has_value(),"Cold atlas scene skipped preparation");auto need=*scene.uploadNeed();require(need.request.indexed,"Scene preparation lost indexed admission");
        scene.supplyUpload(need,prepareResourceUpload(resources.resident(indexed),need.request));
        const auto before=renderer.stats().nativeReadbacks;unsigned ticks=0;while(!scene.drawNext(budget))++ticks;
        require(ticks==0&&scene.uploadStats().bytes==24&&scene.uploadStats().maxTickBytes==24,"Atlas scene byte budget differs");
        require(renderer.stats().nativeReadbacks==before,"Normal atlas drawing read back pixels");scene.presentGpu();
        const auto uploads=scene.cacheStats().uploads;palette[1]=0x07e0;scene.beginFrame({{indexed,0,2,0,true,true,palette}});
        require(scene.drawNext(budget)&&!scene.uploadNeed()&&scene.cacheStats().uploads==uploads,"Changed palette requested new worker/upload");
        require(scene.read().pixels[34]==0x07e0,"Warm palette draw differs");
    }
    {
        // Independent scalar oracle: scratch grows, is reused at new origins,
        // and overlapping draws see the immediately preceding destination.
        Image source{3,2,{0xffff,0x1234,0x07e0,0xf800,0x001f,0x8765}},coverage{3,2,{1,0,1,0,1,1}};
        auto pixels=renderer.create(source,spriteFormat),mask=renderer.create(coverage,{8,{}});
        Image oracle=background;for(unsigned i=0;i<oracle.pixels.size();++i)oracle.pixels[i]=(i*1031u)&65535u;
        renderer.update(actual,0,0,oracle);const auto before=renderer.stats();std::uint64_t warmedAllocations=0;
        for(unsigned pass=0;pass<3;++pass)for(auto mode:{CompositeMode::half,CompositeMode::quarterSource,CompositeMode::displace,CompositeMode::quarterDestination}){
            SpriteComposite operation;operation.mode=mode;operation.rowPeriod=3;operation.rowOffsets={0,16,7};
            const int x=2+int(pass)*2,y=3+int(pass);const auto previous=oracle.pixels;
            for(int sy=0;sy<2;++sy)for(int sx=1;sx<3;++sx){if(!coverage.pixels[sy*3+sx])continue;
                auto& d=oracle.pixels[(y+sy)*32+x+sx-1];const auto v=source.pixels[sy*3+sx];
                if(mode==CompositeMode::half)d=((v>>1)&0x7bef)+((d>>1)&0x7bef);
                else if(mode==CompositeMode::quarterSource){const auto half=(d>>1)&0x7bef;d=half+((half>>1)&0x7bef)+((v>>2)&0x39e7);}
                else if(mode==CompositeMode::quarterDestination){const auto half=(v>>1)&0x7bef;d=half+((half>>1)&0x7bef)+((d>>2)&0x39e7);}
                else d=previous[(y+sy)*32+x+sx-1+operation.rowOffsets[(y+sy)%3]];
            }
            renderer.composite(pixels,actual,{1,0,3,2},x,y,mask,operation);
            require(renderer.read(actual).pixels==oracle.pixels,"Reused regional scratch differs from ordered scalar oracle");
            const Rect rect{x+1,y, x+3,y+3};const std::array<std::uint16_t,3> add{65535,3,5};
            for(int row=rect.top;row<rect.bottom;++row)for(int col=rect.left;col<rect.right;++col){auto& d=oracle.pixels[row*32+col];d=(std::min(((d>>11)+add[0])&65535u,31u)<<11)|(std::min((((d>>5)&63)+add[1])&65535u,63u)<<5)|std::min(((d&31)+add[2])&65535u,31u);}
            renderer.additiveRect(actual,rect,add);require(renderer.read(actual).pixels==oracle.pixels,"Additive scratch origin or ordered destination differs");
            if(pass==0)warmedAllocations=renderer.stats().scratchAllocations;else require(renderer.stats().scratchAllocations==warmedAllocations,"Warm scratch allocated again");
        }
        const auto after=renderer.stats();require(after.scratchAllocations==warmedAllocations&&after.snapshotPixels-before.snapshotPixels<1000&&after.scratchPixels<=2048u*2048u,"Scratch reuse or regional copy bound differs");
        auto undefined=renderer.allocate(32,12,spriteFormat);SpriteComposite half;half.mode=CompositeMode::half;
        rejected([&]{renderer.composite(pixels,undefined,{1,0,3,2},2,3,mask,half);});renderer.destroy(undefined);renderer.destroy(mask);renderer.destroy(pixels);
    }
    {
        Image source{3,2,{1,2,3,4,5,6}},maskPixels{3,2,{1,0,1,1,1,0}},oracle=background;
        auto src=renderer.create(source,spriteFormat),mask=renderer.create(maskPixels,{8,{}});renderer.update(actual,0,0,oracle);
        SpriteColourTable palette{};for(unsigned i=0;i<256;++i)palette[i]=std::uint16_t(i*131);
        auto copyOracle=[&](int x,int y){for(int row=0;row<2;++row)for(int col=0;col<3;++col)if(maskPixels.pixels[row*3+col])oracle.pixels[(y+row)*32+x+col]=palette[source.pixels[row*3+col]];};
        const auto before=renderer.stats();
        renderer.batch([&]{
            for(unsigned i=0;i<1100;++i){const int x=int(i%4)+3,y=int(i%3)+2;copyOracle(x,y);renderer.composite(src,actual,{0,0,3,2},x,y,mask,{},&palette);}
            // Mutating a queued source must flush the old pixels first.
            for(unsigned i=0;i<129;++i){palette[1]=std::uint16_t(i*503);copyOracle(10,5);renderer.composite(src,actual,{0,0,3,2},10,5,mask,{},&palette);}
            source.pixels[0]=7;renderer.update(src,0,0,source);copyOracle(11,4);renderer.composite(src,actual,{0,0,3,2},11,4,mask,{},&palette);
            palette[7]=0xf800;copyOracle(11,4);renderer.composite(src,actual,{0,0,3,2},11,4,mask,{},&palette);
            require(renderer.read(actual).pixels==oracle.pixels,"Pending instance read or source/palette mutation changed order");
            renderer.batch([&]{copyOracle(14,6);renderer.composite(src,actual,{0,0,3,2},14,6,mask,{},&palette);});
            // Storage destruction observes pending copies before releasing names.
            renderer.destroy(src);renderer.destroy(mask);
        });
        require(renderer.read(actual).pixels==oracle.pixels,"Overlapping instanced copies or nested destruction differ");
        const auto after=renderer.stats();require(after.maxCopyBatch==512&&after.copyBatches-before.copyBatches<12,"Consecutive opaque rectangles were not bounded and batched");
    }
    // External renderer handles are still an admission limit, even with free pixels.
    {std::vector<SurfaceId> external;for(unsigned i=0;i<61;++i)external.push_back(renderer.create({1,1,{0}},spriteFormat));
        ResourceCache atlas(renderer,resources,limits);rejected([&]{atlas.draw(indexed,0,actual,2,0);});require(!atlas.stats().frames,"External handle refusal admitted entry");
        for(auto id:external)renderer.destroy(id);}
    renderer.destroy(actual);renderer.destroy(expected);require(!renderer.stats().surfaces&&!renderer.stats().pixels,"Terminal atlas GPU storage leaked");
    std::cout<<"Atlas indexed/word/shadow pixels match expanded path; 256 palette variants use one frame; clipped integer composites, row budgets, page/metadata eviction, revisions, external bounds and cleanup pass\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
