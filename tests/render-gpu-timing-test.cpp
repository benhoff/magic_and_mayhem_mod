#include "../renderer/blit.hpp"
#include <QGuiApplication>
#include <QElapsedTimer>
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <stdexcept>
using namespace mnm::render;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
template<class F> void refuses(F f){bool failed=false;try{f();}catch(const std::runtime_error&){failed=true;}require(failed,"Missing diagnostic refusal");}
int main(int argc,char** argv) try {
    QGuiApplication app(argc,argv);GlBlitter renderer;
    const Image image{32,32,std::vector<std::uint32_t>(1024,0x1234)};
    const auto plain=renderer.create(image,{16,{0xf800,0x07e0,0x001f}});
    const auto timed=renderer.create(image,{16,{0xf800,0x07e0,0x001f}});
    require(renderer.stats().textureUploadCalls==2 && renderer.stats().textureUploadBytes==8192,"Native upload accounting");
    const auto counts=renderer.stats();
    unsigned executed=0;const auto before=renderer.stats().nativeReadbacks;
    for(unsigned i=1;i<=64;++i)require(renderer.profileBatch(i,[&]{++executed;renderer.additiveRect(timed,{0,0,32,32},{1,2,3});}),"Query ring filled early");
    require(renderer.pendingGpuTimings()==64,"Query ring accounting");
    require(!renderer.profileBatch(65,[&]{++executed;renderer.additiveRect(timed,{0,0,32,32},{1,2,3});}),"Full ring failed to skip timing");
    refuses([&]{renderer.profileBatch(1,[&]{++executed;});});
    refuses([&]{renderer.profileBatch(0,[&]{++executed;});});
    require(executed==65 && renderer.stats().nativeReadbacks==before,"Overflow changed execution/readback");
    refuses([&]{renderer.profileBatch(66,[&]{renderer.profileBatch(67,[]{});});});
    require(renderer.pendingGpuTimings()==64,"Dropped nested profile changed query ring");
    unsigned untimed=0;require(!renderer.profileBatch(68,[&]{++untimed;}),"Overflow stopped executing after nested refusal");
    require(untimed==1,"Dropped callback did not execute exactly once");
    QElapsedTimer wait;wait.start();std::uint64_t next=1;
    while(renderer.pendingGpuTimings()){
        for(const auto& t:renderer.pollGpuTimings()){require(t.sequence==next++ && t.timelineMs>=0,"Timestamp order/elapsed");}
        require(wait.elapsed()<10000,"GPU query deadline");QThread::msleep(1);
    }
    require(next==65,"Lost query result");
    refuses([&]{renderer.profileBatch(100,[&]{throw std::runtime_error("Injected operation failure");});});
    require(!renderer.pendingGpuTimings(),"Exception leaked query objects");
    refuses([&]{renderer.profileBatch(101,[&]{renderer.profileBatch(102,[]{});});});
    require(!renderer.pendingGpuTimings(),"Nested profile leaked query objects");
    for(unsigned i=0;i<65;++i)renderer.additiveRect(plain,{0,0,32,32},{1,2,3});
    require(renderer.stats().drawCalls-counts.drawCalls==130 && renderer.stats().drawInstances-counts.drawInstances==130,"Submitted additive draws differ");
    require(renderer.stats().textureUploadBytes==counts.textureUploadBytes,"Destination-dependent draws uploaded CPU pixels");
    // Explicit fixture readbacks occur after all measured/delivery operations.
    require(renderer.read(plain).pixels==renderer.read(timed).pixels,"Timing changed pixels");
    renderer.destroy(plain);renderer.destroy(timed);
    require(!renderer.stats().surfaces,"Surface cleanup");
    // Destruction with queued queries must also clean up without polling/waiting.
    {GlBlitter pending;require(pending.profileBatch(1,[]{}),"Pending destruction setup");}
    std::cout<<QJsonDocument(QJsonObject{{"success",true},{"timestamp_pairs",64},{"overflow_executions",1},
        {"ordinary_readbacks",0},{"diagnostic_readbacks",2},{"words_compared",1024},{"refusals",5},
        {"remaining_queries",0},{"remaining_surfaces",0},{"renderer",QString::fromStdString(renderer.driver().renderer)}}).toJson(QJsonDocument::Compact).constData()<<'\n';
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
