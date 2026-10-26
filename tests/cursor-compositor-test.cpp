#include "cursor_compositor.hpp"
#include <QGuiApplication>
#include <atomic>
#include <climits>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>
using namespace mnm::render;
using C=CursorCompositor;
static unsigned checks=0,pixelCases=0;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);++checks;}
template<class F>static void refuses(F f){bool rejected=false;try{f();}catch(const std::exception&){rejected=true;}require(rejected,"Expected refusal");}
static Image image(int w,int h,unsigned seed,bool keys=false){Image i{w,h,{}};for(int y=0;y<h;++y)for(int x=0;x<w;++x)i.pixels.push_back(keys&&(x+y)%3==0?31:(seed+x*337+y*971)&65535);return i;}
// Independent full-frame oracle: rebuild from the supplied untouched underlay,
// directly select atlas samples, and compare every pixel after each operation.
// It neither copies saved rectangles nor calls the renderer planning functions.
static Image oracle(Image background,const Image& atlas,const C::Frame& f){
    if(f.visible)for(int y=0;y<background.height;++y)for(int x=0;x<background.width;++x){
        const auto sx=std::int64_t(x)-f.x+f.atlas.left,sy=std::int64_t(y)-f.y+f.atlas.top;
        if(sx>=f.atlas.left && sx<f.atlas.right && sy>=f.atlas.top && sy<f.atlas.bottom){const auto pixel=atlas.pixels[std::size_t(sy)*atlas.width+std::size_t(sx)];if(pixel!=31)background.pixels[std::size_t(y)*background.width+x]=pixel;}
    }
    return background;
}
static void pixels(C& c,C::Token token,const Image& expected){const auto got=c.snapshot(token);require(got.width==expected.width&&got.height==expected.height&&got.pixels==expected.pixels,"Complete cursor pixels differ");++pixelCases;}
int main(int argc,char** argv){QGuiApplication app(argc,argv);try{
    C::Publisher survivor;unsigned stressAccepted=0,stressDrained=0;
    {
        C c;auto p=c.publisher();survivor=p;
        const auto background=image(13,9,0x5500),atlas=image(12,8,0x1100,true);
        auto b=background,a=atlas;const auto token=c.create(b,a,5,3);
        // Inputs may die or mutate immediately after allocation.
        b.pixels.assign(b.pixels.size(),0);a.pixels.assign(a.pixels.size(),0);
        pixels(c,token,background);
        const int xs[]={INT_MIN,-5,-3,-1,0,2,8,12,13,INT_MAX};
        const int ys[]={INT_MIN,-3,-1,0,3,7,8,9,INT_MAX};
        for(int y:ys)for(int x:xs)for(unsigned frame=0;frame<2;++frame){
            C::Frame f{{int(frame*5),int(frame*3),int(frame*5+5),int(frame*3+3)},x,y,true};
            require(p.publish(token,f)==C::Publication::accepted,"Frame publication");require(c.drain()==1,"Single frame drain");pixels(c,token,oracle(background,atlas,f));
        }
        C::Frame f{{5,3,10,6},2,3,true};p.publish(token,f);c.drain();
        const auto refreshed=image(13,9,0x7700);c.background(token,refreshed);pixels(c,token,oracle(refreshed,atlas,f));
        C::Frame hide{{},0,0,false};p.publish(token,hide);c.drain();pixels(c,token,refreshed);
        // No accepted job retains a caller-owned Rect or Frame reference.
        C::Frame original{{0,0,5,3},1,1,true},mutableFrame=original;
        require(p.publish(token,mutableFrame)==C::Publication::accepted,"Value publication");mutableFrame=hide;c.drain();pixels(c,token,oracle(refreshed,atlas,original));
        for(const C::Frame bad:std::vector<C::Frame>{{{-1,0,2,2},0,0,true},{{0,0,13,1},0,0,true},{{0,0,6,3},0,0,true},{{0,0,5,4},0,0,true},{{1,1,1,2},0,0,true}})require(p.publish(token,bad)==C::Publication::invalid,"Malformed frame admission");
        require(p.publish(token,original,3)==C::Publication::invalid,"Priority bounds");
        // Full rejection preserves all previously accepted work.
        for(unsigned i=0;i<128;++i){auto next=original;next.x=int(i%7);next.y=int(i%5);require(p.publish(token,next,i%3)==C::Publication::accepted,"Fill bounded queue");}
        require(p.publish(token,hide)==C::Publication::full,"Queue full refusal");require(c.drain()==128,"Full denominator");
        auto last=original;last.x=125%7;last.y=125%5;pixels(c,token,oracle(refreshed,atlas,last)); // Final priority2 publication.
        // Recreate before draining: jobs use their own immutable generation.
        require(p.publish(token,original)==C::Publication::accepted,"Old generation queued");c.retire(token);
        require(p.publish(token,hide)==C::Publication::retired,"Retired publication refusal");require(c.collect()==0&&c.surfaces()==3,"Queued old allocation retained");
        const auto nextBackground=image(13,9,0x9900),nextAtlas=image(12,8,0x3300,true);const auto next=c.create(nextBackground,nextAtlas,5,3);
        require(next!=token&&c.surfaces()==6,"Generation identities distinct");require(p.publish(next,original)==C::Publication::accepted,"New generation queued");require(c.drain()==2,"Drain old and new generation");
        pixels(c,token,oracle(refreshed,atlas,original));pixels(c,next,oracle(nextBackground,nextAtlas,original));require(c.collect()==1&&c.surfaces()==3,"Retired old allocation collected");refuses([&]{c.snapshot(token);});
        C other;const auto foreign=other.create(background,atlas,5,3);require(p.publish(foreign,original)==C::Publication::retired,"Foreign token refusal");other.shutdown();require(!other.surfaces(),"Foreign owner cleanup");
        refuses([&]{c.create(background,atlas,0,3);});
        auto brokenAtlas=atlas;brokenAtlas.pixels.pop_back();refuses([&]{c.create(background,brokenAtlas,5,3);});require(c.surfaces()==3,"Failed allocation rolls back canvas");refuses([&]{c.background(next,{1,1,{0}});});
        std::atomic<unsigned> wrongThread{0};std::thread misplaced([&]{try{c.snapshot(next);}catch(const std::runtime_error&){++wrongThread;}try{c.shutdown();}catch(const std::runtime_error&){++wrongThread;}});misplaced.join();require(wrongThread==2,"Wrong-thread GL work refused");
        // Real foreground/timer producers race retirement and owner draining.
        // Each publisher handles its own accepted/full/retired outcome. No
        // thread receives a graphics object or an owning surface reference.
        std::atomic<unsigned> ready{0},finished{0},accepted{0},unexpected{0};std::atomic<bool> start{false};std::vector<std::thread> producers;
        for(unsigned t=0;t<4;++t)producers.emplace_back([&,t]{if(p.publish(next,original)==C::Publication::accepted)++accepted;else ++unexpected;++ready;while(!start.load())std::this_thread::yield();for(unsigned i=1;i<600;++i){C::Frame frame=original;frame.x=int((i+t)%12)-2;frame.y=int((i+2*t)%8)-1;auto r=p.publish(next,frame,i%3);if(r==C::Publication::accepted)++accepted;else if(r!=C::Publication::full&&r!=C::Publication::retired&&r!=C::Publication::closed)++unexpected;}++finished;});
        while(ready!=4)std::this_thread::yield();
        start=true;
        unsigned rounds=0;while(finished!=4){stressDrained+=unsigned(c.drain());if(++rounds==3)c.retire(next);std::this_thread::yield();}
        for(auto& t:producers)t.join();
        stressDrained+=unsigned(c.drain());stressAccepted=accepted;require(!unexpected&&stressAccepted>=4&&stressDrained==stressAccepted,"Concurrent accepted/drained accounting");
        c.retire(next);require(c.collect()==1&&!c.surfaces(),"Concurrent generation retirement");
        // Every slot owns a pending generation until stop cancels it. stop is
        // safe from another thread, and all GL allocations retire on the owner.
        const auto stopped=c.create(background,atlas,5,3);for(unsigned i=0;i<128;++i)p.publish(stopped,original);
        std::thread stopper([p]{p.stop();p.stop();});stopper.join();require(c.drain()==0,"Stop cancels all queued work");require(p.publish(stopped,original)==C::Publication::closed,"Stop refuses publication");c.retire(stopped);require(c.collect()==1&&!c.surfaces(),"Cancelled generation cleanup");refuses([&]{c.create(background,atlas,5,3);});c.shutdown();c.shutdown();
    }
    // Physically destroy and recreate contexts with a visible clipped cursor,
    // saved underlay, retired generations and accepted work still outstanding.
    for(unsigned cycle=0;cycle<4;++cycle){
        C c;auto p=c.publisher();auto bg=image(13,9,0x2300+cycle),at=image(12,8,0x4300+cycle,true);
        auto token=c.create(bg,at,5,3);C::Frame visible{{0,0,5,3},-2,2,true},moved{{5,3,10,6},7,7,true},hide{{},0,0,false};
        refuses([&]{c.suspendGraphics();});p.publish(token,visible);c.drain();c.checkpointGraphics();
        p.publish(token,moved);c.retire(token);c.suspendGraphics();require(c.graphicsSuspended()&&!c.surfaces(),"Actual graphics context suspended");
        refuses([&]{c.drain();});refuses([&]{c.snapshot(token);});refuses([&]{c.background(token,bg);});refuses([&]{c.collect();});
        require(p.publish(token,hide)==C::Publication::retired,"Retirement preserved across context loss");
        c.recoverGraphics();require(!c.graphicsSuspended()&&c.surfaces()==3,"Graphics recovered with private resources");
        pixels(c,token,oracle(bg,at,visible));require(c.drain()==1,"Pending work survived context loss");pixels(c,token,oracle(bg,at,moved));
        // A new background exercises restoration from the recovered save-under.
        auto fresh=image(13,9,0x6500+cycle);c.background(token,fresh);pixels(c,token,oracle(fresh,at,moved));
        refuses([&]{c.suspendGraphics();});c.checkpointGraphics();c.suspendGraphics();c.recoverGraphics();pixels(c,token,oracle(fresh,at,moved));
        require(c.collect()==1&&!c.surfaces(),"Recovered generation releases all allocations");
        auto next=c.create(bg,at,5,3);c.checkpointGraphics();c.suspendGraphics();p.publish(next,hide);c.shutdown();require(p.publish(next,hide)==C::Publication::closed,"Shutdown while suspended closes publication");
    }
    // The endpoint survives actual backend/context destruction.
    std::atomic<unsigned> closed{0};std::thread late([survivor,&closed]{survivor.stop();if(survivor.publish(1,{})==C::Publication::closed)++closed;});late.join();require(closed==1,"Endpoint outlives GPU owner");
    // Race publication with actual owner teardown, then exercise the endpoint
    // after the complete compositor object has ceased to exist.
    for(unsigned repeat=0;repeat<8;++repeat){auto c=std::make_unique<C>();auto p=c->publisher();auto token=c->create(image(13,9,7),image(12,8,11,true),5,3);std::atomic<bool> go{false};std::thread worker([&]{while(!go)std::this_thread::yield();for(unsigned i=0;i<500;++i)p.publish(token,{{0,0,5,3},int(i%9),1,true});});go=true;c.reset();worker.join();require(p.publish(token,{})==C::Publication::closed,"Concurrent owner destruction");}
    std::cout<<"{\"success\":true,\"checks\":"<<checks<<",\"complete_pixel_cases\":"<<pixelCases<<",\"producer_attempts\":2400,\"accepted\":"<<stressAccepted<<",\"drained\":"<<stressDrained<<",\"teardown_races\":8,\"live_replacement\":false}\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
