#pragma once
#include "blit.hpp"
#include <QByteArray>
#include <array>

namespace mnm::render {
constexpr qint64 maxCommandBytes=64*1024*1024;
struct SurfaceCommand {
    unsigned operation=0;
    std::array<std::uint32_t,10> words{};
    Image image;
    PixelFormat format;
    std::vector<Rgb> colors;
    QByteArray expected;
};
// Complete bounded streams only. Validate ordering, handles and geometry before GL.
std::vector<SurfaceCommand> decodeCommands(const QByteArray& data);
struct CommandResult {
    QByteArray native;
    QImage presentation;
    unsigned checks=0,presents=0;
    RenderStats stats;
    Driver driver;
};
CommandResult replayCommands(const std::vector<SurfaceCommand>& commands);
}
