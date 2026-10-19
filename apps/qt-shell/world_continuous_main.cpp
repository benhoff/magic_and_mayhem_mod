#include "../../compat/legacy/world_continuous.hpp"
#include "../../compat/legacy/world_rolling_handoff.hpp"
#include "../../compat/legacy/world_rolling_session.hpp"
#include "gl_viewport.hpp"
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QOpenGLFunctions>
#include <QRandomGenerator>
#include <QSaveFile>
#include <QTimer>
#include <chrono>
#include <cstring>
#include <optional>
#include <unistd.h>
using namespace mnm::legacy;
namespace {
using Clock = std::chrono::steady_clock;
double ms(Clock::time_point t) {
  return std::chrono::duration<double, std::milli>(Clock::now() - t).count();
}
void save(const QString &path, const QJsonObject &value) {
  QSaveFile f(path);
  if (!f.open(QIODevice::WriteOnly) ||
      f.write(QJsonDocument(value).toJson()) < 0 || !f.commit())
    throw std::runtime_error("Continuous report publication");
}
struct Awaiting {
  WorldContinuousChannel::Block block;
  RollingHandoff handoff;
  Clock::time_point started;
};
} // namespace
int main(int argc, char **argv) {
  QApplication app(argc, argv);
  const auto args = app.arguments();
  const bool resident = args.size() == 6 && args[5] == "--resident";
  const bool verify = args.size() == 6 && args[5] == "--verify";
  if (args.size() != 5 && !verify && !resident)
    return 2;
  bool okay = false;
  const auto target = args[4].toULongLong(&okay);
  if (!okay)
    return 2;
  const auto output = args[3];
  if (!QDir().mkpath(output))
    return 2;
  const auto path =
      QFileInfo(args[1]).dir().filePath("../world-continuous.bin");
  auto nonce = QRandomGenerator::system()->generate64();
  if (!nonce)
    nonce = 1;
  WorldContinuousChannel::create(path, nonce, target, resident);
  WorldContinuousChannel channel(path);
  GlViewport viewport;
  viewport.resize(800, 600);
  viewport.setWindowTitle("Magic & Mayhem continuous native World");
  viewport.show();
  std::unique_ptr<mnm::render::GlBlitter> renderer;
  std::unique_ptr<WorldRollingSession> session;
  std::optional<Awaiting> awaiting;
  std::uint64_t returned = 0, rasters = 0, paints = 0, unpainted = 0,
                lastPainted = 0;
  std::size_t peakBlock = 0, peakPacket = 0;
  QJsonArray samples;
  std::array<std::uint64_t, 151> warmHistogram{};
  std::uint64_t warmCount = 0;
  std::uint32_t warmMaximum = 0;
  QString error;
  QJsonObject driver;
  auto lastProgress = Clock::now();
  Clock::time_point sessionStart;
  Clock::time_point requestedPaint;
  std::uint64_t presentation = 0;
  std::uint32_t warmFirstTick = 0, warmLastTick = 0, previousReturnTick = 0, previousStartTick = 0;
  std::uint64_t warmFirstPaint = 0;
  Clock::time_point warmFirstPaintTime, warmLastPaintTime;
  std::uint64_t warmPaints = 0;
  double coldDeliveryMs = 0;
  std::size_t rssWarm = 0, rssMaximum = 0, producerCacheMax = 0,
              gpuFramesMax = 0;
  auto rss = []() {
    QFile f("/proc/self/statm");
    if (!f.open(QIODevice::ReadOnly))
      return std::size_t(0);
    const auto fields = f.readAll().simplified().split(' ');
    return fields.size() > 1 ? std::size_t(fields[1].toULongLong()) *
                                   std::size_t(sysconf(_SC_PAGESIZE))
                             : std::size_t(0);
  };
  QObject::connect(&viewport, &GlViewport::frameSwapped, [&] {
    if (presentation && presentation != lastPainted) {
      ++paints;
      lastPainted = presentation;
      if (presentation >= 64) {
        if (!warmFirstPaint) {
          warmFirstPaint = presentation;
          warmFirstPaintTime = Clock::now();
        }
        warmLastPaintTime = Clock::now();
        ++warmPaints;
      }
      const auto age = ms(requestedPaint);
      if (!samples.empty()) {
        auto row = samples.last().toObject();
        if (row["generation"].toVariant().toULongLong() == presentation) {
          row["qt_paint_ms"] = age;
          samples[samples.size() - 1] = row;
        }
      }
    }
  });
  QTimer timer;
  QObject::connect(&timer, &QTimer::timeout, [&] {
    try {
      if (!viewport.error().isEmpty())
        throw std::runtime_error(viewport.error().toStdString());
      if (!viewport.ready())
        return;
      if (!renderer) {
        viewport.makeCurrent();
        renderer = std::make_unique<mnm::render::GlBlitter>(viewport.context());
        auto store = mnm::assets::AssetStore::create(args[2].toStdString());
        if (auto *e = std::get_if<mnm::assets::Error>(&store))
          throw std::runtime_error(e->detail);
        session = std::make_unique<WorldRollingSession>(
            *renderer, std::get<mnm::assets::AssetStore>(std::move(store)),
            verify, resident);
        save(QDir(output).filePath("ready"),
             {{"pid", qint64(QCoreApplication::applicationPid())},
              {"nonce", QString::number(nonce)}});
        auto *gl = viewport.context()->functions();
        driver = {
            {"vendor", QString::fromLatin1(reinterpret_cast<const char *>(
                           gl->glGetString(GL_VENDOR)))},
            {"renderer", QString::fromLatin1(reinterpret_cast<const char *>(
                             gl->glGetString(GL_RENDERER)))},
            {"version", QString::fromLatin1(reinterpret_cast<const char *>(
                            gl->glGetString(GL_VERSION)))}};
        sessionStart = Clock::now();
        lastProgress = sessionStart;
      }
      if (channel.cancelled())
        throw std::runtime_error("Original continuous session cancelled");
      if (ms(lastProgress) > 120000)
        throw std::runtime_error("Continuous producer/reply deadline");
      if (awaiting) {
        std::array<std::uint8_t, 192> actual{};
        if (channel.returnRecords(awaiting->block.generation,
                                  awaiting->block.source, actual)) {
          auto close = [&](unsigned offset) {
            CanvasProducer c{};
            for (unsigned i = 0; i < 24; ++i) {
              const auto *p = actual.data() + offset + i * 4;
              c.fields[i] = p[0] | unsigned(p[1]) << 8 | unsigned(p[2]) << 16 |
                            unsigned(p[3]) << 24;
            }
            if (c.fields[0] != 96 || c.fields[18] || c.fields[23])
              throw std::runtime_error("Malformed continuous closing record");
            return c;
          };
          verifyRollingHandoffReturn(awaiting->handoff, close(0), close(96), resident);
          rasters += awaiting->handoff.batch.header[14];
          returned = awaiting->block.generation;
          auto row = samples.last().toObject();
          row["native_request_to_original_return_ms"] = ms(awaiting->started);
          const auto t = channel.ticks(returned);
          if (returned == 64)
            warmFirstTick = t[0];
          if (returned >= 64) {
            warmLastTick = t[3];
            const auto duration = std::uint32_t(t[3] - t[0]);
            ++warmHistogram[std::min(duration, 150u)];
            ++warmCount;
            warmMaximum = std::max(warmMaximum, duration);
          }
          if (returned % 64 == 0) {
            auto resident = rss();
            if (returned == 64)
              rssWarm = resident;
            rssMaximum = std::max(rssMaximum, resident);
          }
          producerCacheMax =
              std::max(producerCacheMax, session->producerCacheStats().bytes);
          gpuFramesMax = std::max(gpuFramesMax, session->profile().cacheFrames);
          if(returned>1){
            row["engine_between_world_ms"]=double(std::uint32_t(t[0]-previousReturnTick));
            row["original_frame_interval_ms"]=double(std::uint32_t(t[0]-previousStartTick));
          }
          row["producer_region_queries"]=double(t[9]);
          row["engine_pacer_ms"]=t[6]==3?double(std::uint32_t(t[5]-t[4])):0.;
          row["engine_pacer_observed"]=t[6]==3;
          row["engine_screen_copy_ms"]=double(t[7]);
          row["engine_screen_copy_calls"]=double(t[8]);
          previousReturnTick=t[3];previousStartTick=t[0];
          row["producer_capture_ms"] = double(std::uint32_t(t[1] - t[0]));
          row["producer_wait_writeback_ms"] =
              double(std::uint32_t(t[2] - t[1]));
          row["producer_whole_world_ms"] = double(std::uint32_t(t[3] - t[0]));
          row["producer_return_ms"] = double(std::uint32_t(t[3] - t[2]));
          samples[samples.size() - 1] = row;
          channel.release(returned);
          awaiting.reset();
          lastProgress = Clock::now();
        }
      }
      if (!awaiting) {
        WorldContinuousChannel::Block block;
        const auto started = Clock::now();
        if (channel.poll(block)) {
          const auto acquired = ms(started);
          peakBlock = std::max(peakBlock, block.source.size());
          auto stream = decodeCanvasProducers(block.source, false);
          auto handoff =
              prepareRollingHandoff(stream, 0, block.request, 1, 0,
                                    WorldBatchSourcePolicy::ownedV5, false);
          const auto prepared = ms(started);
          peakPacket = std::max(peakPacket, block.source.size() + 192);
          viewport.makeCurrent();
          mnm::render::Image image;
          const auto beforeReadbacks = renderer->stats().nativeReadbacks;
          if (resident)session->consumeResident(block.generation, handoff.stream);
          else image = session->consumeOwned(block.generation, handoff.stream);
          if (resident && (renderer->stats().nativeReadbacks != beforeReadbacks ||
                           session->worldReadbacks()))
            throw std::runtime_error("Resident delivery exported GPU pixels");
          const auto consumed = ms(started);
          if (presentation && lastPainted != presentation)
            ++unpainted;
          viewport.setGpuFrame(session->presentGpu());
          presentation = block.generation;
          requestedPaint = Clock::now();
          /* The local V3 ordinal is1. Bind the session's u64 generation first,
           * then adapt the checked image to that local request identity. */
          if (session->completed() != block.generation)
            throw std::runtime_error("Continuous native generation mismatch");
          std::vector<std::uint8_t> reply;
          if(resident) {
            reply.resize(MNM_CONT_ACK_BYTES);
            mnm_ack_encode(reply.data(),block.request.data(),unsigned(block.generation),
                           unsigned(block.generation>>32),unsigned(nonce),unsigned(nonce>>32));
          } else reply = rollingHandoffReply(handoff, RollingCompletion{1, std::move(image)});
          channel.reply(block.generation, reply);
          if (block.generation == 1)
            coldDeliveryMs = ms(started);
          QJsonObject row{{"generation", QString::number(block.generation)},
                          {"source_acquire_ms", acquired},
                          {"prepare_ms", prepared - acquired},
                          {"source_and_prepare_ms", prepared},
                          {"native_consume_ms", consumed - prepared},
                          {"request_to_reply_ms", ms(started)},
                          {"native_work_ms", session->nativeWorkMs()},
                          {"diagnostic_ms", session->checkpointAssertionMs()},
                          {"rasters", int(handoff.batch.header[14])}};
          const auto &detail = session->profile();
          row["gpu_adopt_ms"] = detail.adoptMs;
          row["gpu_prepare_ms"] = detail.prepareMs;
          row["gpu_submit_ms"] = detail.submitMs;
          row["gpu_readback_ms"] = detail.readbackMs;
          row["gpu_compare_ms"] = detail.compareMs;
          row["visible_draws"] = double(detail.visibleDraws);
          row["source_identity_hashes"] = double(detail.identityHashes);
          row["source_identity_hits"] = double(detail.identityHits);
          if (samples.size() == 1024)
            samples.removeFirst();
          samples.append(row);
          awaiting = Awaiting{std::move(block), std::move(handoff), started};
          lastProgress = Clock::now();
        }
      }
      if (channel.finished()) {
        if (awaiting || !target || returned != target)
          throw std::runtime_error("Premature original continuous finish");
        if (lastPainted == returned)
          app.quit();
      }
    } catch (const std::exception &e) {
      error = QString::fromUtf8(e.what());
      channel.cancel();
      app.quit();
    }
  });
  timer.start(1);
  app.exec();
  timer.stop();
  if (!channel.finished())
    channel.cancel();
  const auto elapsed = renderer ? ms(sessionStart) : 0;
  viewport.makeCurrent();
  viewport.setGpuFrame({});
  const auto completed = session ? session->completed() : 0;
  session.reset();
  const auto remaining = renderer ? renderer->stats().surfaces : 0;
  QJsonArray histogram;
  for (auto count : warmHistogram)
    histogram.append(QString::number(count));
  save(QDir(output).filePath("live-report.json"),
       {{"success", error.isEmpty() && target && returned == target &&
                        completed == target},
        {"error", error},
        {"driver", driver},
        {"original_returns", QString::number(returned)},
        {"native_completions", QString::number(completed)},
        {"rasters", QString::number(rasters)},
        {"qt_paints", QString::number(paints)},
        {"coalesced_presentations", QString::number(unpainted)},
        {"elapsed_ms", elapsed},
        {"verify", verify},
        {"resident", resident},
        {"world_readbacks", double(renderer ? renderer->stats().nativeReadbacks : 0)},
        {"native_gpu_uploads", double(renderer ? renderer->stats().uploads : 0)},
        {"pixel_replies", resident ? QString("0") : QString::number(returned)},
        {"original_world_writebacks", resident ? QString("0") : QString::number(returned)},
        {"checked_resident_returns", resident ? QString::number(returned) : QString("0")},
        {"cpu_reference_queues",
         verify ? QString::number(completed) : QString("0")},
        {"cold_delivery_ms", coldDeliveryMs},
        {"warm_original_fps",
         returned > 64 && warmLastTick != warmFirstTick
             ? 1000.0 * double(returned - 64) /
                   double(std::uint32_t(warmLastTick - warmFirstTick))
             : 0.0},
        {"warm_qt_paint_fps",
         warmPaints > 1 ? 1000.0 * double(warmPaints - 1) /
                              std::chrono::duration<double, std::milli>(
                                  warmLastPaintTime - warmFirstPaintTime)
                                  .count()
                        : 0.0},
        {"rss_warm_bytes", double(rssWarm)},
        {"rss_maximum_bytes", double(rssMaximum)},
        {"producer_cache_maximum_bytes", double(producerCacheMax)},
        {"gpu_cache_maximum_frames", double(gpuFramesMax)},
        {"peak_source_bytes", double(peakBlock)},
        {"peak_packet_bytes", double(peakPacket)},
        {"channel_bytes", double(MNM_CONT_BYTES)},
        {"diagnostic_files", 0},
        {"remaining_surfaces", double(remaining)},
        {"image_uploads", double(viewport.imageUploads())},
        {"samples", samples},
        {"all_warm_world_ms", QJsonObject{{"histogram", histogram},
           {"count", QString::number(warmCount)}, {"maximum", double(warmMaximum)}}},
        {"original_pixels_used_as_native_inputs", false}});
  return error.isEmpty() && target && returned == target ? 0 : 1;
}
