#include "owned_surface_wrappers.hpp"
#include "surface_bitmap.hpp"
#include "vertical_bar.h"
#include <limits>
#include <utility>

namespace mnm::reconstruction {
namespace {
render::Rect rectangle(SurfaceRectangle32 r) {
    static_assert(std::numeric_limits<int>::digits>=31, "Renderer coordinates require 32-bit int");
    return {r.left,r.top,r.right,r.bottom};
}
void reload(render::SurfaceBackend& backend, render::SurfaceId id, const render::DibInput& dib) {
    std::uint32_t token=0;
    if(backend.acquireDc(id,token)) throw std::runtime_error("Bitmap reload cannot acquire owned DC");
    try {
        backend.reloadDib(id,dib);
    } catch(...) {
        // Retire the lease on a rejected draw as well. Propagate backend errors;
        // an accepted Restore alone never makes the remaining pixels valid.
        backend.releaseDc(id,token);
        throw;
    }
    if(backend.releaseDc(id,token)) throw std::runtime_error("Bitmap reload cannot release owned DC");
}
SurfaceRetryResult fill(render::SurfaceBackend& backend, render::SurfaceId id,
    std::optional<render::Rect> rect, std::uint32_t color, bool full,
    OwnedFillRecovery recovery, unsigned budget) {
    SurfaceRetryInput input;
    input.wrapper=full ? SurfaceWrapper::FullFill : SurfaceWrapper::PartialFill;
    input.color=color;
    input.destinationRoute=recovery.route;
    input.keyEnabled=recovery.keyEnabled;
    input.destinationKey=recovery.key;
    OwnedSurfaceRetryAdapter adapter(backend,std::nullopt,id,{},rect.value_or(render::Rect{}),
                                    full || !rect,std::move(recovery.bindings));
    return runSurfaceRetry(input,adapter,budget);
}
}
OwnedSurfaceCopyResult copyOwnedSurfaceWrapper(
    render::SurfaceBackend& backend, SurfaceCopyWrapper kind,
    std::optional<render::SurfaceId> source, std::optional<render::SurfaceId> destination,
    SurfaceRectangle32 sourceRect, std::int32_t x, std::int32_t y,
    SurfaceRectangle32& destinationRect, SurfaceWindowContext context) {
    const auto plan=planSurfaceWrapperCopy(kind,sourceRect,x,y,destinationRect,context,
                                          bool(source),bool(destination));
    if(!plan.called) return {};
    render::SurfaceCopyRequest request;
    request.api=plan.fast ? render::SurfaceCopyApi::BltFast : render::SurfaceCopyApi::Blt;
    request.source=rectangle(plan.source);
    request.destination=rectangle(plan.destination);
    request.flags=plan.flags;
    return {true,backend.copy(*source,*destination,request)};
}
SurfaceRetryResult fillOwnedRectangle(render::SurfaceBackend& backend, render::SurfaceId id,
    std::optional<render::Rect> rect, std::uint32_t color,
    OwnedFillRecovery recovery, unsigned budget) {
    return fill(backend,id,rect,color,false,std::move(recovery),budget);
}
SurfaceRetryResult fillOwnedSurface(render::SurfaceBackend& backend, render::SurfaceId id,
    std::uint32_t color, OwnedFillRecovery recovery, unsigned budget) {
    return fill(backend,id,std::nullopt,color,true,std::move(recovery),budget);
}
OwnedSurfaceBitmap::OwnedSurfaceBitmap(const std::vector<std::uint8_t>& bytes) {
    auto dib=parseSurfaceBitmap(bytes);
    if(!dib) throw std::runtime_error("Truncated owned surface bitmap");
    (void)render::decodeDibRgb(*dib); // Validate before any surface, palette or lease mutation.
    dib_=std::make_shared<const render::DibInput>(std::move(*dib));
}
void OwnedSurfaceBitmap::reload(render::SurfaceBackend& backend, render::SurfaceId id) const {
    if(!dib_) throw std::logic_error("Bitmap resource was moved from");
    reconstruction::reload(backend,id,*dib_);
}
OwnedSurfaceRetryAdapter::Reload OwnedSurfaceBitmap::callback() const {
    if(!dib_) throw std::logic_error("Bitmap resource was moved from");
    return [dib=dib_](render::SurfaceBackend& backend,render::SurfaceId id) {
        reconstruction::reload(backend,id,*dib);
    };
}
std::optional<OwnedSurfaceBitmap> loadOwnedCursorBitmap(const SurfaceBitmapReader& reader) {
    if(!reader) throw std::invalid_argument("Cursor bitmap reader is not installed");
    const auto bytes=reader("bitmaps\\cursors.bmp");
    if(!bytes) return std::nullopt;
    return OwnedSurfaceBitmap(*bytes);
}
std::uint32_t restoreOwnedIndexedBitmap(render::SurfaceBackend& backend, render::SurfaceId id,
    render::SurfaceBackend::PaletteId expectedPalette, const OwnedSurfaceBitmap& bitmap,
    std::optional<render::SurfaceBackend::Mode> display) {
    if(!expectedPalette || backend.paletteIdentity(id)!=expectedPalette)
        throw std::invalid_argument("Indexed bitmap recovery palette identity differs");
    if(const auto result=backend.restore(id,display)) return result;
    bitmap.reload(backend,id);
    if(backend.paletteIdentity(id)!=expectedPalette)
        throw std::runtime_error("Indexed bitmap recovery replaced palette identity");
    return 0;
}
render::Image makeVerticalBarImage(unsigned width,unsigned height,unsigned fill,unsigned format) {
    if(!mnm_vertical_bar_valid(width,height,fill,format))
        throw std::invalid_argument("Vertical bar dimensions, fill or format outside bounded contract");
    render::Image image{int(width),int(height),std::vector<std::uint32_t>(std::size_t(width)*height)};
    for(unsigned y=0;y<height;++y) for(unsigned x=0;x<width;++x)
        image.pixels[std::size_t(y)*width+x]=mnm_vertical_bar_pixel(width,height,fill,format,x,y);
    return image;
}
render::SurfaceId createOwnedVerticalBar(render::SurfaceBackend& backend,
    unsigned width,unsigned height,unsigned fill,unsigned format) {
    auto image=makeVerticalBarImage(width,height,fill,format);
    const render::PixelFormat pixels{16,format ? std::array<std::uint32_t,3>{0x7c00,0x3e0,31}
                                             : std::array<std::uint32_t,3>{0xf800,0x7e0,31}};
    return backend.create(image,pixels);
}
}
