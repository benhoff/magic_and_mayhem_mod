#pragma once
#include "canvas_producers.hpp"
#include "world_resources.hpp"
#include <memory>
namespace mnm::legacy {
WorldDraw producerWorldDraw(const CanvasProducer &);
// Native World consumes only source requests and native producer history.
// CPU producer composition remains an independent assertion, never a GPU seed
// after a World batch starts. Completed GPU words become the retained history.
class CanvasWorld final {
public:
  CanvasWorld(render::GlBlitter &, const assets::AssetStore &);
  void begin(const CanvasProducer &, const render::Image &nativeHistory);
  void append(const CanvasProducer &);
  render::Image complete(const render::Image &nativeReference);
  void end(const CanvasProducer &);
  bool active() const { return active_; }
  unsigned queue() const { return queue_; }
  unsigned canvas() const { return canvas_; }
  unsigned completedQueues() const { return completed_; }
  unsigned draws() const { return totalDraws_; }
private:
  render::GlBlitter &renderer_;
  assets::AssetStore store_;
  assets::ResourceManager resources_;
  WorldResources bindings_;
  std::unique_ptr<render::SceneRenderer> scene_;
  WorldFrame pending_;
  unsigned queue_=0,canvas_=0,completed_=0,totalDraws_=0,queueDraws_=0;
  bool active_=false,checked_=false;
};
}
