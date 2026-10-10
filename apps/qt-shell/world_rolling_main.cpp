#include "../../compat/legacy/world_rolling_handoff.hpp"
#include "../../compat/legacy/world_rolling_session.hpp"
#include "gl_viewport.hpp"
#include <QApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QSaveFile>
#include <QTimer>
#include <atomic>
#include <chrono>
#include <cstring>
#include <mutex>
#include <thread>
using namespace mnm::legacy;
namespace {
using Clock = std::chrono::steady_clock;
double ms(Clock::time_point t) {
  return std::chrono::duration<double, std::milli>(Clock::now() - t).count();
}
std::vector<std::uint8_t> read(const QString &path, qint64 bound) {
  QFile f(path);
  if (!f.open(QIODevice::ReadOnly) || f.size() > bound)
    throw std::runtime_error("Unavailable bounded live input");
  auto b = f.readAll();
  return {b.begin(), b.end()};
}
unsigned word(const std::vector<std::uint8_t> &b, std::size_t at) {
  if (at > b.size() || b.size() - at < 4)
    throw std::invalid_argument("Truncated live word");
  return b[at] | unsigned(b[at + 1]) << 8 | unsigned(b[at + 2]) << 16 |
         unsigned(b[at + 3]) << 24;
}
void save(const QString &path, const QByteArray &bytes) {
  if (QFileInfo::exists(path))
    throw std::runtime_error("Refusing existing native completion");
  QSaveFile f(path);
  if (!f.open(QIODevice::WriteOnly) || f.write(bytes) != bytes.size() ||
      !f.commit())
    throw std::runtime_error("Cannot publish native completion");
}
QByteArray pixels(const mnm::render::Image &image) {
  QByteArray b(qsizetype(image.pixels.size() * 2), Qt::Uninitialized);
  for (std::size_t i = 0; i < image.pixels.size(); ++i) {
    b[qsizetype(i * 2)] = char(image.pixels[i]);
    b[qsizetype(i * 2 + 1)] = char(image.pixels[i] >> 8);
  }
  return b;
}
struct BrokerResult {
  QJsonArray batches, rasters, deliveries;
  unsigned records = 0, queues = 0;
  QString error;
};
} // namespace
int main(int argc, char **argv) {
  QApplication app(argc, argv);
  const auto args = app.arguments();
  if (args.size() != 6 || args[4] != "--world-handoff" ||
      args[5] != "--world-batch")
    return 2;
  const QString input = args[1], root = args[2], output = args[3];
  if (!QDir().mkpath(output) ||
      QFileInfo::exists(QDir(output).filePath("ready")))
    return 2;
  const auto channelPath = QDir(output).filePath("rolling-channel.bin");
  auto nonce = QRandomGenerator::system()->generate64();
  if (!nonce)
    nonce = 1;
  WorldRollingChannel::create(channelPath, nonce);
  std::atomic<bool> stopped{false}, brokerDone{false};
  std::mutex resultMutex;
  BrokerResult broker;
  std::thread producer([&] {
    BrokerResult result;
    try {
      WorldRollingChannel channel(channelPath, nonce,
                                  WorldRollingChannel::Role::producer);
      CanvasProducerStream stream{0, {}};
      std::size_t offset = 64, cursor = 0;
      unsigned prior = 0;
      std::optional<RollingHandoff> awaiting;
      const QDir directory = QFileInfo(input).dir();
      auto deadline = Clock::now() + std::chrono::seconds(120);
      std::vector<std::uint8_t> initialHeader;
      while (!stopped) {
        if (channel.cancelled())
          throw std::runtime_error("Rolling consumer cancelled");
        if (Clock::now() > deadline)
          throw std::runtime_error(
              "Original producer or native completion deadline");
        if (!QFileInfo::exists(input)) {
          std::this_thread::sleep_for(std::chrono::milliseconds(2));
          continue;
        }
        QFile f(input);
        if (!f.open(QIODevice::ReadOnly) || f.size() > 128 * 1024 * 1024 ||
            (f.size() < qint64(offset) && !initialHeader.empty()))
          throw std::runtime_error("Original producer tail changed extent");
        const auto head = f.read(64);
        if (head.size() < 64) {
          std::this_thread::sleep_for(std::chrono::milliseconds(2));
          continue;
        }
        std::vector<std::uint8_t> header(head.begin(), head.end());
        if (initialHeader.empty())
          initialHeader = header;
        else if (header != initialHeader)
          throw std::runtime_error("Original producer envelope changed");
        if (!f.seek(offset))
          throw std::runtime_error("Cannot seek original producer tail");
        auto tail = f.readAll();
        std::vector<std::uint8_t> raw(tail.begin(), tail.end());
        std::size_t end = 0;
        while (raw.size() - end >= 96) {
          auto n = word(raw, end);
          if (n < 96 || n > 128 * 1024 * 1024)
            throw std::runtime_error("Invalid original producer extent");
          if (n > raw.size() - end)
            break;
          end += n;
        }
        if (end) {
          header.insert(header.end(), raw.begin(), raw.begin() + end);
          appendCanvasProducers(stream, header, false);
          offset += end;
          deadline = Clock::now() + std::chrono::seconds(30);
        }
        if (awaiting &&
            stream.operations.size() >= awaiting->checkpointSequence + 1) {
          verifyRollingHandoffReturn(
              *awaiting, stream.operations.at(awaiting->checkpointSequence - 1),
              stream.operations.at(awaiting->checkpointSequence));
          cursor = awaiting->checkpointSequence + 1;
          awaiting.reset();
          ++result.queues;
        }
        if (!awaiting && result.queues < 32) {
          const auto base = QString("world-batch-%1")
                                .arg(result.queues + 1, 4, 10, QChar('0'));
          const auto path = directory.filePath(base + ".request");
          if (QFileInfo(path).size() >= 64) {
            auto request = read(path, 64 + 12320 * 16);
            const auto extent = word(request, 44);
            if (extent > 12320 * 16)
              throw std::runtime_error("Original descriptors exceed bound");
            if (request.size() == 64 + extent &&
                word(request, 24) <= stream.operations.size()) {
              const auto started = Clock::now();
              auto handoff = prepareRollingHandoff(stream, cursor, request,
                                                   result.queues + 1, prior);
              if (!channel.publish(handoff.packet))
                throw std::runtime_error("Unexpected outstanding native queue");
              std::optional<RollingCompletion> reply;
              const auto replyDeadline =
                  Clock::now() + std::chrono::seconds(30);
              while (!stopped && !reply) {
                reply = channel.reap();
                if (Clock::now() > replyDeadline)
                  throw std::runtime_error("Native rolling reply deadline");
                if (!reply)
                  std::this_thread::sleep_for(std::chrono::milliseconds(1));
              }
              if (!reply)
                throw std::runtime_error("Native rolling session closed");
              auto bytes = rollingHandoffReply(handoff, *reply);
              save(directory.filePath(base + ".reply"),
                   {reinterpret_cast<const char *>(bytes.data()),
                    qsizetype(bytes.size())});
              result.deliveries.append(
                  QJsonObject{{"queue", int(result.queues + 1)},
                              {"request_to_reply_ms", ms(started)},
                              {"packet_bytes", double(handoff.packet.size())}});
              QJsonArray sequences;
              for (const auto &d : handoff.batch.descriptors) {
                sequences.append(int(d[0]));
                result.rasters.append(
                    QJsonObject{{"queue", int(result.queues + 1)},
                                {"sequence", int(d[0])},
                                {"original_entry", double(d[1])},
                                {"return_ax", int(d[2])},
                                {"source_checksum", double(d[3])}});
              }
              const auto &h = handoff.batch.header;
              result.batches.append(QJsonObject{
                  {"queue", int(h[5])},
                  {"ordinal", int(h[4])},
                  {"sequence", int(h[6])},
                  {"canvas", int(h[7])},
                  {"rasters", int(h[14])},
                  {"cumulative_rasters", int(h[15])},
                  {"wire_base", base},
                  {"guarded", true},
                  {"sequences", sequences},
                  {"pixels", 480000},
                  {"reply_sha256",
                   QString::fromLatin1(
                       QCryptographicHash::hash(
                           {reinterpret_cast<const char *>(bytes.data()),
                            qsizetype(bytes.size())},
                           QCryptographicHash::Sha256)
                           .toHex())}});
              prior = h[15];
              awaiting = std::move(handoff);
              deadline = Clock::now() + std::chrono::seconds(30);
            }
          }
        }
        const auto donePath = directory.filePath("canvas-producers.done");
        if (QFileInfo(donePath).size() == 32) {
          auto marker = read(donePath, 32);
          // A completion marker may appear after this iteration read the tail.
          // Consume its published source bytes before validating final counts.
          if (word(marker, 28) > offset &&
              word(marker, 28) <= 128u * 1024u * 1024u)
            continue;
          if (std::memcmp(marker.data(), "MNMPDONE", 8) ||
              word(marker, 8) != 1 || word(marker, 12) != 32 ||
              word(marker, 16) != stream.operations.size() ||
              word(marker, 20) || word(marker, 28) != offset || awaiting ||
              result.queues != 32 || raw.size() != end)
            throw std::runtime_error(
                "Original final marker did not close rolling acknowledgements");
          decodeCanvasProducers(read(input, 128 * 1024 * 1024));
          result.records = stream.operations.size();
          break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
      if (stopped)
        throw std::runtime_error(
            "Rolling session cancelled before original completion");
    } catch (const std::exception &e) {
      result.error = QString::fromUtf8(e.what());
    }
    {
      std::lock_guard<std::mutex> lock(resultMutex);
      broker = std::move(result);
    }
    brokerDone = true;
  });
  GlViewport viewport;
  viewport.setWindowTitle("Magic & Mayhem native canvas sequence");
  viewport.resize(800, 600);
  viewport.show();
  WorldRollingChannel consumer(channelPath, nonce,
                               WorldRollingChannel::Role::consumer);
  std::unique_ptr<mnm::render::GlBlitter> renderer;
  std::unique_ptr<WorldRollingSession> session;
  mnm::render::SurfaceId mirror = 0;
  QSize mirrorSize;
  QJsonArray checkpoints, frames;
  QString error;
  unsigned presentations = 0;
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
        auto store = mnm::assets::AssetStore::create(root.toStdString());
        if (auto *e = std::get_if<mnm::assets::Error>(&store))
          throw std::runtime_error(e->detail);
        session = std::make_unique<WorldRollingSession>(
            *renderer, std::get<mnm::assets::AssetStore>(std::move(store)));
        save(QDir(output).filePath("ready"),
             QJsonDocument(
                 QJsonObject{
                     {"pid", qint64(QCoreApplication::applicationPid())}})
                 .toJson());
      }
      if (auto packet = consumer.poll()) {
        viewport.makeCurrent();
        auto image = session->consume(
            packet->sequence, packet->bytes,
            [&](const CanvasProducer &c, const mnm::render::Image &image) {
              const auto raw = pixels(image);
              const auto name =
                  QString("native-%1.565").arg(c.fields[14], 4, 10, QChar('0'));
              save(QDir(output).filePath(name), raw);
              // Diagnostic mirror assertions are separate from World
              // composition.
              const QSize size(image.width, image.height);
              if (size != mirrorSize) {
                viewport.setGpuFrame({});
                if (mirror)
                  renderer->destroy(mirror);
                mirror = renderer->allocate(image.width, image.height,
                                            {16, {0xf800, 0x7e0, 0x1f}});
                mirrorSize = size;
              }
              renderer->update(mirror, 0, 0, image);
              if (renderer->read(mirror).pixels != image.pixels)
                throw std::runtime_error("Native diagnostic mirror differs");
              if (size == QSize(800, 600)) {
                viewport.setGpuFrame(renderer->presentGpu(mirror));
                ++presentations;
              }
              checkpoints.append(QJsonObject{
                  {"sequence", int(c.fields[16])},
                  {"canvas", int(c.fields[3])},
                  {"oracle", int(c.fields[14])},
                  {"path", name},
                  {"sha256",
                   QString::fromLatin1(
                       QCryptographicHash::hash(raw, QCryptographicHash::Sha256)
                           .toHex())},
                  {"gpu_equal", true}});
            },
            [&](const CanvasProducer &, const mnm::render::Image &history) {
              save(QDir(output).filePath(
                       QString("world-entry-%1.565")
                           .arg(packet->sequence, 4, 10, QChar('0'))),
                   pixels(history));
            });
        consumer.complete(packet->sequence, image);
        const auto &p = session->profile();
        frames.append(QJsonObject{
            {"queue", int(packet->sequence)},
            {"gpu_world_equal", true},
            {"native_work_ms", session->nativeWorkMs()},
            {"checkpoint_diagnostics_ms", session->checkpointAssertionMs()},
            {"profile", QJsonObject{{"resource_prepare_ms", p.prepareMs},
                                    {"gpu_submit_ms", p.submitMs},
                                    {"gpu_readback_ms", p.readbackMs},
                                    {"gpu_compare_ms", p.compareMs}}}});
      }
      if (brokerDone) {
        std::lock_guard<std::mutex> lock(resultMutex);
        if (!broker.error.isEmpty())
          throw std::runtime_error(broker.error.toStdString());
        app.quit();
      }
    } catch (const std::exception &e) {
      error = QString::fromUtf8(e.what());
      consumer.cancel();
      stopped = true;
      app.quit();
    }
  });
  timer.start(1);
  app.exec();
  timer.stop();
  consumer.cancel();
  stopped = true;
  producer.join();
  if (error.isEmpty() && !broker.error.isEmpty())
    error = broker.error;
  viewport.makeCurrent();
  viewport.setGpuFrame({});
  session.reset();
  if (renderer && mirror)
    renderer->destroy(mirror);
  const auto remaining = renderer ? renderer->stats().surfaces : 0;
  QJsonObject report{
      {"success",
       error.isEmpty() && broker.queues == 32 && frames.size() == 32},
      {"error", error},
      {"records", int(broker.records)},
      {"queues", int(broker.queues)},
      {"world_queues", frames.size()},
      {"world_handoff", true},
      {"world_raster_batch", true},
      {"complete_raster_queues", true},
      {"native_canvas_writebacks", broker.batches.size()},
      {"world_readbacks", frames.size()},
      {"native_bypass_count", broker.rasters.size()},
      {"native_bypass_replies", broker.rasters},
      {"native_batch_replies", broker.batches},
      {"world_frames", frames},
      {"checkpoints", checkpoints},
      {"presentations", int(presentations)},
      {"remaining_surfaces", double(remaining)},
      {"viewport_image_uploads", double(viewport.imageUploads())},
      {"deliveries", broker.deliveries},
      {"original_pixels_used_as_native_inputs", false},
      {"original_oracles_read", false},
      {"wire_version", 3},
      {"rolling_adapter", true},
      {"profile", QJsonObject{{"cpu_composition_ms", 0},
                              {"checkpoint_diagnostics_ms", 0},
                              {"stream_ingest_ms", 0}}}};
  save(QDir(output).filePath("live-report.json"),
       QJsonDocument(report).toJson());
  return report["success"].toBool() ? 0 : 1;
}
