#pragma once
#include "resource_cache.hpp"

namespace mnm::render {
// Ordered, owned display data. The scene adapter supplies projected anchors and
// the complete order (including its equal-depth policy). No simulation is run.
struct SceneDraw {
    assets::ResourceId resource;
    std::size_t frame=0;
    int anchorX=0,anchorY=0;
    bool visible=true,clip=true;
    std::optional<SpriteColourTable> colours{};
    std::optional<Rect> viewport{};
    SpriteComposite composite{};
};
struct SceneLimits {
    std::size_t draws=12320;
    ResourceCacheLimits cache;
};
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
    bool drawNext(std::size_t budget);
    Image read();
    QImage present();
    GpuFrame presentGpu();
    ResourceCacheStats cacheStats() const;
    void release(const assets::ResourceId&);
private:
    std::vector<SceneDraw> pending_;
    std::size_t cursor_=0;
    bool drawing_=false;
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
};
}
