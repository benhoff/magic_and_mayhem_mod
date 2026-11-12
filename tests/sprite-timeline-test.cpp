#include "sprite_timeline.hpp"
#include <QGuiApplication>
#include <algorithm>
#include <iostream>
#include <limits>
using namespace mnm;
using Mode=sprites::TimelineMode;
namespace {
unsigned checks=0,refusals=0;
void require(bool value,const char* message) {++checks;if(!value)throw std::runtime_error(message);}
template<class F> void rejects(F f) {bool caught=false;try{f();}catch(const std::exception&){caught=true;}require(caught,"Expected timeline refusal");++refusals;}
const sprites::Name a={'A',0,255,0,1,2,3,4},b={'B',0,255,0,1,2,3,4},c={'C',0,255,0,1,2,3,4};
const std::uint16_t words[]={0,0xf800,0x7e0,0x1f,0xffff};
const std::size_t selected[]={4,1,0,3};
const unsigned durations[]={2,3,1,4};
const std::int32_t labels[]={7,-9,2147483647,0};
assets::Sprite fixture(bool direct=false,bool empty=false) {
    assets::Sprite s;s.storage=direct?assets::SpriteStorage::rgb565:assets::SpriteStorage::indexed8;
    if(!direct){s.palettes.resize(1);s.palettes[0][1]={255,0,0};s.palettes[0][2]={0,255,0};s.palettes[0][3]={0,0,255};s.palettes[0][4]={255,255,255};}
    for(unsigned i=0;i<5;++i){
        assets::SpriteFrame f;f.name=i==1?b:i==3?c:a;f.originX=int(i)-2;f.originY=1-int(i)%3;
        if(!direct)f.paletteIndex=0;
        if(empty&&i==2){if(direct)f.pixels=std::vector<std::uint16_t>{};}
        else {
            f.width=f.height=2;f.opaqueMask={1,0,1,1};
            if(direct)f.pixels=std::vector<std::uint16_t>{words[i%4+1],0,0,words[(i+1)%4+1]};
            else f.pixels=std::vector<std::uint8_t>{std::uint8_t(i%4+1),0,0,std::uint8_t((i+1)%4+1)};
        }
        s.frames.push_back(std::move(f));
    }
    return s;
}
std::shared_ptr<const sprites::SpriteTable> table(assets::Sprite s) {
    assets::TagAsset tag;std::map<sprites::Name,unsigned> seen;
    for(const auto& f:s.frames)tag.entries.push_back({f.name,seen[f.name]++});
    return std::make_shared<const sprites::SpriteTable>(std::move(s),tag);
}
std::vector<sprites::TimelineEntry> entries(unsigned count=4) {
    std::vector<sprites::TimelineEntry> v={{a,2,2,7},{b,0,3,-9},{a,0,1,2147483647},{c,0,4,0}};
    v.resize(count);return v;
}
bool same(const sprites::TimelineCheckpoint& x,const sprites::TimelineCheckpoint& y) {
    return x.clip==y.clip&&x.mode==y.mode&&x.reverse==y.reverse&&x.paused==y.paused&&x.phase==y.phase&&
           x.numerator==y.numerator&&x.denominator==y.denominator&&x.fraction==y.fraction;
}
std::vector<unsigned> schedule(unsigned count,Mode mode,bool reverse) {
    // Explicit independent itineraries, including endpoint and singleton rules.
    if(count==1)return {0};
    if(count==2)return reverse?std::vector<unsigned>{1,0}:std::vector<unsigned>{0,1};
    if(mode==Mode::pingPong)return reverse?std::vector<unsigned>{3,2,1,0,1,2}:std::vector<unsigned>{0,1,2,3,2,1};
    return reverse?std::vector<unsigned>{3,2,1,0}:std::vector<unsigned>{0,1,2,3};
}
struct Model {
    std::vector<unsigned> tape,order;std::vector<unsigned> starts;
    unsigned phase=0,fraction=0,n=1,d=1;bool once=false,paused=false;
    Model(unsigned count,Mode mode,bool reverse):order(schedule(count,mode,reverse)),once(mode==Mode::once) {
        for(auto entry:order){starts.push_back(tape.size());for(unsigned j=0;j<durations[entry];++j)tape.push_back(entry);}
    }
    std::vector<unsigned> advance(unsigned ticks) {
        std::vector<unsigned> events;if(paused)return events;
        for(unsigned t=0;t<ticks;++t){
            if(once&&phase==tape.size())break;
            fraction+=n;
            while(fraction>=d){
                fraction-=d;++phase;
                if(phase==tape.size()){
                    if(once){fraction=0;break;}
                    phase=0;events.push_back(tape[0]);
                }else if(std::find(starts.begin(),starts.end(),phase)!=starts.end())events.push_back(tape[phase]);
            }
        }
        return events;
    }
    unsigned slot() const {
        unsigned s=0;for(unsigned i=0;i<starts.size();++i)if(starts[i]<=phase)s=i;return s;
    }
    unsigned entry() const {return phase==tape.size()?order.back():tape[phase];}
};
// Binary rational accumulation, independent of the production quotient/remainder
// decomposition. Carries a rational pair while doubling one host tick64 times.
std::pair<std::uint64_t,unsigned> largeOracle(std::uint64_t ticks,unsigned n,unsigned d,
                                            std::uint64_t duration,unsigned initial) {
    std::uint64_t whole=0,step=n/d;unsigned fraction=initial,rest=n%d;
    for(unsigned bit=0;bit<64;++bit){
        if(ticks&1){const auto sum=fraction+rest;whole=(whole+step+sum/d)%duration;fraction=sum%d;}
        const auto sum=rest*2;step=(step*2+sum/d)%duration;rest=sum%d;ticks>>=1;
    }
    return {whole,fraction};
}
void modelEqual(const sprites::SpriteTimeline& timeline,const Model& model) {
    const auto s=timeline.checkpoint();const auto p=timeline.position();const auto slot=model.slot();
    require(s.phase==model.phase&&s.fraction==model.fraction,"Independent rational clock differs");
    require(p.entry==model.entry()&&p.scheduleEntry==slot,"Independent itinerary differs");
    require(p.localTick==model.phase-model.starts[slot],"Independent local phase differs");
    require(p.finished==(model.once&&model.phase==model.tape.size()),"Independent terminal state differs");
    require(timeline.frame()==selected[model.entry()],"Named frame binding differs");
}
}
int main(int argc,char** argv)try {
    QGuiApplication app(argc,argv);auto resource=table(fixture());
    auto clip=std::make_shared<const sprites::TimelineClip>(resource,entries());
    unsigned timingCases=0,pixelCases=0,largeCases=0;
    for(unsigned count:{1u,2u,4u})for(auto mode:{Mode::loop,Mode::once,Mode::pingPong})for(bool reverse:{false,true})
        for(auto rate:std::vector<std::pair<unsigned,unsigned>>{{1,1},{2,3},{5,2},{7,7}}){
            auto local=std::make_shared<const sprites::TimelineClip>(resource,entries(count));
            sprites::SpriteTimeline timeline(local,mode,reverse);Model model(count,mode,reverse);
            timeline.rate(rate.first,rate.second);model.n=rate.first;model.d=rate.second;
            require(timeline.duration()==model.tape.size(),"Native schedule duration differs");
            for(unsigned i=0;i<120;++i){
                if(i%17==0){timeline.pause(!timeline.checkpoint().paused);model.paused=!model.paused;}
                if(i%19==0){const auto tick=(i*13+3)%37;timeline.seek(tick);model.phase=model.once?std::min<unsigned>(tick,model.tape.size()):tick%model.tape.size();model.fraction=0;}
                if(i%23==0){const auto slot=(i/23)%model.order.size();const auto tick=i%durations[model.order[slot]];timeline.seekEntry(slot,tick);model.phase=model.starts[slot]+tick;model.fraction=0;}
                const auto ticks=(i*11)%21;const auto expected=model.advance(ticks);const auto events=timeline.advance(ticks,true);
                require(events.size()==expected.size(),"Boundary event count differs");
                for(unsigned j=0;j<events.size();++j)require(events[j].entry==expected[j]&&events[j].value==labels[expected[j]],"Boundary event order/value differs");
                modelEqual(timeline,model);
            }
            const auto checkpoint=timeline.checkpoint();timeline.seek(0);timeline.restore(checkpoint);
            require(same(checkpoint,timeline.checkpoint()),"Timeline checkpoint round trip differs");++timingCases;
        }
    // Maximal clips, durations, host deltas, rational remainders and reverse
    // schedules exercise every modular product at its declared boundary.
    auto maximum=entries(1);maximum[0].ticks=1000000000;maximum.resize(4096,maximum[0]);
    auto largeClip=std::make_shared<const sprites::TimelineClip>(resource,maximum);
    for(auto mode:{Mode::loop,Mode::once,Mode::pingPong})for(bool reverse:{false,true})
        for(auto rate:std::vector<std::pair<unsigned,unsigned>>{{1000000,1},{1,1000000},{999983,999979}}){
            sprites::SpriteTimeline t(largeClip,mode,reverse);t.rate(rate.first,rate.second);t.seek(t.duration()-3);
            auto s=t.checkpoint();s.fraction=rate.second-1;t.restore(s);
            const auto delta=std::numeric_limits<std::uint64_t>::max();const auto result=largeOracle(delta,rate.first,rate.second,t.duration(),s.fraction);
            t.advance(delta);const auto got=t.checkpoint();
            require(got.phase==(mode==Mode::once?t.duration():(s.phase+result.first)%t.duration()),"Maximal rational phase overflowed");
            require(got.fraction==(mode==Mode::once?0:result.second),"Maximal rational carry differs");++largeCases;
        }
    sprites::SpriteTimeline t(clip,Mode::loop);t.rate(2,3);t.advance(2);
    const auto unchanged=t.checkpoint();
    for(unsigned failure=0;failure<11;++failure){
        auto bad=unchanged;
        if(failure==0)bad.clip.reset();
        if(failure==1)bad.mode=static_cast<Mode>(99);
        if(failure==2)bad.phase=t.duration();
        if(failure==3)bad.phase=std::numeric_limits<std::uint64_t>::max();
        if(failure==4)bad.numerator=0;
        if(failure==5)bad.denominator=0;
        if(failure==6)bad.numerator=1000001;
        if(failure==7)bad.denominator=1000001;
        if(failure==8)bad.fraction=bad.denominator;
        if(failure==9){bad.mode=Mode::once;bad.phase=t.duration()+1;}
        if(failure==10){bad.mode=Mode::once;bad.phase=t.duration();bad.fraction=1;}
        rejects([&]{t.restore(bad);});require(same(unchanged,t.checkpoint()),"Rejected checkpoint changed timeline");
    }
    rejects([&]{t.advance(std::numeric_limits<std::uint64_t>::max(),true);});
    rejects([&]{t.advance(100,true,0);});rejects([&]{t.advance(0,false,4097);});
    rejects([&]{t.rate(0,1);});rejects([&]{t.rate(1,0);});rejects([&]{t.rate(1000001,1);});
    rejects([&]{t.seekEntry(4,0);});rejects([&]{t.seekEntry(0,2);});
    require(same(unchanged,t.checkpoint()),"Failed mutation changed timeline or fraction");
    for(unsigned failure=0;failure<6;++failure){
        auto v=entries();
        if(failure==0)v.clear();
        if(failure==1)v.resize(4097,v[0]);
        if(failure==2)v.back().ticks=0;
        if(failure==3)v.back().ticks=1000000001;
        if(failure==4)v.back().name[7]^=128;
        if(failure==5)v.back().occurrence=0xffffffffu;
        rejects([&]{sprites::TimelineClip invalid(resource,v);});
    }
    rejects([&]{sprites::TimelineClip invalid({},entries());});
    rejects([&]{sprites::SpriteTimeline invalid({},Mode::loop);});
    t.pause(true);const auto paused=t.checkpoint();require(t.advance(std::numeric_limits<std::uint64_t>::max(),true,0).empty()&&same(paused,t.checkpoint()),"Pause accumulated wall time");
    t.seek(5);require(t.checkpoint().paused&&t.checkpoint().fraction==0,"Paused seek did not reset fraction");
    t.rate(5,2);require(t.checkpoint().paused&&t.checkpoint().fraction==0,"Rate edit did not reset fraction");
    t.pause(false);t.advance(1);require(t.checkpoint().fraction==1,"Rational remainder missing");
    auto old=t.checkpoint();const auto oldPlan=t.plan(3,2);auto changed=fixture();std::reverse(changed.frames.begin(),changed.frames.end());
    auto replacement=table(changed);t.reload(replacement);
    auto rebound=t.checkpoint();rebound.clip=old.clip;require(same(old,rebound),"Reload reset time/rate/pause");
    require(t.plan(3,2).source!=oldPlan.source,"Reload reused retired source");
    require(t.frame()==1&&oldPlan.placements[0].frame==3,"Reload failed exact-name rebinding");
    auto missing=fixture();missing.frames.pop_back();const auto good=t.checkpoint();
    rejects([&]{t.reload(table(missing));});rejects([&]{t.reload({});});require(same(good,t.checkpoint()),"Rejected reload changed timeline");
    t.restore(old);require(t.plan(3,2).source==oldPlan.source,"Retired checkpoint lost its resource");
    sprites::SpriteTimeline once(clip,Mode::once);once.seek(once.duration()-1);
    require(once.advance(1,true,0).empty()&&once.position().finished,"Terminal completion invented a frame event");
    const auto done=once.checkpoint();require(once.advance(100,true).empty()&&same(done,once.checkpoint()),"Completed one-shot kept accumulating time");
    sprites::SpriteTimeline singleton(std::make_shared<const sprites::TimelineClip>(resource,entries(1)),Mode::pingPong);
    const auto exact=singleton.advance(8192,true,4096);require(exact.size()==4096,"Exact event budget refused");
    const auto safe=singleton.checkpoint();rejects([&]{singleton.advance(8194,true,4096);});require(same(safe,singleton.checkpoint()),"Event overflow published a prefix");
    render::GlBlitter renderer;const auto baseline=renderer.stats();render::SpriteColourTable colours{};
    for(unsigned i=0;i<256;++i)colours[i]=std::uint16_t((i*1297)^0x55aa);
    for(unsigned profile=0;profile<3;++profile)for(auto mode:{Mode::loop,Mode::once,Mode::pingPong})for(bool reverse:{false,true})
        for(unsigned elapsed:{0u,2u,5u,9u,17u})for(auto anchor:std::vector<std::pair<int,int>>{{0,0},{3,2},{-2,3},{INT32_MIN,INT32_MAX}}){
            Model model(4,mode,reverse);model.advance(elapsed);const auto frame=selected[model.entry()];render::SpriteDrawPlan plan;
            {auto ownedTable=table(fixture(profile==1));auto ownedClip=std::make_shared<const sprites::TimelineClip>(ownedTable,entries());
             sprites::SpriteTimeline player(ownedClip,mode,reverse);player.advance(elapsed);
             plan=player.plan(anchor.first,anchor.second,profile==2?std::optional<render::SpriteColourTable>(colours):std::nullopt);}
            render::Image image{7,6,std::vector<std::uint32_t>(42,0x4208)},expected=image;const render::Rect viewport{1,1,6,5};
            for(int sy=0;sy<2;++sy)for(int sx=0;sx<2;++sx)if(!(sy==0&&sx==1)){
                const auto x=std::int64_t(anchor.first)-(int(frame)-2)+sx,y=std::int64_t(anchor.second)-(1-int(frame)%3)+sy;
                const auto index=sy==1&&sx==0?0:sy==0?frame%4+1:(frame+1)%4+1;
                if(x>=1&&x<6&&y>=1&&y<5)expected.pixels[std::size_t(y)*7+std::size_t(x)]=profile==2?colours[index]:words[index];
            }
            auto cpu=image;render::drawSpritePixels(cpu,plan,viewport);require(cpu.pixels==expected.pixels,"Timeline CPU pixels differ from independent mask/origin oracle");
            const auto destination=renderer.create(image,render::spriteFormat);const auto before=renderer.stats();render::drawSpriteBatch(renderer,destination,plan,viewport);const auto after=renderer.stats();
            require(after.surfaces==before.surfaces&&after.nativeReadbacks==before.nativeReadbacks&&after.rgbaReadbacks==before.rgbaReadbacks,"Timeline draw retained uploads or read back its source");
            require(renderer.read(destination).pixels==expected.pixels,"Timeline GPU pixels differ from independent oracle");renderer.destroy(destination);++pixelCases;
        }
    auto direct=std::make_shared<const sprites::TimelineClip>(table(fixture(true)),entries());sprites::SpriteTimeline directPlayer(direct,Mode::loop);
    rejects([&]{directPlayer.plan(0,0,colours);});
    auto emptyClip=std::make_shared<const sprites::TimelineClip>(table(fixture(false,true)),std::vector<sprites::TimelineEntry>{{a,1,1,0}});
    sprites::SpriteTimeline empty(emptyClip,Mode::loop);render::Image blank{2,2,std::vector<std::uint32_t>(4,17)};auto cpu=blank;
    render::drawSpritePixels(cpu,empty.plan(0,0),{0,0,2,2});require(cpu.pixels==blank.pixels,"Empty timeline frame changed CPU pixels");
    auto surface=renderer.create(blank,render::spriteFormat);render::drawSpriteBatch(renderer,surface,empty.plan(0,0),{0,0,2,2});require(renderer.read(surface).pixels==blank.pixels,"Empty timeline frame changed GPU pixels");renderer.destroy(surface);
    require(renderer.stats().surfaces==baseline.surfaces,"Timeline delivery leaked GPU surfaces");
    std::cout<<"{\"success\":true,\"checks\":"<<checks<<",\"refusals\":"<<refusals<<",\"timing_cases\":"<<timingCases<<",\"large_clock_cases\":"<<largeCases<<",\"cpu_gpu_cases\":"<<pixelCases<<",\"scopes\":10,\"surface_leaks\":0}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
