#pragma once
#include "surface_wrappers.hpp"
#include "../../renderer/surface_copy.hpp"
#include "owned_surface_retry.hpp"
#include <memory>
#include <string_view>

namespace mnm::reconstruction {
struct OwnedSurfaceCopyResult {
    bool called=false;
    render::SurfaceCopyResult result;
};
// One native API call. Recovered geometry and native backend admission remain
// separate: unsupported stretching can throw after explicit rectangle mutation.
OwnedSurfaceCopyResult copyOwnedSurfaceWrapper(
    render::SurfaceBackend&, SurfaceCopyWrapper,
    std::optional<render::SurfaceId> source, std::optional<render::SurfaceId> destination,
    SurfaceRectangle32 sourceRect, std::int32_t x, std::int32_t y,
    SurfaceRectangle32& destinationRect, SurfaceWindowContext);
struct OwnedFillRecovery {
    OwnedSurfaceRetryAdapter::Callbacks bindings;
    SurfaceReloadRoute route=SurfaceReloadRoute::None;
    bool keyEnabled=false;
    std::uint16_t key=0;
};
SurfaceRetryResult fillOwnedRectangle(render::SurfaceBackend&, render::SurfaceId,
    std::optional<render::Rect>, std::uint32_t color,
    OwnedFillRecovery recovery={}, unsigned budget=16);
SurfaceRetryResult fillOwnedSurface(render::SurfaceBackend&, render::SurfaceId,
    std::uint32_t color, OwnedFillRecovery recovery={}, unsigned budget=16);

// Immutable, prevalidated sequential-reader BMP input. Callback copies retain
// the bytes independently of the asset object and never retain a file pointer.
class OwnedSurfaceBitmap final {
public:
    explicit OwnedSurfaceBitmap(const std::vector<std::uint8_t>&);
    void reload(render::SurfaceBackend&, render::SurfaceId) const;
    OwnedSurfaceRetryAdapter::Reload callback() const;
private:
    std::shared_ptr<const render::DibInput> dib_;
};
using SurfaceBitmapReader=std::function<std::optional<std::vector<std::uint8_t>>(std::string_view)>;
std::optional<OwnedSurfaceBitmap> loadOwnedCursorBitmap(const SurfaceBitmapReader&);
// Explicit native recovery policy: require an existing shared indexed palette,
// retain its identity and entries, and define only the bitmap's written region.
std::uint32_t restoreOwnedIndexedBitmap(render::SurfaceBackend&, render::SurfaceId,
    render::SurfaceBackend::PaletteId expectedPalette, const OwnedSurfaceBitmap&,
    std::optional<render::SurfaceBackend::Mode> display=std::nullopt);
// Bounded original logical bar pixels in newly owned GPU storage. Caller owns
// the returned reference; allocation/reuse/resize lifetime is a native policy.
render::Image makeVerticalBarImage(unsigned width, unsigned height, unsigned fill, unsigned format);
render::SurfaceId createOwnedVerticalBar(render::SurfaceBackend&,
    unsigned width, unsigned height, unsigned fill, unsigned format);
}
