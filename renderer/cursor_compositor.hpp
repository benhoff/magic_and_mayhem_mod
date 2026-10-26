#pragma once
#include "surface_backend.hpp"
#include <memory>
#include <thread>

namespace mnm::render {
// Native policy. Private copies of input pixels replace borrowed legacy wrapper
// addresses. Publisher endpoints contain metadata only and can outlive the GPU
// owner. All OpenGL work and final resource destruction stay on the owner thread.
class CursorCompositor final {
    struct Shared;
    struct Generation;
public:
    using Token=std::uint64_t;
    enum class Publication {accepted,closed,retired,full,invalid};
    struct Frame {Rect atlas;int x=0,y=0;bool visible=true;};
    class Publisher final {
    public:
        Publisher()=default;
        Publication publish(Token,const Frame&,unsigned priority=0) const;
        // Cancels queued work. An owner-thread draw already in progress finishes
        // before owner-thread shutdown returns. Never destroys GPU resources.
        void stop() const;
    private:
        friend class CursorCompositor;
        explicit Publisher(std::shared_ptr<Shared> shared):shared_(std::move(shared)){}
        std::shared_ptr<Shared> shared_;
    };
    CursorCompositor();
    ~CursorCompositor();
    CursorCompositor(const CursorCompositor&)=delete;
    CursorCompositor& operator=(const CursorCompositor&)=delete;
    Publisher publisher() const;
    // Makes independently owned RGB565 canvas, atlas and saved-background
    // allocations. No backend surface ID or original COM alias escapes.
    Token create(const Image& canvas,const Image& atlas,int cellWidth=40,
                 int cellHeight=40,std::uint32_t sourceKey=31);
    void retire(Token); // Refuse new jobs; preserve already accepted generations.
    std::size_t collect(); // Owner-only release of retired, drained generations.
    std::size_t drain(); // Three priority passes, stable publication order.
    Image snapshot(Token);
    // Full background refresh preserves the current cursor, saving its new
    // underlay before repaint. This replaces pixels, never a borrowed alias.
    void background(Token,const Image&);
    // Checkpoint private pixels before a device/context loss. Publication stays
    // available while graphics are suspended; jobs retain their generation.
    void checkpointGraphics();
    void suspendGraphics(); // Physically destroy the backend/context.
    void recoverGraphics(); // Recreate exclusively from the exact checkpoint.
    bool graphicsSuspended() const;
    void shutdown(); // Owner thread; cancel queue and retire all resources.
    std::size_t surfaces() const;
private:
    std::thread::id owner_;
    std::shared_ptr<Shared> shared_;
    std::unique_ptr<SurfaceBackend> backend_;
    std::unordered_map<Token,std::unique_ptr<Generation>> generations_;
    void owner() const;
    void graphics() const;
    bool checkpointCurrent_=false;
    Generation& generation(Token);
    void restore(Generation&);
    void paint(Generation&,const Frame&);
    void release(Generation&);
};
}
