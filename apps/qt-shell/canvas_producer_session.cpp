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
#include <chrono>
#include "../../protocols/include/mnm/world_producer_bypass_v1.h"
#include "../../protocols/include/mnm/canvas_producers_v2.h"
#include "../../protocols/include/mnm/canvas_producers_v3.h"
#include "../../compat/legacy/world_raster_batch.hpp"
#include "../../protocols/include/mnm/world_raster_batch_v3.h"
namespace {
using ProfileClock=std::chrono::steady_clock;
double elapsedMs(ProfileClock::time_point start){return std::chrono::duration<double,std::milli>(ProfileClock::now()-start).count();}
QByteArray rgb565(const mnm::render::Image& image){
  QByteArray result(qsizetype(image.pixels.size()*2),Qt::Uninitialized);auto* p=result.data();
  for(auto pixel:image.pixels){*p++=char(pixel);*p++=char(pixel>>8);}return result;
}
QByteArray read(const QString &path, qint64 limit = 128 * 1024 * 1024) {
  QFile f(path);
  if (!f.open(QIODevice::ReadOnly) || f.size() > limit)
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
                                             QString root, QString output, bool worldHandoff, bool worldBatch, QString proofPipe)
    : viewport_(viewport), input_(std::move(input)), output_(std::move(output)),
      resolver_(resolver(root)), replay_([this](const std::string &name) {
        if (auto it = assets_.find(name); it != assets_.end()) return it->second;
        auto resolved = resolver_.resolve(name);
        if (auto *e = std::get_if<mnm::assets::Error>(&resolved)) throw std::runtime_error(e->detail);
        const auto raw = read(QString::fromStdString(std::get<mnm::assets::ResolvedAsset>(resolved).canonicalPath.string()));
        sourceHashes_[QString::fromStdString(name)] = QString::fromLatin1(QCryptographicHash::hash(raw, QCryptographicHash::Sha256).toHex());
        return assets_[name] = {raw.begin(), raw.end()};
      }), gameRoot_(root), worldHandoff_(worldHandoff), worldBatch_(worldBatch), proofPipe_(proofPipe) {
  if((worldBatch||!proofPipe.isEmpty())&&!worldHandoff)throw std::invalid_argument("World batching/proof requires handoff");
  fullRasters_=worldBatch;
  if(!proofPipe.isEmpty()&&!proofPipe_.open(QIODevice::WriteOnly))throw std::runtime_error("Cannot open native proof output pipe");
  if (!QDir().mkpath(output_)) throw std::runtime_error("Cannot create native output directory");
  connect(&timer_, &QTimer::timeout, this, [this] { tick(); });
}
void CanvasProducerSession::start() { clock_.start(); timer_.start(1); }
void CanvasProducerSession::tick() {
  if (stopped_) return;
  try {
    if (clock_.elapsed() > (fullRasters_||proofPipe_.isOpen()?3600000:180000)) throw std::runtime_error("Producer session timed out");
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
      const auto ingestStarted=ProfileClock::now();
      if (!QFile::exists(input_)) return;
      QFile input(input_);if(!input.open(QIODevice::ReadOnly))throw std::runtime_error("Unavailable bounded producer tail");
      QByteArray header=input.read(64);
      if(header.size()<64){if(done)throw std::runtime_error("Missing producer header");return;}
      mnm::legacy::CanvasProducerStream envelope{0,{}};
      mnm::legacy::appendCanvasProducers(envelope,{header.begin(),header.end()},false);
      const qint64 streamLimit=envelope.version==2?MNM_PRODUCER_V2_MAX_BYTES:128u*1024u*1024u;
      if(input.size()>streamLimit)throw std::runtime_error("Producer tail exceeds version bound");
      if(!accepted_.isEmpty()&&accepted_.left(64)!=header)throw std::runtime_error("Producer envelope changed");
      const unsigned offset=accepted_.isEmpty()?64:unsigned(accepted_.size());
      if(input.size()<offset||!input.seek(offset))throw std::runtime_error("Producer input shrank");
      auto tail=input.readAll();unsigned end=0;
      while(unsigned(tail.size())-end>=96){const auto size=word(tail,end);if(size<96||size>unsigned(streamLimit))throw std::runtime_error("Invalid producer extent");if(size>unsigned(tail.size())-end)break;end+=size;}
      if(done&&end!=unsigned(tail.size()))throw std::runtime_error("Incomplete final producer record");
      tail.truncate(end);
      if(accepted_.isEmpty())accepted_=header;
      if(!tail.isEmpty()||done){auto batch=header+tail;mnm::legacy::appendCanvasProducers(pending_,{batch.begin(),batch.end()},done);accepted_+=tail;}
      ingestMs_+=elapsedMs(ingestStarted);
    }

    unsigned budget = (worldBatch_ || pending_.version==3) ? 4096 : 256;
    while (applied_ < pending_.operations.size() && budget--) {
      const auto &c = pending_.operations[applied_]; const auto &r = c.fields;
      if(world_&&world_->active()&&r[2]==9&&!worldBatch_&&!done){
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
      const auto cpuStarted=ProfileClock::now();replay_.apply(c);
      const auto cpuElapsed=elapsedMs(cpuStarted);cpuMs_+=cpuElapsed;
      if(world_&&world_->active())queueCpuMs_+=cpuElapsed;
      if(r[2]==22||r[2]==MNM_PRODUCER_OWNED_MINIMAP){minimapMs_+=cpuElapsed;++minimapOperations_;}
      if(world_){
        if(r[2]==11){
          queueClock_.start();queueCpuMs_=queueCheckpointMs_=batchValidationMs_=batchPublishMs_=0;
          const auto native=replay_.read(r[3]);world_->begin(c,native);
          batchStart_=applied_+1;batchReplied_=false;proof(c,1,&native);
          const auto entry=rgb565(native);
          const auto name=QString("world-entry-%1.565").arg(r[14],4,10,QChar('0'));
          save(QDir(output_).filePath(name),entry);
          worldFrames_.append(QJsonObject{{"queue",int(r[14])},{"canvas",int(r[3])},{"entry",name},{"entry_sha256",QString::fromLatin1(QCryptographicHash::hash(entry,QCryptographicHash::Sha256).toHex())},{"native_history_nonzero_pixels",double(std::count_if(native.pixels.begin(),native.pixels.end(),[](auto p){return p!=0;}))}});
        }else if(world_->active()){
          if(worldBatch_&&batchReplied_&&r[2]!=10&&r[2]!=12)throw std::runtime_error("World work arrived after batch completion");
          if(r[2]==9){world_->append(c);if(proofPipe_.isOpen()){const auto image=replay_.read(r[3]);proof(c,2,&image);}}
          else if(r[2]==10&&r[3]==world_->canvas()){if(!worldBatch_||!batchReplied_)replay_.commitNativeWorld(r[3],world_->complete(replay_.read(r[3])));}
          else if(r[2]==12){
            if(worldBatch_&&r[20]&&!batchReplied_)throw std::runtime_error("World returned without batch completion");
            const auto &p=world_->profile();
            auto frame=worldFrames_.last().toObject();
            frame["profile"]=QJsonObject{{"consumer_entry_to_return_ms",double(queueClock_.nsecsElapsed())/1e6},{"cpu_composition_ms",queueCpuMs_},{"checkpoint_diagnostics_ms",queueCheckpointMs_},{"history_adopt_ms",p.adoptMs},{"resource_prepare_ms",p.prepareMs},{"gpu_submit_ms",p.submitMs},{"gpu_readback_ms",p.readbackMs},{"gpu_compare_ms",p.compareMs},{"batch_validation_ms",batchValidationMs_},{"batch_publish_ms",batchPublishMs_},{"visible_draws",int(p.visibleDraws)},{"gpu_copy_batches",double(p.copyBatches)},{"gpu_snapshot_pixels",double(p.snapshotPixels)},{"gpu_scratch_allocations",double(p.scratchAllocations)}};
            frame["reuse"]=QJsonObject{{"visual_checks",double(p.visualChecks)},{"visual_reuses",double(p.visualReuses)},{"cache_uploads",double(p.cacheUploads)},{"cache_hits",double(p.cacheHits)},{"cache_evictions",double(p.cacheEvictions)},{"cache_frames",double(p.cacheFrames)},{"cache_surfaces",double(p.cacheSurfaces)}};
            frame["identities"]=QJsonObject{{"hashes",double(p.identityHashes)},{"hits",double(p.identityHits)},{"evictions",double(p.identityEvictions)},{"bypasses",double(p.identityBypasses)},{"frames",double(p.identityFrames)},{"bytes",double(p.identityBytes)}};
            world_->end(c);proof(c,3,nullptr);frame["gpu_world_equal"]=true;frame["return_sequence"]=int(r[1]);worldFrames_.replace(worldFrames_.size()-1,frame);
          }
          else if(r[2]!=4&&r[2]!=10&&r[2]!=MNM_PRODUCER_PAYLOAD_SOURCE)throw std::runtime_error("Unadmitted write inside native World handoff");
        }
      }
      if (r[2] == 1) surfaces_[r[3]] = renderer_->allocate(r[5], r[6], mnm::render::PixelFormat{16, {0xf800, 0x07e0, 0x001f}});
      if (r[2] == 2) { renderer_->destroy(surfaces_.at(r[3])); surfaces_.erase(r[3]); }
      if (r[2] == 12) {
        ++queues_;
        producerFrames_.append(QJsonObject{{"queue",int(r[14])},{"cpu_composition_ms",cpuMs_-lastCpuMs_},{"checkpoint_diagnostics_ms",checkpointMs_-lastCheckpointMs_},{"stream_ingest_ms",ingestMs_-lastIngestMs_},{"minimap_composition_ms",minimapMs_-lastMinimapMs_}});
        lastCpuMs_=cpuMs_;lastCheckpointMs_=checkpointMs_;lastIngestMs_=ingestMs_;lastMinimapMs_=minimapMs_;
      }
      if (r[2] == 10) {
        const auto checkpointStarted=ProfileClock::now();
        const auto image = replay_.read(r[3]);
        if (image.width != int(r[5]) || image.height != int(r[6])) throw std::runtime_error("Native completion extent changed");
        const auto surface = surfaces_.at(r[3]); renderer_->update(surface, 0, 0, image);
        if (renderer_->read(surface).pixels != image.pixels) throw std::runtime_error("Native GPU mirror differs");
        const auto pixels=rgb565(image);
        const auto name = QString("native-%1.565").arg(r[14], 4, 10, QChar('0'));
        save(QDir(output_).filePath(name), pixels);
        if (image.width == 800 && image.height == 600) { viewport_.setGpuFrame(renderer_->presentGpu(surface)); ++presentations_; }
        checkpoints_.append(QJsonObject{{"sequence", int(r[1])}, {"oracle", int(r[14])}, {"canvas", int(r[3])}, {"path", name}, {"sha256", QString::fromLatin1(QCryptographicHash::hash(pixels, QCryptographicHash::Sha256).toHex())}, {"gpu_equal", true}});
        const auto checkpointElapsed=elapsedMs(checkpointStarted);checkpointMs_+=checkpointElapsed;
        if(world_&&world_->active())queueCheckpointMs_+=checkpointElapsed;
      }
      ++applied_;
      if(!worldBatch_&&!done)bypassReply();
    }
    if(worldBatch_)batchReply();else if(!done)bypassReply();
    if (done && applied_ == pending_.operations.size()) {
      // Re-read complete input on the next tick if it grew during this batch.
      if (read(input_,pending_.version==2?MNM_PRODUCER_V2_MAX_BYTES:128u*1024u*1024u) != accepted_) return;
      const auto marker = read(donePath);
      if (marker.size() != 32 || marker.left(8) != "MNMPDONE" || word(marker,8) != 1 || word(marker,12) != queues_ || word(marker,16) != applied_ || word(marker,20) || word(marker,24) != unsigned(checkpoints_.size()) || word(marker,28) != unsigned(accepted_.size()) || queues_ != pending_.queues)
        throw std::runtime_error("Final producer marker does not close the native sequence");
      mnm::legacy::decodeCanvasProducers({accepted_.begin(), accepted_.end()});
      finish();
    }
  } catch (const std::exception &e) { finish(QString::fromUtf8(e.what())); }
}
void CanvasProducerSession::proof(const mnm::legacy::CanvasProducer& c,unsigned type,const mnm::render::Image* image){
  if(!proofPipe_.isOpen())return;
  static_assert(Q_BYTE_ORDER==Q_LITTLE_ENDIAN,"Native proof wire requires explicit conversion on big-endian hosts");
  const auto &r=c.fields;QByteArray h("MNMBPF01",8);
  for(unsigned w:{type,r[1],world_->queue(),r[3],r[5],r[6],image?unsigned(image->pixels.size()*2):0u,type==2?mnm::legacy::worldRasterAX(c):0u})for(unsigned k=0;k<4;++k)h.append(char(w>>(k*8)));
  auto writeAll=[this](const char* p,qint64 n){while(n){const auto done=proofPipe_.write(p,n);if(done<=0)throw std::runtime_error("Native proof pipe closed");p+=done;n-=done;}};
  writeAll(h.data(),h.size());if(image){
    // Image stores UInt32 native pixels; the proof wire carries tight RGB565.
    const std::vector<std::uint16_t> pixels(image->pixels.begin(),image->pixels.end());
    writeAll(reinterpret_cast<const char*>(pixels.data()),qint64(pixels.size()*2));
  }
  if(!proofPipe_.flush())throw std::runtime_error("Native proof pipe flush failed");
}
void CanvasProducerSession::batchReply(){
  if(!world_||!world_->active()||batchReplied_)return;
  const auto base=QString("world-batch-%1").arg(world_->queue(),4,10,QChar('0'));
  const auto directory=QFileInfo(input_).dir();const auto requestPath=directory.filePath(base+".request");
  const auto size=QFileInfo(requestPath).size();if(size<64)return;
  if(size>64+MNM_WORLD_BATCH_MAX*16)throw std::runtime_error("World batch request exceeded bound");
  const auto request=read(requestPath);if(request.size()<64)return;
  const auto bytes=word(request,44);if(bytes>MNM_WORLD_BATCH_MAX*16)throw std::runtime_error("World batch descriptor extent exceeded bound");
  if(unsigned(request.size())<64+bytes)return;
  const auto sequence=word(request,24);if(applied_<sequence)return;
  if(applied_!=sequence)throw std::runtime_error("Stale World batch completion sequence");
  std::vector<const mnm::legacy::CanvasProducer*> rasters;
  auto started=ProfileClock::now();
  for(auto i=batchStart_;i<applied_;++i){const auto& c=pending_.operations.at(i);if(c.fields[2]==9)rasters.push_back(&c);else if(c.fields[2]!=4&&c.fields[2]!=MNM_PRODUCER_PAYLOAD_SOURCE)throw std::runtime_error("Unadmitted producer before World batch completion");}
  const auto nativeReference=replay_.read(world_->canvas());
  const auto plan=mnm::legacy::validateWorldRasterBatch({request.begin(),request.end()},unsigned(batches_.size())+1,world_->queue(),unsigned(applied_),world_->canvas(),nativeReference.width,nativeReference.height,unsigned(batchRasterLogs_.size()),rasters);
  batchValidationMs_+=elapsedMs(started);
  const auto native=world_->complete(nativeReference);started=ProfileClock::now();replay_.commitNativeWorld(world_->canvas(),native);
  const std::vector<std::uint16_t> words(native.pixels.begin(),native.pixels.end());
  const auto reply=mnm::legacy::worldRasterBatchReply(plan,words);
  const auto temporary=directory.filePath(base+".reply.tmp"),published=directory.filePath(base+".reply");
  save(temporary,{reinterpret_cast<const char*>(reply.data()),qsizetype(reply.size())});if(!QFile::rename(temporary,published))throw std::runtime_error("Cannot atomically publish World batch completion");
  QJsonArray sequences;
  for(const auto& d:plan.descriptors){sequences.append(int(d[0]));batchRasterLogs_.push_back({world_->queue(),d[0],d[1],d[2],d[3]});}
  batches_.append(QJsonObject{{"queue",int(world_->queue())},{"ordinal",int(plan.header[4])},{"sequence",int(sequence)},{"canvas",int(world_->canvas())},{"rasters",int(plan.header[14])},{"cumulative_rasters",int(plan.header[15])},{"wire_base",base},{"guarded",true},{"sequences",sequences},{"pixels",double(native.pixels.size())},{"reply_sha256",QString::fromLatin1(QCryptographicHash::hash({reinterpret_cast<const char*>(reply.data()),qsizetype(reply.size())},QCryptographicHash::Sha256).toHex())}});
  batchPublishMs_+=elapsedMs(started);
  batchReplied_=true;
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
    QByteArray source(reinterpret_cast<const char*>(operation.bytes().data()),operation.bytes().size());
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
  for(const auto& r:batchRasterLogs_)bypasses_.append(QJsonObject{{"queue",int(r[0])},{"sequence",int(r[1])},{"original_entry",double(r[2])},{"return_ax",int(r[3])},{"source_checksum",double(r[4])}});
  QJsonObject report{{"success", error.isEmpty()}, {"error", error}, {"records", int(applied_)}, {"queues", int(queues_)}, {"presentations", int(presentations_)}, {"checkpoints", checkpoints_}, {"encoded_source_sha256", sourceHashes_}, {"input_sha256", QString::fromLatin1(QCryptographicHash::hash(accepted_, QCryptographicHash::Sha256).toHex())}, {"original_pixels_used_as_native_inputs", false}, {"original_oracles_read", false}, {"live_replacement", false}, {"gpu_policy", "Owned native CPU composition mirrored to retained GPU surfaces at completion"}, {"viewport_image_uploads", double(viewport_.imageUploads())}};
  viewport_.setGpuFrame({});
  report["complete_raster_queues"]=fullRasters_;report["native_bypass_replies"]=bypasses_;report["native_bypass_count"]=bypasses_.size();
  report["world_raster_batch"]=worldBatch_;report["native_batch_replies"]=batches_;report["native_canvas_writebacks"]=batches_.size();
  report["profile"]=QJsonObject{{"clock","steady host clock, milliseconds"},{"scope","Diagnostic consumer; GPU submission is host time, readback includes deferred GPU work. Queue wall time includes producer/timer waits; excludes original simulation and pre-entry work. Checkpoints include GPU mirror, readback, serialization, hashing and disk writes. CPU reference remains enabled; proof FIFO blocking is not a frame benchmark."},{"cpu_composition_ms",cpuMs_},{"checkpoint_diagnostics_ms",checkpointMs_},{"stream_ingest_ms",ingestMs_}};
  report["native_build_type"]=MNM_CANVAS_PRODUCER_BUILD_TYPE;report["wire_version"]=int(pending_.version);report["stream_bytes"]=accepted_.size();
  report["owned_payload_sources"]=int(pending_.ownedPayloads.size());report["owned_payload_bytes"]=double(pending_.ownedPayloadBytes);
  report["producer_frames"]=producerFrames_;report["minimap_operations"]=int(minimapOperations_);report["minimap_composition_ms"]=minimapMs_;
  if(world_){report["world_handoff"]=true;report["world_queues"]=int(world_->completedQueues());report["world_draws"]=int(world_->draws());report["world_readbacks"]=int(world_->readbacks());report["world_frames"]=worldFrames_;world_.reset();}
  if (renderer_) {
    for (auto [id, handle] : surfaces_) { (void)id; renderer_->destroy(handle); }
    surfaces_.clear(); report["remaining_surfaces"] = double(renderer_->stats().surfaces);
  }
  try { save(QDir(output_).filePath("live-report.json"), QJsonDocument(report).toJson()); }
  catch (const std::exception &) { QApplication::exit(2); return; }
  QApplication::exit(error.isEmpty() ? 0 : 1);
}
