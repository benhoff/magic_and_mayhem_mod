#include "sprite_session.hpp"
#include <QGuiApplication>
#include <QTemporaryDir>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
using namespace mnm;
namespace {
unsigned checks=0,refusals=0;
void require(bool ok,const char* message) { ++checks;if(!ok)throw std::runtime_error(message); }
template<class F> void rejects(F f) { bool refused=false;try{f();}catch(const std::exception&){refused=true;}require(refused,"Expected refusal");++refusals; }
const sprites::Name walk={'W',0,0xff,'K',0,0,0,1}, idle={'W',0,0xff,'K',0,0,0,2};
assets::Sprite fixture(bool direct=false) {
    assets::Sprite s;s.storage=direct?assets::SpriteStorage::rgb565:assets::SpriteStorage::indexed8;
    if(!direct) { s.palettes.resize(1);s.palettes[0][1]={255,0,0};s.palettes[0][2]={0,255,0}; }
    for(unsigned i=0;i<8;++i) {
        assets::SpriteFrame f;f.name=i%2?idle:walk;f.originX=1;f.originY=-1;
        if(!direct)f.paletteIndex=0;
        if(i!=7) {
            f.width=3;f.height=2;f.opaqueMask={1,0,1,1,1,0};
            if(direct)f.pixels=std::vector<std::uint16_t>{0xf800,0,0x7e0,0,0xf800,0};
            else f.pixels=std::vector<std::uint8_t>{1,0,2,0,1,0};
            f.opaqueRuns={{0,0,1},{0,2,3},{1,0,2}};
        } else if(direct) f.pixels=std::vector<std::uint16_t>{};
        s.frames.push_back(std::move(f));
    }
    return s;
}
assets::TagAsset tags(const assets::Sprite& s) {
    assets::TagAsset t;std::map<sprites::Name,unsigned> seen;
    for(const auto& f:s.frames)t.entries.push_back({f.name,seen[f.name]++});
    return t;
}
bool same(const sprites::Checkpoint& a,const sprites::Checkpoint& b) {
    return a.table==b.table && a.name==b.name && a.first==b.first && a.count==b.count &&
           a.mode==b.mode && a.frameTicks==b.frameTicks && a.phase==b.phase;
}
void put(std::vector<std::uint8_t>& b,std::size_t at,std::uint32_t value) {
    for(unsigned i=0;i<4;++i)b.at(at+i)=std::uint8_t(value>>(8*i));
}
void save(const std::filesystem::path& p,const std::vector<std::uint8_t>& b) {
    std::ofstream f(p,std::ios::binary);f.write(reinterpret_cast<const char*>(b.data()),b.size());
    if(!f)throw std::runtime_error("Fixture write failed");
}
}
int main(int argc,char** argv)try {
    QGuiApplication app(argc,argv);auto raw=fixture();const auto tag=tags(raw);
    auto table=std::make_shared<const sprites::SpriteTable>(raw,tag);
    raw.frames.clear();require(table->sprite()->frames.size()==8,"Sprite source ownership differs");
    for(unsigned i=0;i<4;++i) {
        require(table->find(walk,i)==2*i,"Exact name occurrence lookup differs");
        require(table->find(idle,i)==2*i+1,"Bytes after NUL must remain distinct");
    }
    require(table->range(walk,1,3)==std::vector<std::size_t>({2,4,6}),"Interleaved named clip differs");
    rejects([&]{table->find({},0);});rejects([&]{table->find(walk,4);});
    for(auto range:std::vector<std::pair<unsigned,unsigned>>{{0,0},{4,1},{3,2},{0,0xffffffffu},{0xffffffffu,1}})
        rejects([&]{table->range(walk,range.first,range.second);});
    for(unsigned failure=0;failure<10;++failure) {
        auto s=fixture();auto t=tags(s);
        if(failure==0)t.entries.pop_back();
        if(failure==1)t.entries[0].nameBytes[7]^=1;
        if(failure==2)t.entries[4].occurrence=0;
        if(failure==3)t.entries[2].occurrence=0xffffffffu;
        if(failure==4)s.frames[0].opaqueMask[0]=2;
        if(failure==5)s.frames[0].paletteIndex=2;
        if(failure==6)s.frames[0].width=0xffffffffu;
        if(failure==7)s.frames[0].opaqueRuns[0].right=4;
        if(failure==8)s.frames[0].pixels=std::vector<std::uint16_t>(6);
        if(failure==9)s.frames.clear();
        rejects([&]{sprites::SpriteTable rejected(std::move(s),t);});
    }
    // Independent tick-by-tick model, then explicit maximal elapsed values.
    unsigned timingCases=0;
    for(auto mode:{sprites::Playback::loop,sprites::Playback::once})for(unsigned rate:{1u,3u,1000u}) {
        sprites::SpriteSession session(table,walk,1,3,rate,mode);std::uint64_t expected=0;
        for(unsigned i=0;i<500;++i) {
            const auto delta=(i*17)%91;
            for(unsigned tick=0;tick<delta;++tick) {
                if(mode==sprites::Playback::loop)expected=(expected+1)%(3*rate);
                else if(expected<3*rate)++expected;
            }
            session.advance(delta);require(session.checkpoint().phase==expected,"Playback phase differs from independent ticks");
            require(session.frame()==2*(1+std::min<std::uint64_t>(expected/rate,2)),"Playback frame differs");
            require(session.finished()==(mode==sprites::Playback::once && expected==3*rate),"One-shot completion differs");
        }
        const auto old=session.checkpoint();session.advance(std::numeric_limits<std::uint64_t>::max());
        const auto wanted=mode==sprites::Playback::once?3*rate:(expected+std::numeric_limits<std::uint64_t>::max()%(3*rate))%(3*rate);
        require(session.checkpoint().phase==wanted,"Maximal elapsed ticks overflowed");
        session.restore(old);require(same(old,session.checkpoint()),"Checkpoint round trip differs");++timingCases;
    }
    sprites::SpriteSession session(table,walk,0,4,2,sprites::Playback::loop);session.advance(5);
    const auto before=session.checkpoint();
    for(unsigned failure=0;failure<8;++failure) {
        auto c=before;
        if(failure==0)c.table.reset();
        if(failure==1)c.count=0;
        if(failure==2)c.frameTicks=0;
        if(failure==3)c.frameTicks=1000000001;
        if(failure==4)c.mode=static_cast<sprites::Playback>(3);
        if(failure==5)c.phase=8;
        if(failure==6)c.first=0xffffffffu;
        if(failure==7)c.name={};
        rejects([&]{session.restore(c);});require(same(before,session.checkpoint()),"Bad checkpoint modified session");
    }
    rejects([&]{session.reload(nullptr);});require(same(before,session.checkpoint()),"Null reload changed session");
    auto newSource=fixture(true);auto newTable=std::make_shared<const sprites::SpriteTable>(newSource,tags(newSource));
    render::SpriteColourTable palette{};palette[0]=0x1f;palette[1]=0xffff;palette[2]=0xf81f;
    auto retained=session.plan(3,2,palette);palette[1]=0;
    session.reload(newTable);require(session.checkpoint().phase==0 && session.checkpoint().table==newTable,"Reload failed to reset phase");
    require(retained.source==table->sprite() && retained.placements[0].colours->at(1)==65535,"Reload or caller mutation changed owned plan");
    rejects([&]{session.plan(0,0,palette);});
    auto incomplete=fixture();incomplete.frames.erase(incomplete.frames.begin()+2,incomplete.frames.end());
    auto shorter=std::make_shared<const sprites::SpriteTable>(incomplete,tags(incomplete));const auto reloadBefore=session.checkpoint();
    rejects([&]{session.reload(shorter);});require(same(reloadBefore,session.checkpoint()),"Missing reload clip changed session");
    // Actual pair acquisition and file-independent lifetime, including failures.
    QTemporaryDir directory;require(directory.isValid(),"Fixture directory failed");
    const auto root=std::filesystem::path(directory.path().toStdString());const unsigned base=24+768+4;
    std::vector<std::uint8_t> spr(base+52,0),tagBytes(12,0);
    put(spr,0,0x00525053);put(spr,4,spr.size());put(spr,8,4);put(spr,12,1);put(spr,16,1);
    put(spr,base,52);put(spr,base+4,2);put(spr,base+8,1);
    std::copy(walk.begin(),walk.end(),spr.begin()+base+20);put(spr,base+40,48);put(spr,base+44,50);
    spr[base+49]=2;spr[base+51]=1;std::copy(walk.begin(),walk.end(),tagBytes.begin());
    save(root/"sheet.spr",spr);save(root/"sheet.tag",tagBytes);
    auto store=std::get<assets::AssetStore>(assets::AssetStore::create(root));
    auto load=[&] {
        auto s=std::get<std::unique_ptr<assets::AssetFile>>(store.open("SHEET.SPR"));
        auto t=std::get<std::unique_ptr<assets::AssetFile>>(store.open("SHEET.TAG"));
        return sprites::SpriteTable::load(*s,*t);
    };
    auto loaded=load();require(loaded->find(walk,0)==0,"Companion file load differs");
    for(std::size_t length=0;length<spr.size();length+=37) {
        save(root/"sheet.spr",{spr.begin(),spr.begin()+length});rejects(load);
    }
    save(root/"sheet.spr",spr);
    for(unsigned length=0;length<12;++length) {save(root/"sheet.tag",{tagBytes.begin(),tagBytes.begin()+length});rejects(load);}
    save(root/"sheet.tag",tagBytes);tagBytes[8]=1;save(root/"sheet.tag",tagBytes);rejects(load);
    std::filesystem::remove(root/"sheet.spr");std::filesystem::remove(root/"sheet.tag");
    require(loaded->sprite()->frames[0].opaqueMask==std::vector<std::uint8_t>({1,1}),"Loaded source borrowed a file");
    render::GlBlitter renderer;const auto baseline=renderer.stats();unsigned pixelCases=0;
    for(bool direct:{false,true})for(bool overridePalette:{false,true}) {
        if(direct && overridePalette)continue;
        auto source=fixture(direct);auto owner=std::make_shared<const sprites::SpriteTable>(source,tags(source));
        for(int x:{-3,0,3,10,std::numeric_limits<int>::min(),std::numeric_limits<int>::max()})for(int y:{-2,1,5}) {
            sprites::SpriteSession pen(owner,walk,0,4,1,sprites::Playback::loop);
            render::SpriteColourTable colours{};colours[0]=0x1f;colours[1]=0xffff;colours[2]=0xf81f;
            auto plan=pen.plan(x,y,overridePalette?std::make_optional(colours):std::nullopt);
            plan.placements.push_back({1,4,2,{}});plan.placements.push_back({7,0,0,{}});
            render::Image cpu{12,8,std::vector<std::uint32_t>(96,0x1234)},expected=cpu;const render::Rect clip{1,1,11,7};
            // Independent sparse fixture raster. Never calls preparation/draw.
            for(const auto& p:plan.placements)if(p.frame!=7) {
                const unsigned indices[]={1,0,2,0,1,0};const bool mask[]={true,false,true,true,true,false};
                for(int row=0;row<2;++row)for(int col=0;col<3;++col) {
                    const auto dx=std::int64_t(p.x)-1+col,dy=std::int64_t(p.y)+1+row;
                    if(mask[row*3+col] && dx>=1 && dx<11 && dy>=1 && dy<7) {
                        const auto index=indices[row*3+col];const auto value=p.colours?(*p.colours)[index]:index==1?0xf800:index==2?0x7e0:0;
                        expected.pixels[std::size_t(dy)*12+std::size_t(dx)]=value;
                    }
                }
            }
            render::drawSpritePixels(cpu,plan,clip);require(cpu.pixels==expected.pixels,"CPU sprite batch pixels differ");
            const auto dest=renderer.create({12,8,std::vector<std::uint32_t>(96,0x1234)},render::spriteFormat);
            auto stats=renderer.stats();render::drawSpriteBatch(renderer,dest,plan,clip);auto after=renderer.stats();
            require(after.nativeReadbacks==stats.nativeReadbacks && after.rgbaReadbacks==stats.rgbaReadbacks,"GPU batch read back pixels");
            require(after.surfaces==stats.surfaces,"Temporary GPU sprite planes leaked");
            require(renderer.read(dest).pixels==expected.pixels,"GPU sprite batch pixels differ");
            auto bad=plan;bad.placements.push_back({9999,0,0,{}});const auto prior=renderer.read(dest).pixels;
            rejects([&]{render::drawSpriteBatch(renderer,dest,bad,clip);});require(renderer.read(dest).pixels==prior,"Late GPU refusal changed destination");
            const auto unchanged=cpu.pixels;rejects([&]{render::drawSpritePixels(cpu,bad,clip);});require(cpu.pixels==unchanged,"Late CPU refusal changed destination");
            renderer.destroy(dest);require(renderer.stats().surfaces==baseline.surfaces,"GPU batch lifetime leak");++pixelCases;
        }
    }
    for(auto mode:{sprites::Playback::loop,sprites::Playback::once}) {
        sprites::SpriteSession emptyFrame(table,idle,3,1,1000000000,mode);
        emptyFrame.advance(0);require(emptyFrame.frame()==7 && !emptyFrame.finished(),"Single-frame initial phase differs");
        emptyFrame.advance(std::numeric_limits<std::uint64_t>::max());
        require(emptyFrame.frame()==7,"Single-frame maximal tick advance differs");
        require(emptyFrame.finished()==(mode==sprites::Playback::once),"Single-frame finished state differs");
    }
    session.restore(before);require(session.checkpoint().table==table,"Checkpoint did not restore pre-reload ownership");
    {render::Image emptyImage{4,4,std::vector<std::uint32_t>(16,3)};const auto unchanged=emptyImage.pixels;
     render::SpriteDrawPlan emptyPlan{table->sprite(),{}};render::drawSpritePixels(emptyImage,emptyPlan,{0,0,4,4});
     require(emptyImage.pixels==unchanged,"Empty batch changed CPU image");
     const auto destination=renderer.create(emptyImage,render::spriteFormat);const auto stats=renderer.stats();
     render::drawSpriteBatch(renderer,destination,emptyPlan,{0,0,4,4});
     require(renderer.stats().uploads==stats.uploads && renderer.read(destination).pixels==unchanged,"Empty batch changed GPU image");
     renderer.destroy(destination);}
    render::Image image{4,4,std::vector<std::uint32_t>(16,0)};auto plan=retained;
    for(auto clip:std::vector<render::Rect>{{-1,0,4,4},{0,0,5,4},{0,0,0,4},{0,0,4,5}})rejects([&]{render::drawSpritePixels(image,plan,clip);});
    auto null=plan;null.source.reset();rejects([&]{render::drawSpritePixels(image,null,{0,0,4,4});});
    auto oversized=plan;oversized.placements.resize(4097);rejects([&]{render::drawSpritePixels(image,oversized,{0,0,4,4});});
    auto huge=fixture();huge.frames[0].width=2048;huge.frames[0].height=2048;
    auto budget=plan;budget.source=std::make_shared<const assets::Sprite>(huge);budget.placements={{0,0,0,{}}};
    rejects([&]{render::drawSpritePixels(image,budget,{0,0,4,4});});
    image.pixels[0]=65536;rejects([&]{render::drawSpritePixels(image,plan,{0,0,4,4});});
    require(renderer.stats().surfaces==baseline.surfaces,"Final sprite surfaces leaked");
    std::cout<<"{\"success\":true,\"checks\":"<<checks<<",\"refusals\":"<<refusals<<",\"cpu_gpu_cases\":"<<pixelCases<<",\"timing_cases\":"<<timingCases<<",\"scopes\":10,\"surface_leaks\":0}\n";
}catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
