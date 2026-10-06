#pragma once
#include "blit.hpp"
#include <QByteArray>
#include <array>
#include <functional>

namespace mnm::render {
constexpr qint64 maxCommandBytes=64*1024*1024;
enum class CommandStreamMode {Bounded,Streaming};
constexpr qint64 maxStreamingRecordBytes=12+28+2048*2048*4;
constexpr qint64 maxStreamingAppendBytes=65536;
struct SurfaceCommand {
    unsigned operation=0,sequence=0;
    std::array<std::uint32_t,10> words{};
    Image image;
    PixelFormat format;
    std::vector<Rgb> colors;
    QByteArray expected;
};
// Complete bounded streams only. Validate ordering, handles and geometry before GL.
std::vector<SurfaceCommand> decodeCommands(const QByteArray& data);
// Stateful framing; streaming bounds retained data rather than session totals.
class CommandDecoder final {
public:
    explicit CommandDecoder(CommandStreamMode mode=CommandStreamMode::Bounded);
    ~CommandDecoder();
    CommandDecoder(const CommandDecoder&)=delete;
    CommandDecoder& operator=(const CommandDecoder&)=delete;
    std::vector<SurfaceCommand> append(const QByteArray& bytes);
    void finish();
    void abort();
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
struct CommandResult {
    QByteArray native;
    QImage presentation;
    unsigned commands=0,checks=0,presents=0,colorChecks=0,skippedChecks=0,skippedColorChecks=0;
    std::size_t liveSurfaces=0,livePixels=0;
    RenderStats stats;
    Driver driver;
};
enum class CommandDiagnostics {Skip,Verify};
enum class CommandConsumerState {Active,Ended,Aborted,Failed};
struct CommandConsumerOptions {
    CommandDiagnostics diagnostics=CommandDiagnostics::Skip;
    // Explicit offline export mode; ordinary GPU execution never reads PRESENT.
    bool exportImages=false;
    CommandStreamMode stream=CommandStreamMode::Bounded;
};
// GUI-thread session. Renderer must outlive consumer; GPU leases may outlive both.
// Batches commit command by command, never transactionally. An execution or
// admission failure terminates the session and releases only its owned surfaces.
// CHECK records are structurally validated even when comparison is disabled.
class CommandConsumer final {
public:
    CommandConsumer(GlBlitter& renderer,std::function<void(GpuFrame)> present,
                    CommandConsumerOptions options={});
    ~CommandConsumer();
    CommandConsumer(const CommandConsumer&)=delete;
    CommandConsumer& operator=(const CommandConsumer&)=delete;
    void submit(const SurfaceCommand& command);
    void submit(const SurfaceCommand* commands,std::size_t count);
    // Signal producer completion. Requires an accepted END; missing END fails.
    void finish();
    void abort();
    CommandConsumerState state() const;
    const CommandResult& result() const;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
CommandResult replayCommands(const std::vector<SurfaceCommand>& commands);
// Decoded, complete bounded replay. PRESENT sends a GPU lease; CHECK operations
// retain their explicit diagnostic readbacks. Callback runs on the GUI thread.
CommandResult replayCommandsGpu(const std::vector<SurfaceCommand>& commands,
                               GlBlitter& renderer,const std::function<void(GpuFrame)>& present);
}
