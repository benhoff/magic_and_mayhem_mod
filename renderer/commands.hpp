#pragma once
#include "blit.hpp"
#include <QByteArray>
#include <array>
#include <functional>

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
    unsigned checks=0,presents=0,colorChecks=0;
    RenderStats stats;
    Driver driver;
};
CommandResult replayCommands(const std::vector<SurfaceCommand>& commands);
// Decoded, complete bounded replay. PRESENT sends a GPU lease; CHECK operations
// retain their explicit diagnostic readbacks. Callback runs on the GUI thread.
CommandResult replayCommandsGpu(const std::vector<SurfaceCommand>& commands,
                               GlBlitter& renderer,const std::function<void(GpuFrame)>& present);
}
