#include "../renderer/gpu_production_queue.hpp"
#include <QGuiApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
using namespace mnm::render;
static void require(bool v,const char* why){if(!v)throw std::runtime_error(why);}
template<class F> void until(F predicate){const auto end=std::chrono::steady_clock::now()+std::chrono::seconds(10);while(!predicate()){require(std::chrono::steady_clock::now()<end,"Production fixture timeout");std::this_thread::sleep_for(std::chrono::milliseconds(1));}}
int main(int argc,char** argv)try{
    QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);
    QGuiApplication app(argc,argv);QOpenGLContext root;root.setFormat(format);require(root.create(),"Share context");
    QOffscreenSurface output,producer;output.setFormat(root.format());producer.setFormat(root.format());output.create();producer.create();require(output.isValid()&&producer.isValid(),"Platform targets");
    GlBlitter client(&root,&output);const PixelFormat rgb{16,{0xf800,0x7e0,31}};
    auto target=client.allocate(16,16,rgb);SurfaceId source=0;GlBlitter* owned=nullptr;std::atomic<unsigned> cleaned=0;
    std::atomic<bool> entered=false,release=false,thirdStarted=false;
    GpuProductionQueue queue(&root,&producer,[&]{if(source)owned->destroy(source);++cleaned;});
    const auto render=[&](GlBlitter& renderer,unsigned colour){owned=&renderer;
        if(!source)source=renderer.create({16,16,std::vector<std::uint32_t>(256,colour)},rgb);
        else renderer.update(source,0,0,{16,16,std::vector<std::uint32_t>(256,colour)});
        require(!renderer.stats().nativeReadbacks&&!renderer.stats().rgbaReadbacks,"Producer read pixels");return renderer.presentGpu(source);
    };
    require(queue.submit(1,[&](auto& renderer){return render(renderer,0xf800);}),"First submission");
    GpuProductionQueue::Completion first,second,third;until([&]{return queue.take(first);});
    const auto check=[&](const GpuFrame& frame,unsigned colour){client.importGpu565(target,frame);const auto image=client.read(target);require(std::all_of(image.pixels.begin(),image.pixels.end(),[colour](auto p){return p==colour;}),"Immutable World pixels differ");};
    check(first.frame,0xf800);
    require(queue.submit(2,[&](auto& renderer){entered=true;const auto limit=std::chrono::steady_clock::now()+std::chrono::seconds(5);while(!release.load()&&std::chrono::steady_clock::now()<limit)std::this_thread::sleep_for(std::chrono::milliseconds(1));return render(renderer,0x001f);}),"Delayed submission");
    until([&]{return entered.load();});
    bool gapRefused=false;try{queue.submit(4,[&](auto& renderer){return render(renderer,0);});}catch(const std::invalid_argument&){gapRefused=true;}require(gapRefused,"Skipped source admitted");
    require(queue.submit(3,[&](auto& renderer){thirdStarted=true;return render(renderer,0x07e0);}),"Third bounded submission");
    require(!queue.submit(4,[&](auto& renderer){return render(renderer,0);}),"Third outstanding job admitted");
    unsigned presentations=0;const auto deadline=std::chrono::steady_clock::now()+std::chrono::milliseconds(100);
    while(std::chrono::steady_clock::now()<deadline){client.importGpu565(target,first.frame);require(client.presentGpu(target).valid(),"Independent presentation failed");++presentations;std::this_thread::sleep_for(std::chrono::milliseconds(5));}
    require(presentations>=8,"World production blocked presentation owner");release=true;until([&]{return queue.take(second);});check(second.frame,0x001f);check(first.frame,0xf800);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));require(!thirdStarted.load(),"Leased immutable slot overwritten");first.frame={};
    until([&]{return queue.take(third);});require(third.sequence==3&&thirdStarted.load(),"Source completion order");check(third.frame,0x07e0);check(second.frame,0x001f);
    second.frame={};third.frame={};queue.join();auto stats=queue.stats();require(stats.stopped&&stats.submitted==3&&stats.produced==3&&stats.taken==3&&stats.highWater==2&&stats.allocations==2&&cleaned==1,"Bounded owner teardown/counts");
    {
        GpuProductionQueue failed(&root,&producer);require(failed.submit(1,[](GlBlitter&)->GpuFrame{throw std::runtime_error("authored production failure");}),"Failure submission");
        until([&]{return !failed.stats().error.empty();});bool refused=false;try{GpuProductionQueue::Completion result;failed.take(result);}catch(const std::runtime_error&){refused=true;}require(refused,"Failed production remained usable");failed.join();require(failed.stats().stopped,"Failed owner did not retire");
    }
    client.destroy(target);require(!client.stats().surfaces,"Client surfaces leaked");
    std::cout<<"{\"success\":true,\"presentations_during_blocked_production\":"<<presentations<<",\"source_updates\":3,\"immutable_slots\":2,\"outstanding_bound\":2,\"leased_slot_preserved\":true,\"source_gap_refused\":true,\"terminal_failure_refused\":true,\"joined_owner_cleanup\":true,\"ordinary_readbacks\":0,\"renderer\":\""<<client.driver().renderer<<"\"}\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
