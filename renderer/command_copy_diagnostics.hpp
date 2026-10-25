#pragma once
#include "blit.hpp"
#include <QString>
namespace mnm::render {
struct SurfaceCommand;
// Explicit diagnostic readbacks only. Captured/expected pixels never enter a
// rendering operation, World provider, CPU reply or facade history transfer.
class CommandCopyDiagnostics final {
public:
    enum class Selection { WorldAncestry, AllCopies };
    explicit CommandCopyDiagnostics(const QString& directory,Selection selection=Selection::WorldAncestry);
    ~CommandCopyDiagnostics();
    bool before(const SurfaceCommand&,GlBlitter&,SurfaceId source,SurfaceId target);
    void after(const SurfaceCommand&,GlBlitter&,SurfaceId target);
    void committed(const SurfaceCommand&);
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
