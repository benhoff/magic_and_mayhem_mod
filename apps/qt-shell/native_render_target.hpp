#pragma once
#include "blit.hpp"
#include <QString>
#include <functional>
class GlViewport;
class QOffscreenSurface;
// Presentation boundary: a render owner never needs to access a QWidget.
struct NativeRenderTarget {
    std::function<bool()> ready;
    std::function<QOpenGLContext*()> context;
    std::function<void(mnm::render::GpuFrame)> present;
    std::function<QString()> error;
    QOffscreenSurface* offscreen=nullptr;
    QOffscreenSurface* worldOffscreen=nullptr;
};
NativeRenderTarget viewportRenderTarget(GlViewport&);
