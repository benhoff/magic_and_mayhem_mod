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
  struct Profile {
    double adoptMs=0, prepareMs=0, submitMs=0, readbackMs=0, compareMs=0;
    unsigned visibleDraws=0;
    std::uint64_t copyBatches=0,snapshotPixels=0,scratchAllocations=0;
    std::uint64_t visualChecks=0,visualReuses=0,cacheUploads=0,cacheHits=0,cacheEvictions=0;
    std::size_t cacheFrames=0,cacheSurfaces=0;
    std::uint64_t identityHashes=0,identityHits=0,identityEvictions=0,identityBypasses=0;
    std::size_t identityFrames=0,identityBytes=0;
  };
  CanvasWorld(render::GlBlitter &, const assets::AssetStore &);
  ~CanvasWorld();
  void enablePreparedReplay(bool retainCompletedImage = false);
  render::GpuFrame replayPrepared();
  std::uint64_t preparedReplays() const { return preparedReplays_; }
  std::uint64_t retainedImageCaptures() const { return retainedImageCaptures_; }
  void begin(const CanvasProducer &, const render::Image &nativeHistory);
  void beginRolling(const CanvasProducer &,const render::Image &nativeHistory,std::uint64_t sequence);
  void beginResident(const CanvasProducer &, render::SurfaceId, std::uint64_t sequence);
  void append(const CanvasProducer &);
  render::Image complete(const render::Image &nativeReference);
  render::Image completeNormal();
  void completeResident();
  void end(const CanvasProducer &);
  render::GpuFrame presentGpu();
  bool active() const { return active_; }
  unsigned queue() const { return queue_; }
  unsigned canvas() const { return canvas_; }
  unsigned completedQueues() const { return completed_; }
  std::uint64_t completedSequence() const {return rolling_?rollingCompleted_:completed_;}
  std::uint64_t draws() const { return totalDraws_; }
  std::uint64_t readbacks() const { return readbacks_; }
  const Profile &profile() const { return profile_; }
  render::ColourRectangleCacheStats colourCacheStats() const { return scene_?scene_->colourCacheStats():render::ColourRectangleCacheStats{}; }
private:
  render::GlBlitter &renderer_;
  assets::AssetStore store_;
  assets::ResourceManager resources_;
  WorldResources bindings_;
  std::unique_ptr<render::SceneRenderer> scene_;
  WorldFrame pending_;
  void beginFrame(const CanvasProducer&,const render::Image&);
  render::Image completeFrame(const render::Image*, bool exportPixels = true);
  unsigned queue_=0,canvas_=0,completed_=0,queueDraws_=0;
  std::uint64_t rollingSequence_=0,rollingCompleted_=0,totalDraws_=0,readbacks_=0;
  render::SurfaceId residentSurface_=0;
  bool rolling_=false, resident_=false;
  bool active_=false,checked_=false;
  Profile profile_;
  bool preparedReplay_=false,baselineCaptured_=false;
  bool retainCompletedImage_=false;
  render::SurfaceId replayBaseline_=0;
  std::vector<render::SceneDraw> replayDraws_;
  std::uint64_t preparedReplays_=0;
  std::uint64_t retainedImageCaptures_=0;
};
}
