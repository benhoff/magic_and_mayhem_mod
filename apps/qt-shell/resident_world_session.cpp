#include "resident_world_session.hpp"
#include "gl_viewport.hpp"
#include <QJsonDocument>
#include <QSaveFile>
#include <QFileInfo>
#include <QDir>
#include <QRandomGenerator>
#include <stdexcept>
#include <algorithm>
using namespace mnm::legacy;
namespace {
QString create(const QString& path){auto nonce=QRandomGenerator::system()->generate64();if(!nonce)nonce=1;WorldContinuousChannel::create(path,nonce,0,true);return path;}
CanvasProducer close(const std::array<std::uint8_t,192>& bytes,unsigned offset){
    CanvasProducer result{};for(unsigned i=0;i<24;++i){const auto* p=bytes.data()+offset+i*4;result.fields[i]=p[0]|unsigned(p[1])<<8|unsigned(p[2])<<16|unsigned(p[3])<<24;}
    if(result.fields[0]!=96 || result.fields[18] || result.fields[23])throw std::runtime_error("Resident World closing record");
    return result;
}
}
ResidentWorldSession::ResidentWorldSession(GlViewport& viewport,const QString& path,const QString& game):viewport_(viewport),channel_(create(path)),game_(game){
    stageProfile_=qEnvironmentVariableIsSet("MNM_RESIDENT_STAGE_PROFILE");profileClock_.start();
    const auto capture=qEnvironmentVariable("MNM_RESIDENT_PROFILE_CAPTURE");
    if(!capture.isEmpty()){
        if(!stageProfile_)throw std::runtime_error("Resident capture requires stage profiling");
        capture_=std::make_unique<QFile>(capture);
        if(!capture_->open(QIODevice::WriteOnly|QIODevice::NewOnly) || capture_->write("MNMPROF1",8)!=8)
            throw std::runtime_error("Cannot create resident profile corpus");
        captureBytes_=8;
    }
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
ResidentWorldSession::~ResidentWorldSession(){channel_.cancel();}
void ResidentWorldSession::abort(const QString& error){error_=error;channel_.cancel();}
void ResidentWorldSession::poll(bool acceptedNormalQuit){
    try {
        if(!error_.isEmpty())throw std::runtime_error(error_.toStdString());
        collectGpuTimings();
        if(normalQuitRetired_)return;
        if(channel_.cancelled()){
            if(acceptedNormalQuit && !pending_ && completed_==imports_ && completed_==returns_){
                normalQuitRetired_=true;if(capture_ && !capture_->flush())throw std::runtime_error("Resident capture flush");saveReport(QFileInfo(game_).dir().filePath("resident-world-report.json"));return;
            }
            throw std::runtime_error("Resident World producer cancelled");
        }
        if(!viewport_.ready())return;
        if(!world_){
            renderer_=std::make_unique<mnm::render::GlBlitter>(viewport_.context());auto store=mnm::assets::AssetStore::create(game_.toStdString());
            if(auto* e=std::get_if<mnm::assets::Error>(&store))throw std::runtime_error(e->detail);
            world_=std::make_unique<WorldRollingSession>(*renderer_,std::get<mnm::assets::AssetStore>(std::move(store)),false,true);
        }
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
            if(pending_->returned && pending_->imported){completed_=pending_->block.generation;channel_.release(completed_);pending_.reset();if(completed_%64==0)saveReport(QFileInfo(game_).dir().filePath("resident-world-report.json"));}
        }
        if(!pending_){WorldContinuousChannel::Block block;if(channel_.poll(block)){
            const bool sampled=stageProfile_ && block.generation<=8192;
            const double sampleStart=double(profileClock_.nsecsElapsed())/1000000.;
            QElapsedTimer timing;timing.start();double captureIo=0;
            if(capture_ && !captureTruncated_){
                const auto size=16ull+block.source.size()+block.request.size();
                if(block.generation>8192 || size>16ull*1024*1024*1024-captureBytes_)captureTruncated_=true;
                else {
                    QByteArray header(16,0);const std::uint64_t values[]={block.generation,block.source.size(),block.request.size()};
                    for(unsigned i=0;i<8;++i)header[int(i)]=char(values[0]>>(i*8));
                    for(unsigned k=1;k<3;++k)for(unsigned i=0;i<4;++i)header[int(4+k*4+i)]=char(values[k]>>(i*8));
                    if(capture_->write(header)!=16 || capture_->write(reinterpret_cast<const char*>(block.source.data()),qint64(block.source.size()))!=qint64(block.source.size()) || capture_->write(reinterpret_cast<const char*>(block.request.data()),qint64(block.request.size()))!=qint64(block.request.size()))
                        throw std::runtime_error("Resident profile capture write");
                    captureBytes_+=size;++capturedWorlds_;
                }
                captureIo=double(timing.nsecsElapsed())/1000000.;timing.restart();
            }
            auto stream=decodeCanvasProducers(block.source,false);
            const auto decode=double(timing.nsecsElapsed())/1000000.;decodeMs_+=decode;timing.restart();
            auto handoff=prepareRollingHandoff(stream,0,block.request,1,0,WorldBatchSourcePolicy::ownedV5,false);
            const auto handoffTime=double(timing.nsecsElapsed())/1000000.;handoffMs_+=handoffTime;timing.restart();
            mnm::render::GpuFrame frame;
            if(sampled){if(!renderer_->profileBatch(block.generation,[&]{frame=world_->consumeResident(block.generation,handoff.stream);}))++gpuDropped_;}
            else frame=world_->consumeResident(block.generation,handoff.stream);
            const auto native=double(timing.nsecsElapsed())/1000000.;nativeMs_+=native;maxNativeMs_=std::max(maxNativeMs_,native);
            prepareMs_+=world_->profile().prepareMs;submitMs_+=world_->profile().submitMs;producerMs_+=world_->producerWorkMs();++profiled_;
            if(sampled)stageSamples_.append(QJsonObject{{"generation",QString::number(block.generation)},
                {"host_elapsed_ms",sampleStart},{"capture_io_cpu_ms",captureIo},{"decode_ms",decode},{"handoff_ms",handoffTime},
                {"native_consume_ms",native},{"native_cpu_producer_ms",world_->producerWorkMs()},
                {"native_prepare_ms",world_->profile().prepareMs},{"native_submit_cpu_ms",world_->profile().submitMs},
                {"world_rasters",int(handoff.batch.header[14])},{"source_bytes",double(block.source.size())}});
            else if(stageProfile_)++samplesDropped_;
            if(world_->completed()!=block.generation || world_->worldReadbacks() || renderer_->stats().nativeReadbacks || renderer_->stats().rgbaReadbacks)
                throw std::runtime_error("Resident World completion/readback contract");
            Pending pending{std::move(block),std::move(handoff),std::move(frame),false,false,{}};pending.deadline.start();pending_=std::move(pending);
            std::vector<std::uint8_t> ack(MNM_CONT_ACK_BYTES);
            mnm_ack_encode(ack.data(),pending_->block.request.data(),unsigned(pending_->block.generation),unsigned(pending_->block.generation>>32),unsigned(channel_.nonce()),unsigned(channel_.nonce()>>32));
            channel_.reply(pending_->block.generation,ack);
        }}
    }catch(const std::exception& e){error_=e.what();channel_.cancel();throw;}
}
mnm::render::GpuFrame ResidentWorldSession::import(std::uint64_t generation,std::uint64_t nonce){
    if(!error_.isEmpty() || normalQuitRetired_ || !pending_ || pending_->imported || generation!=pending_->block.generation || nonce!=channel_.nonce())
        throw std::runtime_error("Resident World command/import identity");
    pending_->imported=true;++imports_;return pending_->frame;
}
QJsonObject ResidentWorldSession::report() const {
    const auto stats=renderer_?renderer_->stats():mnm::render::RenderStats{};
    QJsonObject result{{"completed",QString::number(completed_)},{"imports",QString::number(imports_)},{"verified_returns",QString::number(returns_)},{"rasters",QString::number(rasters_)},
        {"world_readbacks",double(world_?world_->worldReadbacks():0)},{"native_readbacks",double(stats.nativeReadbacks)},{"rgba_readbacks",double(stats.rgbaReadbacks)},
        {"nonce",QString::number(channel_.nonce())},{"error",error_},{"pending",bool(pending_)},{"normal_quit_retired",normalQuitRetired_},{"renderer",renderer_?QString::fromStdString(renderer_->driver().renderer):QString()},
        {"reply_bytes_per_world",int(MNM_CONT_ACK_BYTES)},{"world_pixel_reply_bytes",0},
        {"actual_region_queries_per_world",returns_?double(regionQueries_)/returns_:0.},{"profiled_worlds",double(profiled_)},{"decode_ms_per_world",profiled_?decodeMs_/profiled_:0.},
        {"handoff_ms_per_world",profiled_?handoffMs_/profiled_:0.},{"native_ms_per_world",profiled_?nativeMs_/profiled_:0.},
        {"producer_ms_per_world",profiled_?producerMs_/profiled_:0.},{"prepare_ms_per_world",profiled_?prepareMs_/profiled_:0.},{"submit_ms_per_world",profiled_?submitMs_/profiled_:0.},{"max_native_ms",maxNativeMs_},
        {"engine_fps",returns_>64 && lastTick_!=firstTick_?double(returns_-64)*1000/double(std::uint32_t(lastTick_-firstTick_)):0.}};
    if(stageProfile_)result["stage_profile"]=QJsonObject{{"samples",stageSamples_},{"sample_limit",8192},
        {"samples_dropped",double(samplesDropped_)},{"gpu_queries_dropped",double(gpuDropped_)},
        {"gpu_queries_pending",double(renderer_?renderer_->pendingGpuTimings():0)},
        {"capture_path",capture_?capture_->fileName():QString()},{"capture_bytes",double(captureBytes_)},
        {"captured_worlds",double(capturedWorlds_)},{"capture_truncated",captureTruncated_},
        {"capture_complete",bool(capture_ && normalQuitRetired_ && !captureTruncated_ && capturedWorlds_==completed_)},
        {"scope","Opt-in bounded host timings and asynchronous renderer-context GPU timestamps. GPU timeline includes CPU submission gaps, excludes public UI import/paint/scanout. Existing original ticks are uint32 millisecond intervals; pacing and screen-copy fields describe the preceding interval. Non-pacing residual is unclassified original work, not simulation CPU time. Capture I/O and instrumentation affect live cadence."}};
    return result;
}
void ResidentWorldSession::saveReport(const QString& path) const {QSaveFile file(path);if(!file.open(QIODevice::WriteOnly) || file.write(QJsonDocument(report()).toJson())<0 || !file.commit())throw std::runtime_error("Resident World report publication");}
