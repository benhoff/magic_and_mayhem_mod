#pragma once
#include "blit.hpp"
#include <QByteArray>

namespace mnm::render {
constexpr qint64 maxCaptureBytes=128+256*256*4+2*2048*2048*4+2048;
// Validate MNMBLT01, extract drawing inputs, and skip the recorded output.
Blit decodeCapture(const QByteArray& data);
QByteArray encodeNative(const Image& image,unsigned bits);
}
