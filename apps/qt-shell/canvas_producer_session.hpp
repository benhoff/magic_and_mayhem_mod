#pragma once
#include "../../compat/legacy/canvas_producers.hpp"
#include "../../compat/legacy/canvas_world.hpp"
#include "../../assets/path_resolver.hpp"
#include "../../renderer/blit.hpp"
#include <QObject>
#include <QElapsedTimer>
#include <QJsonArray>
#include <QJsonObject>
#include <QTimer>
#include <QFile>
#include <map>
#include <memory>
class GlViewport;
// GUI-thread lifecycle for a bounded closed producer stream. The viewport only
// accepts native GPU frames. Source resolution and original wire fields stay here.
class CanvasProducerSession final : public QObject {
public:
  CanvasProducerSession(GlViewport &, QString input, QString gameRoot, QString output, bool worldHandoff=false, bool worldBatch=false, QString proofPipe={});
  void start();
private:
  void tick();
  void finish(const QString &error = {});
  void bypassReply();
  void batchReply();
  void proof(const mnm::legacy::CanvasProducer&, unsigned type, const mnm::render::Image*);
  GlViewport &viewport_;
  QString input_, output_;
  mnm::assets::PathResolver resolver_;
  mnm::legacy::CanvasProducerReplay replay_;
  std::unique_ptr<mnm::render::GlBlitter> renderer_;
  std::unique_ptr<mnm::legacy::CanvasWorld> world_;
  QString gameRoot_;
  bool worldHandoff_=false;
  QJsonArray worldFrames_;
  QJsonArray bypasses_;
  QString rasterBase(unsigned ordinal) const;
  bool fullRasters_=false;
  bool worldBatch_=false,batchReplied_=false;
  std::size_t batchStart_=0;
  QJsonArray batches_;
  QFile proofPipe_;
  std::map<std::uint32_t, mnm::render::SurfaceId> surfaces_;
  std::map<std::string, std::vector<std::uint8_t>> assets_;
  QJsonObject sourceHashes_;
  QJsonArray checkpoints_;
  QByteArray accepted_;
  mnm::legacy::CanvasProducerStream pending_{0, {}};
  std::size_t applied_ = 0;
  unsigned queues_ = 0, presentations_ = 0;
  bool stopped_ = false;
  QElapsedTimer clock_;
  QTimer timer_;
};
