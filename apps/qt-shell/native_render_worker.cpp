#include "native_render_worker.hpp"
#include "gl_viewport.hpp"
#include <QOpenGLContext>
#include <QSaveFile>
#include <QJsonDocument>
#include <algorithm>
#include <chrono>
#include <vector>
NativeRenderWorker::NativeRenderWorker(GlViewport& v,const QString& path,const QString& game,const QString& reportPath):viewport_(v),reportPath_(reportPath){
    NativeRenderTarget target{[this]{return share_!=nullptr;},[this]{return share_;},
        [this](auto frame){rendered_=std::move(frame);frameChanged_=rendered_.valid();},[]{return QString();},&surface_};
    world_=std::make_unique<ResidentWorldSession>(target,path,game,true);
    commands_=std::make_unique<LiveCommandSession>(target);
    commands_->residentWorld=[this](auto g,auto n){return world_->import(g,n);};
    commands_->residentReady=[this](auto g,auto n){return world_->ready(g,n);};
    commands_->prepareResidentFrames=true;
}
NativeRenderWorker::~NativeRenderWorker(){join(false);}
bool NativeRenderWorker::create(const QString& path,quint32 session,quint32 version,int timeout){
    if(version!=2){status_.error="Render worker requires streaming owned World commands";return false;}
    if(!commands_->create(path,session,version,timeout)){status_.error=commands_->error();return false;}
    return true;
}
bool NativeRenderWorker::poll(bool normalQuit,bool drawAllowed,bool paused){
    mnm::render::GpuFrame frame;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        normalQuit_=normalQuit;drawAllowed_=drawAllowed;paused_=paused;
        status_=published_;frame=std::move(readyFrame_);
    }
    wake_.notify_one();
    if(!started_ && viewport_.ready()){
        share_=viewport_.context();surface_.setFormat(share_->format());surface_.create();
        if(!surface_.isValid()){status_.error="Render worker offscreen surface creation failed";return false;}
        started_=true;start();
    }
    if(!status_.error.isEmpty())return false;
    if(frame.valid()){
        viewport_.setGpuFrame(std::move(frame));
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
    auto updateStatus=[&]{
        Status next;next.state=commands_->state();next.recoveries=commands_->recoveries();next.error=commands_->error();
        if(const auto* result=commands_->result()){next.hasResult=true;next.result=*result;}
        next.report=world_->report();
        next.report["native_render_owner"]="dedicated-thread";
        next.report["presentation_source_allocations"]=QString::number(next.hasResult?next.result.stats.gpuPresentationAllocations:0);
        next.report["presentation_exports"]=QString::number(exports_);
        next.report["presentation_export_allocations"]=QString::number(exportAllocations_);
        next.report["presentation_exports_skipped"]=QString::number(skipped_);
        next.report["presentation_lease_high_water"]=int(highWater_);
        next.report["render_owner_turns"]=QString::number(turns_);
        {std::lock_guard<std::mutex> lock(mutex_);published_=std::move(next);}
    };
    auto save=[&]{
        world_->saveReport(reportPath_);
        QJsonObject report;{std::lock_guard<std::mutex> lock(mutex_);report=published_.report;}
        QSaveFile file(reportPath_);const auto bytes=QJsonDocument(report).toJson();
        if(!file.open(QIODevice::WriteOnly)||file.write(bytes)!=bytes.size()||!file.commit())throw std::runtime_error("Render owner report publication");
    };
    try{
        for(;;){
            bool stop,exited,quit,draw,pause,request;
            {std::lock_guard<std::mutex> lock(mutex_);stop=stop_;exited=producerExited_;quit=normalQuit_;draw=drawAllowed_;pause=paused_;request=requestStop_;requestStop_=false;}
            if(stop){
                if(exited){world_->poll(quit);commands_->finishProducer();}
                else{world_->abort("Render owner cancelled");commands_->abort();}
                updateStatus();save();break;
            }
            if(request)commands_->requestStop();
            if(!pause)world_->poll(quit);
            if(!pause && !commands_->ended()){
                const auto before=commands_->result()?commands_->result()->presents:0;
                if(!commands_->poll())throw std::runtime_error(commands_->error().toStdString());
                const auto after=commands_->result()?commands_->result()->presents:0;
                if(before==after && draw){
                    const auto generation=commands_->preparedGeneration();
                    if(generation){auto frame=world_->drawIndependent(generation);if(frame.valid()){
                        QElapsedTimer clock;clock.start();
                        if(!commands_->presentPrepared(frame,generation))throw std::runtime_error(commands_->error().toStdString());
                        world_->noteIndependentPresentation(clock.nsecsElapsed()/1000000.0);
                    }}
                }
                if(frameChanged_ && rendered_.valid()){
                    // Bounded immutable output leases. Display coalescing never
                    // discards/reorders engine source updates or command imports.
                    auto available=std::find_if(leases.begin(),leases.end(),[&](const auto& f){return f.unique() && f.size()==rendered_.size();});
                    if(available!=leases.end() || leases.size()<2){
                        mnm::render::GpuFrame frozen;
                        if(available!=leases.end()){frozen=rendered_.snapshot(std::move(*available));*available=frozen;}
                        else{frozen=rendered_.snapshot();leases.push_back(frozen);++exportAllocations_;}
                        highWater_=std::max(highWater_,std::uint64_t(leases.size()));++exports_;
                        std::lock_guard<std::mutex> lock(mutex_);readyFrame_=std::move(frozen);
                    }else ++skipped_;
                    frameChanged_=false;
                }
            }
            ++turns_;updateStatus();
            std::unique_lock<std::mutex> lock(mutex_);wake_.wait_for(lock,std::chrono::milliseconds(1),[this]{return stop_||requestStop_;});
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
