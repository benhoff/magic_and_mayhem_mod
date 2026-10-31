#pragma once
#include "blit.hpp"
#include <QString>
#include <QJsonObject>
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
    // Optional diagnostic registration, implemented by the presentation owner.
    // A backend never accesses a widget or carries a host pointer on a wire.
    std::function<void(std::function<QJsonObject()>,const void*)> observeAcceptance={};
    std::function<void(const void*)> releaseAcceptance={};
};
NativeRenderTarget viewportRenderTarget(GlViewport&);
