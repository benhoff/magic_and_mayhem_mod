#include "native_render_worker.hpp"
#include "gl_viewport.hpp"
#include <QOpenGLContext>
#include <QSaveFile>
#include <QJsonDocument>
#include <QTimer>
#include <algorithm>
#include <chrono>
#include <vector>
NativeRenderWorker::NativeRenderWorker(GlViewport& v,const QString& path,const QString& game,const QString& reportPath):viewport_(v),reportPath_(reportPath){
    baselineUiClock_=qEnvironmentVariableIntValue("MNM_WORLD_UI_CLOCK_BASELINE")==1;
    NativeRenderTarget target{[this]{return share_!=nullptr;},[this]{return share_;},
        [this](auto frame){rendered_=std::move(frame);frameChanged_=rendered_.valid();},[]{return QString();},&surface_,&worldSurface_};
    world_=std::make_unique<ResidentWorldSession>(target,path,game,true);
    commands_=std::make_unique<LiveCommandSession>(target);
    commands_->cpuReadPending=[this](auto ticket){pendingCpuRead_.store(ticket,std::memory_order_release);};
    commands_->residentWorld=[this](auto g,auto n){return world_->import(g,n);};
    commands_->residentReady=[this](auto g,auto n){return world_->ready(g,n);};
    commands_->prepareResidentFrames=true;
    commands_->reuseSealedUi=!baselineUiClock_ && qEnvironmentVariableIntValue("MNM_WORLD_FIRST_SEAL_BASELINE")!=1;
    if(frameTrace_.enabled())QObject::connect(&viewport_,&QOpenGLWidget::frameSwapped,this,[this]{
        std::lock_guard<std::mutex> lock(mutex_);
        frameTrace_.recordGui({paintedSerial_,paintedGeneration_,NativeFrameTrace::now(),1,drawAllowed_,paused_});
    });
    if(frameTrace_.enabled())viewport_.framePaintTiming=[this](bool complete){
        std::lock_guard<std::mutex> lock(mutex_);
        if(complete){paintedSerial_=guiSerial_;paintedGeneration_=guiGeneration_;}
        frameTrace_.recordGui({guiSerial_,guiGeneration_,NativeFrameTrace::now(),complete?3:2,drawAllowed_,paused_});
    };
    if(frameTrace_.enabled()){
        viewport_.activityTiming=[this](GlViewport::Activity activity,bool complete){
            std::lock_guard<std::mutex> lock(mutex_);
            frameTrace_.recordGui({guiSerial_,guiGeneration_,NativeFrameTrace::now(),complete?5:4,drawAllowed_,paused_,int(activity)});
        };
        QObject::connect(&viewport_,&QOpenGLWidget::aboutToCompose,this,[this]{
            std::lock_guard<std::mutex> lock(mutex_);
            frameTrace_.recordGui({paintedSerial_,paintedGeneration_,NativeFrameTrace::now(),8,drawAllowed_,paused_});
        });
        auto* heartbeat=new QTimer(this);heartbeat->setTimerType(Qt::PreciseTimer);heartbeat->setInterval(16);
        QObject::connect(heartbeat,&QTimer::timeout,this,[this,last=NativeFrameTrace::now()]() mutable {
            const auto now=NativeFrameTrace::now();const auto late=std::max<std::int64_t>(0,now-last-16000);last=now;
            std::lock_guard<std::mutex> lock(mutex_);
            frameTrace_.recordGui({guiSerial_,guiGeneration_,now,6,drawAllowed_,paused_,0,late});
        });heartbeat->start();
    }
}
NativeRenderWorker::~NativeRenderWorker(){if(frameTrace_.enabled()){viewport_.framePaintTiming={};viewport_.activityTiming={};}join(false);}
bool NativeRenderWorker::create(const QString& path,quint32 session,quint32 version,int timeout){
    if(version!=2){status_.error="Render worker requires streaming owned World commands";return false;}
    if(!commands_->create(path,session,version,timeout)){status_.error=commands_->error();return false;}
    return true;
}
bool NativeRenderWorker::poll(bool normalQuit,bool drawAllowed,bool paused){
    mnm::render::GpuFrame frame;
    std::uint64_t serial=0,generation=0;
    {GlViewport::ActivityScope pickup(&viewport_,GlViewport::Activity::RenderStatusPickup);
     {
        std::lock_guard<std::mutex> lock(mutex_);
        normalQuit_=normalQuit;drawAllowed_=drawAllowed;paused_=paused;
        status_=published_;frame=std::move(readyFrame_);
        serial=readySerial_;generation=readyGeneration_;
     }
    }
    wake_.notify_one();
    if(!started_ && viewport_.ready()){
        share_=viewport_.context();surface_.setFormat(share_->format());surface_.create();
        if(!surface_.isValid()){status_.error="Render worker offscreen surface creation failed";return false;}
        if(qEnvironmentVariableIntValue("MNM_ASYNC_WORLD_GPU")==1){
            worldSurface_.setFormat(share_->format());worldSurface_.create();
            if(!worldSurface_.isValid()){status_.error="World producer offscreen surface creation failed";return false;}
        }
        started_=true;start();
    }
    if(!status_.error.isEmpty())return false;
    if(frame.valid() && !paused){
        viewport_.setGpuFrame(std::move(frame));
        if(frameTrace_.enabled()){
            std::lock_guard<std::mutex> lock(mutex_);guiSerial_=serial;guiGeneration_=generation;
            frameTrace_.recordGui({serial,generation,NativeFrameTrace::now(),0,drawAllowed_,paused_});
        }
        if(framePresented)framePresented();
    }
    if(stateChanged)stateChanged(status_.state);
    return true;
}
bool NativeRenderWorker::requestStop(){std::lock_guard<std::mutex> lock(mutex_);requestStop_=true;wake_.notify_one();return true;}
void NativeRenderWorker::abort(){join(false);}
bool NativeRenderWorker::finishProducer(){join(true);return status_.error.isEmpty()&&ended();}
void NativeRenderWorker::join(bool producerExited){
    // Return the GUI lease before joining. The worker retains every exported
    // texture until its last consumer lease is gone, and destroys it itself.
    viewport_.setGpuFrame({});
    {std::lock_guard<std::mutex> lock(mutex_);stop_=true;producerExited_=producerExited;readyFrame_={};}
    wake_.notify_one();
    if(started_){wait();std::lock_guard<std::mutex> lock(mutex_);status_=published_;}
    else{commands_.reset();world_.reset();}
}
void NativeRenderWorker::run(){
    std::vector<mnm::render::GpuFrame> leases;
    bool mediaPaused=false;
    NativeWorkerStatusGate statusGate(statusEveryTurn_);
    auto reportedState=LiveCommandSession::State::WaitingFrame;
    unsigned reportedRecoveries=0;bool reportedPaused=false,reportedQuit=false,reportedHasResult=false;
    auto updateStatus=[&]{
        ++statusBuilds_;
        Status next;next.state=commands_->state();next.recoveries=commands_->recoveries();next.error=commands_->error();
        if(const auto* result=commands_->result()){next.hasResult=true;next.result=*result;}
        next.report=world_->report();
        next.report["media_presentation_paused"]=mediaPaused;
        next.report["native_render_owner"]="dedicated-thread";
        next.report["status_publication"]=QJsonObject{{"policy",statusEveryTurn_?"every-turn":"bounded-16ms"},{"snapshots",QString::number(statusBuilds_)},{"skipped_turns",QString::number(statusSkipped_)},{"period_ms",16},{"scope","Heavy status construction only; lifecycle/pause/quit/error/stop publish immediately. Render/source/command work and output leases remain independent. Busy turns may delay nonterminal reports beyond16ms."}};
        next.report["presentation_source_allocations"]=QString::number(next.hasResult?next.result.stats.gpuPresentationAllocations:0);
        next.report["presentation_exports"]=QString::number(exports_);
        next.report["presentation_export_allocations"]=QString::number(exportAllocations_);
        next.report["presentation_exports_skipped"]=QString::number(skipped_);
        next.report["presentation_lease_high_water"]=int(highWater_);
        next.report["render_owner_turns"]=QString::number(turns_);
        static const char* reasons[]={"ui_publication","incomplete_ui","pending_world_import","hidden","source_completed","render_attempt","no_recipe","first_seal","static_ui","generation_mismatch"};
        QJsonObject reasonsJson;
        for(unsigned i=0;i<RenderDeadlineProbe::ReasonCount;++i)reasonsJson[reasons[i]]=QJsonObject{{"turns",QString::number(deadlineProbe_.turns[i])},{"opportunities",QString::number(deadlineProbe_.opportunities[i])}};
        const auto timing=[](const RenderStageTiming& t){QJsonArray bins;for(auto n:t.bins)bins.append(double(n));return QJsonObject{{"samples",double(t.samples)},{"mean_ms",t.samples?t.totalMs/t.samples:0},{"max_ms",t.maxMs},{"bins",bins}};};
        QJsonObject compilation;
        if(next.hasResult){
            const auto cpu=[](const mnm::render::CommandCpuTiming& t){return QJsonObject{{"samples",double(t.samples)},{"total_ms",t.totalMs},{"mean_ms",t.samples?t.totalMs/t.samples:0},{"max_ms",t.maxMs}};};
            static const char* phases[]={"record","reset","seal","import"};
            for(unsigned i=0;i<4;++i)compilation[phases[i]]=cpu(next.result.preparationTiming[i]);
            compilation["inherited_ui_imports"]=QString::number(next.result.inheritedUiImports);
            compilation["inherited_source_presentations"]=QString::number(next.result.inheritedSourcePresentations);
            compilation["inherited_ui_redraws"]=QString::number(inheritedUiRedraws_);
            compilation["reuse_sealed_ui_enabled"]=commands_->reuseSealedUi;
            compilation["first_world_seal_wall"]=cpu(next.result.firstWorldSeal);
            compilation["superseded_before_first_world_seal"]=QString::number(next.result.supersededBeforeSeal);
        }
        next.report["display_deadline_probe"]=QJsonObject{{"policy",baselineUiClock_?"ui-present-barrier":"independent-sealed-ui"},{"ui_publication_redraws",QString::number(uiPublicationRedraws_)},{"partial_ui_sealed_redraws",QString::number(sealedUiRedraws_)},{"reasons",reasonsJson},{"max_late_ms",deadlineProbe_.maxLateMs},
            {"active_world_poll",timing(activeWorldTiming_)},{"active_command_poll",timing(activeCommandTiming_)},{"compilation",compilation},{"world_poll",timing(worldTiming_)},{"command_poll",timing(commandTiming_)},{"extra_world_ui",timing(extraTiming_)},{"export",timing(exportTiming_)},{"turn",timing(turnTiming_)},{"slow_turns",slowTurns_},
            {"scope","Observation after source/command work. Per-reason opportunities deduplicate global16.667ms monotonic buckets, may overlap and are not additive. CPU elapsed times include driver waits, not GPU execution. Active polls contain accepted work; first_world_seal_wall runs from WORLD compilation start to first sealed World-mapped PRESENT, including intervening polling/source/UI waits. Static UI counts do not imply missing visible World frames. Histograms upper bounds1/2/4/8/16.667/33.334/100ms then overflow. Slow turns bounded256."}};
        {std::lock_guard<std::mutex> lock(mutex_);published_=std::move(next);}
    };
    auto save=[&]{
        world_->saveReport(reportPath_);
        QJsonObject report;{std::lock_guard<std::mutex> lock(mutex_);report=published_.report;report["frame_trace"]=frameTrace_.report();}
        QSaveFile file(reportPath_);const auto bytes=QJsonDocument(report).toJson();
        if(!file.open(QIODevice::WriteOnly)||file.write(bytes)!=bytes.size()||!file.commit())throw std::runtime_error("Render owner report publication");
    };
    try{
        for(;;){
            NativeFrameTrace::Turn trace;trace.begin=frameTrace_.stamp();trace.turn=turns_;
            bool stop,exited,quit,draw,pause,request;
            {std::lock_guard<std::mutex> lock(mutex_);stop=stop_;exited=producerExited_;quit=normalQuit_;draw=drawAllowed_;pause=paused_;request=requestStop_;requestStop_=false;}
            if(stop){
                if(exited){world_->poll(quit);commands_->finishProducer();}
                else{world_->abort("Render owner cancelled");commands_->abort();}
                updateStatus();save();break;
            }
            QElapsedTimer turnClock,stageClock;turnClock.start();stageClock.start();
            if(request)commands_->requestStop();
            const auto beforeState=world_->displayState();const auto beforeWorld=beforeState.generation;
            trace.sourceBefore=beforeWorld;trace.framesBefore=world_->nativeFrames();trace.draw=draw;trace.paused=pause;
            trace.deadline=beforeState.deadline.time_since_epoch().count()?std::chrono::duration_cast<std::chrono::microseconds>(beforeState.deadline.time_since_epoch()).count():0;
            mediaPaused=pause;commands_->setMediaActive(pause);
            world_->poll(quit);
            const double worldMs=stageClock.nsecsElapsed()/1000000.;worldTiming_.add(worldMs);if(world_->displayState().generation!=beforeWorld)activeWorldTiming_.add(worldMs);stageClock.restart();
            trace.worldEnd=frameTrace_.stamp();trace.commandEnd=trace.extraEnd=trace.worldEnd;
            trace.sourceAfter=world_->displayState().generation;
            if(frameTrace_.enabled()&&trace.sourceAfter!=trace.sourceBefore)trace.source=world_->lastSourceTiming();
            double commandMs=0,extraMs=0,exportMs=0;unsigned presents=0;std::uint64_t generation=0;
            int reason=-1;
            if(!commands_->ended()){
                const auto before=commands_->result()?commands_->result()->presents:0;
                const auto commandsBefore=commands_->result()?commands_->result()->commands:0;
                const auto importsBefore=commands_->result()?commands_->result()->residentWorldImports:0;
                if(!commands_->poll())throw std::runtime_error(commands_->error().toStdString());
                const auto after=commands_->result()?commands_->result()->presents:0;
                presents=after-before;commandMs=stageClock.nsecsElapsed()/1000000.;commandTiming_.add(commandMs);stageClock.restart();
                if(commands_->result()&&commands_->result()->commands!=commandsBefore)activeCommandTiming_.add(commandMs);
                trace.commands=commands_->result()?commands_->result()->commands-commandsBefore:0;
                trace.imports=commands_->result()?commands_->result()->residentWorldImports-importsBefore:0;
                trace.presents=presents;trace.commandEnd=frameTrace_.stamp();
                const auto uiState=commands_->displayState();
                const auto strictGeneration=commands_->preparedGeneration();
                if(frameTrace_.capturesHitches()){
                    trace.uiGeneration=uiState.generation;trace.preparedGeneration=strictGeneration;trace.sealedSource=uiState.sealedSourceGeneration;
                    trace.uiSealed=uiState.sealed;trace.worldOutput=uiState.worldOutput;
                }
                generation=baselineUiClock_?strictGeneration:commands_->sealedGeneration();
                const auto state=world_->displayState();const auto now=std::chrono::steady_clock::now();
                const bool imported=commands_->result()&&commands_->result()->residentWorldImports!=importsBefore;
                const bool sourceCompleted=state.generation!=beforeWorld || imported;
                const bool displayedCompleted=world_->displaysPendingWorld()?imported:sourceCompleted;
                if(state.active && now>=state.deadline){
                    reason=(!draw || pause)?RenderDeadlineProbe::Hidden:state.replayPending?RenderDeadlineProbe::PendingWorldImport:
                        displayedCompleted?RenderDeadlineProbe::SourceCompleted:
                        nativeUiBlockReason(uiState,state.replayGeneration)!=RenderDeadlineProbe::RenderAttempt?nativeUiBlockReason(uiState,state.replayGeneration):
                        !generation?RenderDeadlineProbe::IncompleteUi:(baselineUiClock_&&presents)?RenderDeadlineProbe::UiPublication:RenderDeadlineProbe::RenderAttempt;
                    deadlineProbe_.observe(state.deadline,now,RenderDeadlineProbe::Reason(reason));
                }
                if(!pause && (baselineUiClock_?(before==after && draw):nativeDisplayEligible(draw,state.replayPending,displayedCompleted,generation,state.replayGeneration))){
                    if(generation){auto frame=world_->drawIndependent(generation);if(frame.valid()){
                        QElapsedTimer clock;clock.start();
                        if(!(baselineUiClock_?commands_->presentPrepared(frame,generation):commands_->presentSealed(frame,generation)))throw std::runtime_error(commands_->error().toStdString());
                        if(!baselineUiClock_&&presents)++uiPublicationRedraws_;
                        if(!baselineUiClock_&&!strictGeneration)++sealedUiRedraws_;
                        if(uiState.sealedSourceGeneration && uiState.sealedSourceGeneration<uiState.generation)++inheritedUiRedraws_;
                        world_->noteIndependentPresentation(clock.nsecsElapsed()/1000000.0);
                    }}
                }
                extraMs=stageClock.nsecsElapsed()/1000000.;if(extraMs>0.1)extraTiming_.add(extraMs);stageClock.restart();
                trace.extraEnd=frameTrace_.stamp();
                if(!pause && frameChanged_ && rendered_.valid()){
                    // Bounded immutable output leases. Display coalescing never
                    // discards/reorders engine source updates or command imports.
                    auto available=std::find_if(leases.begin(),leases.end(),[&](const auto& f){return f.unique() && f.size()==rendered_.size();});
                    if(available!=leases.end() || leases.size()<2){
                        mnm::render::GpuFrame frozen;
                        if(available!=leases.end()){frozen=rendered_.snapshot(std::move(*available));*available=frozen;}
                        else{frozen=rendered_.snapshot();leases.push_back(frozen);++exportAllocations_;}
                        highWater_=std::max(highWater_,std::uint64_t(leases.size()));++exports_;
                        trace.exportSerial=exports_;
                        std::lock_guard<std::mutex> lock(mutex_);trace.published=frameTrace_.stamp();readyFrame_=std::move(frozen);readySerial_=exports_;readyGeneration_=uiState.worldOutput?uiState.generation:0;
                    }else{++skipped_;trace.exportBlocked=true;}
                    frameChanged_=false;
                }
            }
            exportMs=stageClock.nsecsElapsed()/1000000.;if(exportMs>0.1)exportTiming_.add(exportMs);
            if(frameTrace_.capturesHitches())trace.delivery=world_->traceDeliveryState();
            trace.exportEnd=frameTrace_.stamp();trace.framesAfter=world_->nativeFrames();trace.pending=world_->displayState().pending;trace.reason=reason;
            const auto totalMs=turnClock.nsecsElapsed()/1000000.;turnTiming_.add(totalMs);
            if(totalMs>33.334 && slowTurns_.size()<256)slowTurns_.append(QJsonObject{{"turn",QString::number(turns_)},{"source_generation",QString::number(world_->displayState().generation)},{"prepared_generation",QString::number(generation)},{"command_presents",int(presents)},{"reason",reason},{"world_ms",worldMs},{"command_ms",commandMs},{"extra_ms",extraMs},{"export_ms",exportMs},{"total_ms",totalMs}});
            ++turns_;
            const bool immediate=request || commands_->state()!=reportedState || commands_->recoveries()!=reportedRecoveries || pause!=reportedPaused || quit!=reportedQuit || bool(commands_->result())!=reportedHasResult;
            if(statusGate.due(NativeFrameTrace::now(),immediate)){
                updateStatus();reportedState=commands_->state();reportedRecoveries=commands_->recoveries();reportedPaused=pause;reportedQuit=quit;reportedHasResult=bool(commands_->result());
            }else ++statusSkipped_;
            trace.statusEnd=frameTrace_.stamp();
            std::unique_lock<std::mutex> lock(mutex_);wake_.wait_for(lock,std::chrono::milliseconds(1),[this]{return stop_||requestStop_;});
            trace.waitEnd=frameTrace_.stamp();frameTrace_.record(trace);
        }
    }catch(const std::exception& e){
        world_->abort(QString::fromUtf8(e.what()));commands_->abort();updateStatus();
        {std::lock_guard<std::mutex> lock(mutex_);published_.error=QString::fromUtf8(e.what());}
        try{save();}catch(const std::exception&){}
        // Keep resource ownership alive until the GUI returns its last frame.
        std::unique_lock<std::mutex> lock(mutex_);wake_.wait(lock,[this]{return stop_;});
    }
    {std::lock_guard<std::mutex> lock(mutex_);readyFrame_={};}
    rendered_={};leases.clear();commands_.reset();world_.reset();
}
