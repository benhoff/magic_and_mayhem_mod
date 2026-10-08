#include "live_world_session.hpp"
#include <QElapsedTimer>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <stdexcept>
namespace {
mnm::assets::AssetStore store(const QString& root){auto configured=mnm::assets::AssetStore::create(root.toStdString());if(auto* e=std::get_if<mnm::assets::Error>(&configured))throw std::runtime_error(e->detail);return std::get<mnm::assets::AssetStore>(std::move(configured));}
}
LiveWorldSession::LiveWorldSession(GlViewport& viewport,const QString& path,const QString& root,const QString& diagnostics):
    viewport_(viewport),channel_(path),store_(store(root)),resources_(store_),catalogue_(store_,resources_),diagnostics_(diagnostics){
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
        if(!pending_){pending_=channel_.poll();if(!pending_)return true;
            if(const auto refusal=mnm::legacy::decodeWorldRefusal(pending_->inputs)){
                const auto* input=reinterpret_cast<const unsigned char*>(pending_->inputs.constData());
                const auto sequence=quint32(input[20])|quint32(input[21])<<8|quint32(input[22])<<16|quint32(input[23])<<24;
                if(sequence!=pending_->sequence||!pending_->oracle.isEmpty()||channel_.verify())throw std::runtime_error("Invalid World refusal mode or sequence");
                viewport_.setGpuFrame({});++refused_;
                const auto key=QString::number(*refusal);refusalCounts_[key]=refusalCounts_.value(key).toInteger()+1;
                if(refusals_.size()<64)refusals_.append(QJsonObject{{"sequence",qint64(sequence)},{"reason",int(*refusal)}});
                status_=*refusal==MNM_WORLD_UNSUPPORTED_WAVE?"Waiting for World drawing to initialize.":"World drawing is not supported yet. Use the original game window.";
                pending_.reset();maxPollMs_=std::max(maxPollMs_,timer.elapsed());return true;
            }
            frame_=mnm::legacy::decodeWorldFrame(pending_->inputs);
            if(frame_->sequence!=pending_->sequence)throw std::runtime_error("World packet sequence differs from owned inputs");
            const QSize size(int(frame_->width),int(frame_->height));
            if(channel_.verify()&&quint64(pending_->oracle.size())!=quint64(frame_->width)*frame_->height*2)throw std::runtime_error("Missing complete original World oracle");
            const auto draws=catalogue_.display(*frame_);
            if(!renderer_)renderer_=std::make_unique<mnm::render::GlBlitter>(viewport_.context());
            if(!scene_||size!=size_){viewport_.setGpuFrame({});scene_.reset();
                scene_=std::make_unique<mnm::render::SceneRenderer>(*renderer_,resources_,mnm::render::Image{size.width(),size.height(),std::vector<std::uint32_t>(std::size_t(size.width())*size.height(),0)});size_=size;}
            scene_->beginFrame(draws);
            maxPollMs_=std::max(maxPollMs_,timer.elapsed());return true;
        }
        if(!scene_->drawNext(32)){maxPollMs_=std::max(maxPollMs_,timer.elapsed());return true;}
        const auto& frame=*frame_;const auto& packet=pending_;const auto size=size_;
        QJsonObject operations;for(const auto& draw:frame.draws){const auto key=QString::number(int(draw.composite.mode));operations[key]=operations.value(key).toInt()+1;}
        QJsonObject record{{"sequence",qint64(frame.sequence)},{"draws",qint64(frame.draws.size())},{"width",size.width()},{"height",size.height()},{"operations",operations}};
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
        pending_.reset();frame_.reset();
        maxPollMs_=std::max(maxPollMs_,timer.elapsed());return true;
    }catch(const std::exception& e){error_=QString::fromUtf8(e.what());close();return false;}
}
void LiveWorldSession::close(){if(closed_)return;closed_=true;channel_.cancel();viewport_.setGpuFrame({});pending_.reset();frame_.reset();scene_.reset();resources_.unloadAll();
    if(renderer_){const auto stats=renderer_->stats();readbacks_=stats.nativeReadbacks;remaining_=stats.surfaces;renderer_.reset();}}
QJsonObject LiveWorldSession::report() const{
    const auto stats=renderer_?renderer_->stats():mnm::render::RenderStats{};
    return {{"success",error_.isEmpty()&&presentations_>0},{"error",error_},{"presentations",int(presentations_)},{"verify",channel_.verify()},
        {"pixels_compared",qint64(compared_)},{"mismatches",qint64(mismatches_)},{"native_readbacks",qint64(renderer_?stats.nativeReadbacks:readbacks_)},
        {"viewport_image_uploads",qint64(viewport_.imageUploads())},{"remaining_surfaces",qint64(renderer_?stats.surfaces:remaining_)},
        {"dropped",int(channel_.dropped())},{"superseded",int(channel_.superseded())},{"producer_state",int(channel_.state())},{"producer_reason",int(channel_.reason())},
        {"max_poll_ms",maxPollMs_},{"frames",frames_},{"capture_refusals",qint64(refused_)},{"refusal_counts",refusalCounts_},{"refusals",refusals_},{"status",status_},
        {"original_pixels_used_as_native_inputs",false},{"original_work_bypassed",false}};
}
