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
ResidentWorldSession::ResidentWorldSession(GlViewport& viewport,const QString& path,const QString& game):viewport_(viewport),channel_(create(path)),game_(game){}
ResidentWorldSession::~ResidentWorldSession(){channel_.cancel();}
void ResidentWorldSession::abort(const QString& error){error_=error;channel_.cancel();}
void ResidentWorldSession::poll(){
    try {
        if(!error_.isEmpty())throw std::runtime_error(error_.toStdString());
        if(!viewport_.ready())return;
        if(!world_){
            renderer_=std::make_unique<mnm::render::GlBlitter>(viewport_.context());auto store=mnm::assets::AssetStore::create(game_.toStdString());
            if(auto* e=std::get_if<mnm::assets::Error>(&store))throw std::runtime_error(e->detail);
            world_=std::make_unique<WorldRollingSession>(*renderer_,std::get<mnm::assets::AssetStore>(std::move(store)),false,true);
        }
        if(channel_.cancelled())throw std::runtime_error("Resident World producer cancelled");
        if(pending_){
            if(pending_->deadline.elapsed()>120000)throw std::runtime_error("Resident World import/return deadline");
            if(!pending_->returned){std::array<std::uint8_t,192> actual{};
                if(channel_.returnRecords(pending_->block.generation,pending_->block.source,actual)){
                    verifyRollingHandoffReturn(pending_->handoff,close(actual,0),close(actual,96),true);
                    pending_->returned=true;++returns_;rasters_+=pending_->handoff.batch.header[14];
                    const auto ticks=channel_.ticks(pending_->block.generation);if(returns_==64)firstTick_=ticks[0];if(returns_>=64)lastTick_=ticks[3];
                }
            }
            if(pending_->returned && pending_->imported){completed_=pending_->block.generation;channel_.release(completed_);pending_.reset();if(completed_%64==0)saveReport(QFileInfo(game_).dir().filePath("resident-world-report.json"));}
        }
        if(!pending_){WorldContinuousChannel::Block block;if(channel_.poll(block)){
            QElapsedTimer timing;timing.start();
            auto stream=decodeCanvasProducers(block.source,false);
            decodeMs_+=double(timing.nsecsElapsed())/1000000.;timing.restart();
            auto handoff=prepareRollingHandoff(stream,0,block.request,1,0,WorldBatchSourcePolicy::ownedV5,false);
            handoffMs_+=double(timing.nsecsElapsed())/1000000.;timing.restart();
            auto frame=world_->consumeResident(block.generation,handoff.stream);
            const auto native=double(timing.nsecsElapsed())/1000000.;nativeMs_+=native;maxNativeMs_=std::max(maxNativeMs_,native);
            prepareMs_+=world_->profile().prepareMs;submitMs_+=world_->profile().submitMs;producerMs_+=world_->producerWorkMs();++profiled_;
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
    if(!error_.isEmpty() || !pending_ || pending_->imported || generation!=pending_->block.generation || nonce!=channel_.nonce())
        throw std::runtime_error("Resident World command/import identity");
    pending_->imported=true;++imports_;return pending_->frame;
}
QJsonObject ResidentWorldSession::report() const {
    const auto stats=renderer_?renderer_->stats():mnm::render::RenderStats{};
    return {{"completed",QString::number(completed_)},{"imports",QString::number(imports_)},{"verified_returns",QString::number(returns_)},{"rasters",QString::number(rasters_)},
        {"world_readbacks",double(world_?world_->worldReadbacks():0)},{"native_readbacks",double(stats.nativeReadbacks)},{"rgba_readbacks",double(stats.rgbaReadbacks)},
        {"nonce",QString::number(channel_.nonce())},{"error",error_},{"pending",bool(pending_)},{"renderer",renderer_?QString::fromStdString(renderer_->driver().renderer):QString()},
        {"reply_bytes_per_world",int(MNM_CONT_ACK_BYTES)},{"world_pixel_reply_bytes",0},
        {"profiled_worlds",double(profiled_)},{"decode_ms_per_world",profiled_?decodeMs_/profiled_:0.},
        {"handoff_ms_per_world",profiled_?handoffMs_/profiled_:0.},{"native_ms_per_world",profiled_?nativeMs_/profiled_:0.},
        {"producer_ms_per_world",profiled_?producerMs_/profiled_:0.},{"prepare_ms_per_world",profiled_?prepareMs_/profiled_:0.},{"submit_ms_per_world",profiled_?submitMs_/profiled_:0.},{"max_native_ms",maxNativeMs_},
        {"engine_fps",returns_>64 && lastTick_!=firstTick_?double(returns_-64)*1000/double(std::uint32_t(lastTick_-firstTick_)):0.}};
}
void ResidentWorldSession::saveReport(const QString& path) const {QSaveFile file(path);if(!file.open(QIODevice::WriteOnly) || file.write(QJsonDocument(report()).toJson())<0 || !file.commit())throw std::runtime_error("Resident World report publication");}
