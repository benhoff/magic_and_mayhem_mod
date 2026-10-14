#include "resident_world_session.hpp"
#include "resident_profile_capture.hpp"
#include "gl_viewport.hpp"
#include <QJsonDocument>
#include <QSaveFile>
#include <QFileInfo>
#include <QDir>
#include <QRandomGenerator>
#include <stdexcept>
#include <algorithm>
#include <cstdio>
#include <chrono>
using namespace mnm::legacy;
struct ResidentWorldGpuBackend {std::unique_ptr<WorldRollingSession> world;};
namespace {
struct WorldGpuJob {
    OwnedWorldSnapshot source;
    mnm::render::RenderStats stats;
    CanvasWorld::Profile profile;
    QString driver;
    double nativeMs=0,producerMs=0;
    std::uint64_t readbacks=0,captures=0;
};
QString create(const QString& path,bool owned){auto nonce=QRandomGenerator::system()->generate64();if(!nonce)nonce=1;WorldContinuousChannel::create(path,nonce,0,true,owned);return path;}
CanvasProducer close(const std::array<std::uint8_t,192>& bytes,unsigned offset){
    CanvasProducer result{};for(unsigned i=0;i<24;++i){const auto* p=bytes.data()+offset+i*4;result.fields[i]=p[0]|unsigned(p[1])<<8|unsigned(p[2])<<16|unsigned(p[3])<<24;}
    if(result.fields[0]!=96 || result.fields[18] || result.fields[23])throw std::runtime_error("Resident World closing record");
    return result;
}
}
ResidentWorldSession::ResidentWorldSession(GlViewport& viewport,const QString& path,const QString& game,bool independent):ResidentWorldSession(viewportRenderTarget(viewport),path,game,independent){}
ResidentWorldSession::ResidentWorldSession(NativeRenderTarget target,const QString& path,const QString& game,bool independent):target_(std::move(target)),channel_(create(path,independent)),game_(game),independent_(independent){
    stageProfile_=qEnvironmentVariableIsSet("MNM_RESIDENT_STAGE_PROFILE");profileClock_.start();
    const auto capture=qEnvironmentVariable("MNM_RESIDENT_PROFILE_CAPTURE");
    if(!capture.isEmpty()){
        if(!stageProfile_)throw std::runtime_error("Resident capture requires stage profiling");
        capture_=std::make_unique<ResidentProfileCapture>(capture);
    }
    frameTrace_=qEnvironmentVariableIntValue("MNM_NATIVE_FRAME_TRACE")==1;
    retainCompletedImage_=qEnvironmentVariableIntValue("MNM_WORLD_IMAGE_REUSE")==1;
    if(retainCompletedImage_ && !independent_)throw std::invalid_argument("Retained World image requires independent resident presentation");
    asynchronous_=qEnvironmentVariableIntValue("MNM_ASYNC_WORLD_GPU")==1;
    if(asynchronous_ && (!independent_||!retainCompletedImage_||!target_.worldOffscreen||stageProfile_))
        throw std::invalid_argument("Async World requires independent retained images and a dedicated platform target; stage capture profiling is separate");
    displayWhilePending_=qEnvironmentVariableIntValue("MNM_DISPLAY_WHILE_PENDING")==1;
    if(displayWhilePending_ && !asynchronous_)throw std::invalid_argument("Pending World display requires async native production");
    if(independent_)snapshotQueue_=std::make_unique<WorldSnapshotQueue>(channel_,bool(capture_),frameTrace_);
}
void ResidentWorldSession::collectGpuTimings(){
    if(!stageProfile_ || !renderer_)return;
    for(const auto& timing:renderer_->pollGpuTimings()){
        if(!timing.sequence || timing.sequence>std::uint64_t(stageSamples_.size()))
            throw std::runtime_error("Resident GPU profile identity");
        auto row=stageSamples_[int(timing.sequence-1)].toObject();
        if(row["generation"].toString()!=QString::number(timing.sequence))
            throw std::runtime_error("Resident GPU profile generation");
        row["gpu_timeline_ms"]=timing.timelineMs;stageSamples_[int(timing.sequence-1)]=row;
    }
}
ResidentWorldSession::~ResidentWorldSession(){if(snapshotQueue_)snapshotQueue_->stop();displayedFrame_={};ownedFrame_={};ownedPending_.reset();if(gpuProduction_)gpuProduction_->join();channel_.cancel();if(reportWork_.valid())reportWork_.wait();}
void ResidentWorldSession::abort(const QString& error){error_=error;channel_.cancel();}
void ResidentWorldSession::poll(bool acceptedNormalQuit){
    if(independent_){pollOwned(acceptedNormalQuit);return;}
    QElapsedTimer phaseClock;phaseClock.start();
    const bool phaseLog=qEnvironmentVariableIsSet("MNM_RESIDENT_POLL_TIMING");
    const auto mark=[&](const char* stage){
        const double ms=double(phaseClock.nsecsElapsed())/1000000.;phaseClock.restart();
        if(phaseLog&&ms>=50.&&phaseLogRows_++<256)std::fprintf(stderr,"RESIDENT_PHASE stage=%s ms=%.3f completed=%llu pending=%d\n",stage,ms,static_cast<unsigned long long>(completed_),bool(pending_));
    };
    try {
        if(!error_.isEmpty())throw std::runtime_error(error_.toStdString());
        serviceReport();mark("report-worker");
        collectGpuTimings();mark("gpu-timing-poll");
        if(normalQuitRetired_)return;
        if(channel_.cancelled()){
            if(acceptedNormalQuit && !pending_ && completed_==imports_ && completed_==returns_){
                normalQuitRetired_=true;if(capture_)capture_->finish();saveReport(QFileInfo(game_).dir().filePath("resident-world-report.json"));return;
            }
            throw std::runtime_error("Resident World producer cancelled");
        }
        if(!target_.ready())return;
        if(!world_){
            renderer_=std::make_unique<mnm::render::GlBlitter>(target_.context(),target_.offscreen);auto store=mnm::assets::AssetStore::create(game_.toStdString());
            if(auto* e=std::get_if<mnm::assets::Error>(&store))throw std::runtime_error(e->detail);
            world_=std::make_unique<WorldRollingSession>(*renderer_,std::get<mnm::assets::AssetStore>(std::move(store)),false,true);
        }
        mark("initialization");
        if(pending_){
            if(pending_->deadline.elapsed()>120000)throw std::runtime_error("Resident World import/return deadline");
            if(!pending_->returned){std::array<std::uint8_t,192> actual{};
                if(channel_.returnRecords(pending_->block.generation,pending_->block.source,actual)){
                    verifyRollingHandoffReturn(pending_->handoff,close(actual,0),close(actual,96),true);
                    pending_->returned=true;++returns_;rasters_+=pending_->handoff.batch.header[14];
                    const auto ticks=channel_.ticks(pending_->block.generation);regionQueries_+=ticks[9];if(returns_==64)firstTick_=ticks[0];if(returns_>=64)lastTick_=ticks[3];
                    if(stageProfile_ && pending_->block.generation<=std::uint64_t(stageSamples_.size())){
                        const int index=int(pending_->block.generation-1);auto row=stageSamples_[index].toObject();
                        row["producer_capture_ms"]=double(std::uint32_t(ticks[1]-ticks[0]));
                        row["producer_wait_ack_ms"]=double(std::uint32_t(ticks[2]-ticks[1]));
                        row["producer_world_ms"]=double(std::uint32_t(ticks[3]-ticks[0]));
                        row["producer_return_ms"]=double(std::uint32_t(ticks[3]-ticks[2]));
                        row["engine_pacer_observed"]=ticks[6]==3;
                        row["engine_pacer_mask"]=int(ticks[6]);
                        row["engine_pacer_ms"]=ticks[6]==3?double(std::uint32_t(ticks[5]-ticks[4])):0.;
                        row["engine_screen_copy_ms"]=double(ticks[7]);row["engine_screen_copy_calls"]=double(ticks[8]);
                        row["producer_region_queries"]=double(ticks[9]);
                        if(returns_>1){
                            row["original_frame_interval_ms"]=double(std::uint32_t(ticks[0]-previousStart_));
                            row["engine_between_world_ms"]=double(std::uint32_t(ticks[0]-previousReturn_));
                        }
                        stageSamples_[index]=row;
                    }
                    previousStart_=ticks[0];previousReturn_=ticks[3];
                }
            }
            mark("return-verification");
            if(pending_->returned && pending_->imported){completed_=pending_->block.generation;channel_.release(completed_);pending_.reset();if(completed_%64==0)queueReport(QFileInfo(game_).dir().filePath("resident-world-report.json"));}
        }
        mark("release-and-report");
        if(!pending_){WorldContinuousChannel::Block block;QElapsedTimer acquire;if(stageProfile_)acquire.start();const bool available=channel_.poll(block);mark("source-acquire");if(available){
            const auto acquireMs=stageProfile_?double(acquire.nsecsElapsed())/1000000.:0.;
            const bool sampled=stageProfile_ && block.generation<=8192;
            const double sampleStart=double(profileClock_.nsecsElapsed())/1000000.;
            QElapsedTimer timing;timing.start();double captureIo=0;
            if(capture_){
                capture_->enqueue(block.generation,block.source,block.request);
                captureIo=double(timing.nsecsElapsed())/1000000.;timing.restart();
            }
            mark("capture-enqueue");
            auto stream=decodeCanvasProducers(block.source,false);mark("decode");
            const auto decode=double(timing.nsecsElapsed())/1000000.;decodeMs_+=decode;timing.restart();
            auto handoff=prepareRollingHandoff(stream,0,block.request,1,0,WorldBatchSourcePolicy::ownedV5,false);
            mark("handoff");
            const auto handoffTime=double(timing.nsecsElapsed())/1000000.;handoffMs_+=handoffTime;timing.restart();
            mnm::render::GpuFrame frame;
            if(sampled){if(!renderer_->profileBatch(block.generation,[&]{frame=world_->consumeResident(block.generation,handoff.stream);}))++gpuDropped_;}
            else frame=world_->consumeResident(block.generation,handoff.stream);
            mark("native-consume");
            const auto native=double(timing.nsecsElapsed())/1000000.;nativeMs_+=native;maxNativeMs_=std::max(maxNativeMs_,native);
            prepareMs_+=world_->profile().prepareMs;submitMs_+=world_->profile().submitMs;producerMs_+=world_->producerWorkMs();++profiled_;
            if(sampled)stageSamples_.append(QJsonObject{{"generation",QString::number(block.generation)},
                {"host_elapsed_ms",sampleStart},{"source_acquire_ms",acquireMs},{"capture_enqueue_cpu_ms",captureIo},{"decode_ms",decode},{"handoff_ms",handoffTime},
                {"native_consume_ms",native},{"native_cpu_producer_ms",world_->producerWorkMs()},
                {"native_prepare_ms",world_->profile().prepareMs},{"native_submit_cpu_ms",world_->profile().submitMs},
                {"world_rasters",int(handoff.batch.header[14])},{"source_bytes",double(block.source.size())}});
            else if(stageProfile_)++samplesDropped_;
            if(world_->completed()!=block.generation || world_->worldReadbacks() || renderer_->stats().nativeReadbacks || renderer_->stats().rgbaReadbacks)
                throw std::runtime_error("Resident World completion/readback contract");
            Pending pending{std::move(block),std::move(handoff),std::move(frame),false,false,{}};pending.deadline.start();pending_=std::move(pending);
            std::vector<std::uint8_t> ack(MNM_CONT_ACK_BYTES);
            mnm_ack_encode(ack.data(),pending_->block.request.data(),unsigned(pending_->block.generation),unsigned(pending_->block.generation>>32),unsigned(channel_.nonce()),unsigned(channel_.nonce()>>32));
            channel_.reply(pending_->block.generation,ack);mark("reply");
        }}
    }catch(const std::exception& e){error_=e.what();channel_.cancel();throw;}
}
bool ResidentWorldSession::ready(std::uint64_t generation,std::uint64_t nonce) const{
    if(!error_.isEmpty() || normalQuitRetired_ || nonce!=channel_.nonce())return false;
    if(independent_)return ownedPending_ && ownedPending_->generation==generation && ownedFrame_.valid();
    return pending_ && !pending_->imported && pending_->block.generation==generation;
}
void ResidentWorldSession::pollOwned(bool acceptedNormalQuit){
    if(asynchronous_){pollAsynchronous(acceptedNormalQuit);return;}
    try{
        if(!error_.isEmpty())throw std::runtime_error(error_.toStdString());
        serviceReport();const auto queue=snapshotQueue_->stats();returns_=queue.returned;
        if(!queue.error.empty())throw std::runtime_error(queue.error);
        if(normalQuitRetired_)return;
        if(channel_.cancelled()){
            const auto p=gpuProduction_?gpuProduction_->stats():mnm::render::GpuProductionQueue::Stats{};
            if(acceptedNormalQuit && productionRetired(asynchronous_,completed_,p) && !ownedPending_ && !queue.outstanding && completed_==imports_ && completed_==queue.returned && queue.accepted==queue.returned){
                returns_=queue.returned;normalQuitRetired_=true;if(capture_)capture_->finish();saveReport(QFileInfo(game_).dir().filePath("resident-world-report.json"));return;
            }
            throw std::runtime_error("Owned World producer cancelled with outstanding work");
        }
        if(!target_.ready() || ownedPending_)return;
        const auto now=std::chrono::steady_clock::now();
        if(nextFrame_.time_since_epoch().count() && now<nextFrame_)return;
        OwnedWorldSnapshot source;if(!snapshotQueue_->take(source))return;
        const auto sourceTakenAt=frameTrace_?std::chrono::steady_clock::now():std::chrono::steady_clock::time_point{};
        if(!world_){
            renderer_=std::make_unique<mnm::render::GlBlitter>(target_.context(),target_.offscreen);auto store=mnm::assets::AssetStore::create(game_.toStdString());
            if(auto* e=std::get_if<mnm::assets::Error>(&store))throw std::runtime_error(e->detail);
            world_=std::make_unique<WorldRollingSession>(*renderer_,std::get<mnm::assets::AssetStore>(std::move(store)),false,true,true,retainCompletedImage_);
        }
        if(source.generation!=completed_+1)throw std::runtime_error("Owned native source sequence gap");
        if(capture_)capture_->enqueue(source.generation,source.wireSource,source.wireRequest);
        QElapsedTimer timing;timing.start();ownedFrame_=world_->consumeResident(source.generation,source.handoff.stream);
        const auto native=double(timing.nsecsElapsed())/1000000.;nativeMs_+=native;maxNativeMs_=std::max(maxNativeMs_,native);
        if(frameTrace_){
            lastSourceTiming_={std::chrono::duration<double,std::milli>(sourceTakenAt-source.readyAt).count(),std::chrono::duration<double,std::milli>(source.readyAt-source.acquiredAt).count(),source.decodeMs,source.prepareMs,world_->producerWorkMs(),world_->profile().prepareMs,world_->profile().submitMs,source.ticks};
        }
        if(world_->completed()!=source.generation || world_->worldReadbacks() || renderer_->stats().nativeReadbacks || renderer_->stats().rgbaReadbacks)
            throw std::runtime_error("Owned native completion/readback contract");
        decodeMs_+=source.decodeMs;handoffMs_+=source.prepareMs;producerMs_+=world_->producerWorkMs();prepareMs_+=world_->profile().prepareMs;submitMs_+=world_->profile().submitMs;++profiled_;
        if(source.generation==64)firstTick_=source.ticks[0];
        if(source.generation>=64)lastTick_=source.ticks[3];
        regionQueries_+=source.ticks[9];rasters_+=source.handoff.batch.header[14];
        completed_=source.generation;returns_=queue.returned;ownedPending_=std::move(source);
        if(completed_==64){renderRate_.start();renderRateBase_=completed_+extraFrames_;}
        if(!nextFrame_.time_since_epoch().count())nextFrame_=now;
        const auto late=std::chrono::duration_cast<std::chrono::microseconds>(now-nextFrame_).count();
        if(late>=16667){frameMisses_+=std::uint64_t(late/16667);nextFrame_=now;}
        nextFrame_+=std::chrono::microseconds(16667);
    }catch(const std::exception& e){error_=e.what();channel_.cancel();snapshotQueue_->stop();throw;}
}
void ResidentWorldSession::pollAsynchronous(bool acceptedNormalQuit){
    try{
        if(!error_.isEmpty())throw std::runtime_error(error_.toStdString());
        serviceReport();const auto queue=snapshotQueue_->stats();returns_=queue.returned;
        if(!queue.error.empty())throw std::runtime_error(queue.error);
        if(gpuProduction_ && !gpuProduction_->stats().error.empty())throw std::runtime_error(gpuProduction_->stats().error);
        if(normalQuitRetired_)return;
        if(channel_.cancelled()){
            const auto p=gpuProduction_?gpuProduction_->stats():mnm::render::GpuProductionQueue::Stats{};
            if(acceptedNormalQuit && productionRetired(asynchronous_,completed_,p) && !ownedPending_ && !queue.outstanding && completed_==imports_ && completed_==queue.returned && queue.accepted==queue.returned){
                normalQuitRetired_=true;saveReport(QFileInfo(game_).dir().filePath("resident-world-report.json"));return;
            }
            throw std::runtime_error("Async World producer cancelled with outstanding work");
        }
        if(!target_.ready())return;
        if(!gpuProduction_){
            gpuBackend_=std::make_shared<ResidentWorldGpuBackend>();auto backend=gpuBackend_;
            gpuProduction_=std::make_unique<mnm::render::GpuProductionQueue>(target_.context(),target_.worldOffscreen,[backend]{backend->world.reset();});
        }
        if(gpuProduction_->canSubmit()){
            OwnedWorldSnapshot source;
            if(snapshotQueue_->take(source)){
                if(source.generation!=submittedWorlds_+1)throw std::runtime_error("Async World submission sequence gap");
                auto job=std::make_shared<WorldGpuJob>();job->source=std::move(source);auto backend=gpuBackend_;const auto game=game_;
                const auto produce=[backend,job,game](mnm::render::GlBlitter& renderer){
                    QElapsedTimer clock;clock.start();
                    if(!backend->world){auto store=mnm::assets::AssetStore::create(game.toStdString());
                        if(auto* e=std::get_if<mnm::assets::Error>(&store))throw std::runtime_error(e->detail);
                        backend->world=std::make_unique<WorldRollingSession>(renderer,std::get<mnm::assets::AssetStore>(std::move(store)),false,true,true,true);
                    }
                    auto frame=backend->world->consumeResident(job->source.generation,job->source.handoff.stream);
                    job->nativeMs=clock.nsecsElapsed()/1000000.;job->producerMs=backend->world->producerWorkMs();job->profile=backend->world->profile();
                    job->stats=renderer.stats();job->driver=QString::fromStdString(renderer.driver().renderer);job->readbacks=backend->world->worldReadbacks();job->captures=backend->world->retainedImageCaptures();
                    if(backend->world->completed()!=job->source.generation||job->readbacks||job->stats.nativeReadbacks||job->stats.rgbaReadbacks)throw std::runtime_error("Async World completion/readback contract");
                    return frame;
                };
                if(!gpuProduction_->submit(job->source.generation,produce,job))throw std::logic_error("Async World admission changed on its owner");
                ++submittedWorlds_;
            }
        }
        if(ownedPending_)return;
        const auto now=std::chrono::steady_clock::now();
        if(!displayWhilePending_ && nextFrame_.time_since_epoch().count()&&now<nextFrame_)return;
        mnm::render::GpuProductionQueue::Completion result;if(!gpuProduction_->take(result))return;
        if(result.sequence!=completed_+1)throw std::runtime_error("Async World completion sequence gap");
        auto job=std::static_pointer_cast<WorldGpuJob>(result.metadata);auto& source=job->source;
        const double production=std::chrono::duration<double,std::milli>(result.completed-result.started).count();
        productionMs_+=production;maxProductionMs_=std::max(maxProductionMs_,production);
        producerReadbacks_=job->readbacks;producerCaptures_=job->captures;
        nativeMs_+=job->nativeMs;maxNativeMs_=std::max(maxNativeMs_,job->nativeMs);producerStats_=job->stats;producerDriver_=job->driver;
        decodeMs_+=source.decodeMs;handoffMs_+=source.prepareMs;producerMs_+=job->producerMs;prepareMs_+=job->profile.prepareMs;submitMs_+=job->profile.submitMs;++profiled_;
        if(frameTrace_)lastSourceTiming_={std::chrono::duration<double,std::milli>(result.started-source.readyAt).count(),std::chrono::duration<double,std::milli>(source.readyAt-source.acquiredAt).count(),source.decodeMs,source.prepareMs,job->producerMs,job->profile.prepareMs,job->profile.submitMs,source.ticks};
        regionQueries_+=source.ticks[9];rasters_+=source.handoff.batch.header[14];completed_=result.sequence;ownedFrame_=std::move(result.frame);ownedPending_=std::move(source);
        if(!displayWhilePending_){
        if(completed_==64){renderRate_.start();renderRateBase_=completed_+extraFrames_;}
        if(!nextFrame_.time_since_epoch().count())nextFrame_=now;
        const auto late=std::chrono::duration_cast<std::chrono::microseconds>(now-nextFrame_).count();
        if(late>=16667){frameMisses_+=std::uint64_t(late/16667);nextFrame_=now;}nextFrame_+=std::chrono::microseconds(16667);
        }
    }catch(const std::exception& e){error_=e.what();channel_.cancel();snapshotQueue_->stop();throw;}
}
mnm::render::GpuFrame ResidentWorldSession::drawIndependent(std::uint64_t generation){
    if(!independent_ || normalQuitRetired_ || !error_.isEmpty() || (ownedPending_&&!displayWhilePending_) || (!world_&&!asynchronous_) || generation!=(displayWhilePending_?imports_:completed_) || (displayWhilePending_&&!displayedFrame_.valid()) || channel_.cancelled())return {};
    const auto now=std::chrono::steady_clock::now();if(now<nextFrame_)return {};
    const auto late=std::chrono::duration_cast<std::chrono::microseconds>(now-nextFrame_).count();
    if(late>=16667){frameMisses_+=std::uint64_t(late/16667);nextFrame_=now;}
    nextFrame_+=std::chrono::microseconds(16667);
    try{QElapsedTimer timing;timing.start();auto frame=displayWhilePending_?displayedFrame_:asynchronous_?ownedFrame_:world_->replayPrepared();if(displayWhilePending_&&ownedPending_)++pendingRedraws_;extraWorldMs_+=timing.nsecsElapsed()/1000000.0;++extraFrames_;return frame;}
    catch(const std::exception& e){error_=e.what();channel_.cancel();snapshotQueue_->stop();throw;}
}
void ResidentWorldSession::noteIndependentPresentation(double milliseconds){
    if(!independent_ || extraPresented_>=extraFrames_ || milliseconds<0)throw std::logic_error("Independent native presentation accounting");
    ++extraPresented_;extraUiMs_+=milliseconds;maxExtraUiMs_=std::max(maxExtraUiMs_,milliseconds);
}
mnm::render::GpuFrame ResidentWorldSession::import(std::uint64_t generation,std::uint64_t nonce){
    if(independent_){
        if(!ready(generation,nonce))throw std::runtime_error("Owned World command/import identity");
        auto frame=ownedFrame_;snapshotQueue_->imported(generation);++imports_;ownedPending_.reset();
        if(displayWhilePending_){
            displayedFrame_=frame;
            if(generation==64){renderRate_.start();renderRateBase_=nativeFrames();}
            const auto now=std::chrono::steady_clock::now();
            if(nextFrame_.time_since_epoch().count()){
                const auto late=std::chrono::duration_cast<std::chrono::microseconds>(now-nextFrame_).count();
                if(late>=16667)frameMisses_+=std::uint64_t(late/16667);
            }
            nextFrame_=now+std::chrono::microseconds(16667);
        }
        if(completed_%64==0)queueReport(QFileInfo(game_).dir().filePath("resident-world-report.json"));
        return frame;
    }

    if(!error_.isEmpty() || normalQuitRetired_ || !pending_ || pending_->imported || generation!=pending_->block.generation || nonce!=channel_.nonce())
        throw std::runtime_error("Resident World command/import identity");
    pending_->imported=true;++imports_;return pending_->frame;
}
QJsonObject ResidentWorldSession::report() const {
    const auto stats=asynchronous_?producerStats_:renderer_?renderer_->stats():mnm::render::RenderStats{};
    const auto owned=snapshotQueue_?snapshotQueue_->stats():WorldSnapshotQueue::Stats{};
    QJsonObject result{{"completed",QString::number(completed_)},{"imports",QString::number(imports_)},{"verified_returns",QString::number(returns_)},{"rasters",QString::number(rasters_)},
        {"world_readbacks",double(asynchronous_?producerReadbacks_:world_?world_->worldReadbacks():0)},{"native_readbacks",double(stats.nativeReadbacks)},{"rgba_readbacks",double(stats.rgbaReadbacks)},
        {"nonce",QString::number(channel_.nonce())},{"error",error_},{"pending",bool(pending_)||bool(ownedPending_)||(independent_&&owned.outstanding!=0)},{"normal_quit_retired",normalQuitRetired_},{"renderer",asynchronous_?producerDriver_:renderer_?QString::fromStdString(renderer_->driver().renderer):QString()},
        {"draw_calls",QString::number(stats.drawCalls)},{"draw_instances",QString::number(stats.drawInstances)},
        {"texture_upload_calls",QString::number(stats.textureUploadCalls)},{"texture_upload_bytes",QString::number(stats.textureUploadBytes)},
        {"program_changes",QString::number(stats.programChanges)},{"framebuffer_attachments",QString::number(stats.framebufferAttachments)},
        {"reply_bytes_per_world",int(MNM_CONT_ACK_BYTES)},{"world_pixel_reply_bytes",0},
        {"actual_region_queries_per_world",returns_?double(regionQueries_)/returns_:0.},{"profiled_worlds",double(profiled_)},{"decode_ms_per_world",profiled_?decodeMs_/profiled_:0.},
        {"handoff_ms_per_world",profiled_?handoffMs_/profiled_:0.},{"native_ms_per_world",profiled_?nativeMs_/profiled_:0.},
        {"producer_ms_per_world",profiled_?producerMs_/profiled_:0.},{"prepare_ms_per_world",profiled_?prepareMs_/profiled_:0.},{"submit_ms_per_world",profiled_?submitMs_/profiled_:0.},{"max_native_ms",maxNativeMs_},
        {"engine_fps",returns_>64 && lastTick_!=firstTick_?double(returns_-64)*1000/double(std::uint32_t(lastTick_-firstTick_)):0.}};
    if(independent_){
        result["pending_world_display"]=QJsonObject{{"enabled",displayWhilePending_},{"policy","last-imported-native-world"},{"generation",QString::number(imports_)},{"redraws",QString::number(pendingRedraws_)},{"scope","Redraw only last imported immutable World with matching sealed UI while newer completed generation awaits command import. No source credit/import/return/order or simulation advance."}};
        if(asynchronous_){const auto p=gpuProduction_?gpuProduction_->stats():mnm::render::GpuProductionQueue::Stats{};
            result["world_gpu_production"]=QJsonObject{{"policy","dedicated-native-owner"},{"submitted",QString::number(p.submitted)},{"produced",QString::number(p.produced)},{"taken",QString::number(p.taken)},{"outstanding",int(p.outstanding)},{"high_water",int(p.highWater)},{"immutable_allocations",QString::number(p.allocations)},{"error",QString::fromStdString(p.error)},{"mean_cpu_ms",profiled_?productionMs_/profiled_:0.},{"max_cpu_ms",maxProductionMs_},{"scope","Completed native World snapshots; two source credits and two immutable GPU slots. Repeated cached images do not advance simulation. CPU wall includes driver waits; presentation owner is separate."}};
        }
        result["delivery_mode"]="owned-v5";result["source_accepted"]=QString::number(owned.accepted);result["source_returned"]=QString::number(owned.returned);
        result["source_outstanding"]=int(owned.outstanding);result["source_high_water"]=int(owned.highWater);
        result["native_extra_world_frames"]=QString::number(extraFrames_);
        result["world_repeat_policy"]=retainCompletedImage_?"retained-native-image":"prepared-native-draws";
        result["retained_world_image_captures"]=QString::number(asynchronous_?producerCaptures_:world_?world_->retainedImageCaptures():0);
        result["native_extra_presented_frames"]=QString::number(extraPresented_);
        result["mean_extra_world_cpu_ms"]=extraFrames_?extraWorldMs_/extraFrames_:0;
        result["mean_extra_ui_cpu_ms"]=extraPresented_?extraUiMs_/extraPresented_:0;
        result["max_extra_ui_cpu_ms"]=maxExtraUiMs_;result["native_total_world_frames"]=QString::number(completed_+extraFrames_);
        result["native_frame_deadlines_missed"]=QString::number(frameMisses_);
        result["native_render_fps"]=renderRate_.isValid()&&renderRate_.elapsed()>0?double(nativeFrames()-renderRateBase_)*1000./renderRate_.elapsed():0.;
        result["engine_fps"]=owned.returned>64 && owned.lastTick!=owned.firstTick?double(owned.returned-64)*1000./double(std::uint32_t(owned.lastTick-owned.firstTick)):0.;
        result["scope"]="Owned source rate and independent native drawing are separate; repeated frames do not interpolate simulation state.";
    }
    const auto capture=capture_?capture_->report():QJsonObject{};
    if(stageProfile_)result["stage_profile"]=QJsonObject{{"samples",stageSamples_},{"sample_limit",8192},
        {"samples_dropped",double(samplesDropped_)},{"gpu_queries_dropped",double(gpuDropped_)},
        {"gpu_queries_pending",double(renderer_?renderer_->pendingGpuTimings():0)},
        {"capture",capture},{"capture_path",capture["path"]},{"capture_bytes",capture["file_bytes"]},
        {"captured_worlds",capture["written_worlds"]},{"capture_truncated",capture["truncated"]},
        {"capture_complete",bool(capture_ && normalQuitRetired_ && capture["complete"].toBool() && std::uint64_t(capture["written_worlds"].toDouble())==completed_)},
        {"scope","Opt-in bounded host timings and asynchronous renderer-context GPU timestamps. GPU timeline includes CPU submission gaps, excludes public UI import/paint/scanout. Existing original ticks are uint32 millisecond intervals; pacing and screen-copy fields describe the preceding interval. Non-pacing residual is unclassified original work, not simulation CPU time. Capture I/O and instrumentation affect live cadence."}};
    return result;
}
namespace {
void publishReport(const QString& path,const QByteArray& bytes){
    QSaveFile file(path);
    if(!file.open(QIODevice::WriteOnly)||file.write(bytes)!=bytes.size()||!file.commit())throw std::runtime_error("Resident World report publication");
}
}
void ResidentWorldSession::launchReport(ReportSnapshot snapshot){
    reportWork_=std::async(std::launch::async,[snapshot=std::move(snapshot)]{publishReport(snapshot.path,snapshot.bytes);});
}
void ResidentWorldSession::serviceReport(){
    if(!reportWork_.valid()||reportWork_.wait_for(std::chrono::seconds(0))!=std::future_status::ready)return;
    reportWork_.get();
    if(latestReport_){auto snapshot=std::move(*latestReport_);latestReport_.reset();launchReport(std::move(snapshot));}
}
void ResidentWorldSession::queueReport(const QString& path){
    auto bytes=QJsonDocument(report()).toJson(QJsonDocument::Compact);
    if(bytes.size()>32*1024*1024)throw std::runtime_error("Resident report snapshot budget");
    serviceReport();
    if(reportWork_.valid())latestReport_=ReportSnapshot{path,std::move(bytes)};
    else launchReport({path,std::move(bytes)});
}
void ResidentWorldSession::saveReport(const QString& path){
    if(reportWork_.valid())reportWork_.get();
    latestReport_.reset();publishReport(path,QJsonDocument(report()).toJson());
}
