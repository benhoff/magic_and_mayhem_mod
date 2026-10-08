#include "live_world_session.hpp"
#include <QElapsedTimer>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <stdexcept>
namespace {
mnm::assets::AssetStore store(const QString& root){auto configured=mnm::assets::AssetStore::create(root.toStdString());if(auto* e=std::get_if<mnm::assets::Error>(&configured))throw std::runtime_error(e->detail);return std::get<mnm::assets::AssetStore>(std::move(configured));}
}
LiveWorldSession::LiveWorldSession(GlViewport& viewport,const QString& path,const QString& root,const QString& diagnostics,mnm::legacy::WorldPreparationCheckpoint checkpoint):
    viewport_(viewport),channel_(path),store_(store(root)),resources_(store_),preparation_(store_,std::move(checkpoint)),diagnostics_(diagnostics){
    if(!diagnostics_.isEmpty()){
        const auto directory=std::filesystem::canonical(diagnostics_.toStdString());const auto assetRoot=std::filesystem::canonical(store_.root());
        if(directory==assetRoot||std::mismatch(assetRoot.begin(),assetRoot.end(),directory.begin(),directory.end()).first==assetRoot.end())throw std::invalid_argument("Diagnostics must be outside asset root");
    }
}
LiveWorldSession::~LiveWorldSession(){close();}
bool LiveWorldSession::poll(){
    if(closed_)return false;
    QElapsedTimer timer;timer.start();
    try{
        if(!viewport_.error().isEmpty())throw std::runtime_error(viewport_.error().toStdString());
        if(!viewport_.ready())return true;
        if(!pending_){pending_=channel_.poll();if(!pending_)return true;frameTimer_.start();
            if(const auto refusal=mnm::legacy::decodeWorldRefusal(pending_->inputs)){
                const auto* input=reinterpret_cast<const unsigned char*>(pending_->inputs.constData());
                const auto sequence=quint32(input[20])|quint32(input[21])<<8|quint32(input[22])<<16|quint32(input[23])<<24;
                if(sequence!=pending_->sequence||!pending_->oracle.isEmpty()||channel_.verify()||channel_.history())throw std::runtime_error("Invalid World refusal mode or sequence");
                viewport_.setGpuFrame({});++refused_;
                const auto key=QString::number(*refusal);refusalCounts_[key]=refusalCounts_.value(key).toInteger()+1;
                if(refusals_.size()<64)refusals_.append(QJsonObject{{"sequence",qint64(sequence)},{"reason",int(*refusal)}});
                status_=*refusal==MNM_WORLD_UNSUPPORTED_WAVE?"Waiting for World drawing to initialize.":"World drawing is not supported yet. Use the original game window.";
                pending_.reset();maxPollMs_=std::max(maxPollMs_,timer.elapsed());return true;
            }
            preparation_.plan(pending_->inputs);status_="Preparing World assets.";
            maxPollMs_=std::max(maxPollMs_,timer.elapsed());return true;
        }
        if(!drawing_){
            auto reply=preparation_.take();if(!reply){maxPollMs_=std::max(maxPollMs_,timer.elapsed());return true;}
            if(const auto* error=std::get_if<std::string>(&*reply))throw std::runtime_error(*error);
            if(auto* plan=std::get_if<mnm::legacy::WorldPreparationPlan>(&*reply)){
                frame_=plan->frame;
                if(frame_->sequence!=pending_->sequence)throw std::runtime_error("World packet sequence differs from owned inputs");
                if(channel_.verify()&&quint64(pending_->oracle.size())!=quint64(frame_->width)*frame_->height*2)throw std::runtime_error("Missing complete original World oracle");
                std::vector<mnm::assets::ResourcePreparation> requests;
                for(const auto& file:plan->files){resources_.bind(file.id,{mnm::assets::ResourceImageFormat::sprite,file.path,{},{},{}});
                    if(!resources_.isResident(file.id))requests.push_back(resources_.request(file.id));}
                const auto resident=resources_.stats();const mnm::assets::ResourceLimits limits;
                preparation_.prepare(std::move(requests),limits.decodedBytes-resident.decodedBytes,limits.residentResources-resident.residentResources);
                maxPollMs_=std::max(maxPollMs_,timer.elapsed());return true;
            }
            auto& prepared=std::get<mnm::legacy::PreparedWorld>(*reply);
            if(prepared.frame!=frame_)throw std::runtime_error("Stale prepared World frame");
            for(auto& resource:prepared.resources)resources_.adopt(std::move(resource));
            const QSize size(int(frame_->width),int(frame_->height));
            if(!renderer_)renderer_=std::make_unique<mnm::render::GlBlitter>(viewport_.context());
            if(!scene_||size!=size_){viewport_.setGpuFrame({});collectUploads();scene_.reset();
                mnm::render::SceneLimits limits;limits.cache.residentOnly=true;limits.cache.preparedOnly=true;limits.cache.indexedAtlas=true;
                scene_=std::make_unique<mnm::render::SceneHistory>(*renderer_,resources_,mnm::render::Image{size.width(),size.height(),std::vector<std::uint32_t>(std::size_t(size.width())*size.height(),0)},limits);size_=size;}
            scene_->beginFrame({pending_->canvas,pending_->sourceSequence},prepared.draws,pending_->reset);drawing_=true;
            maxPollMs_=std::max(maxPollMs_,timer.elapsed());return true;
        }
        if(uploadPending_){
            auto reply=preparation_.take();if(!reply){maxPollMs_=std::max(maxPollMs_,timer.elapsed());return true;}
            if(const auto* error=std::get_if<std::string>(&*reply))throw std::runtime_error(*error);
            auto& upload=std::get<mnm::legacy::PreparedWorldUpload>(*reply);
            if(upload.frame!=frame_)throw std::runtime_error("Stale World upload completion");
            scene_->supplyUpload(upload.need,std::move(upload.planes));uploadPending_=false;
            maxPollMs_=std::max(maxPollMs_,timer.elapsed());return true;
        }
        if(!scene_->drawNext(mnm::render::SceneBatchBudget{})){
            if(auto need=scene_->uploadNeed()){preparation_.upload(*need);uploadPending_=true;}
            maxPollMs_=std::max(maxPollMs_,timer.elapsed());return true;
        }
        const auto& frame=*frame_;const auto& packet=pending_;const auto size=size_;
        QJsonObject operations;for(const auto& draw:frame.draws){const auto key=QString::number(int(draw.composite.mode));operations[key]=operations.value(key).toInt()+1;}
        QJsonObject record{{"sequence",qint64(frame.sequence)},{"draws",qint64(frame.draws.size())},{"width",size.width()},{"height",size.height()},{"operations",operations},{"canvas",qint64(packet->canvas)},{"source_queue",qint64(packet->sourceSequence)},{"native_zero_reset",packet->reset},{"frame_processing_ms",frameTimer_.elapsed()}};
        if(channel_.verify()){
            const auto native=scene_->read();QByteArray bytes;bytes.reserve(int(native.pixels.size()*2));
            quint64 mismatch=0;QJsonArray samples;
            for(std::size_t i=0;i<native.pixels.size();++i){const auto word=native.pixels[i];bytes.append(char(word));bytes.append(char(word>>8));
                const auto original=quint32(quint8(packet->oracle[qsizetype(i*2)]))|(quint32(quint8(packet->oracle[qsizetype(i*2+1)]))<<8);
                if(word!=original){++mismatch;if(samples.size()<64)samples.append(QJsonObject{{"x",int(i%frame.width)},{"y",int(i/frame.width)},{"native",int(word)},{"original",int(original)}});}}
            compared_+=native.pixels.size();mismatches_+=mismatch;
            record["mismatches"]=qint64(mismatch);record["native_sha256"]=QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex());
            if(mismatch)record["mismatch_samples"]=samples;
            if(!diagnostics_.isEmpty()&&(mismatch||presentations_==0)){
                const auto prefix=QDir(diagnostics_).filePath(QString("world-%1").arg(frame.sequence));
                for(const auto& output:std::vector<std::pair<QString,QByteArray>>{{".bin",packet->inputs},{".original.565",packet->oracle},{".native.565",bytes}}){
                    QFile file(prefix+output.first);if(!file.open(QIODevice::NewOnly|QIODevice::WriteOnly)||file.write(output.second)!=output.second.size()||!file.flush())throw std::runtime_error("Cannot retain new World diagnostics");}
            }
            if(mismatch){frames_.append(record);throw std::runtime_error("Native World differs from original; frame refused");}
        }
        viewport_.setGpuFrame(scene_->presentGpu());channel_.presented(frame.sequence);++presentations_;
        status_="Native World preview. Use the original game window for input.";
        // Keep finite diagnostics even when the session is used interactively.
        if(frames_.size()<256)frames_.append(record);
        pending_.reset();frame_.reset();drawing_=false;
        maxPollMs_=std::max(maxPollMs_,timer.elapsed());return true;
    }catch(const std::exception& e){error_=QString::fromUtf8(e.what());close();return false;}
}
void LiveWorldSession::collectUploads(){if(!scene_)return;const auto current=scene_->uploadStats();uploadStats_.bytes+=current.bytes;uploadStats_.ticks+=current.ticks;uploadStats_.maxTickBytes=std::max(uploadStats_.maxTickBytes,current.maxTickBytes);}
void LiveWorldSession::close(){if(closed_)return;closed_=true;preparation_.cancel();channel_.cancel();viewport_.setGpuFrame({});pending_.reset();frame_.reset();collectUploads();scene_.reset();resources_.unloadAll();
    if(renderer_){const auto stats=renderer_->stats();readbacks_=stats.nativeReadbacks;remaining_=stats.surfaces;renderer_.reset();}}
