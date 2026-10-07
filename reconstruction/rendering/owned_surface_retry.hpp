#pragma once
#include "surface_retry.hpp"
#include "../../renderer/surface_backend.hpp"
#include <functional>

namespace mnm::reconstruction {
// Recovered wrapper actions over an owned backend. Asset callbacks provide
// explicit bytes; successful Restore alone never defines restored contents.
class OwnedSurfaceRetryAdapter:public SurfaceRetryAdapter {
public:
    using Backend=render::SurfaceBackend;
    using SurfaceId=render::SurfaceId;
    using Reload=std::function<void(Backend&,SurfaceId)>;
    struct Callbacks {
        Reload source,destination,cursor;
        std::optional<SurfaceId> cursorSurface;
        std::function<void(std::uint32_t)> report;
        std::optional<Backend::Mode> display;
    };
private:
    Backend& backend_;
    std::optional<SurfaceId> source_;
    SurfaceId destination_;
    render::Rect sourceRect_,destinationRect_;
    bool fullFill_;
    Callbacks callbacks_;
    SurfaceId target(RecoverySurface id) const {
        if(id==RecoverySurface::Destination)return destination_;
        if(id!=RecoverySurface::Source || !source_)throw std::runtime_error("Recovery source is unavailable");
        return *source_;
    }
public:
    OwnedSurfaceRetryAdapter(Backend& backend,std::optional<SurfaceId> source,SurfaceId destination,
                             render::Rect sourceRect,render::Rect destinationRect,bool fullFill,
                             Callbacks callbacks)
        :backend_(backend),source_(source),destination_(destination),sourceRect_(sourceRect),
         destinationRect_(destinationRect),fullFill_(fullFill),callbacks_(std::move(callbacks)){}
    std::uint32_t draw(bool fast,bool fill,std::uint32_t flags,std::uint16_t color) override {
        if(fill){
            // The recovered wrapper's lost-draw branch is admission from its
            // platform adapter. Standalone Wine COLORFILL can succeed while lost.
            // This native wrapper policy refuses until explicitly restored.
            if(const auto h=backend_.isLost(destination_))return h;
            return backend_.fill(destination_,fullFill_?std::nullopt:std::optional(destinationRect_),color,flags).hresult;
        }
        if(!source_)throw std::runtime_error("Copy source is unavailable");
        render::SurfaceCopyRequest request;request.api=fast?render::SurfaceCopyApi::BltFast:render::SurfaceCopyApi::Blt;
        request.source=sourceRect_;request.destination=destinationRect_;request.flags=flags;
        return backend_.copy(*source_,destination_,request).hresult;
    }
    std::uint32_t restore(RecoverySurface id) override {return backend_.restore(target(id),callbacks_.display);}
    std::uint32_t setSourceKey(RecoverySurface id,std::uint16_t key) override {backend_.setSourceKey(target(id),key);return 0;}
    void reload(RecoverySurface id) override {
        auto& callback=id==RecoverySurface::Source?callbacks_.source:callbacks_.destination;
        if(!callback)throw std::runtime_error("Recovery callback is not installed");
        callback(backend_,target(id));
    }
    void reloadCursorBitmap() override {
        if(!callbacks_.cursor || !callbacks_.cursorSurface)throw std::runtime_error("Global cursor recovery is not installed");
        callbacks_.cursor(backend_,*callbacks_.cursorSurface);
    }
    void report(std::uint32_t hresult) override {if(callbacks_.report)callbacks_.report(hresult);}
};
}
