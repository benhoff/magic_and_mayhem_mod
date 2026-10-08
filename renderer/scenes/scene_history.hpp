#pragma once
#include "scene_renderer.hpp"

namespace mnm::render {
// Caller-owned logical identity and actual source sequence, independent of any
// runtime address or publication counter. A reset uses the constructor's native
// background. It does not recover an unknown original initialization.
struct CanvasStamp { std::uint64_t canvas=0,sequence=0; };
class SceneHistory final {
public:
    SceneHistory(GlBlitter&,assets::ResourceManager&,const Image& nativeBackground,SceneLimits limits={});
    void beginFrame(CanvasStamp,const std::vector<SceneDraw>&,bool reset);
    bool drawNext(std::size_t budget);
    Image read();
    GpuFrame presentGpu();
    CanvasStamp completed() const { return completed_; }
private:
    SceneRenderer scene_;
    CanvasStamp admitted_{},completed_{};
    std::size_t drawLimit_;
    bool drawing_=false,poisoned_=false;
};
}