QJsonObject LiveWorldSession::report() const{
    const auto stats=renderer_?renderer_->stats():mnm::render::RenderStats{};
    const auto preparation=preparation_.stats();
    auto uploads=uploadStats_;if(scene_){const auto current=scene_->uploadStats();uploads.bytes+=current.bytes;uploads.ticks+=current.ticks;uploads.maxTickBytes=std::max(uploads.maxTickBytes,current.maxTickBytes);}
    return {{"success",error_.isEmpty()&&presentations_>0},{"error",error_},{"presentations",int(presentations_)},{"verify",channel_.verify()},{"history",channel_.history()},{"target_frames",int(channel_.targetFrames())},
        {"pixels_compared",qint64(compared_)},{"mismatches",qint64(mismatches_)},{"native_readbacks",qint64(renderer_?stats.nativeReadbacks:readbacks_)},
        {"viewport_image_uploads",qint64(viewport_.imageUploads())},{"remaining_surfaces",qint64(renderer_?stats.surfaces:remaining_)},
        {"dropped",int(channel_.dropped())},{"superseded",int(channel_.superseded())},{"producer_state",int(channel_.state())},{"producer_reason",int(channel_.reason())},
        {"max_poll_ms",maxPollMs_},{"catalogue_ms",qint64(preparation.catalogueMs)},{"asset_prepare_ms",qint64(preparation.prepareMs)},{"resources_prepared",qint64(preparation.resourcesPrepared)},
        {"upload_prepare_ms",qint64(preparation.uploadPrepareMs)},{"uploads_prepared",qint64(preparation.uploadsPrepared)},{"retained_upload_source_bytes",qint64(preparation.retainedDecodedBytes)},
        {"sprite_upload_bytes",qint64(uploads.bytes)},{"upload_ticks",qint64(uploads.ticks)},{"max_tick_upload_bytes",qint64(uploads.maxTickBytes)},
        {"frames",frames_},{"capture_refusals",qint64(refused_)},{"refusal_counts",refusalCounts_},{"refusals",refusals_},{"status",status_},
        {"original_pixels_used_as_native_inputs",false},{"original_work_bypassed",false}};
}
