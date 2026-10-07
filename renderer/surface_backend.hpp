#pragma once
#include "bitmap_dc.hpp"
#include "surface_access.hpp"
#include <unordered_map>

namespace mnm::render {
// GUI-thread owned surfaces. No COM pointer, real HDC or shared-wire ownership.
class SurfaceBackend final {
public:
    using Palette=BitmapDcState::Palette;
    using Descriptor=SurfaceAccessState::Descriptor;
    SurfaceBackend();
    ~SurfaceBackend();
    SurfaceBackend(const SurfaceBackend&)=delete;
    SurfaceBackend& operator=(const SurfaceBackend&)=delete;
    SurfaceId create(const Image&,PixelFormat,unsigned caps=0x840,
                     std::optional<Palette> defaults=std::nullopt);
    void destroy(SurfaceId);
    void update(SurfaceId,int x,int y,const Image&);
    std::uint32_t lock(SurfaceId,Descriptor&);
    const Image& lockedPixels(SurfaceId) const;
    void writeLocked(SurfaceId,int x,int y,const Image&);
    std::uint32_t unlock(SurfaceId);
    std::uint32_t acquireDc(SurfaceId,std::uint32_t& output);
    std::uint32_t releaseDc(SurfaceId,std::uint32_t token);
    const Palette& dcPalette(SurfaceId) const;
    const std::optional<std::vector<Rect>>& dcRegions(SurfaceId) const;
    void selectDcRegions(SurfaceId,std::optional<std::vector<Rect>>);
    void setDcColors(SurfaceId,unsigned first,const std::vector<Rgb>&);
    void bindPalette(SurfaceId,std::optional<Palette>);
    void updatePalette(SurfaceId,unsigned first,const std::vector<Rgb>&);
    void reloadDib(SurfaceId,const DibInput&);
    void setClipper(SurfaceId,const ClipperState&);
    void setSourceKey(SurfaceId,std::optional<std::uint16_t>);
    SurfaceCopyResult copy(SurfaceId source,SurfaceId destination,const SurfaceCopyRequest&);
    // Canonical RGB565 native word; the original wrapper truncates its argument
    // before this API. Supported flags: COLORFILL, optionally WAIT.
    SurfaceCopyResult fill(SurfaceId,std::optional<Rect>,std::uint16_t color,
                           std::uint32_t flags=0x01000400);
    Image read(SurfaceId);
    QImage present(SurfaceId);
    GpuFrame presentGpu(SurfaceId);
    Image readDc(SurfaceId);
    QImage presentDc(SurfaceId);
    void invalidateContents(SurfaceId);
    bool borrowed(SurfaceId) const;
    bool poisoned(SurfaceId) const;
    int mapBalance(SurfaceId) const;
    RenderStats stats() const;
    Driver driver() const;
private:
    struct Entry;
    GlBlitter gl_;
    std::unordered_map<SurfaceId,std::unique_ptr<Entry>> entries_;
    std::uint32_t nextToken_=1;
    Entry& entry(SurfaceId);
    const Entry& entry(SurfaceId) const;
    static void available(const Entry&);
    static void raster(const Entry&);
    void installBinding(SurfaceId,const Entry&);
};
}
