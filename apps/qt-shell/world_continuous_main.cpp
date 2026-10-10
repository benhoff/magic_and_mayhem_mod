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
#include <QRandomGenerator>
#include <QSaveFile>
#include <QTimer>
#include <chrono>
#include <cstring>
#include <optional>
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
  if (args.size() != 5)
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
  WorldContinuousChannel::create(path, nonce, target);
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
  QString error;
  auto lastProgress = Clock::now();
  Clock::time_point sessionStart;
  Clock::time_point requestedPaint;
  std::uint64_t presentation = 0;
  QObject::connect(&viewport, &QOpenGLWidget::frameSwapped, [&] {
    if (presentation && presentation != lastPainted) {
      ++paints;
      lastPainted = presentation;
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
            *renderer, std::get<mnm::assets::AssetStore>(std::move(store)));
        save(QDir(output).filePath("ready"),
             {{"pid", qint64(QCoreApplication::applicationPid())},
              {"nonce", QString::number(nonce)}});
        sessionStart = Clock::now();
        lastProgress = sessionStart;
      }
      if (channel.cancelled())
        throw std::runtime_error("Original continuous session cancelled");
      if (ms(lastProgress) > 120000)
        throw std::runtime_error("Continuous producer/reply deadline");
      if (awaiting) {
        std::vector<std::uint8_t> actual;
        if (channel.returned(awaiting->block.generation, actual)) {
          if (actual.size() != awaiting->block.source.size() + 192 ||
              !std::equal(awaiting->block.source.begin(),
                          awaiting->block.source.end(), actual.begin()))
            throw std::runtime_error(
                "Original changed published continuous sources");
          auto closed = decodeCanvasProducers(actual, false);
          verifyRollingHandoffReturn(
              awaiting->handoff,
              closed.operations.at(awaiting->handoff.checkpointSequence - 1),
              closed.operations.at(awaiting->handoff.checkpointSequence));
          rasters += awaiting->handoff.batch.header[14];
          returned = awaiting->block.generation;
          auto row = samples.last().toObject();
          row["native_request_to_original_return_ms"] = ms(awaiting->started);
          const auto t = channel.ticks(returned);
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
          auto handoff = prepareRollingHandoff(stream, 0, block.request, 1, 0);
          const auto prepared = ms(started);
          peakPacket = std::max(peakPacket, handoff.packet.size());
          viewport.makeCurrent();
          auto image = session->consume(block.generation, handoff.packet);
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
          auto reply = rollingHandoffReply(
              handoff, RollingCompletion{1, std::move(image)});
          channel.reply(block.generation, reply);
          QJsonObject row{{"generation", QString::number(block.generation)},
                          {"source_acquire_ms", acquired},
                          {"prepare_ms", prepared - acquired},
                          {"native_consume_ms", consumed - prepared},
                          {"request_to_reply_ms", ms(started)},
                          {"native_work_ms", session->nativeWorkMs()},
                          {"diagnostic_ms", session->checkpointAssertionMs()},
                          {"rasters", int(handoff.batch.header[14])}};
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
  save(QDir(output).filePath("live-report.json"),
       {{"success", error.isEmpty() && target && returned == target &&
                        completed == target},
        {"error", error},
        {"original_returns", QString::number(returned)},
        {"native_completions", QString::number(completed)},
        {"rasters", QString::number(rasters)},
        {"qt_paints", QString::number(paints)},
        {"coalesced_presentations", QString::number(unpainted)},
        {"elapsed_ms", elapsed},
        {"peak_source_bytes", double(peakBlock)},
        {"peak_packet_bytes", double(peakPacket)},
        {"channel_bytes", double(MNM_CONT_BYTES)},
        {"diagnostic_files", 0},
        {"remaining_surfaces", double(remaining)},
        {"image_uploads", double(viewport.imageUploads())},
        {"samples", samples},
        {"original_pixels_used_as_native_inputs", false}});
  return error.isEmpty() && target && returned == target ? 0 : 1;
}
