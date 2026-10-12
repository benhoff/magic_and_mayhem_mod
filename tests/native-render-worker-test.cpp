#include "native_render_worker.hpp"
#include "gl_viewport.hpp"
#include "../protocols/include/mnm/render_command_ring.h"
#include <QApplication>
#include <QElapsedTimer>
#include <QTemporaryDir>
#include <QFile>
#include <QtEndian>
#include <thread>
#include <chrono>
#include <cstdio>
namespace {
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
void word(QByteArray& out,unsigned v){char b[4];qToLittleEndian(v,b);out.append(b,4);}
void record(QByteArray& out,unsigned op,unsigned& sequence,std::initializer_list<unsigned> fields,QByteArray pixels={}){
    word(out,op);word(out,++sequence);word(out,unsigned(fields.size()*4+pixels.size()));
    for(auto v:fields)word(out,v);
    out+=pixels;
}
void pixel(GlViewport& viewport,QColor expected){
    viewport.repaint();const auto image=viewport.grabFramebuffer();
    require(viewport.error().isEmpty()&&!image.isNull(),"Threaded viewport paint failed");
    require(image.pixelColor(image.width()/2,image.height()/2)==expected,"Immutable native frame changed or disagreed with pixel oracle");
}
}
int main(int argc,char** argv){
    QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);
    QApplication app(argc,argv);
    try{
        RenderDeadlineProbe probe;using Clock=std::chrono::steady_clock;
        const auto start=Clock::time_point(std::chrono::seconds(10));
        // A 1ms polling loop must not count fifteen misses for one frame tick.
        for(unsigned i=0;i<16;++i)probe.observe(start,start+std::chrono::milliseconds(i),RenderDeadlineProbe::UiPublication);
        require(probe.turns[RenderDeadlineProbe::UiPublication]==16&&probe.opportunities[RenderDeadlineProbe::UiPublication]<=2,"Deadline observations counted polls as frames");
        const auto before=probe.turns;probe.observe(start,start-std::chrono::milliseconds(1),RenderDeadlineProbe::Hidden);probe.observe({},start,RenderDeadlineProbe::Hidden);require(probe.turns==before,"Inactive/early deadline counted");
        probe.observe(start,start+std::chrono::milliseconds(40),RenderDeadlineProbe::IncompleteUi);probe.observe(start,start+std::chrono::milliseconds(41),RenderDeadlineProbe::PendingWorldImport);
        require(probe.opportunities[RenderDeadlineProbe::IncompleteUi]==1&&probe.opportunities[RenderDeadlineProbe::PendingWorldImport]==1,"Independent blocked reasons collapsed");
        using Ui=mnm::render::CommandDisplayState;
        require(nativeUiBlockReason(Ui{},4)==RenderDeadlineProbe::NoRecipe,"No recipe misclassified");
        require(nativeUiBlockReason(Ui{4,false,false,false},4)==RenderDeadlineProbe::FirstSeal,"First seal wait misclassified");
        require(nativeUiBlockReason(Ui{4,true,false,true},4)==RenderDeadlineProbe::StaticUi,"Static menu misclassified as World seal wait");
        require(nativeUiBlockReason(Ui{3,true,true,true},4)==RenderDeadlineProbe::GenerationMismatch,"Stale generation misclassified as render attempt");
        require(nativeUiBlockReason(Ui{4,true,true,false},4)==RenderDeadlineProbe::RenderAttempt,"Safe sealed partial UI misclassified");
        for(auto reason:{RenderDeadlineProbe::NoRecipe,RenderDeadlineProbe::FirstSeal,RenderDeadlineProbe::StaticUi,RenderDeadlineProbe::GenerationMismatch}){
            probe.observe(start,start+std::chrono::milliseconds(42),reason);
            require(probe.opportunities[reason]==1,"Detailed UI blockage collapsed");
        }
        RenderStageTiming timing;for(auto ms:{0.,1.,2.,4.,8.,16.667,33.334,100.,101.})timing.add(ms);require(timing.samples==9&&timing.bins.front()==2&&timing.bins.back()==1,"Elapsed histogram boundary error");
        // A continuous UI publication stream must not starve display deadlines.
        // World updates are kept ordered at20Hz; every source covers one display
        // opportunity and cannot receive a duplicate redraw in the same turn.
        unsigned frames=0,sources=0;std::uint64_t generation=1;int next=0;
        for(int now=0;now<1000000;now+=1000){
            const bool source=now%50000==0;if(source){++generation;++sources;}
            const bool eligible=nativeDisplayEligible(true,false,source,generation,generation);
            if(source || (now>=next && eligible)){++frames;next+=16667;}
            require(!nativeDisplayEligible(true,true,false,generation,generation),"Pending source admitted redraw");
            require(!nativeDisplayEligible(true,false,false,generation-1,generation),"Mixed World generation admitted redraw");
        }
        require(sources==20&&frames==60,"Continuous UI traffic starved deterministic display deadlines");
        require(!nativeDisplayEligible(false,false,false,1,1)&&!nativeDisplayEligible(true,false,false,0,1),"Hidden/incomplete first frame admitted redraw");
        GlViewport viewport;viewport.resize(320,240);viewport.show();app.processEvents();require(viewport.ready(),"Viewport initialization");
        {
            mnm::render::GlBlitter renderer(viewport.context());
            auto source=renderer.create({800,600,std::vector<std::uint32_t>(480000,0x7e0)},{16,{0xf800,0x7e0,31}});
            mnm::render::CommandConsumer consumer(renderer,[](auto){},{mnm::render::CommandDiagnostics::Skip,false,mnm::render::CommandStreamMode::Streaming,[&](auto,auto){return renderer.presentGpu(source);},true});
            unsigned seq=0;
            auto submit=[&](unsigned op,std::initializer_list<unsigned> fields){
                mnm::render::SurfaceCommand c;c.version=4;c.operation=op;c.sequence=++seq;std::copy(fields.begin(),fields.end(),c.words.begin());
                if(op==1){c.image={800,600,std::vector<std::uint32_t>(480000)};c.format={16,{0xf800,0x7e0,31}};}
                consumer.submit(c);
            };
            for(unsigned id:{1,2,3})submit(1,{id,800,600,16,0xf800,0x7e0,31});
            submit(19,{1,1,0,123,0});
            require(!consumer.displayState().sealed&&consumer.displayState().generation==1,"New WORLD retained sealed UI");
            submit(6,{3});require(consumer.displayState().sealed&&!consumer.displayState().worldOutput&&!consumer.result().firstWorldSeal.samples,"Static PRESENT counted as first World seal");
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            submit(3,{1,2,0,0,800,600,0,0,0,0});submit(6,{2});
            require(consumer.result().firstWorldSeal.samples==1&&consumer.result().firstWorldSeal.totalMs>=5,"First World seal omitted intervening wait");
            submit(6,{2});require(consumer.result().firstWorldSeal.samples==1,"Repeated PRESENT double counted first seal");
            submit(19,{1,2,0,123,0});submit(19,{1,3,0,123,0});
            require(consumer.result().supersededBeforeSeal==1&&!consumer.sealedGeneration(),"Superseded generation reused sealed UI");
            require(consumer.result().preparationTiming[1].samples==3&&consumer.result().preparationTiming[2].samples==3&&consumer.result().preparationTiming[3].samples==3,"Compilation phase counts disagree with commands");
            consumer.abort();renderer.destroy(source);require(!renderer.stats().surfaces,"Diagnostic controls leaked surfaces");
        }
        QTemporaryDir dir;require(dir.isValid(),"Temporary directory");
        NativeRenderWorker worker(viewport,dir.filePath("world"),dir.path(),dir.filePath("report.json"));
        require(worker.create(dir.filePath("commands"),271,2,10000),"Worker channel create");
        QFile file(dir.filePath("commands"));require(file.open(QIODevice::ReadWrite),"Producer file open");
        auto* mapping=file.map(0,MNM_RENDER_COMMANDS_V2_SIZE);require(mapping,"Producer mapping");
        mnm_ring_writer producer{};require(mnm_ring_writer_bind(&producer,mapping,MNM_RENDER_COMMANDS_V2_SIZE),"Producer claim");
        unsigned sequence=0,shown=0;worker.framePresented=[&]{++shown;};
        QByteArray input("MNMCMD01");word(input,1);word(input,16);
        record(input,1,sequence,{1,1,1,16,0xf800,0x7e0,31},QByteArray::fromHex("00f8"));record(input,6,sequence,{1});
        require(mnm_ring_write(&producer,input.data(),unsigned(input.size()))==1,"Initial publication");
        const auto wait=[&](auto predicate){QElapsedTimer clock;clock.start();while(!predicate()&&clock.elapsed()<10000){require(worker.poll(false,true),"Worker poll refused");app.processEvents();std::this_thread::sleep_for(std::chrono::milliseconds(1));}require(predicate(),"Worker wait timeout");};
        wait([&]{return shown>0;});pixel(viewport,Qt::red);
        input.clear();for(unsigned i=0;i<150;++i){record(input,2,sequence,{1,0,0,1,1},QByteArray::fromHex("1f00"));record(input,6,sequence,{1});}
        require(mnm_ring_write(&producer,input.data(),unsigned(input.size()))==1,"Burst publication");
        // Hold the GUI lease without consuming output while the owner continues.
        std::this_thread::sleep_for(std::chrono::milliseconds(200));pixel(viewport,Qt::red);
        wait([&]{return worker.result()&&worker.result()->presents==151;});
        require(worker.report()["presentation_lease_high_water"].toInt()==2,"Presentation lease bound not exercised");
        require(worker.report()["presentation_exports_skipped"].toString().toULongLong()>0,"Slow GUI did not exercise bounded output coalescing");
        pixel(viewport,Qt::blue);
        // Exercise returned-buffer reuse over separately published frames.
        for(unsigned i=0;i<20;++i){
            input.clear();record(input,2,sequence,{1,0,0,1,1},QByteArray::fromHex(i%2?"1f00":"00f8"));record(input,6,sequence,{1});
            const auto before=shown;require(mnm_ring_write(&producer,input.data(),unsigned(input.size()))==1,"Reuse publication");
            wait([&]{return shown>before;});pixel(viewport,i%2?Qt::blue:Qt::red);
        }
        require(worker.report()["presentation_export_allocations"].toString().toULongLong()==2,"Output textures were reallocated instead of reused");
        input.clear();record(input,7,sequence,{1});record(input,8,sequence,{});
        require(mnm_ring_write(&producer,input.data(),unsigned(input.size()))==1&&mnm_ring_end(&producer),"Clean producer END");
        wait([&]{return worker.ended();});require(worker.finishProducer(),"Joined normal teardown failed");
        require(viewport.frameSize().isEmpty(),"GUI retained worker lease after join");
        const auto* result=worker.result();require(result&&result->presents==171&&!result->stats.nativeReadbacks&&!result->stats.rgbaReadbacks,"Native counts/readback contract");
        require(result->stats.gpuPresentationAllocations==1,"Mutable producer presentation storage was recreated between polls");
        file.unmap(mapping);
        // Malformed input is terminal and cancellation must return retained leases.
        NativeRenderWorker failed(viewport,dir.filePath("world-fail"),dir.path(),dir.filePath("fail-report.json"));
        require(failed.create(dir.filePath("commands-fail"),272,2,10000),"Refusal create");
        QFile bad(dir.filePath("commands-fail"));require(bad.open(QIODevice::ReadWrite),"Refusal mapping open");auto* badMap=bad.map(0,MNM_RENDER_COMMANDS_V2_SIZE);
        mnm_ring_writer broken{};require(mnm_ring_writer_bind(&broken,badMap,MNM_RENDER_COMMANDS_V2_SIZE),"Refusal claim");
        const QByteArray invalid(16,'x');require(mnm_ring_write(&broken,invalid.data(),unsigned(invalid.size()))==1,"Refusal publication");
        QElapsedTimer deadline;deadline.start();while(failed.poll(false,true)&&deadline.elapsed()<10000){app.processEvents();std::this_thread::sleep_for(std::chrono::milliseconds(1));}
        require(!failed.error().isEmpty(),"Malformed command accepted");failed.abort();bad.unmap(badMap);
        std::printf("{\"success\":true,\"native_presentations\":171,\"immutable_frame_checked\":true,\"slow_gui_checked\":true,\"presentation_lease_bound\":2,\"malformed_refused\":true,\"joined_teardown\":true,\"native_readbacks\":0,\"rgba_readbacks\":0,\"diagnostic_framebuffer_checks\":23,\"presentation_export_allocations\":2,\"presentation_source_allocations\":1,\"first_world_seal_timing\":true,\"detailed_ui_block_states\":true,\"deadline_deduplication\":true,\"elapsed_histogram_boundaries\":true,\"continuous_ui_clock_frames\":60,\"ordered_source_updates\":20,\"mixed_generation_refused\":true}\n");return 0;
    }catch(const std::exception& e){std::fprintf(stderr,"Render worker controls failed: %s\n",e.what());return 1;}
}
