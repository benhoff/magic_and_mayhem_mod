#include "canvas_producer_session.hpp"
#include "gl_viewport.hpp"
#include <QApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <stdexcept>
#include <algorithm>
#include "../../protocols/include/mnm/world_producer_bypass_v1.h"
namespace {
QByteArray read(const QString &path) {
  QFile f(path);
  if (!f.open(QIODevice::ReadOnly) || f.size() > 128 * 1024 * 1024)
    throw std::runtime_error("Unavailable bounded input: " + path.toStdString());
  return f.readAll();
}
void save(const QString &path, const QByteArray &raw) {
  QFile f(path);
  if (!f.open(QIODevice::WriteOnly | QIODevice::NewOnly) || f.write(raw) != raw.size())
    throw std::runtime_error("Cannot write native completion: " + path.toStdString());
}
unsigned word(const QByteArray &b, unsigned p) {
  if (p + 4 > unsigned(b.size())) throw std::runtime_error("Truncated producer word");
  unsigned result = 0;
  for (unsigned k = 0; k < 4; ++k) result |= unsigned(static_cast<unsigned char>(b[p+k])) << (k*8);
  return result;
}
mnm::assets::PathResolver resolver(const QString &root) {
  auto value = mnm::assets::PathResolver::create(root.toStdString());
  if (auto *e = std::get_if<mnm::assets::Error>(&value)) throw std::runtime_error(e->detail);
  return std::get<mnm::assets::PathResolver>(std::move(value));
}
}
CanvasProducerSession::CanvasProducerSession(GlViewport &viewport, QString input,
                                             QString root, QString output, bool worldHandoff)
    : viewport_(viewport), input_(std::move(input)), output_(std::move(output)),
      resolver_(resolver(root)), replay_([this](const std::string &name) {
        if (auto it = assets_.find(name); it != assets_.end()) return it->second;
        auto resolved = resolver_.resolve(name);
        if (auto *e = std::get_if<mnm::assets::Error>(&resolved)) throw std::runtime_error(e->detail);
        const auto raw = read(QString::fromStdString(std::get<mnm::assets::ResolvedAsset>(resolved).canonicalPath.string()));
        sourceHashes_[QString::fromStdString(name)] = QString::fromLatin1(QCryptographicHash::hash(raw, QCryptographicHash::Sha256).toHex());
        return assets_[name] = {raw.begin(), raw.end()};
      }), gameRoot_(root), worldHandoff_(worldHandoff) {
  if (!QDir().mkpath(output_)) throw std::runtime_error("Cannot create native output directory");
  connect(&timer_, &QTimer::timeout, this, [this] { tick(); });
}
void CanvasProducerSession::start() { clock_.start(); timer_.start(1); }
void CanvasProducerSession::tick() {
  if (stopped_) return;
  try {
    if (clock_.elapsed() > (fullRasters_?3600000:180000)) throw std::runtime_error("Producer session timed out");
    if (!viewport_.error().isEmpty()) throw std::runtime_error(viewport_.error().toStdString());
    if (!viewport_.ready()) return;
    if (!renderer_) {
      renderer_ = std::make_unique<mnm::render::GlBlitter>(viewport_.context());
      if (worldHandoff_) {
        auto result=mnm::assets::AssetStore::create(gameRoot_.toStdString());
        if(auto *e=std::get_if<mnm::assets::Error>(&result))throw std::runtime_error(e->detail);
        world_=std::make_unique<mnm::legacy::CanvasWorld>(*renderer_,std::get<mnm::assets::AssetStore>(std::move(result)));
      }
      save(QDir(output_).filePath("ready"), "Native renderer ready\n");
    }
    const QString donePath = QFileInfo(input_).dir().filePath("canvas-producers.done");
    const bool done = QFileInfo(donePath).size() == 32;
    if (applied_ == pending_.operations.size()) {
      if (!QFile::exists(input_)) return;
      QFile input(input_);if(!input.open(QIODevice::ReadOnly)||input.size()>128*1024*1024)throw std::runtime_error("Unavailable bounded producer tail");
      QByteArray header=input.read(64);
      if(header.size()<64){if(done)throw std::runtime_error("Missing producer header");return;}
      if(!accepted_.isEmpty()&&accepted_.left(64)!=header)throw std::runtime_error("Producer envelope changed");
      const unsigned offset=accepted_.isEmpty()?64:unsigned(accepted_.size());
      if(input.size()<offset||!input.seek(offset))throw std::runtime_error("Producer input shrank");
      auto tail=input.readAll();unsigned end=0;
      while(unsigned(tail.size())-end>=96){const auto size=word(tail,end);if(size<96||size>128*1024*1024)throw std::runtime_error("Invalid producer extent");if(size>unsigned(tail.size())-end)break;end+=size;}
      if(done&&end!=unsigned(tail.size()))throw std::runtime_error("Incomplete final producer record");
      tail.truncate(end);
      if(accepted_.isEmpty())accepted_=header;
      if(!tail.isEmpty()||done){auto batch=header+tail;mnm::legacy::appendCanvasProducers(pending_,{batch.begin(),batch.end()},done);accepted_+=tail;}
    }

    unsigned budget = 256;
    while (applied_ < pending_.operations.size() && budget--) {
      const auto &c = pending_.operations[applied_]; const auto &r = c.fields;
      if(world_&&world_->active()&&r[2]==9){
        const auto ordinal=unsigned(bypasses_.size())+1;
        const auto directory=QFileInfo(input_).dir();
        if(QFileInfo(directory.filePath(QString("world-raster-%1.request").arg(ordinal,6,10,QChar('0')))).size()==64)fullRasters_=true;
        const auto requestPath=directory.filePath(rasterBase(ordinal)+".request");
        if(QFileInfo(requestPath).size()==64&&word(read(requestPath),24)==r[1]){
          const auto image=replay_.read(world_->canvas());QByteArray pixels;pixels.reserve(image.pixels.size()*2);
          for(auto p:image.pixels){pixels.append(char(p));pixels.append(char(p>>8));}
          save(QDir(output_).filePath(rasterBase(ordinal)+".before.565"),pixels);
        }
      }
      replay_.apply(c);
      if(world_){
        if(r[2]==11){
          const auto native=replay_.read(r[3]);world_->begin(c,native);
          QByteArray entry;entry.reserve(native.pixels.size()*2);
          for(auto p:native.pixels){entry.append(char(p));entry.append(char(p>>8));}
          const auto name=QString("world-entry-%1.565").arg(r[14],4,10,QChar('0'));
          save(QDir(output_).filePath(name),entry);
          worldFrames_.append(QJsonObject{{"queue",int(r[14])},{"canvas",int(r[3])},{"entry",name},{"entry_sha256",QString::fromLatin1(QCryptographicHash::hash(entry,QCryptographicHash::Sha256).toHex())},{"native_history_nonzero_pixels",double(std::count_if(native.pixels.begin(),native.pixels.end(),[](auto p){return p!=0;}))}});
        }else if(world_->active()){
          if(r[2]==9)world_->append(c);
          else if(r[2]==10&&r[3]==world_->canvas())replay_.commitNativeWorld(r[3],world_->complete(replay_.read(r[3])));
          else if(r[2]==12){world_->end(c);auto frame=worldFrames_.last().toObject();frame["gpu_world_equal"]=true;frame["return_sequence"]=int(r[1]);worldFrames_.replace(worldFrames_.size()-1,frame);}
          else if(r[2]!=4&&r[2]!=10)throw std::runtime_error("Unadmitted write inside native World handoff");
        }
      }
      if (r[2] == 1) surfaces_[r[3]] = renderer_->allocate(r[5], r[6], mnm::render::PixelFormat{16, {0xf800, 0x07e0, 0x001f}});
      if (r[2] == 2) { renderer_->destroy(surfaces_.at(r[3])); surfaces_.erase(r[3]); }
      if (r[2] == 12) ++queues_;
      if (r[2] == 10) {
        const auto image = replay_.read(r[3]);
        if (image.width != int(r[5]) || image.height != int(r[6])) throw std::runtime_error("Native completion extent changed");
        const auto surface = surfaces_.at(r[3]); renderer_->update(surface, 0, 0, image);
        if (renderer_->read(surface).pixels != image.pixels) throw std::runtime_error("Native GPU mirror differs");
        QByteArray pixels; pixels.reserve(image.pixels.size()*2);
        for (auto p : image.pixels) { pixels.append(char(p)); pixels.append(char(p >> 8)); }
        const auto name = QString("native-%1.565").arg(r[14], 4, 10, QChar('0'));
        save(QDir(output_).filePath(name), pixels);
        if (image.width == 800 && image.height == 600) { viewport_.setGpuFrame(renderer_->presentGpu(surface)); ++presentations_; }
        checkpoints_.append(QJsonObject{{"sequence", int(r[1])}, {"oracle", int(r[14])}, {"canvas", int(r[3])}, {"path", name}, {"sha256", QString::fromLatin1(QCryptographicHash::hash(pixels, QCryptographicHash::Sha256).toHex())}, {"gpu_equal", true}});
      }
      ++applied_;
      bypassReply();
    }
    bypassReply();
    if (done && applied_ == pending_.operations.size()) {
      // Re-read complete input on the next tick if it grew during this batch.
      if (read(input_) != accepted_) return;
      const auto marker = read(donePath);
      if (marker.size() != 32 || marker.left(8) != "MNMPDONE" || word(marker,8) != 1 || word(marker,12) != queues_ || word(marker,16) != applied_ || word(marker,20) || word(marker,24) != unsigned(checkpoints_.size()) || word(marker,28) != unsigned(accepted_.size()) || queues_ != pending_.queues)
        throw std::runtime_error("Final producer marker does not close the native sequence");
      mnm::legacy::decodeCanvasProducers({accepted_.begin(), accepted_.end()});
      finish();
    }
  } catch (const std::exception &e) { finish(QString::fromUtf8(e.what())); }
}
QString CanvasProducerSession::rasterBase(unsigned ordinal) const{return fullRasters_?QString("world-raster-%1").arg(ordinal,6,10,QChar('0')):QString("world-bypass-%1").arg(ordinal,4,10,QChar('0'));}
void CanvasProducerSession::bypassReply(){
  if(!world_||!world_->active()||bypasses_.size()>=(fullRasters_?MNM_WORLD_RASTER_MAX:MNM_WORLD_BYPASS_MAX))return;
  const auto ordinal=unsigned(bypasses_.size())+1;
  const auto base=rasterBase(ordinal);
  const auto directory=QFileInfo(input_).dir();const auto requestPath=directory.filePath(base+".request");
  if(QFileInfo(requestPath).size()!=64)return;
  const auto request=read(requestPath);
  if(request.left(8)!=(fullRasters_?MNM_WORLD_RASTER_REQUEST:MNM_WORLD_BYPASS_REQUEST)||word(request,8)!=(fullRasters_?2u:1u)||word(request,12)!=64||word(request,16)!=ordinal||!word(request,20)||word(request,20)>pending_.queues||word(request,44)||word(request,48)||word(request,52)||(!fullRasters_&&word(request,56)!=0x5947b2&&word(request,56)!=0x59521a)||word(request,60)>(fullRasters_?1u:0u))
    throw std::runtime_error("Malformed native World bypass request");
  if(word(request,20)>world_->queue())return;
  if(word(request,20)!=world_->queue()||word(request,28)!=world_->canvas())throw std::runtime_error("Stale native World bypass queue or canvas");
  const auto sequence=word(request,24);
  if(applied_<sequence)return;
  if(applied_!=sequence||!sequence||pending_.operations.at(sequence-1).fields[2]!=9)
    throw std::runtime_error("Native World bypass names stale or non-raster producer");
  const auto &operation=pending_.operations.at(sequence-1);
  if(operation.fields[3]!=word(request,28)||operation.fields[5]!=word(request,32)||operation.fields[6]!=word(request,36)||word(request,40)!=word(request,32)||(!fullRasters_&&(operation.fields[14]||operation.fields[15]!=1||!operation.fields[17]))||(fullRasters_&&operation.fields[21]!=word(request,60)))
    throw std::runtime_error("Native World bypass source identity changed");
  if(fullRasters_){
    const unsigned entry=word(request,56),backend=operation.fields[15];
    const std::pair<unsigned,unsigned> admitted[]={{0x595677,0},{0x5947b2,1},{0x59521a,1},{0x595b47,2},{0x59603e,2},{0x57de00,3},{0x57ec90,4},{0x57f0f0,5},{0x57f5f0,6},{0x5806f0,7},{0x596490,9},{0x5968a4,9},{0x57e540,10}};
    if(std::none_of(std::begin(admitted),std::end(admitted),[&](auto pair){return pair.first==entry&&pair.second==backend;}))throw std::runtime_error("Unreviewed complete-queue raster entry");
    QByteArray source(reinterpret_cast<const char*>(operation.payload.data()),operation.payload.size());
    const auto left=std::int64_t(std::int32_t(operation.fields[8]))-std::int32_t(word(source,12));
    const auto width=word(source,4),height=word(source,8);
    const unsigned ax=(backend==0||backend==9)&&width&&height&&(left<std::int32_t(operation.fields[10])||left+width>=std::int32_t(operation.fields[12]));
    if(ax!=word(request,60))throw std::runtime_error("Recovered caller AX differs from owned source geometry");
  }
  auto native=world_->complete(replay_.read(world_->canvas()));replay_.commitNativeWorld(world_->canvas(),native);
  QByteArray pixels;pixels.reserve(native.pixels.size()*2);
  unsigned checksum=2166136261u;
  for(auto p:native.pixels){pixels.append(char(p));pixels.append(char(p>>8));}
  for(auto p:pixels)checksum=(checksum^unsigned(static_cast<unsigned char>(p)))*16777619u;
  QByteArray reply=request;reply.replace(0,8,fullRasters_?MNM_WORLD_RASTER_REPLY:MNM_WORLD_BYPASS_REPLY);
  auto put=[&](unsigned at,unsigned value){for(unsigned k=0;k<4;++k)reply[at+k]=char(value>>(k*8));};
  put(44,pixels.size());put(48,checksum);put(52,1);reply.append(pixels);
  const auto temporary=directory.filePath(base+".reply.tmp"),published=directory.filePath(base+".reply");
  save(temporary,reply);if(!QFile::rename(temporary,published))throw std::runtime_error("Cannot atomically publish native World bypass completion");
  bypasses_.append(QJsonObject{{"ordinal",int(ordinal)},{"queue",int(world_->queue())},{"sequence",int(sequence)},{"canvas",int(world_->canvas())},{"original_entry",double(word(request,56))},{"return_ax",int(word(request,60))},{"wire_base",base},{"native_before",rasterBase(ordinal)+".before.565"},{"pixels",double(native.pixels.size())},{"native_pixel_sha256",QString::fromLatin1(QCryptographicHash::hash(pixels,QCryptographicHash::Sha256).toHex())},{"gpu_world_equal",true}});
}
void CanvasProducerSession::finish(const QString &error) {
  stopped_ = true; timer_.stop();
  QJsonObject report{{"success", error.isEmpty()}, {"error", error}, {"records", int(applied_)}, {"queues", int(queues_)}, {"presentations", int(presentations_)}, {"checkpoints", checkpoints_}, {"encoded_source_sha256", sourceHashes_}, {"input_sha256", QString::fromLatin1(QCryptographicHash::hash(accepted_, QCryptographicHash::Sha256).toHex())}, {"original_pixels_used_as_native_inputs", false}, {"original_oracles_read", false}, {"live_replacement", false}, {"gpu_policy", "Owned native CPU composition mirrored to retained GPU surfaces at completion"}, {"viewport_image_uploads", double(viewport_.imageUploads())}};
  viewport_.setGpuFrame({});
  report["complete_raster_queues"]=fullRasters_;report["native_bypass_replies"]=bypasses_;report["native_bypass_count"]=bypasses_.size();
  if(world_){report["world_handoff"]=true;report["world_queues"]=int(world_->completedQueues());report["world_draws"]=int(world_->draws());report["world_frames"]=worldFrames_;world_.reset();}
  if (renderer_) {
    for (auto [id, handle] : surfaces_) { (void)id; renderer_->destroy(handle); }
    surfaces_.clear(); report["remaining_surfaces"] = double(renderer_->stats().surfaces);
  }
  try { save(QDir(output_).filePath("live-report.json"), QJsonDocument(report).toJson()); }
  catch (const std::exception &) { QApplication::exit(2); return; }
  QApplication::exit(error.isEmpty() ? 0 : 1);
}
