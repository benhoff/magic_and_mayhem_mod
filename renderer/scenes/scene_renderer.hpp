#pragma once
#include "resource_cache.hpp"
#include <chrono>

namespace mnm::render {
// Ordered, owned display data. The scene adapter supplies projected anchors and
// the complete order (including its equal-depth policy). No simulation is run.
struct AdditiveRectangle {int width=0,height=0;std::array<std::uint16_t,3> channels{};};
struct ColourRectangle {Image pixels;};
struct SceneDraw {
    assets::ResourceId resource;
    std::size_t frame=0;
    int anchorX=0,anchorY=0;
    bool visible=true,clip=true;
    std::optional<SpriteColourTable> colours{};
    std::optional<Rect> viewport{};
    SpriteComposite composite{};
    std::optional<AdditiveRectangle> additive{};
    std::optional<ColourRectangle> colourRectangle{};
};
struct SceneLimits {
    std::size_t draws=12320;
    ResourceCacheLimits cache;
};
struct SceneBatchBudget {
    std::size_t draws=32,uploadBytes=256*1024;
    std::chrono::microseconds time{2000};
};
struct SceneUploadNeed {std::size_t draw=0;ResourceUploadRequest request;};
struct SceneUploadStats {std::uint64_t bytes=0,ticks=0,maxTickBytes=0;};
enum class SceneStart { background, retainedCanvas };
// GUI-thread persistent RGB565 canvas/background and resource upload ownership.
// Manager/renderer outlive this object. read/present are explicit sync points;
// draw retains GPU storage and never advances an ANI/simulation clock.
class SceneRenderer final {
public:
    SceneRenderer(GlBlitter&,assets::ResourceManager&,const Image& background,SceneLimits limits={});
    ~SceneRenderer();
    SceneRenderer(const SceneRenderer&)=delete;
    SceneRenderer& operator=(const SceneRenderer&)=delete;
    void draw(const std::vector<SceneDraw>&);
    // Admit once, then draw finite batches without presenting partial pixels.
    void beginFrame(const std::vector<SceneDraw>&,SceneStart start=SceneStart::background);
    // Explicit native producer handoff. Caller supplies fully defined owned
    // native pixels; original destination snapshots are not an initialization.
    void adoptNativeCanvas(const Image&);
    bool drawNext(std::size_t budget);
    bool drawNext(const SceneBatchBudget&);
    std::optional<SceneUploadNeed> uploadNeed() const;
    void supplyUpload(const SceneUploadNeed&,PreparedSpriteFrame);
    SceneUploadStats uploadStats() const {checkThread();return uploadStats_;}
    Image read();
    QImage present();
    GpuFrame presentGpu();
    ResourceCacheStats cacheStats() const;
    void release(const assets::ResourceId&);
private:
    std::vector<SceneDraw> pending_;
    std::size_t cursor_=0;
    bool drawing_=false;
    std::optional<SceneUploadNeed> need_;
    bool supplied_=false;
    SceneUploadStats uploadStats_;
    GlBlitter& renderer_;
    assets::ResourceManager& resources_;
    ResourceCache cache_;
    SceneLimits limits_;
    SurfaceId background_=0,canvas_=0;
    Rect viewport_;
    bool valid_=false;
    std::thread::id thread_;
    void checkThread() const;
    void validate(const SceneDraw&);
    std::optional<ResourceUploadRequest> request(const SceneDraw&);
};
}
