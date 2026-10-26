#include "sprite_transforms.hpp"
#include "../apps/sprite-session/sprite_session.hpp"
#include <QGuiApplication>
#include <cmath>
#include <iostream>
#include <limits>
using namespace mnm;
namespace {
unsigned checks=0,refusals=0;
void require(bool ok,const char* message) { ++checks;if(!ok)throw std::runtime_error(message); }
template<class F> void rejects(F f) {bool failed=false;try{f();}catch(const std::exception&){failed=true;}require(failed,"Expected transform refusal");++refusals;}
using Kind=render::SpriteTransformKind;
const unsigned samples[20]={0,0,0,0,0, 0,0,1,3,0, 0,2,0,4,0, 0,0,0,0,0};
const bool coverage[20]={0,0,0,0,0, 0,1,1,1,0, 0,1,0,1,0, 0,0,0,0,0};
const std::uint16_t words[5]={0,0xf800,0x7e0,0x1f,0xffff};
assets::Sprite fixture(bool direct=false) {
    assets::Sprite s;s.storage=direct?assets::SpriteStorage::rgb565:assets::SpriteStorage::indexed8;
    if(!direct) {s.palettes.resize(1);s.palettes[0][1]={255,0,0};s.palettes[0][2]={0,255,0};s.palettes[0][3]={0,0,255};s.palettes[0][4]={255,255,255};}
    assets::SpriteFrame f;f.width=5;f.height=4;f.originX=-2;f.originY=3;
    f.name={'A',0,0xff,'X',0,0,0,1};f.sourceOffset=73;f.encodedSize=91;
    f.auxiliaryOffsets={1,2};f.auxiliaryData[0]={8,9};f.legacyPaletteWord=123;
    std::vector<std::uint8_t> indices;std::vector<std::uint16_t> pixels;
    for(unsigned i=0;i<20;++i) {indices.push_back(samples[i]);pixels.push_back(words[samples[i]]);f.opaqueMask.push_back(coverage[i]);}
    if(direct)f.pixels=std::move(pixels);else {f.pixels=std::move(indices);f.paletteIndex=0;}
    f.opaqueRuns={{1,1,4},{2,1,2},{2,3,4}};s.frames.push_back(std::move(f));return s;
}
render::SpriteTransform request(Kind kind) {return {kind,{0,1,4,4},7,3,0xf81f};}
struct Expected {int width=5,height=4,ox=-2,oy=3;std::vector<std::uint16_t> pixels;std::vector<std::uint8_t> mask;};
Expected oracle(const render::SpriteTransform& r,const render::SpriteColourTable* colours) {
    Expected e;
    switch(r.kind) {
    case Kind::crop:e.width=4;e.height=3;e.oy=2;break;
    case Kind::trim:e.width=3;e.height=2;e.ox=-3;e.oy=2;break;
    case Kind::mirrorX:e.ox=6;break;
    case Kind::mirrorY:e.oy=0;break;
    case Kind::clockwise:e.width=4;e.height=5;e.ox=0;e.oy=-2;break;
    case Kind::halfTurn:e.ox=6;e.oy=0;break;
    case Kind::counterclockwise:e.width=4;e.height=5;e.ox=3;e.oy=6;break;
    case Kind::nearestResize:e.width=7;e.height=3;e.ox=-3;e.oy=2;break;
    case Kind::outerOutline:e.width=7;e.height=6;e.ox=-1;e.oy=4;break;
    default:break;
    }
    e.pixels.resize(e.width*e.height);e.mask.resize(e.pixels.size());
    // Forward-map original covered cells. Resize uses independently derived
    // destination intervals instead of the production inverse sampler.
    for(int sy=0;sy<4;++sy)for(int sx=0;sx<5;++sx)if(coverage[sy*5+sx]) {
        int x=sx,y=sy,rx=1,ry=1;
        switch(r.kind) {
        case Kind::crop:--y;break;
        case Kind::trim:--x;--y;break;
        case Kind::mirrorX:x=4-sx;break;
        case Kind::mirrorY:y=3-sy;break;
        case Kind::clockwise:x=3-sy;y=sx;break;
        case Kind::halfTurn:x=4-sx;y=3-sy;break;
        case Kind::counterclockwise:x=sy;y=4-sx;break;
        case Kind::nearestResize:x=int(std::ceil(sx*7.0/5));y=int(std::ceil(sy*3.0/4));
            rx=int(std::ceil((sx+1)*7.0/5))-x;ry=int(std::ceil((sy+1)*3.0/4))-y;break;
        case Kind::outerOutline:++x;++y;break;
        default:break;
        }
        for(int dy=y;dy<y+ry;++dy)for(int dx=x;dx<x+rx;++dx)if(dx>=0&&dy>=0&&dx<e.width&&dy<e.height) {
            const auto at=std::size_t(dy)*e.width+dx;e.mask[at]=1;
            e.pixels[at]=r.kind==Kind::silhouette?r.colour:colours?(*colours)[samples[sy*5+sx]]:words[samples[sy*5+sx]];
        }
    }
    if(r.kind==Kind::outerOutline) {
        const auto interior=e.mask;
        for(int y=0;y<e.height;++y)for(int x=0;x<e.width;++x)if(interior[y*e.width+x])
            for(auto p:{std::pair<int,int>{x-1,y},{x+1,y},{x,y-1},{x,y+1}})
                if(p.first>=0&&p.second>=0&&p.first<e.width&&p.second<e.height&&!interior[p.second*e.width+p.first]) {
                    e.mask[p.second*e.width+p.first]=1;e.pixels[p.second*e.width+p.first]=r.colour;
                }
    }
    return e;
}
void equivalent(const assets::Sprite& a,const assets::Sprite& b) {
    const auto& x=a.frames.at(0);const auto& y=b.frames.at(0);
    require(x.width==y.width&&x.height==y.height&&x.originX==y.originX&&x.originY==y.originY&&x.pixels==y.pixels&&x.opaqueMask==y.opaqueMask,"Transform composition identity differs");
}
}
int main(int argc,char** argv)try {
    QGuiApplication app(argc,argv);render::GlBlitter renderer;const auto baseline=renderer.stats();
    const Kind kinds[]={Kind::crop,Kind::trim,Kind::mirrorX,Kind::mirrorY,Kind::clockwise,Kind::halfTurn,Kind::counterclockwise,Kind::nearestResize,Kind::silhouette,Kind::outerOutline};
    render::SpriteColourTable colours{};for(unsigned i=0;i<256;++i)colours[i]=std::uint16_t((i*1297)^0x55aa);
    unsigned cases=0,geometry=0;
    for(bool direct:{false,true})for(bool overridePalette:{false,true}) {
        if(direct&&overridePalette)continue;
        for(auto kind:kinds) {
            auto source=fixture(direct);const auto original=source.frames[0].pixels;const auto r=request(kind);
            const auto expected=oracle(r,overridePalette?&colours:nullptr);
            auto derived=render::transformSpriteFrame(source,0,r,overridePalette?&colours:nullptr);
            require(source.frames[0].pixels==original&&source.frames[0].originX==-2&&source.frames[0].auxiliaryData[0].size()==2,"Transform mutated the source");
            const auto& f=derived.frames.at(0);
            require(derived.storage==assets::SpriteStorage::rgb565&&derived.palettes.empty(),"Derived resource is not direct RGB565");
            require(f.width==unsigned(expected.width)&&f.height==unsigned(expected.height)&&f.originX==expected.ox&&f.originY==expected.oy,"Transformed extent/origin differs");
            require(std::get<std::vector<std::uint16_t>>(f.pixels)==expected.pixels&&f.opaqueMask==expected.mask,"Forward oracle pixels/mask differ");
            require(f.name==source.frames[0].name&&!f.paletteIndex&&!f.legacyPaletteWord&&f.opaqueRuns.empty()&&f.auxiliaryData[0].empty()&&f.auxiliaryOffsets[0]==0&&f.sourceOffset==0&&f.encodedSize==0,"Derived frame retained encoded metadata");
            // Only the draw plan survives the temporary table and session.
            const auto name=f.name;auto plan=[&] {
                assets::TagAsset tag;tag.entries.push_back({name,0});
                auto table=std::make_shared<const sprites::SpriteTable>(std::move(derived),tag);
                sprites::SpriteSession session(table,name,0,1,1,sprites::Playback::loop);return session.plan(0,0);
            }();
            source.frames.clear();source.palettes.clear();++geometry;
            for(auto anchor:{std::pair<int,int>{2,2},{-3,1},{9,9},{std::numeric_limits<int>::min(),1},{1,std::numeric_limits<int>::max()}}) {
                auto& placement=plan.placements[0];placement.x=anchor.first;placement.y=anchor.second;
                render::Image wanted{12,10,std::vector<std::uint32_t>(120,0x1234)},cpu=wanted;
                const render::Rect clip{1,1,11,9};
                for(int sy=0;sy<expected.height;++sy)for(int sx=0;sx<expected.width;++sx)if(expected.mask[sy*expected.width+sx]) {
                    const auto x=std::int64_t(anchor.first)-expected.ox+sx,y=std::int64_t(anchor.second)-expected.oy+sy;
                    if(x>=1&&x<11&&y>=1&&y<9)wanted.pixels[std::size_t(y)*12+std::size_t(x)]=expected.pixels[sy*expected.width+sx];
                }
                render::drawSpritePixels(cpu,plan,clip);require(cpu.pixels==wanted.pixels,"Transformed CPU delivery differs");
                const auto destination=renderer.create({12,10,std::vector<std::uint32_t>(120,0x1234)},render::spriteFormat);
                const auto stats=renderer.stats();render::drawSpriteBatch(renderer,destination,plan,clip);const auto after=renderer.stats();
                require(after.nativeReadbacks==stats.nativeReadbacks&&after.rgbaReadbacks==stats.rgbaReadbacks,"Transform delivery read back source pixels");
                require(after.surfaces==stats.surfaces,"Derived upload retained GPU planes");
                require(renderer.read(destination).pixels==wanted.pixels,"Transformed GPU delivery differs");
                renderer.destroy(destination);require(renderer.stats().surfaces==baseline.surfaces,"Transform lifetime leaked surfaces");++cases;
            }
        }
    }
    auto source=fixture(true);auto normal=render::transformSpriteFrame(source,0,{Kind::crop,{0,0,5,4}});
    for(auto kind:{Kind::mirrorX,Kind::mirrorY,Kind::halfTurn}) {
        auto first=render::transformSpriteFrame(source,0,request(kind));equivalent(normal,render::transformSpriteFrame(first,0,request(kind)));
    }
    auto turned=source;for(unsigned i=0;i<4;++i)turned=render::transformSpriteFrame(turned,0,request(Kind::clockwise));equivalent(normal,turned);
    auto first=render::transformSpriteFrame(source,0,request(Kind::clockwise));equivalent(normal,render::transformSpriteFrame(first,0,request(Kind::counterclockwise)));
    first=render::transformSpriteFrame(source,0,request(Kind::trim));equivalent(first,render::transformSpriteFrame(first,0,request(Kind::trim)));
    first=render::transformSpriteFrame(source,0,request(Kind::silhouette));equivalent(first,render::transformSpriteFrame(first,0,request(Kind::silhouette)));
    auto identity=request(Kind::nearestResize);identity.width=5;identity.height=4;equivalent(normal,render::transformSpriteFrame(source,0,identity));
    auto empty=source;empty.frames[0].width=empty.frames[0].height=0;empty.frames[0].pixels=std::vector<std::uint16_t>{};empty.frames[0].opaqueMask.clear();empty.frames[0].opaqueRuns.clear();
    for(auto kind:kinds) {
        if(kind==Kind::crop||kind==Kind::nearestResize)rejects([&]{render::transformSpriteFrame(empty,0,request(kind));});
        else {const auto result=render::transformSpriteFrame(empty,0,request(kind));require(result.frames[0].empty()&&result.frames[0].originX==0&&result.frames[0].originY==0,"Empty transform is not canonical");}
    }
    auto transparent=fixture(true);transparent.frames[0].opaqueMask.assign(20,0);transparent.frames[0].opaqueRuns.clear();
    require(render::transformSpriteFrame(transparent,0,request(Kind::trim)).frames[0].empty(),"Transparent trim did not become empty");
    for(auto crop:std::vector<render::Rect>{{-1,0,1,1},{0,-1,1,1},{0,0,0,1},{0,0,6,1},{0,0,1,5},{3,2,2,3}})
        rejects([&]{render::transformSpriteFrame(source,0,{Kind::crop,crop});});
    for(int w:{-1,0,1,1024,1025,2048,2049,std::numeric_limits<int>::max()})for(int h:{-1,0,1025,2049})
        if(w>0&&h>0&&w<=2048&&h<=2048&&std::int64_t(w)*h<=1024*1024) {
            const auto result=render::transformSpriteFrame(source,0,{Kind::nearestResize,{},w,h});
            require(result.frames[0].width==unsigned(w)&&result.frames[0].height==unsigned(h),"Valid resize extent refused");
        } else rejects([&]{render::transformSpriteFrame(source,0,{Kind::nearestResize,{},w,h});});
    for(auto kind:{Kind::crop,Kind::mirrorX,Kind::mirrorY,Kind::clockwise,Kind::halfTurn,Kind::counterclockwise,Kind::nearestResize,Kind::outerOutline}) {
        auto huge=source;auto r=request(kind);
        huge.frames[0].originX=huge.frames[0].originY=std::numeric_limits<std::int32_t>::min();
        if(kind==Kind::crop)r.region={1,1,4,3};
        if(kind==Kind::outerOutline)huge.frames[0].originX=std::numeric_limits<std::int32_t>::max();
        if(kind==Kind::nearestResize){r.width=10;r.height=8;}
        rejects([&]{render::transformSpriteFrame(huge,0,r);});
    }
    for(unsigned failure=0;failure<6;++failure) {
        auto bad=source;
        if(failure==0)bad.frames[0].width=2049;
        if(failure==1){bad.frames[0].width=1025;bad.frames[0].height=1024;}
        if(failure==2)bad.frames[0].opaqueMask[0]=2;
        if(failure==3)bad.frames[0].pixels=std::vector<std::uint8_t>(20);
        if(failure==4)bad.frames[0].paletteIndex=0;
        if(failure==5)bad.frames[0].opaqueRuns[0].right=6;
        rejects([&]{render::transformSpriteFrame(bad,0,request(Kind::trim));});
    }
    rejects([&]{render::transformSpriteFrame(source,1,request(Kind::trim));});
    rejects([&]{render::transformSpriteFrame(source,0,{static_cast<Kind>(99)});});
    rejects([&]{render::transformSpriteFrame(source,0,request(Kind::trim),&colours);});
    auto resize=request(Kind::nearestResize);resize.width=2048;resize.height=512;
    const auto bounded=render::transformSpriteFrame(source,0,resize);require(bounded.frames[0].opaqueMask.size()==1024*1024,"Exact pixel budget refused");
    resize.width=1025;resize.height=1024;rejects([&]{render::transformSpriteFrame(source,0,resize);});
    auto wide=source;auto& frame=wide.frames[0];frame.width=2048;frame.height=1;frame.pixels=std::vector<std::uint16_t>(2048,1);frame.opaqueMask.assign(2048,1);frame.opaqueRuns.clear();
    rejects([&]{render::transformSpriteFrame(wide,0,request(Kind::outerOutline));});
    require(renderer.stats().surfaces==baseline.surfaces,"Final transform surfaces leaked");
    std::cout<<"{\"success\":true,\"checks\":"<<checks<<",\"refusals\":"<<refusals<<",\"cpu_gpu_cases\":"<<cases<<",\"geometry_cases\":"<<geometry<<",\"scopes\":10,\"surface_leaks\":0}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
