#pragma once
#include "blit.hpp"
#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

namespace mnm::render {
// Local native service, not a wire contract. Platform surface/share context
// outlive join(); release all consumer frame leases before joining. The owner
// keeps both immutable slots until their final consumer references return.
class GpuProductionQueue final {
public:
    using Work=std::function<GpuFrame(GlBlitter&)>;
    using Clock=std::chrono::steady_clock;
    struct Completion {
        std::uint64_t sequence=0;
        GpuFrame frame;
        std::shared_ptr<void> metadata;
        Clock::time_point started{},completed{};
    };
    struct Stats {std::uint64_t submitted=0,produced=0,taken=0,allocations=0;std::size_t outstanding=0,highWater=0;bool stopped=false;std::string error;};
    GpuProductionQueue(QOpenGLContext*,QOffscreenSurface*,std::function<void()> cleanup={});
    ~GpuProductionQueue();
    GpuProductionQueue(const GpuProductionQueue&)=delete;
    bool canSubmit() const;
    bool submit(std::uint64_t,Work,std::shared_ptr<void> metadata={});
    bool take(Completion&);
    Stats stats() const;
    // Serialized on the production owner; caller has quiesced frame sampling.
    void recoverContext(QOpenGLContext*);
    void join();
private:
    struct Job {std::uint64_t sequence;Work work;std::shared_ptr<void> metadata;};
    void run();
    QOpenGLContext* share_;
    QOffscreenSurface* surface_;
    std::function<void()> cleanup_;
    mutable std::mutex mutex_;
    std::condition_variable wake_;
    std::deque<Job> jobs_;
    std::deque<Completion> ready_;
    Stats state_;
    bool stopping_=false;
    bool recoveryRequested_=false;
    QOpenGLContext* recoveryShare_=nullptr;
    std::thread owner_;
};
}
